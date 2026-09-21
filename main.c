
#include "funciones.c"

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

            printf("[%04X] ", inicio);
            for (int i = inicio; i <= ip; i++) printf("%02X ", RAM[i]);
            printf("| %s %s, %s\n", mnem, strOpA, strOpB);

        } else if (primerByte >> 6 != 0){ // un operando
            tipo_opa = (primerByte >> 6) & 0x03;
            cod_op = primerByte & 0x0F;

            valor_opa = p_tipo_op[tipo_opa](&ip);

            mnem = (cod_op <= 0x0A) ? MNEM_UNO[cod_op] : "??";
            formatearOperando(strOpA, tipo_opa, valor_opa);

            printf("[%04X] ", inicio);
            for (int i = inicio; i <= ip; i++) printf("%02X ", RAM[i]);
            printf("| %s %s\n", mnem, strOpA);

        } else { // sin operandos
            cod_op = primerByte & 0x0F;
            mnem = (cod_op == 0x0F) ? "STOP" : "??";
            printf("[%04X] %02X | %s\n", inicio, primerByte, mnem);
        }

        ip++; // arranca la proxima instruccion
    }
    printf("----------------------------\n\n");
}

void imprimir_binario(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

int main(int argc, char *argv[]) {
    
    FILE *archivo = fopen("prueba.vmx", "rb");
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

    printf("--- Cabecera VMX leída con éxito ---\n");
    printf("Identificador: %.5s\n", p.identificador);
    printf("Versión: %u\n", p.version);
    printf("Tamaño de código: %u bytes\n\n", p.tam_codigo);

    // Cargar el código máquina en la memoria RAM 
    size_t bytes_leidos = fread(RAM, sizeof(unsigned char), p.tam_codigo, archivo);// Lee directamente todo el codigo del archivo binario 
    printf("Bytes de código máquina cargados en RAM: %zu\n\n", bytes_leidos);         // en el vector RAM


    printf("--- Memoria Principal ---\n");// Muestra el codigo en Hexadecimal y binario dentro de la memoria principal
    printf("Pos  | Binario  | Hex \n");
    printf("------------------------\n");
    for (size_t i = 0; i < bytes_leidos; i++) {
        printf("[%02zu] | ", i);           
        imprimir_binario(RAM[i]);          
        printf(" | %02X\n", RAM[i]);       
    }
    printf("\n");

    tabla_seg[0].base = 0;  //Inicializamos tabla de segmentos
    tabla_seg[0].tam = p.tam_codigo;
    tabla_seg[1].base = p.tam_codigo;  //Tener en cuenta que en la segunda parte tendremos que calcularlo y no inicializarlo
    tabla_seg[1].tam = TAM_MEMORIA - p.tam_codigo; // Puede haber un error acá tam_codigo tiene valor erroneo

    // Inicializamos Registros
    registros[CS] = tabla_seg[0].base; // CS
    registros[DS] = tabla_seg[1].base; // DS
    registros[IP] = registros[CS]; // IP

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

   if (error_actual == ERR_NINGUNO){

        printf("--- Memoria Luego del Programa ---\n");// Muestra el codigo en Hexadecimal y binario dentro de la memoria principal
        printf("Pos  | Binario  | Hex \n");
        printf("------------------------\n");
        for (size_t i = 0; i < 100; i++) {
            printf("[%02zu] | ", i);           
            imprimir_binario(RAM[i]);          
            printf(" | %02X\n", RAM[i]);
        }
     printf("\n================================================\n");
     printf("--- ESTADO FINAL DE LOS REGISTROS ---\n");
    
     // Registros de control e internos
     printf("IP  : %08X | OPC : %08X\n", registros[IP], registros[OPC]);
     printf("OP1 : %08X | OP2 : %08X\n", registros[OP1], registros[OP2]);
     printf("LAR : %08X | MAR : %08X | MBR : %08X\n", registros[LAR], registros[MAR], registros[MBR]);
    
     printf("------------------------------------------------\n");
    
     // Registros de propósito general
     printf("EAX : %08X | EBX : %08X\n", registros[EAX], registros[EBX]);
     printf("ECX : %08X | EDX : %08X\n", registros[ECX], registros[EDX]);
     printf("EEX : %08X | EFX : %08X\n", registros[EEX], registros[EFX]);
    
     printf("------------------------------------------------\n");
    
     // Registros de estado y segmentos
     printf("AC  : %08X | CC  : %08X\n", registros[AC], registros[CC]);
     printf("CS  : %08X | DS  : %08X\n", registros[CS], registros[DS]);
     printf("================================================\n");


     desensamblar();
   }
    fclose(archivo);
    return 1;
}