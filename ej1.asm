inicio: mov eax, 1         ; acumulador del factorial
        mov ebx, 5         ; contador
fact: cmp ebx, 0
        jz fin_fact
        mul eax, ebx        ; eax = eax * ebx
        sub ebx, 1
        jmp fact
fin_fact: mov [0], eax     ; guardo 5! en DS+0
        xor ecx, ecx         ; acumulador de la suma
        mov edx, 1            ; contador
suma: cmp edx, 6
        jz fin_suma
        add ecx, edx
        add edx, 1
        jmp suma
fin_suma: mov [4], ecx     ; guardo suma(1..5) en DS+4

        mov eax, 0x0B         ; valor a analizar: 1011 en binario
        xor ac, ac             ; contador de bits en 1
        mov ebx, eax
bits: cmp ebx, 0
        jz fin_bits
        jnn sigue
        add ac, 1
sigue: shl ebx, 1
        jmp bits
fin_bits: mov [8], ac      ; guardo cantidad de bits en 1 en DS+8

        ; ==== impresión de los tres resultados ====
        mov edx, DS         ; apunto al comienzo de los datos guardados
        mov ecx, 0
        ldh ecx, 4            ; tamaño de cada valor: 4 bytes
        ldl ecx, 3             ; cantidad de valores: 3
        mov eax, 0x19          ; formato: decimal(0x01) + hex(0x08) + binario(0x10)
        sys 0x2

        stop