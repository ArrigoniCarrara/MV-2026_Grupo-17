mov eax, 0x02      
mov edx, DS        ; apunto al inicio del data segment        
ldh ecx, 1           ; tamaño de cada valor: 1 byte
ldl ecx, 4            ; cantidad de valores a leer: 4
sys 0x1                ; syscall READ
mov edx, DS        ; apunto al inicio del data segment
ldh ecx, 1          ; tamaño de cada valor: 1 byte
ldl ecx, 4           ; cantidad de valores: 4
mov eax, 0x12         ; modo: binario (0x10) + caracter (0x02)
sys 0x2                ; syscall WRITE