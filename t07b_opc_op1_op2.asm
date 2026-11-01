; t07b - valores de OPC, OP1 y OP2
; Truco: una instruccion que LEE OPC/OP1/OP2 ve los de ella misma (se cargan antes de ejecutarla).
;   OPC = codigo de operacion (ADD=0x11 ...)
;   OPx = tipo en el byte alto (01 registro, 10 inmediato, 11 memoria, 00 no existe) + operando tal cual esta codificado:
;     registro : (sector<<5)|codigo   (EAX=0x0A, AL=0x2A, AH=0x4A, AX=0x6A)
;     inmediato: 2 bytes
;     memoria  : desplazamiento(2 bytes) y luego (tamanio<<5)|codigo de registro (b=1, w=2, l=4; sin registro se usa DS=27)
;   Ej. del enunciado: ADD [DS+5], 10  ->  OPC=0x11  OP1=0x0300059B  OP2=0x0200000A
; ======================================================================
; PARA EJECUTAR:  vmt t07b_opc_op1_op2.asm   y despues   vmx t07b_opc_op1_op2.vmx
; SALIDA ESPERADA:
; | --- A: OPC ---
; | --- B: OP1 / OP2 con operandos de registro ---
; | --- C: OP1 con operandos de memoria ---
; | --- D: instruccion de 1 operando (OP2 = 0) ---
; | RESULTADO: 32/32 OK
; ======================================================================
\\include "util.asm"
t_a     equ "--- A: OPC ---\n"
t_b     equ "--- B: OP1 / OP2 con operandos de registro ---\n"
t_c     equ "--- C: OP1 con operandos de memoria ---\n"
t_d     equ "--- D: instruccion de 1 operando (OP2 = 0) ---\n"

main:   push    bp
        mov     bp, sp
        call    util_init

        push    t_a
        call    puts
        add     sp, 4
        mov     eax, opc            ; MOV = 0x10
        ; check #1
        push    eax
        push    16
        push    1
        call    check
        add     sp, 12
        mov     eax, 0
        add     eax, opc            ; ADD = 0x11
        ; check #2
        push    eax
        push    17
        push    2
        call    check
        add     sp, 12
        mov     eax, 0
        sub     eax, opc            ; SUB = 0x12 -> 0 - 18
        ; check #3
        push    eax
        push    -18
        push    3
        call    check
        add     sp, 12
        mov     eax, 1
        mul     eax, opc            ; MUL = 0x13
        ; check #4
        push    eax
        push    19
        push    4
        call    check
        add     sp, 12
        mov     eax, 100
        div     eax, opc            ; DIV = 0x14 = 20 -> 100 / 20
        ; check #5
        push    eax
        push    5
        push    5
        call    check
        add     sp, 12
        mov     eax, 21             ; CMP = 0x15
        mov     ecx, 0
        cmp     eax, opc
        jnz     a_cmp
        mov     ecx, 1
a_cmp: push    ecx
        push    1
        push    6
        call    check
        add     sp, 12
        mov     eax, -1
        and     eax, opc            ; AND = 0x16
        ; check #7
        push    eax
        push    22
        push    7
        call    check
        add     sp, 12
        mov     eax, 0
        or      eax, opc            ; OR = 0x17
        ; check #8
        push    eax
        push    23
        push    8
        call    check
        add     sp, 12
        mov     eax, 0
        xor     eax, opc            ; XOR = 0x18
        ; check #9
        push    eax
        push    24
        push    9
        call    check
        add     sp, 12
        mov     eax, 7
        swap    eax, opc            ; SWAP = 0x19  (EAX recibe OPC)
        ; check #10
        push    eax
        push    25
        push    10
        call    check
        add     sp, 12
        mov     eax, 1
        shl     eax, opc            ; SHL = 0x1A = 26 -> 1 << 26
        ; check #11 (esperado 67108864)
        ldh     efx, 1024
        ldl     efx, 0
        push    eax
        push    efx
        push    11
        call    check
        add     sp, 12
        ldh     eax, 16384          ; 0x40000000
        ldl     eax, 0
        shr     eax, opc            ; SHR = 0x1B = 27 -> 0x40000000 >> 27
        ; check #12
        push    eax
        push    8
        push    12
        call    check
        add     sp, 12
        mov     eax, -1024
        sar     eax, opc            ; SAR = 0x1C = 28 -> -1024 >> 28 = -1
        ; check #13
        push    eax
        push    -1
        push    13
        call    check
        add     sp, 12
        mov     eax, 0
        ldl     eax, opc            ; LDL = 0x1D
        ; check #14
        push    eax
        push    29
        push    14
        call    check
        add     sp, 12
        mov     eax, 0
        ldh     eax, opc            ; LDH = 0x1E -> 0x001E0000
        shr     eax, 16
        ; check #15
        push    eax
        push    30
        push    15
        call    check
        add     sp, 12
        mov     eax, 99
        rnd     eax, opc            ; RND = 0x1F -> numero entre 0 y 31
        mov     ecx, 1
        cmp     eax, 0
        jnn     a_r1
        mov     ecx, 0
