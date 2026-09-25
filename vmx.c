#include "funciones.c"
#include <time.h>  // Para la operacion RND
const char *NOMBRE_REG[32] = {
    "IP","OPC","OP1","OP2","LAR","MAR","MBR","-","-","-",
    "EAX","EBX","ECX","EDX","EEX","EFX","AC","CC","-","-",
    "-","-","-","-","-","-","CS","DS","-","-","-","-"
};

// indice = cod_op (0x00 a 0x0A)
const char *MNEM_UNO[] = {
    "SYS","JMP","JP","JN","JZ","JC","JV","JNP","JNN","JNZ","NOT"
};

// indice = cod_op - 0x10 (0x10 a 0x1F)
const char *MNEM_DOS[] = {
    "MOV","ADD","SUB","MUL","DIV","CMP","AND","OR","XOR","SWAP",
    "SHL","SHR","SAR","LDL","LDH","RND"
};

void formatearOperando(char *buffer, unsigned char tipo, int32_t valor){
    switch (tipo) {
        case 0x00: // ninguno
            buffer[0] = '\0';
            break;
        case 0x01: { // registro
            uint8_t reg = valor & 0x1F;
            sprintf(buffer, "%s", NOMBRE_REG[reg]);
            break;
        }
        case 0x02: { // inmediato
            int16_t inm = (int16_t) valor;
            sprintf(buffer, "%d", inm);
            break;
        }
        case 0x03: { // memoria: [REG+offset]
            uint8_t reg = valor & 0x1F;
            int16_t offset = (int16_t)(valor >> 8);
            if (offset > 0)
                sprintf(buffer, "[%s+%d]", NOMBRE_REG[reg], offset);
            else if (offset < 0)
                sprintf(buffer, "[%s%d]", NOMBRE_REG[reg], offset); // el signo va incluido en %d
            else
                sprintf(buffer, "[%s]", NOMBRE_REG[reg]);
            break;
        }
    }
}

void desensamblar(){
    int32_t ip = 0;

    while (ip < tabla_seg[0].tam){
        int32_t inicio = ip;
        unsigned char primerByte = RAM[ip];
        unsigned char tipo_opa, tipo_opb, cod_op;
        int32_t valor_opa = 0, valor_opb = 0;
        char strOpA[16] = "", strOpB[16] = "";
        char strBytes[32] = "";
        char strOperandos[40] = "";
        const char *mnem;

        if ((primerByte >> 4 & 0x01) == 1){ // dos operandos
            tipo_opb = (primerByte >> 6) & 0x03;
            tipo_opa = (primerByte >> 4) & 0x03;
            cod_op = primerByte & 0x1F;

            valor_opb = p_tipo_op[tipo_opb](&ip); // mismo orden de lectura que dosOperandos
            valor_opa = p_tipo_op[tipo_opa](&ip);

            mnem = (cod_op >= 0x10) ? MNEM_DOS[cod_op - 0x10] : "??";
            formatearOperando(strOpA, tipo_opa, valor_opa);
            formatearOperando(strOpB, tipo_opb, valor_opb);
            snprintf(strOperandos, sizeof(strOperandos), "%s, %s", strOpA, strOpB);

        } else if (primerByte >> 6 != 0){ // un operando
            tipo_opa = (primerByte >> 6) & 0x03;
            cod_op = primerByte & 0x0F;

            valor_opa = p_tipo_op[tipo_opa](&ip);

            mnem = (cod_op <= 0x0A) ? MNEM_UNO[cod_op] : "??"; // si existe le da el valor correspondiente a mnem, si es invalido le da mnem: ?? -> desconocido
            formatearOperando(strOpA, tipo_opa, valor_opa);
            snprintf(strOperandos, sizeof(strOperandos), "%s", strOpA);

        } else { // sin operandos
            cod_op = primerByte & 0x0F;
            mnem = (cod_op == 0x0F) ? "STOP" : "??";
            strOperandos[0] = '\0';
        }

        // arma la columna de bytes en hex (ancho variable según cant. de bytes leídos)
        int pos = 0;
        for (int i = inicio; i <= ip; i++)
            pos += snprintf(strBytes + pos, sizeof(strBytes) - pos, "%02X ", RAM[i]);

        // %-24s = bytes en hex con ancho fijo 24 (ajustá según tu instrucción más larga)
        // %-6s  = mnemónico con ancho fijo 6 (el más largo, ej. "JNP"/"STOP", tiene margen)
        printf("[%04X] %-24s| %-6s %s\n", inicio, strBytes, mnem, strOperandos);

        ip++; // arranca la próxima instrucción
    }
    printf("----------------------------\n\n");
}

