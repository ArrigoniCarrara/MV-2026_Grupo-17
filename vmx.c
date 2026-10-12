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
    uint32_t ipGuardado = registros[IP];   // el desensamblado usa el IP como cursor: lo restauro al final
    registros[IP] = registros[CS];         // arranca en el primer byte del segmento de código

    while ((registros[IP] & 0xFFFF) < tabla_seg[registros[IP] >> 16].tam){
        int32_t inicio = IPdirfisica();    // dirección física de la instrucción
        unsigned char primerByte = RAM[inicio];
        unsigned char tipo_opa, tipo_opb, cod_op;
        int32_t valor_opa = 0, valor_opb = 0;
        char strOpA[16] = "", strOpB[16] = "";
        char strBytes[32] = "";
        char strOperandos[40] = "";
        const char *mnem;

        // Las funciones de operando (registro, inmediato, memoria) avanzan registros[IP]
        // hasta el ÚLTIMO byte de la instrucción.
        if ((primerByte >> 4 & 0x01) == 1){ // dos operandos
            tipo_opb = (primerByte >> 6) & 0x03;
            tipo_opa = (primerByte >> 4) & 0x03;
            cod_op = primerByte & 0x1F;

            valor_opb = p_tipo_op[tipo_opb]();
            valor_opa = p_tipo_op[tipo_opa]();

            mnem = (cod_op >= 0x10) ? MNEM_DOS[cod_op - 0x10] : "??";
            formatearOperando(strOpA, tipo_opa, valor_opa);
            formatearOperando(strOpB, tipo_opb, valor_opb);
            snprintf(strOperandos, sizeof(strOperandos), "%s, %s", strOpA, strOpB);

        } else if (primerByte >> 6 != 0){ // un operando
            tipo_opa = (primerByte >> 6) & 0x03;
            cod_op = primerByte & 0x0F;

            valor_opa = p_tipo_op[tipo_opa]();

            mnem = (cod_op <= 0x0A) ? MNEM_UNO[cod_op] : "??";
            formatearOperando(strOpA, tipo_opa, valor_opa);
            snprintf(strOperandos, sizeof(strOperandos), "%s", strOpA);

        } else { // sin operandos
            cod_op = primerByte & 0x0F;
            mnem = (cod_op == 0x0F) ? "STOP" : "??";
            strOperandos[0] = '\0';
        }

        // bytes de la instrucción: desde 'inicio' hasta el último byte leído (IP actual)
        int pos = 0;
        for (int i = inicio; i <= IPdirfisica(); i++)
            pos += snprintf(strBytes + pos, sizeof(strBytes) - pos, "%02X ", RAM[i]);

        printf("[%04X] %-24s| %-6s %s\n", inicio, strBytes, mnem, strOperandos);

        registros[IP]++; // próxima instrucción
    }
    printf("----------------------------\n\n");

    registros[IP] = ipGuardado;
}

static int terminaEn(const char *s, const char *ext){
    size_t ls = strlen(s), le = strlen(ext);
    return ls >= le && strcmp(s + ls - le, ext) == 0;
}

int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL)); // Para la operacion RND

    char    *archivo_vmx = NULL;
    char    *archivo_vmi = NULL;      // por ahora se acepta pero se ignora
    uint32_t M = 16384;                  // tamaño de la memoria en KiB (16 por defecto)
    int      modo_disassembler = 0;   // 0 = ejecucion normal, 1 = -d
    int      cant_params = 0;
    char   **params = NULL;

    int i = 1;
    if (i < argc && terminaEn(argv[i], ".vmx")) 
        archivo_vmx = argv[i++];

    if (i < argc && terminaEn(argv[i], ".vmi")) 
        archivo_vmi = argv[i++];

    if (i < argc && strncmp(argv[i], "m=", 2) == 0){
        char *fin;
        unsigned long m = strtoul(argv[i] + 2, &fin, 10);
        if (argv[i][2] == '\0' || *fin != '\0' || m == 0 || m > 64){   // base/tam de tipo_seg son de 16 bits
            printf("Tamaño de memoria invalido: %s (valor entre 1 y 64 KiB)\n", argv[i]);
            return -1;
        }
        M = (uint32_t) m;
        i++;
    }

    if (i < argc && strcmp(argv[i], "-d") == 0){ 
        modo_disassembler = 1; i++; 
    }

    if (i < argc && strcmp(argv[i], "-p") == 0){
        params = &argv[i + 1];
        cant_params = argc - i - 1;
        i = argc;                      
    }

    if (i < argc || (!archivo_vmx && !archivo_vmi)){
        if (i < argc) printf("Argumento no reconocido o fuera de orden: %s\n", argv[i]);
        printf("Uso: %s [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN]\n"
               "Es obligatorio indicar al menos un archivo .vmx o .vmi\n", argv[0]);
        return -1;
    }

    if (!archivo_vmx){
        printf("La ejecucion desde una imagen .vmi todavia no esta implementada\n");
        return -1;
    }

    (void) archivo_vmi;

    
    uint32_t tam_memoria = M * 1024; // CUIDADO CON ESTO

    FILE *archivo = fopen(archivo_vmx, "rb");
    if (archivo == NULL) {
        printf("Error al abrir el archivo .vmx");
        return -1;
    }

    // Leer la cabecera completa 
    
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

    for (int i = 2; i < TAM_SEG; i++){ // entradas sin usar: -1 (0xFFFFFFFF)
        tabla_seg[i].base = 0xFFFF;
        tabla_seg[i].tam = 0xFFFF;
    }

    // Inicializamos Registros
    registros[CS] = tabla_seg[0].base; // CS
    registros[DS] = 0x00010000; // DS
    registros[IP] = registros[CS]; // IP 

    if(modo_disassembler == 1)
        desensamblar();

    // Ciclo Principal
     while(error_actual == ERR_NINGUNO && registros[IP] != -1 && IPdirfisica() < tabla_seg[registros[IP] >> 16].base + tabla_seg[registros[IP] >> 16].tam){
        buscoOperacion(); 
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