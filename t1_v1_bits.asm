        mov eax, 0x01       ; modo decimal
        mov edx, DS         ; leer a partir del inicio del Data Segment
        ldh ecx, 4          ; celdas de 4 bytes
        ldl ecx, 2          ; 2 celdas
        sys 0x1             ; lee los dos numeros: [0] y [4]

        mov eax, [edx]      ; primer numero
        add eax, [edx+4]    ; + segundo numero
        mov [edx+8], eax    ; guarda el resultado en la celda 8

        mov eax, 0x01       ; escribir en decimal
        add edx, 8
        ldl ecx, 1          ; 1 sola celda (el tamaño de 4 bytes sigue en ECX)
        sys 0x2
        stop