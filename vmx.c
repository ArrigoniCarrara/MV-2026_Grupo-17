#include "desensamblador.c"
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
 
    // sin .vmx no hay proceso nuevo, entonces se ignoran los parámetros -p
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
 
    memcpy(p->identificador, bytes, 5); // memcpy(destino, origen, n): Copia n bytes de un lugar a otro.
    p->version = bytes[5];
 
    if (memcmp(p->identificador, FIRMA_VMX, 5) != 0 || p->version != VERSION_VMX){ // memcmp(a, b, n): Compara n bytes. Devuelve 0 si son iguales, y un número distinto de cero si no.
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
 
    ubicoseg(&seg, tam_param, PS); 
    ubicoseg(&seg, p->tam_const, KS);
    ubicoseg(&seg, p->tam_code, CS);
    ubicoseg(&seg, p->tam_data, DS);
    ubicoseg(&seg, p->tam_extra, ES);
    ubicoseg(&seg, p->tam_stack, SS);
 
    registros[SP] = registros[SS] + p->tam_stack; 
    registros[IP] = registros[CS] + p->entry;
}
 
int cargoImagen(argumentos_vm *args){
    FILE *archivo = fopen(args->archivo_vmi, "rb");
    if (archivo == NULL){
        printf("Error al abrir el archivo .vmi\n");
        return -1;
    }
 
    uint8_t cabecera_vmi[8];  // leo cabecera .vmi
    if (fread(cabecera_vmi, 1, 8, archivo) != 8){
        printf("Error al leer la cabecera del archivo .vmi\n");
        fclose(archivo);
        return -1;
    }
 
    if (memcmp(cabecera_vmi, "VMI26", 5) != 0 || cabecera_vmi[5] != 1){
        printf("Formato no válido: no se encontró la firma VMI26 o la version no es la correcta\n");
        fclose(archivo);
        return -1;
    }
 
    uint32_t memoria_kib = (cabecera_vmi[6] << 8) | cabecera_vmi[7];
    if (memoria_kib == 0 || memoria_kib * 1024 > sizeof(RAM)){
        printf("Tamaño de memoria invalido en el archivo .vmi: %u KiB\n", memoria_kib);
        fclose(archivo);
        return -1;
    }
 
    for (int i = 0; i < 32; i++){       // Leo los registros uno por uno
        registros[i] = 0;
        for (int b = 0; b < 4; b++)
            registros[i] = (registros[i] << 8) | fgetc(archivo);
    }
 
    for (int i = 0; i < 8; i++){        // leo la tabla de segmentos
        tabla_seg[i].base = fgetc(archivo) << 8;
        tabla_seg[i].base |= fgetc(archivo);
        tabla_seg[i].tam = fgetc(archivo) << 8;
        tabla_seg[i].tam |= fgetc(archivo);
    }
 
    // Memoria principal completa. En el archivo vmi es la totalidad de la RAM aunque no este ocupada en su totalidad
    if (fread(RAM, 1, memoria_kib * 1024, archivo) != memoria_kib * 1024){
        printf("Error: el archivo .vmi esta incompleto\n");
        fclose(archivo);
        return -1;
    }
    fclose(archivo);
 
    args->memoria_kib = memoria_kib;    // M sale de la imagen (se ignora m=)
    return 0;
}
 
int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL)); // Para la operacion RND
    
    argumentos_vm args;
    if (parsearArgumentos(argc, argv, &args) != 0)
        return -1;
 
    if (args.archivo_vmx != NULL){
 
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
 
        // Cargo el Code Segment y el Const Segment 
        uint32_t base_code = tabla_seg[registros[CS] >> 16].base;   // dirección física donde empieza el Code Segment
        if (fread(&RAM[base_code], 1, p.tam_code, archivo) != p.tam_code){
            printf("Error: el archivo .vmx esta incompleto\n");
            fclose(archivo);
            return -1;
        }
        
        if (p.tam_const != 0){ // REVISAR TEMAS CONSTANTES
            uint32_t base_const = tabla_seg[registros[KS] >> 16].base;
            if (fread(&RAM[base_const], 1, p.tam_const, archivo) != p.tam_const){
                printf("Error: el archivo .vmx esta incompleto\n");
                fclose(archivo);
                return -1;
            }
        }
        
        fclose(archivo);
 
        if(tam_param != 0){ // Escribo en memoria los parametros (Param Segment)
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

            uint32_t dir_array = registros[PS] | pos_memoria; // direccion logica del arreglo de punteros (argv)

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
            PUSH(2, 0, dir_array, 0);
            PUSH(2, 0, args.cant_params, 0);
            PUSH(2, 0, -1, 0);
        }else{
            
            PUSH(2, 0, -1, 0);
            PUSH(2, 0, 0, 0);
            PUSH(2, 0, -1, 0);
        }
        
        if (args.archivo_vmi != NULL){
            // .vmx + .vmi: en cada breakpoint (SYS 0xF) se genera la imagen en el .vmi
            hay_vmi = 1;
            nombre_vmi = args.archivo_vmi;
        }
        else{
            // Solo .vmx: los breakpoints (SYS 0xF) se ignoran porque no hay donde guardar la imagen
            hay_vmi = 0;
        }
    }
    else{
        //Solo .vmi
        if (cargoImagen(&args) != 0)
            return -1;
        hay_vmi = 1;
        nombre_vmi = args.archivo_vmi;
    }
 
    memoria_vm_kib = args.memoria_kib; // SYS breakpoint necesita el tamaño de la memoria
 
    if(args.modo_disassembler){
        desensamblar();
    }

    while(error_actual == ERR_NINGUNO && registros[IP] != -1 && IPdirfisica() < tabla_seg[registros[IP] >> 16].base + tabla_seg[registros[IP] >> 16].tam){
        buscoOperacion();
        if (breakpoint_enter && error_actual == ERR_NINGUNO && registros[IP] != -1)
            Breakpoint();
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
            case ERR_OVERFLOW:
                printf("ERROR: STACK overflow - ejecucion abortada\n");
                break;
            case ERR_UNDERFLOW:
                printf("ERROR: STACK underflow - ejecucion abortada\n");
                break;
            default:
                break;
        }
        registros[IP] = -1; // dejamos IP consistente con el comportamiento de STOP
    }
 
    return 0;
}