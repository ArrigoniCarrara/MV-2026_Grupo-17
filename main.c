#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Definimos la estructura exacta del Header (8 bytes en total)
#pragma pack(push, 1) // Evita que el compilador agregue bytes de padding[cite: 1]
typedef struct {
    char identificador[5]; // Debería contener "VMX26"
    uint8_t version;       // Debe ser 1
    uint16_t tamano_codigo; // Cantidad de bytes del segmento de código
} HeaderVMX;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    FILE *archivo = fopen("prueba.vmx", "rb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo .vmx");
        return EXIT_FAILURE;
    }

    // 1. Leer la cabecera completa
    HeaderVMX header;
    if (fread(&header, sizeof(HeaderVMX), 1, archivo) != 1) {
        fprintf(stderr, "Error al leer la cabecera del archivo\n");
        fclose(archivo);
        return EXIT_FAILURE;
    }

    // 2. Validar que sea un archivo VMX válido[cite: 1, 2]
    if (strncmp(header.identificador, "VMX26", 5) != 0) {
        fprintf(stderr, "Formato no válido: no se encontró la firma VMX26\n");
        fclose(archivo);
        return EXIT_FAILURE;
    }

    printf("--- Cabecera VMX leída con éxito ---\n");
    printf("Identificador: %.5s\n", header.identificador);
    printf("Versión: %u\n", header.version);
    printf("Tamaño de código: %u bytes\n\n", header.tamano_codigo);

    // 3. Cargar el código máquina en la memoria RAM[cite: 1, 2]
    // (Ejemplo: RAM simulada como un arreglo continuo de 16 KiB)[cite: 1, 2]
    unsigned char RAM[16384] = {0}; 

    size_t bytes_leidos = fread(RAM, sizeof(unsigned char), header.tamano_codigo, archivo);
    printf("Bytes de código máquina cargados en RAM: %zu\n", bytes_leidos);

    // 4. Mostrar el código máquina en formato Hexadecimal[cite: 1, 2]
    printf("Hexdump del código cargado:\n");
    for (size_t i = 0; i < bytes_leidos; i++) {
        printf("%02X ", RAM[i]);
    }
    printf("\n");

    fclose(archivo);
    return EXIT_SUCCESS;
}