a_r1:   cmp     eax, 31
        jnp     a_r2
        mov     ecx, 0
a_r2: push    ecx
        push    1
        push    16
        call    check
        add     sp, 12

        push    t_b
        call    puts
        add     sp, 4
        mov     eax, op1            ; OP1 = registro EAX -> 01 00 00 0A
        ; check #20 (esperado 16777226)
        ldh     efx, 256
        ldl     efx, 10
        push    eax
        push    efx
        push    20
        call    check
        add     sp, 12
        mov     eax, op2            ; OP2 = registro OP2 (codigo 3) -> 01 00 00 03
        ; check #21 (esperado 16777219)
        ldh     efx, 256
        ldl     efx, 3
        push    eax
        push    efx
        push    21
        call    check
        add     sp, 12
        mov     ebx, op1            ; OP1 = EBX (codigo 11) -> 01 00 00 0B
        ; check #22 (esperado 16777227)
        ldh     efx, 256
        ldl     efx, 11
        push    ebx
        push    efx
        push    22
        call    check
        add     sp, 12
        mov     eax, 0
        mov     al, op1             ; OP1 = AL -> 0x2A en el byte bajo
        ; check #23
        push    eax
        push    42
        push    23
        call    check
        add     sp, 12
        mov     eax, 0
        mov     ah, op1             ; OP1 = AH -> 0x4A
        ; check #24
        push    eax      ; 0x4A00
        push    18944
        push    24
        call    check
        add     sp, 12
        mov     eax, 0
        mov     ax, op1             ; OP1 = AX -> 0x6A
        ; check #25
        push    eax
        push    106
        push    25
        call    check
        add     sp, 12

        push    t_c
        call    puts
        add     sp, 4
        mov     [v0], 0
        mov     [v0], op1           ; OP1 = [DS+64] de 4 bytes: 03 00 40 9B
        ; check #30 (esperado 50348187)
        ldh     efx, 768
        ldl     efx, 16539
        push    [v0]
        push    efx
        push    30
        call    check
        add     sp, 12
        mov     [v0], 0
        mov     w[v0], op1          ; OP1 = w[DS+64]: 03 00 40 5B -> se guardan los 2 bytes bajos 0x405B
        ; check #31
        push    w[v0]
        push    16475
        push    31
        call    check
        add     sp, 12
        mov     [v0], 0
        mov     b[v0], op1          ; OP1 = b[DS+64]: 03 00 40 3B -> byte bajo 0x3B
        mov     ebx, ds
        add     ebx, v0
        ; check #32
        push    b[ebx]
        push    59
        push    32
        call    check
        add     sp, 12
        mov     ebx, ds
        add     ebx, v1
        mov     [ebx+8], op1        ; OP1 = [EBX+8]: 03 00 08 8B
        ; check #33 (esperado 50333835)
        ldh     efx, 768
        ldl     efx, 2187
        push    [ebx+8]
        push    efx
        push    33
        call    check
        add     sp, 12
        mov     [ebx-4], op1        ; OP1 = [EBX-4]: 03 FF FC 8B (desplazamiento negativo en 16 bits)
        ; check #34 (esperado 67107979)
        ldh     efx, 1023
        ldl     efx, -885
        push    [ebx-4]
        push    efx
        push    34
        call    check
        add     sp, 12
        mov     [es], op1           ; OP1 = [ES+0]: 03 00 00 9C
        ; check #35 (esperado 50331804)
        ldh     efx, 768
        ldl     efx, 156
        push    [es]
        push    efx
        push    35
        call    check
        add     sp, 12
        mov     [sp-4], op1         ; OP1 = [SP-4]: 03 FF FC 87 (celda libre debajo del tope)
        ; check #36 (esperado 67107975)
        ldh     efx, 1023
        ldl     efx, -889
        push    [sp-4]
        push    efx
        push    36
        call    check
        add     sp, 12

        push    t_d
        call    puts
        add     sp, 4
        mov     eax, 5
        push    op2                 ; instruccion de 1 operando: OP2 no existe -> vale 0
        pop     eax
        ; check #40
        push    eax
        push    0
        push    40
        call    check
        add     sp, 12
        push    op1                 ; OP1 = registro OP1 (codigo 2) -> 01 00 00 02
        pop     eax
        ; check #41 (esperado 16777218)
        ldh     efx, 256
        ldl     efx, 2
        push    eax
        push    efx
        push    41
        call    check
        add     sp, 12
        push    opc                 ; PUSH = 0x0B
        pop     eax
        ; check #42
        push    eax
        push    11
        push    42
        call    check
        add     sp, 12
        call    summary
        mov     sp, bp
        pop     bp
        stop