int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL)); // Para la operacion RND

    if (argc < 2 || argc > 3) {
        printf("Uso: %s <archivo.vmx> [-d]\n", argv[0]);
        return -1;
    }

    char *nombre_archivo = argv[1];
    int modo_disassembler = 0; // 0 = Ejecución normal, 1 = Desensamblador

    if (argc == 3) {
        if (strcmp(argv[2], "-d") == 0) {
            modo_disassembler = 1; 
        } else {
            printf("Argumento no reconocido: %s\n", argv[2]);
            return -1;
        }
    }

    FILE *archivo = fopen(nombre_archivo, "rb");
    if (archivo == NULL) {
        printf("Error al abrir el archivo .vmx");
        return -1;
    }

    // Leer la cabecera completa de 8 bytes
    cabecera_programa p;
    
    if (fread(&p, sizeof(cabecera_programa), 1, archivo) != 1) { 
        printf("Error al leer la cabecera del archivo\n");
        fclose(archivo);
        return -1;
    }

    p.tam_codigo = (p.tam_codigo >> 8) | (p.tam_codigo << 8); // Nuestro procesador trabaja con little endians y vmx utiliza big endians
    // Validar que sea un archivo VMX 
    if (strncmp(p.identificador, "VMX26", 5) != 0 || p.version != 1) { 
        printf("Formato no válido: no se encontró la firma VMX26 o la version no es la correcta\n");
        fclose(archivo);
        return -1;
    }

    // Cargar el código máquina en la memoria RAM 
    size_t bytes_leidos = fread(RAM, sizeof(unsigned char), p.tam_codigo, archivo);// Lee directamente todo el codigo del archivo binario 

    tabla_seg[0].base = 0;  //Inicializamos tabla de segmentos
    tabla_seg[0].tam = p.tam_codigo;
    tabla_seg[1].base = p.tam_codigo;  //Tener en cuenta que en la segunda parte tendremos que calcularlo y no inicializarlo
    tabla_seg[1].tam = TAM_MEMORIA - p.tam_codigo; // Puede haber un error acá tam_codigo tiene valor erroneo

    // Inicializamos Registros
    registros[CS] = tabla_seg[0].base; // CS
    registros[DS] = tabla_seg[1].base; // DS
    registros[IP] = registros[CS]; // IP
    if(modo_disassembler == 1)
        desensamblar();

    // Ciclo Principal
     while(error_actual == ERR_NINGUNO && registros[IP] < tabla_seg[1].base && registros[IP] != -1){
        buscoOperacion(&registros[IP]); 
        if (registros[IP] != -1 && error_actual == ERR_NINGUNO)
           registros[IP] = registros[IP] + 1;
     }

    if (error_actual != ERR_NINGUNO) {
        printf("\n================================================\n");
        switch (error_actual) {
            case ERR_INSTRUCCION_INVALIDA:
                printf("ERROR: instruccion invalida - ejecucion abortada\n");
                break;
            case ERR_DIVISION_CERO:
                printf("ERROR: division por cero - ejecucion abortada\n");
                break;
            case ERR_FALLO_SEGMENTO:
                printf("ERROR: fallo de segmento - ejecucion abortada\n");
                break;
            default:
                break;
        }
        registros[IP] = -1; // dejamos IP consistente con el comportamiento de STOP
    }

    fclose(archivo);
    return 1;
}