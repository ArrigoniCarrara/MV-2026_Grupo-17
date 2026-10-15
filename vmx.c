#include "funciones.c"
#include <time.h>  // Para la operacion RND

#define MEMORIA_DEFAULT_KIB 16
#define TAM_CABECERA   18
#define FIRMA_VMX      "VMX26"
#define VERSION_VMX    2

typedef struct {
    char    *archivo_vmx;        // NULL si no se indicó
    char    *archivo_vmi;        // NULL si no se indicó
    uint32_t memoria_kib;        // valor de m=M (16 por defecto)
    int      modo_disassembler;  // 1 si se indicó -d
    int      cant_params;        // cantidad de parámetros después de -p
    char   **params;             // apunta dentro de argv (no se copia)
} argumentos_vm;

char *NOMBRE_REG[32] = {
    "IP","OPC","OP1","OP2","LAR","MAR","MBR","-","-","-",
    "EAX","EBX","ECX","EDX","EEX","EFX","AC","CC","-","-",
    "-","-","-","-","-","-","CS","DS","-","-","-","-"
};
 
// indice = cod_op (0x00 a 0x0A)
char *MNEM_UNO[] = {
    "SYS","JMP","JP","JN","JZ","JC","JV","JNP","JNN","JNZ","NOT"
};
 
