\\DATA 64
\\STACK 256

; Cada par contiene: resultado del shift, valor bruto del CC.
; Los valores de 32 bits se construyen usando LDH/LDL.

main:       mov     eax, 0
            ldh     eax, 0x4000       ; 0x40000000
            shl     eax, 2
            mov     [4], cc
            mov     [0], eax          ; 0; N=0 Z=1 C=1 V=1 (ejemplo guia)

            mov     eax, 1
            ldh     eax, 0x8000       ; 0x80000001
            shr     eax, 1
            mov     [12], cc
            mov     [8], eax          ; 0x40000000

            mov     eax, 0
            ldh     eax, 0x8000       ; 0x80000000
            sar     eax, 1
            mov     [20], cc
            mov     [16], eax         ; 0xC0000000 (-1073741824)

            mov     eax, 0xFFFF
            ldh     eax, 0x7FFF       ; 0x7FFFFFFF
            sar     eax, 1
            mov     [28], cc
            mov     [24], eax         ; 0x3FFFFFFF (1073741823)

            ; Shift sobre un byte de memoria: b[32] = 0x81 -> 0x02.
            mov     b[32], 0x81
            shl     b[32], 1
            mov     [36], cc
            mov     eax, 0
            mov     al, b[32]
            mov     [32], eax         ; resultado byte extendido a long: 2

            ; Imprime resultado/CC de los cinco casos.
            mov     eax, 0x09
            mov     edx, ds
            add     edx, 0
            ldh     ecx, 0x04
            ldl     ecx, 10
            sys     0x02
            ret
