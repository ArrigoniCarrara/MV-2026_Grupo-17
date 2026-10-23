#include <stdint.h> // Tipos de tamaños de BYTES
#define MAX_MEMORIA 64
#define TAM_SEG 8
#define CANT_REG 32
#define IP 0
#define OPC 1
#define OP1 2
#define OP2 3
#define LAR 4
#define MAR 5 
#define MBR 6
#define SP 7
#define BP 8
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15
#define AC 16
#define CC 17
#define CS 26
#define DS 27
#define ES 28
#define SS 29
#define KS 30
#define PS 31

unsigned char RAM[MAX_MEMORIA * 1024]; // Memoria principal

typedef struct { 

    char identificador[5];
    uint8_t version;
    uint32_t tam_code, tam_data, tam_extra, tam_stack, tam_const;
    uint16_t entry;  

} cabecera_programa; // Cabecera de Programa

typedef struct{
    uint16_t base;
    uint16_t tam;
} tipo_seg;

tipo_seg tabla_seg[TAM_SEG]; // Tabla de segmentos

uint32_t registros[CANT_REG]; // Registros

// --- Manejo de errores ---
typedef enum {
    ERR_NINGUNO = 0,
    ERR_INSTRUCCION_INVALIDA,
    ERR_DIVISION_CERO,
    ERR_FALLO_SEGMENTO,
    ERR_OVERFLOW,
    ERR_UNDERFLOW
} tipo_error;
 
tipo_error error_actual = ERR_NINGUNO; 

int      hay_vmi     = 0;     
char    *nombre_vmi  = NULL;  
uint32_t memoria_vm_kib = 16; 
int breakpoint_enter = 0;