// indice = cod_op - 0x10 (0x10 a 0x1F)
char *MNEM_DOS[] = {
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
        char *mnem;
 
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
 
int terminaEn(char *s, char *ext){
    size_t ls = strlen(s), le = strlen(ext);
    return ls >= le && strcmp(s + ls - le, ext) == 0;
}
 
// Convierte el texto de "m=M" a KiB. Devuelve 1 si es válido, 0 si no.
int parsearMemoria(char *texto, uint32_t *kib){
    char *fin;
    unsigned long m = strtoul(texto, &fin, 10);
 
    if (*texto == '\0' || *fin != '\0' || m == 0 || m > MAX_MEMORIA)
        return 0;
 
    *kib = (uint32_t) m;
    return 1;
}
 
// Devuelve 0 si los argumentos son correctos, -1 si hay un error.
int parsearArgumentos(int argc, char *argv[], argumentos_vm *args){
    int i = 1;
 
    args->archivo_vmx       = NULL;
    args->archivo_vmi       = NULL;
    args->memoria_kib       = MEMORIA_DEFAULT_KIB;
    args->modo_disassembler = 0;
    args->cant_params       = 0;
    args->params            = NULL;
 
    if (i < argc && terminaEn(argv[i], ".vmx"))
        args->archivo_vmx = argv[i++];
 
    if (i < argc && terminaEn(argv[i], ".vmi"))
        args->archivo_vmi = argv[i++];
 
    if (i < argc && strncmp(argv[i], "m=", 2) == 0){
        if (!parsearMemoria(argv[i] + 2, &args->memoria_kib)){
            printf("Tamaño de memoria invalido: %s (valor entre 1 y %d KiB)\n", argv[i], MAX_MEMORIA);
            return -1;
        }
        i++;
    }
 
    if (i < argc && strcmp(argv[i], "-d") == 0){
        args->modo_disassembler = 1;
        i++;
    }
 
    if (i < argc && strcmp(argv[i], "-p") == 0){   
        args->params      = &argv[i + 1];
        args->cant_params = argc - i - 1;
        i = argc;
    }
 
    if (i < argc || (!args->archivo_vmx && !args->archivo_vmi)){
        if (i < argc)
            printf("Argumento no reconocido o fuera de orden: %s\n", argv[i]);
        printf("Uso: %s [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 ... paramN]\n"
               "Es obligatorio indicar al menos un archivo .vmx o .vmi\n", argv[0]);
        return -1;
    }
 
    // Sin .vmx no hay proceso nuevo, entonces se ignoran los parámetros -p
    if (!args->archivo_vmx){
        args->params      = NULL;
        args->cant_params = 0;
    }
 
    return 0;
}
 
// Tamaño del Param Segment (0 si no se usó -p y por lo tanto no existe).
// Contenido: nombre del .vmx + strings de los parámetros (cada uno con su '\0')
// y al final un puntero de 4 bytes por cada string (el nombre del .vmx incluido).
uint32_t tamParamSegment(argumentos_vm *args){
    if (args->cant_params == 0)
        return 0;
 
    uint32_t tam_strings = strlen(args->archivo_vmx) + 1;
    for (int i = 0; i < args->cant_params; i++)
        tam_strings += strlen(args->params[i]) + 1;
 
    uint32_t cant_punteros = args->cant_params + 1;
    return tam_strings + 4 * cant_punteros;
}
 
// Arma un valor de 16 bits a partir de 2 bytes del archivo (byte alto primero).
uint16_t leerCampo16(uint8_t *bytes){
    return (uint16_t)((bytes[0] << 8) | bytes[1]);
}
 
// Lee la cabecera desde el comienzo del archivo y deja el cursor justo donde
// empieza el código máquina. Devuelve 0 si salió bien, -1 si hubo un error
int leerCabecera(FILE *archivo, cabecera_programa *p){
    uint8_t bytes[TAM_CABECERA];
 
    if (fread(bytes, 1, TAM_CABECERA, archivo) != TAM_CABECERA){
        printf("Error al leer la cabecera del archivo\n");
        return -1;
    }
 
    memcpy(p->identificador, bytes, 5);
    p->version = bytes[5];
 
    if (memcmp(p->identificador, FIRMA_VMX, 5) != 0 || p->version != VERSION_VMX){
        printf("Formato no válido: no se encontró la firma VMX26 o la version no es la correcta\n");
        return -1;
    }
 
    p->tam_code  = leerCampo16(bytes + 6);
    p->tam_data  = leerCampo16(bytes + 8);
    p->tam_extra = leerCampo16(bytes + 10);
    p->tam_stack = leerCampo16(bytes + 12);
    p->tam_const = leerCampo16(bytes + 14);
    p->entry     = leerCampo16(bytes + 16);
 
    return 0;
}
 
// Devuelve 1 si todos los segmentos del proceso (incluido el Param Segment)
// entran en la memoria principal, 0 si no (error "memoria insuficiente").
int cabeceraEntraEnMemoria( cabecera_programa *p, uint32_t tam_param, uint32_t tam_memoria){
    uint32_t total = tam_param + p->tam_const + p->tam_code + p->tam_data + p->tam_extra + p->tam_stack;
    return total <= tam_memoria;
}

void ubicoseg(int *seg, uint32_t tam_seg, int reg){
    if(tam_seg != 0){
        tabla_seg[*seg].tam = tam_seg;
        registros[reg] = 0;
        registros[reg] = *seg;
        registros[reg] = registros[reg] << 16;
        *seg = *seg + 1;
        tabla_seg[*seg].base = tabla_seg[*seg - 1].base + tabla_seg[*seg - 1].tam; 
    }
    else{
        registros[reg] = -1;
    }
}

void inicializo_tabla_segmentos(cabecera_programa *p, uint32_t tam_param){
    tabla_seg[0].base = 0;
    int seg = 0;

    ubicoseg(&seg, tam_param, PS); // Me falta agregar los parametros al param segment IMPORTANTE
    ubicoseg(&seg, p->tam_const, KS);
    ubicoseg(&seg, p->tam_code, CS);
    ubicoseg(&seg, p->tam_data, DS);
    ubicoseg(&seg, p->tam_extra, ES);
    ubicoseg(&seg, p->tam_stack, SS);

    registros[SP] = registros[SS] + p->tam_stack; 
    registros[IP] = registros[CS] + p->entry;
}
 
int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL)); // Para la operacion RND
 
    argumentos_vm args;
    if (parsearArgumentos(argc, argv, &args) != 0)
        return -1;
 
    if (!args.archivo_vmx){
        printf(".vmi todavia no esta implementado\n");
        return -1;
    }
 
    uint32_t tam_memoria = args.memoria_kib * 1024;
 
    FILE *archivo = fopen(args.archivo_vmx, "rb");
    if (archivo == NULL) {
        printf("Error al abrir el archivo .vmx\n");
        return -1;
    }
 
    cabecera_programa p;
    if (leerCabecera(archivo, &p) != 0){
        fclose(archivo);
        return -1;
    }
 
    uint32_t tam_param = tamParamSegment(&args);
    if (!cabeceraEntraEnMemoria(&p, tam_param, tam_memoria)){
        printf("ERROR: memoria insuficiente para alojar el proceso\n");
        fclose(archivo);
        return -1;
    }
 
    inicializo_tabla_segmentos(&p, tam_param);

    if(tam_param != 0){ // Escribo en memoria los parametros
        int pos_memoria = 0; 
        uint32_t direcciones[args.cant_params + 1];
        int indice_dir = 0;

        direcciones[indice_dir] = pos_memoria; 
        indice_dir++;

        int len_vmx = strlen(args.archivo_vmx); //Primero nombre de archivo
        for (int car = 0; car < len_vmx; car++) {
            RAM[pos_memoria] = args.archivo_vmx[car];
            pos_memoria++;
        }
        RAM[pos_memoria] = 0; 
        pos_memoria++;

        for (int j = 0; j < args.cant_params; j++) { //Resto de parametros
            direcciones[indice_dir] = pos_memoria;
            indice_dir++;

            int len_param = strlen(args.params[j]);
            for (int car = 0; car < len_param; car++) {
                RAM[pos_memoria] = args.params[j][car];
                pos_memoria++;
            }
            RAM[pos_memoria] = 0; 
            pos_memoria++;
        }
        
        for (int pos = 0; pos < indice_dir; pos++) { // Argv de la maquina
            uint32_t dir_actual = direcciones[pos];

            RAM[pos_memoria] = (dir_actual >> 24) & 0xFF;         
            pos_memoria++;

            RAM[pos_memoria] = (dir_actual >> 16) & 0xFF;         
            pos_memoria++;

            RAM[pos_memoria] = (dir_actual >> 8) & 0xFF;          
            pos_memoria++;
        
            RAM[pos_memoria] = dir_actual & 0xFF;                 
            pos_memoria++;
        }
        // Pongo los parametros argv, argc y ret = -1 en la pila (A implementar)
    }
    else{
            //Pongo en la pila ret = 0, argv = -1, argc = -1 (o algo asi, fijarse en la especificacion) 
    }

    uint32_t base_code = tabla_seg[registros[CS] >> 16].base;   // dirección física donde empieza el Code Segment
    if (fread(&RAM[base_code], 1, p.tam_code, archivo) != p.tam_code){
        printf("Error: el archivo .vmx esta incompleto\n");
        fclose(archivo);
        return -1;
    }

    if (p.tam_const != 0){
        uint32_t base_const = tabla_seg[registros[KS] >> 16].base;
        if (fread(&RAM[base_const], 1, p.tam_const, archivo) != p.tam_const){
            printf("Error: el archivo .vmx esta incompleto\n");
            fclose(archivo);
            return -1;
        }
    }

    if(args.modo_disassembler)
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