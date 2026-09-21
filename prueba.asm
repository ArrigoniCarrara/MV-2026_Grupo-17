INICIO:
    MOV EAX, 0x08       ; Configuración: Modo caracteres (bit 1 en 1)
    MOV EDX, DS         ; Puntero base del Segmento de Datos
    ADD EDX, 0          ; Apuntar a la dirección de memoria inicial [0]
    LDH ECX, 1          ; Tamaño del elemento: 1 byte por carácter
    LDL ECX, 4          ; Cantidad de elementos a leer: 4 caracteres
    SYS 0x1             ; Llamada al sistema para LEER (READ)
    MOV EAX, 0x02       ; Configuración: Modo caracteres
    MOV EDX, DS         ; Puntero base del Segmento de Datos
    ADD EDX, 0          ; Apuntar al inicio de donde se leyeron [0]
    LDH ECX, 1          ; Tamaño de cada dato: 1 byte
    LDL ECX, 4          ; Cantidad de elementos: 4 caracteres
    SYS 0x2             ; Llamada al sistema para ESCRIBIR (WRITE)
    STOP                ; Detener la ejecución