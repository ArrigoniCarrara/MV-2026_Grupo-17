\\DATA 16
\\EXTRA 16
\\STACK 64

main:   ; --- Paso 0: dejo el Extra Segment en cero para detectar contaminacion ---
        mov [ES+0], 0
        mov [ES+4], 0
        sys 0xF
        ; --- Control: leer 1 celda de 4 bytes en DS+12 (ultimos 4 bytes validos) ---
        mov eax, 0x01       ; decimal
        mov edx, DS
        add edx, 12
        ldh ecx, 4          ; celdas de 4 bytes
        ldl ecx, 1          ; 1 celda
        sys 1               ; ingresar 7 -> debe andar

        ; --- CASO 1 (SYS 1): leer 2 celdas desde DS+12, la 2da cae afuera ---
        mov eax, 0x01
        mov edx, DS
        add edx, 12
        ldh ecx, 4
        ldl ecx, 2          ; 2 celdas de 4 bytes = 8 bytes, solo entran 4
        sys 1               ; ingresar 11 y 22 -> fallo de segmento en la 2da

        ; --- CASO 2 (SYS 2): escribir 3 celdas desde DS+8 ---
        ; mov eax, 0x01
        ; mov edx, DS
        ; add edx, 8
        ; ldh ecx, 4
        ; ldl ecx, 3        ; 12 bytes desde el 8 -> hasta el 19, afuera
        ; sys 2

        ; --- CASO 3 (SYS 2): EDX arranca fuera del segmento ---
        ; mov eax, 0x01
        ; mov edx, DS
        ; add edx, 100
        ; ldh ecx, 4
        ; ldl ecx, 1
        ; sys 2

        ; --- CASO 4 (SYS 1): EDX apunta a un segmento inexistente ---
        ; mov eax, 0x01
        ; mov edx, KS       ; no hay Const Segment: KS = -1
        ; ldh ecx, 4
        ; ldl ecx, 1
        ; sys 1

        ; --- CASO 5 (SYS 1): celdas de 1 byte, justo el limite ---
        ; mov eax, 0x01
        ; mov edx, DS
        ; add edx, 15
        ; ldh ecx, 1
        ; ldl ecx, 1        ; 1 byte en el offset 15 -> debe andar
        ; sys 1
        ; (cambiar a ldl ecx, 2 -> el 2do byte cae en el 16 -> fallo)

        ; --- Dump del Extra Segment: si NO se detecto el error, aca aparece lo que se escribio afuera ---
        mov eax, 0x01
        mov edx, ES
        ldh ecx, 4
        ldl ecx, 1
        sys 2
        ret