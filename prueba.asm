\\DATA 64                 ; Data Segment de 64 bytes (ajustá la directiva a tu ensamblador)

        ; --- 1 leo un nombre de hasta 20 caracteres en DS+0 y lo imprimo
main: MOV EDX, DS
        SYS 15       ; EDX = direccion logica del inicio del Data Segment
        MOV ECX, 20       ; maximo 20 caracteres (mas el '\0')
        SYS 0x03          ; STRING READ: guarda lo que escribo en [EDX]
        MOV EDX, DS
        SYS 0x04          ; STRING WRITE: imprime desde [EDX] hasta el '\0'

        ; --- 2 limito a 3 caracteres: si escribo "Mundo" guarda "Mun"
        MOV EDX, DS
        ADD EDX, 30       ; ahora escribo en la celda 30 del Data Segment
        MOV ECX, 3
        SYS 0x03
        MOV EDX, DS
        ADD EDX, 30
        SYS 0x04

        ; --- 3 sin limite (ECX = -1): guarda toda la linea que escriba
        MOV EDX, DS
        ADD EDX, 40
        MOV ECX, -1
        SYS 0x03
        MOV EDX, DS
        ADD EDX, 40
        SYS 0x04

        STOP