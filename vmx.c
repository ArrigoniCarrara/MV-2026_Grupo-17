#include "desensamblador.c"
#include <time.h>  // Para la operacion RND
//  COSAS FLATANTES!!!!!
//  DISSASEMBLER CONSTANTES(strings)->listo
//  DISTINTAS CABECERAS -> RESPONDIDO, se refiere al distinto uso de la memoria RAM, y la tabla de segmento  
//  argc cantidad de argumentos -> RESPONDIDO, se tiene en cuenta el nombre
//  tabla de segmentos inicializarla en 0 -> RESPONDIDO, si inicializarla en 0
//  INSTRUCCION INVALIDA -> no deberia haber una instruccion invalida, pero lo debemos implementar
//  IMPLEMENTAR REGISTROS EN RDN 
//  agregar a jp y jn el tema de los registros compactos, no soporta por ejemplo AX o AL o FH
//  revisar SYS 1 y 2
//  PONER EN LA SYS SI NOS PASAMOS DE SEGMENTO ERROR FALLA DE SEGMENTO 
#define MEMORIA_DEFAULT_KIB 16
#define TAM_CABECERA_V1  8
#define TAM_CABECERA_V2  18
#define FIRMA_VMX      "VMX26"
#define MEM_V1_BYTES   16384

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

// Lee la cabecera (version 1 o 2) y deja el cursor donde empieza el codigo.
// Devuelve 0 si salio bien, -1 si hubo un error.
int leerCabecera(FILE *archivo, cabecera_programa *p){
    uint8_t b[12];

    if (fread(b, 1, 6, archivo) != 6){
        printf("Error al leer la cabecera del archivo\n");
        return -1;
    }

    memcpy(p->identificador, b, 5);
    if (memcmp(p->identificador, FIRMA_VMX, 5) != 0){
        printf("Formato no valido: no se encontro la firma VMX26\n");
        return -1;
    }
    p->version = b[5];

    if (p->version == 1){
        // Parte 1: solo el tamaño del codigo. El Data Segment ocupa el resto de los 16 KiB.
        if (fread(b, 1, TAM_CABECERA_V1 - 6, archivo) != TAM_CABECERA_V1 - 6){
            printf("Error al leer la cabecera del archivo\n");
            return -1;
        }
        p->tam_code = leerCampo16(b);
        if (p->tam_code > MEM_V1_BYTES){
            printf("Cabecera invalida: el codigo supera la memoria de 16 KiB\n");
            return -1;
        }
        p->tam_data  = MEM_V1_BYTES - p->tam_code;
        p->tam_extra = 0;
        p->tam_stack = 0;
        p->tam_const = 0;
        p->entry     = 0;
    }
    else if (p->version == 2){
        if (fread(b, 1, TAM_CABECERA_V2 - 6, archivo) != TAM_CABECERA_V2 - 6){
            printf("Error al leer la cabecera del archivo\n");
            return -1;
        }
        p->tam_code  = leerCampo16(b);
        p->tam_data  = leerCampo16(b + 2);
        p->tam_extra = leerCampo16(b + 4);
        p->tam_stack = leerCampo16(b + 6);
        p->tam_const = leerCampo16(b + 8);
        p->entry     = leerCampo16(b + 10);
    }
    else{
        printf("Version de archivo no soportada: %d\n", p->version);
        return -1;
    }

    return 0;
}

// Devuelve 1 si todos los segmentos del proceso (incluido el Param Segment)
// entran en la memoria principal, 0 si no (error "memoria insuficiente").
int cabeceraEntraEnMemoria( cabecera_programa *p, uint32_t tam_param, uint32_t tam_memoria){
    uint32_t total = tam_param + p->tam_const + p->tam_code + p->tam_data + p->tam_extra + p->tam_stack;
    return total <= tam_memoria;
}

// todas las entradas de la tabla arrancan invalidas (0xFFFFFFFF)
void limpiarTablaSegmentos(){
    for (int i = 0; i < TAM_SEG; i++){
        tabla_seg[i].base = 0xFFFF;
        tabla_seg[i].tam  = 0xFFFF;
    }
}

// crea la entrada del segmento (si tiene tamaño > 0) y deja cargado su registro
void ubicoseg(int *seg, uint32_t *base, uint32_t tam_seg, int reg){
    if (tam_seg != 0){
        tabla_seg[*seg].base = *base;
        tabla_seg[*seg].tam  = tam_seg;
        registros[reg] = ((uint32_t)*seg) << 16;
        *base += tam_seg;
        *seg  += 1;
    }
    else
        registros[reg] = 0xFFFFFFFF;
}

