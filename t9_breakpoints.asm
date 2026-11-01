; PRUEBA 9 - SYS 0xF (breakpoint) para ver como cambian los registros
; Traducir:   vmt t9_breakpoints.asm t9.vmx v=2
; Consola 1:  vmx t9.vmx t9.vmi
; Consola 2:  vmg t9.vmi -r -s
; En la consola 1 escribi 'g' + Enter en cada pausa (o solo Enter para ejecutar 1 instruccion).
;
; Layout (sin constantes ni parametros): CS=0x00000000 DS=0x00010000 ES=0x00020000 SS=0x00030000
;   Tabla: [0] base 0 tam 46 | [1] base 46 tam 32 | [2] base 78 tam 16 | [3] base 94 tam 32
; Offsets de las instrucciones (el codigo mide 46 bytes):
;   0 4 8(BP1) 11 14 19(BP2) 22 24 27(BP3) 30 32 34 38 42(BP4) 45(ret)
\\DATA 32
\\EXTRA 16
\\STACK 32

main:   mov eax, 5              ; off 0
        mov ebx, 7              ; off 4
        sys 0xF                 ; off 8   -> BREAKPOINT 1
        add eax, ebx            ; off 11  EAX = 12
        mov [0], eax            ; off 14  escribe 12 en DS+0
        sys 0xF                 ; off 19  -> BREAKPOINT 2
        push eax                ; off 22
        push 0x1234             ; off 24
        sys 0xF                 ; off 27  -> BREAKPOINT 3
        pop ecx                 ; off 30  ECX = 0x1234
        pop edx                 ; off 32  EDX = 12
        mov ac, 0x55            ; off 34
        sub eax, 20             ; off 38  EAX = -8, N = 1
        sys 0xF                 ; off 42  -> BREAKPOINT 4
        ret                     ; off 45  termina el proceso
