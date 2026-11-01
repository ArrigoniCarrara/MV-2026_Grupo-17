; PRUEBA 10 - LAR / MAR / MBR en SYS 1, 2, 3, 4, en una escritura de 2 bytes y en CALL
; Traducir:   vmt t10_breakpoints_registros.asm t10.vmx v=2
; Consola 1:  vmx t10.vmx t10.vmi
; Consola 2:  vmg t10.vmi -r -s
;
; Entradas: en SYS 1 escribi 11 + Enter y 22 + Enter. En SYS 3 (no muestra prompt) escribi Hola + Enter.
; En cada pausa escribi g + Enter.
;
; Layout: KS=0x00000000 (tam 4) CS=0x00010000 (tam 86) DS=0x00020000 ES=0x00030000 SS=0x00040000
; Tabla: [0] base 0 tam 4 | [1] base 4 tam 86 | [2] base 90 tam 32 | [3] base 122 tam 16 | [4] base 138 tam 32
\\DATA 32
\\EXTRA 16
\\STACK 32

MSG EQU "Hi\n"

main:   mov edx, DS             ; off 0
        add edx, 4              ; off 3
        ldh ecx, 4              ; off 7
        ldl ecx, 2              ; off 11   ECX = 0x00040002 (2 celdas de 4 bytes)
        mov eax, 1              ; off 15   decimal
        sys 1                   ; off 19   lee 11 y 22 en DS+4 y DS+8
        sys 0xF                 ; off 22   BREAKPOINT 1

        ldl ecx, 1              ; off 25   ECX = 0x00040001
        mov eax, 0x08           ; off 29   hexadecimal
        sys 2                   ; off 33   muestra 1 celda (DS+4)
        sys 0xF                 ; off 36   BREAKPOINT 2

        mov edx, ks             ; off 39
        add edx, MSG            ; off 42   EDX = KS + 0
        sys 4                   ; off 46   imprime "Hi" y baja una linea
        sys 0xF                 ; off 49   BREAKPOINT 3

        mov edx, DS             ; off 52
        add edx, 16             ; off 55
        mov ecx, 10             ; off 59   maximo 10 caracteres
        sys 3                   ; off 63   lee un string en DS+16
        sys 0xF                 ; off 66   BREAKPOINT 4

        mov w[ES+2], 0x1234     ; off 69   escritura de 2 bytes
        sys 0xF                 ; off 75   BREAKPOINT 5

        call rutina             ; off 78
        ret                     ; off 81
rutina: sys 0xF                 ; off 82   BREAKPOINT 6
        ret                     ; off 85
