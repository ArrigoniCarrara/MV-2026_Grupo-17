; PRUEBA 8C - IP, registros de segmento (CS DS ES SS KS PS), SP, BP y pila inicial
; Traducir: vmt t8c_segmentos_pila.asm t8c.vmx v=2
;
; EJECUCION A:  vmx t8c.vmx             (sin parametros: no hay Param Segment)
; EJECUCION B:  vmx t8c.vmx -p uno      (con Param Segment)
;
; Salida: 20 celdas en hexadecimal.
;   celda  contenido                    A (sin -p)    B (-p uno)
;      0   IP (instruccion siguiente)   0x00010003    0x00020003
;      1   CS                           0x00010000    0x00020000
;      2   DS                           0x00020000    0x00030000
;      3   ES                           0x00030000    0x00040000
;      4   SS                           0x00040000    0x00050000
;      5   KS                           0x00000000    0x00010000
;      6   PS                           0xFFFFFFFF    0x00000000
;      7   SP al comenzar main          0x00040034    0x00050034   (SS + 64 - 12)
;      8   BP al comenzar               0x00000000    0x00000000
;      9   [SP]   retorno de main       0xFFFFFFFF    0xFFFFFFFF
;     10   [SP+4] argc                  0x00000000    0x00000002   (nombre del .vmx + 1 param)
;     11   [SP+8] argv                  0xFFFFFFFF    0x0000000C   (offset del arreglo en PS)
;     12   SP tras PUSH AX              0x00040030    0x00050030
;     13   4 bytes del tope             0xFFFF8001    0xFFFF8001   (AX = 0x8001 extendido con signo)
;     14   b[SP]   (byte mas signif.)   0xFFFFFFFF    0xFFFFFFFF
;     15   b[SP+3] (byte menos signif.) 0x00000001    0x00000001
;     16   SP tras POP                  0x00040034    0x00050034
;     17   BP tras MOV BP, SP           0x00040034    0x00050034
;     18   LAR tras PUSH (= SP)         0x00040030    0x00050030
;     19   MBR tras PUSH                0xFFFF8001    0xFFFF8001
; Nota B: argv = 0x0C porque "t8c.vmx\0" ocupa 8 bytes y "uno\0" 4. Si ejecutas con otra
; ruta o nombre, es (largo del nombre + 1) + (largo del parametro + 1).
\\DATA 96
\\EXTRA 32
\\STACK 64

TITULO EQU "Registros de segmento, pila e IP:\n"

main:   mov eax, ip             ; PRIMERA instruccion (3 bytes): IP = CS | 3
        mov [0], eax
        mov [4], cs
        mov [8], ds
        mov [12], es
        mov [16], ss
        mov [20], ks
        mov [24], ps
        mov [28], sp
        mov [32], bp

        mov eax, [sp]
        mov [36], eax
        mov eax, [sp+4]
        mov [40], eax
        mov eax, [sp+8]
        mov [44], eax

        mov eax, 0
        ldl eax, 0x8001         ; AX = 0x8001
        push ax                 ; en la pila: 0xFFFF8001
        mov ebx, lar            ; LAR/MBR de la escritura del PUSH
        mov edx, mbr
        mov [72], ebx
        mov [76], edx
        mov [48], sp
        mov ecx, [sp]
        mov [52], ecx
        mov edx, b[sp]
        mov [56], edx
        mov edx, b[sp+3]
        mov [60], edx
        pop ebx
        mov [64], sp
        mov bp, sp
        mov [68], bp

        mov edx, ks
        add edx, TITULO
        sys 4

        mov edx, DS
        ldh ecx, 4
        ldl ecx, 20
        mov eax, 0x08
        sys 2
        ret