void inicializo_tabla_segmentos(cabecera_programa *p, uint32_t tam_param){
    int seg = 0;
    uint32_t base = 0;
    
    limpiarTablaSegmentos();

    ubicoseg(&seg, &base, tam_param,    PS);
    ubicoseg(&seg, &base, p->tam_const, KS);
    ubicoseg(&seg, &base, p->tam_code,  CS);
    ubicoseg(&seg, &base, p->tam_data,  DS);
    ubicoseg(&seg, &base, p->tam_extra, ES);
    ubicoseg(&seg, &base, p->tam_stack, SS);

    if (p->tam_stack != 0)
        registros[SP] = registros[SS] + p->tam_stack;   // pila vacia: apunta justo despues del segmento
    if (p->tam_code != 0)
        registros[IP] = registros[CS] + p->entry;
    else
        registros[IP] = 0xFFFFFFFF;
}

int leerRegistroImagen(FILE *f, uint32_t *valor, int n){
    *valor = 0;
    for (int b = 0; b < n; b++){
        int c = fgetc(f);
        if (c == EOF)
            return -1;
        *valor = (*valor << 8) | (uint32_t)c;
    }
    return 0;
}

int cargoImagen(argumentos_vm *args){
    FILE *archivo = fopen(args->archivo_vmi, "rb");
    if (archivo == NULL){
        printf("Error al abrir el archivo .vmi\n");
        return -1;
    }

    uint8_t cabecera_vmi[8];
    if (fread(cabecera_vmi, 1, 8, archivo) != 8){
        printf("Error al leer la cabecera del archivo .vmi\n");
        fclose(archivo);
        return -1;
    }

    if (memcmp(cabecera_vmi, "VMI26", 5) != 0 || cabecera_vmi[5] != 1){ 
        printf("Formato no valido: no se encontro la firma VMI26 o la version no es la correcta\n");
        fclose(archivo);
        return -1;
    }

    uint32_t memoria_kib = (cabecera_vmi[6] << 8) | cabecera_vmi[7];
    if (memoria_kib == 0 || memoria_kib * 1024 > sizeof(RAM)){
        printf("Tamaño de memoria invalido en el archivo .vmi: %u KiB\n", memoria_kib);
        fclose(archivo);
        return -1;
    }

    uint32_t v;
    for (int i = 0; i < CANT_REG; i++){
        if (leerRegistroImagen(archivo, &v, 4) != 0){
            printf("Error: el archivo .vmi esta incompleto (registros)\n");
            fclose(archivo);
            return -1;
        }
        registros[i] = v;
    }

    for (int i = 0; i < TAM_SEG; i++){
        uint32_t base, tam;
        if (leerRegistroImagen(archivo, &base, 2) != 0 || leerRegistroImagen(archivo, &tam, 2) != 0){
            printf("Error: el archivo .vmi esta incompleto (tabla de segmentos)\n");
            fclose(archivo);
            return -1;
        }
        tabla_seg[i].base = (uint16_t)base;
        tabla_seg[i].tam  = (uint16_t)tam;
    }

    if (fread(RAM, 1, memoria_kib * 1024, archivo) != memoria_kib * 1024){
        printf("Error: el archivo .vmi esta incompleto\n");
        fclose(archivo);
        return -1;
    }
    fclose(archivo);

    // El IP debe ser -1 o apuntar a un segmento existente
    if (registros[IP] != 0xFFFFFFFF){
        uint32_t seg = registros[IP] >> 16;
        if (seg >= TAM_SEG || (tabla_seg[seg].base == 0xFFFF && tabla_seg[seg].tam == 0xFFFF)){
            printf("Imagen invalida: el IP apunta a un segmento inexistente\n");
            return -1;
        }
    }

    args->memoria_kib = memoria_kib;    // M sale de la imagen (se ignora m=)
    return 0;
}

// Escribe el Param Segment (a partir de la direccion fisica 0) y apila argc / argv
void cargoParametros(argumentos_vm *args){
    int pos_memoria = 0;
    uint32_t direcciones[args->cant_params + 1];
    int indice_dir = 0;

    direcciones[indice_dir++] = pos_memoria;

    int len_vmx = strlen(args->archivo_vmx);
    for (int car = 0; car < len_vmx; car++)
        RAM[pos_memoria++] = args->archivo_vmx[car];
    RAM[pos_memoria++] = 0;

    for (int j = 0; j < args->cant_params; j++){
        direcciones[indice_dir++] = pos_memoria;
        int len_param = strlen(args->params[j]);
        for (int car = 0; car < len_param; car++)
            RAM[pos_memoria++] = args->params[j][car];
        RAM[pos_memoria++] = 0;
    }

    uint32_t dir_array = registros[PS] | pos_memoria;   // direccion logica del arreglo argv

    for (int pos = 0; pos < indice_dir; pos++){
        uint32_t dir_actual = direcciones[pos];
        RAM[pos_memoria++] = (dir_actual >> 24) & 0xFF;
        RAM[pos_memoria++] = (dir_actual >> 16) & 0xFF;
        RAM[pos_memoria++] = (dir_actual >> 8) & 0xFF;
        RAM[pos_memoria++] = dir_actual & 0xFF;
    }

    PUSH(2, 0, dir_array, 0);
    PUSH(2, 0, args->cant_params + 1, 0);
    PUSH(2, 0, -1, 0);
}

