#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estructuras.c"


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

    if (fread(&p, sizeof(cabecera_programa), 1, archivo) != 1) { // (CORREGIR)se toman los 0 como bytes y se aumenta una bocha los BYTES
        printf("Error al leer la cabecera del archivo\n");
        fclose(archivo);
        return -1;
    }

    // Validar que sea un archivo VMX válido verificando la firma
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
    tabla_seg[1].tam = TAM_MEMORIA - p.tam_codigo;


    fclose(archivo);
    return -1;
}