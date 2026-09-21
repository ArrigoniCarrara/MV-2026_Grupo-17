#include <stdint.h> // Tipos de tamaños de BYTES
#define TAM_MEMORIA 16384
#define TAM_SEG 7
#define CANT_REG 31
#define IP 0
#define OPC 1
#define OP1 2
#define OP2 3
#define LAR 4
#define MAR 5 
#define MBR 6
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

unsigned char RAM[TAM_MEMORIA]; // Memoria principal

typedef struct { 

    char identificador[5];  
    uint8_t version;        
    uint16_t tam_codigo; 

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
    ERR_FALLO_SEGMENTO
} tipo_error;
 
tipo_error error_actual = ERR_NINGUNO;