int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL)); // Para la operacion RND

    argumentos_vm args;
    if (parsearArgumentos(argc, argv, &args) != 0)
        return 1;

    limpiarTablaSegmentos();

    if (args.archivo_vmx != NULL){

        FILE *archivo = fopen(args.archivo_vmx, "rb");
        if (archivo == NULL) {
            printf("Error al abrir el archivo .vmx\n");
            return 1;
        }

        cabecera_programa p;
        if (leerCabecera(archivo, &p) != 0){
            fclose(archivo);
            return 1;
        }

        uint32_t tam_param = 0;
        if (p.version == 1){
            // Parte 1: memoria fija de 16 KiB, sin Param ni pila (se ignoran m= y -p)
            args.memoria_kib = 16;
            args.cant_params = 0;
            args.params      = NULL;
            p.tam_extra = 0;
            p.tam_stack = 0;
            p.tam_const = 0;
        }
        else{
            tam_param = tamParamSegment(&args);

            if (p.tam_stack == 0){
                printf("ERROR: el programa no define Stack Segment\n");
                fclose(archivo);
                return 1;
            }
            if (p.tam_code != 0 && p.entry >= p.tam_code){
                printf("ERROR: el entry point esta fuera del Code Segment\n");
                fclose(archivo);
                return 1;
            }
            if (tam_param > 0xFFFF){
                printf("ERROR: los parametros no entran en el Param Segment\n");
                fclose(archivo);
                return 1;
            }
        }

        uint32_t tam_memoria = args.memoria_kib * 1024;

        if (!cabeceraEntraEnMemoria(&p, tam_param, tam_memoria)){
            printf("ERROR: memoria insuficiente para alojar el proceso\n");
            fclose(archivo);
            return 1;
        }

        inicializo_tabla_segmentos(&p, tam_param);

        // Cargo el Code Segment y el Const Segment
        if (p.tam_code != 0){
            uint32_t base_code = tabla_seg[registros[CS] >> 16].base;
            if (fread(&RAM[base_code], 1, p.tam_code, archivo) != p.tam_code){
                printf("Error: el archivo .vmx esta incompleto\n");
                fclose(archivo);
                return 1;
            }
        }

        if (p.tam_const != 0){
            uint32_t base_const = tabla_seg[registros[KS] >> 16].base;
            if (fread(&RAM[base_const], 1, p.tam_const, archivo) != p.tam_const){
                printf("Error: el archivo .vmx esta incompleto\n");
                fclose(archivo);
                return 1;
            }
        }

        fclose(archivo);

        if (p.version == 2){
            if (tam_param != 0)
                cargoParametros(&args);
            else{
                PUSH(2, 0, -1, 0);   // argv
                PUSH(2, 0, 0, 0);    // argc
                PUSH(2, 0, -1, 0);   // direccion de retorno
            }
        }

        if (args.archivo_vmi != NULL){
            hay_vmi = 1;
            nombre_vmi = args.archivo_vmi;
        }
        else
            hay_vmi = 0;
    }
    else{
        // Solo .vmi
        if (cargoImagen(&args) != 0)
            return 1;
        hay_vmi = 1;
        nombre_vmi = args.archivo_vmi;
    }

    memoria_vm_kib = args.memoria_kib;

    if (args.modo_disassembler)
        desensamblar();

    // Se ejecuta mientras el IP apunte dentro del Code Segment
    while (error_actual == ERR_NINGUNO
           && registros[CS] != 0xFFFFFFFF
           && registros[IP] != 0xFFFFFFFF
           && (registros[IP] >> 16) == (registros[CS] >> 16)
           && (registros[IP] & 0xFFFF) < tabla_seg[registros[CS] >> 16].tam){

        buscoOperacion();

        // Si la instruccion ejecutada fue SYS 0xF, el breakpoint ya se atendio dentro de SYS
        int fue_breakpoint = (registros[OPC] == 0 && (registros[OP1] >> 24) == 2 && (registros[OP1] & 0xFFFF) == 0xF);

        if (!fue_breakpoint && breakpoint_enter && error_actual == ERR_NINGUNO && registros[IP] != 0xFFFFFFFF)
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
        registros[IP] = 0xFFFFFFFF;
        return 1;
    }

    return 0;
}