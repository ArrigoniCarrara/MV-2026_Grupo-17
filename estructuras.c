#include <stdint.h> // Tipos de tamaños de BYTES
#define TAM_MEMORIA 16384
#define TAM_SEG 7

unsigned char RAM[TAM_MEMORIA] = {0}; // Memoria principal

typedef struct { 

    char identificador[5];  
    uint8_t version;        
    uint16_t tamano_codigo; 

} programa; // Cabecera de Programa

typedef struct{
    uint16_t base;
    uint16_t tam;
} tipo_seg;

tipo_seg tabla_seg[TAM_SEG]; 

