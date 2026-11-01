; PRUEBA 8B - registro CC (flags N Z C V = bits 31 30 29 28) y AC
; Traducir: vmt t8b_cc_ac.asm t8b.vmx v=2
; Ejecutar: vmx t8b.vmx
; Cada "mov [n], cc" guarda el CC que dejo la instruccion anterior (MOV lee antes de modificar CC).
; Salida: 17 celdas en hexadecimal:
;   celda  0  0x90000000  ADD 0x7FFFFFFF+0x7FFFFFFF   N=1 V=1
;   celda  1  0x60000000  ADD -1 + 1                  Z=1 C=1
;   celda  2  0xA0000000  SUB -3 - 5                  N=1 C=1   (tabla del enunciado)
;   celda  3  0x30000000  MUL 1073741827 * 4          C=1 V=1   (tabla del enunciado)
;   celda  4  0x40000000  DIV 6 / 7                   Z=1       (tabla del enunciado)
;   celda  5  0x00000006  AC = resto de la division
;   celda  6  0x70000000  SHL 0x40000000 << 2         Z=1 C=1 V=1 (tabla del enunciado)
;   celda  7  0x80000000  MOV -1                      N=1       (tabla del enunciado)
;   celda  8  0x20000000  CMP 9,5                     C=1
;   celda  9  0x80000000  CMP 5,9                     N=1
;   celda 10  0x60000000  CMP 5,5                     Z=1 C=1 (acarreo de A + (-B))
;   celda 11  0x40000000  AND 0xF0 & 0x0F = 0
;   celda 12  0x80000000  OR  0x80000000 | 1
;   celda 13  0x40000000  XOR eax,eax
;   celda 14  0x80000000  NOT 0 = -1
;   celda 15  0x90000000  CC intacto tras LDL, LDH, PUSH y POP
;   celda 16  0x00000057  AC usado como registro comun (0x50 + 7)
\\DATA 80
\\STACK 64

main:   ; 0) ADD con overflow
        mov eax, 0
        ldh eax, 0x7FFF
        ldl eax, 0xFFFF
        add eax, eax
        mov [0], cc

        ; 1) ADD con acarreo
        mov eax, -1
        add eax, 1
        mov [4], cc

        ; 2) SUB
        mov eax, -3
        sub eax, 5
        mov [8], cc

        ; 3) MUL
        mov eax, 0
        ldh eax, 0x4000
        ldl eax, 3
        mul eax, 4
        mov [12], cc

        ; 4 y 5) DIV y AC
        mov eax, 6
        div eax, 7
        mov [16], cc
        mov [20], ac

        ; 6) SHL
        mov eax, 0
        ldh eax, 0x4000
        shl eax, 2
        mov [24], cc

        ; 7) MOV
        mov eax, -1
        mov [28], cc

        ; 8, 9 y 10) CMP
        mov eax, 9
        cmp eax, 5
        mov [32], cc
        mov eax, 5
        cmp eax, 9
        mov [36], cc
        cmp eax, 5
        mov [40], cc

        ; 11) AND
        mov eax, 0xF0
        and eax, 0x0F
        mov [44], cc

        ; 12) OR
        mov eax, 0
        ldh eax, 0x8000
        or eax, 1
        mov [48], cc

        ; 13) XOR
        xor eax, eax
        mov [52], cc

        ; 14) NOT
        mov eax, 0
        not eax
        mov [56], cc

        ; 15) instrucciones que NO deben tocar CC
        mov eax, 0
        ldh eax, 0x7FFF
        ldl eax, 0xFFFF
        add eax, eax            ; CC = 0x90000000
        ldl ecx, 5
        ldh ecx, 6
        push eax
        pop ecx
        mov [60], cc

        ; 16) AC como registro de uso general
        mov ac, 0x50
        add ac, 7
        mov [64], ac

        mov edx, DS
        ldh ecx, 4
        ldl ecx, 17
        mov eax, 0x08
        sys 2
        ret
