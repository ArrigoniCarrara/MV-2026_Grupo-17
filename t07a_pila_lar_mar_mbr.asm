; t07a - PUSH / POP / CALL / RET y los registros LAR, MAR y MBR
; Despues de cada operacion de pila se copian LAR, MAR y MBR a registros comunes (MOV entre registros no accede a memoria,
; asi que no los modifica) y se verifican:
;   LAR = direccion logica accedida (SP)   MAR = (cantidad de bytes << 16) | direccion fisica   MBR = valor leido/escrito
; ======================================================================
; PARA EJECUTAR:  vmt t07a_pila_lar_mar_mbr.asm   y despues   vmx t07a_pila_lar_mar_mbr.vmx
; SALIDA ESPERADA:
; | --- A: PUSH ---
; | MAR (completo) despues del primer PUSH = 0x00041AA4
; | base fisica del Stack Segment (MAR bajo - offset de SP) = 5816
; | --- B: POP ---
; | (informativo) MBR tras POP w[..] de 0x12345678 = 0x00005678
; | --- C: CALL / RET ---
; | --- D: otros accesos y no-accesos ---
; | RESULTADO: 67/67 OK
; ======================================================================
\\include "util.asm"
t_a     equ "--- A: PUSH ---\n"
t_b     equ "--- B: POP ---\n"
t_c     equ "--- C: CALL / RET ---\n"
t_d     equ "--- D: otros accesos y no-accesos ---\n"
s_base  equ "base fisica del Stack Segment (MAR bajo - offset de SP) = "
s_mar   equ "MAR (completo) despues del primer PUSH = "
s_mbr   equ "(informativo) MBR tras POP w[..] de 0x12345678 = "

main:   push    bp
        mov     bp, sp
        call    util_init
        push    t_a
        call    puts
        add     sp, 4
        ldh     eax, 4660
        ldl     eax, 22136          ; EAX = 0x12345678
        push    eax
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- PUSH EAX (0x12345678)
        ; check #1
        push    [v0]
        push    [v3]
        push    1
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #2
        push    eax
        push    ebx
        push    2
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #3
        push    eax
        push    4
        push    3
        call    check
        add     sp, 12
        ; check #4 (esperado 305419896)
        ldh     efx, 4660
        ldl     efx, 22136
        push    [v2]
        push    efx
        push    4
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     [v12], eax
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        mov     [v4], eax        ; base fisica del SS
        push    s_mar
        call    puts
        add     sp, 4
        push    [v1]
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_base
        call    puts
        add     sp, 4
        push    [v4]
        call    print_int
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    -1
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- PUSH -1 (inmediato)
        ; check #5
        push    [v0]
        push    [v3]
        push    5
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #6
        push    eax
        push    ebx
        push    6
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #7
        push    eax
        push    4
        push    7
        call    check
        add     sp, 12
        ; check #8
        push    [v2]
        push    -1
        push    8
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     [v13], eax
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        ; check #9
        push    eax
        push    [v4]
        push    9
        call    check
        add     sp, 12
        mov     eax, [v12]
        sub     eax, [v13]
        ; check #10
        push    eax
        push    4
        push    10
        call    check
        add     sp, 12
        ldh     eax, 4660
        ldl     eax, -21555
        push    ax
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- PUSH AX con EAX=0x1234ABCD (se expande con signo a 0xFFFFABCD)
        ; check #11
        push    [v0]
        push    [v3]
        push    11
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #12
        push    eax
        push    ebx
        push    12
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #13
        push    eax
        push    4
        push    13
        call    check
        add     sp, 12
        ; check #14
        push    [v2]
        push    -21555
        push    14
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     [v14], eax
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        ; check #15
        push    eax
        push    [v4]
        push    15
        call    check
        add     sp, 12
        mov     eax, [v13]
        sub     eax, [v14]
        ; check #16
        push    eax
        push    4
        push    16
        call    check
        add     sp, 12
        mov     [v5], 777
        push    [v5]
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- PUSH [v5] (memoria 4 bytes): el ultimo acceso es la escritura en la pila
        ; check #17
        push    [v0]
        push    [v3]
        push    17
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #18
        push    eax
        push    ebx
        push    18
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #19
        push    eax
        push    4
        push    19
        call    check
        add     sp, 12
        ; check #20
        push    [v2]
        push    777
        push    20
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     [v15], eax
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        ; check #21
        push    eax
        push    [v4]
        push    21
        call    check
        add     sp, 12
        mov     eax, [v14]
        sub     eax, [v15]
        ; check #22
        push    eax
        push    4
        push    22
        call    check
        add     sp, 12
        mov     [v6], 0
        mov     w[v6], 258
        push    w[v6]
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- PUSH w[v6] (memoria 2 bytes)
        ; check #23
        push    [v0]
        push    [v3]
        push    23
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #24
        push    eax
        push    ebx
        push    24
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #25
        push    eax
        push    4
        push    25
        call    check
        add     sp, 12
        ; check #26
        push    [v2]
        push    258
        push    26
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     [v16], eax
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        ; check #27
        push    eax
        push    [v4]
        push    27
        call    check
        add     sp, 12
        mov     eax, [v15]
        sub     eax, [v16]
        ; check #28
        push    eax
        push    4
        push    28
        call    check
        add     sp, 12
        push    t_b
        call    puts
        add     sp, 4
        ; v5 se usa para guardar SP; los PUSH de arriba ya terminaron
        mov     [v6], sp        ; SP antes del POP (v6 libre ahora)
        pop     ebx
        mov     eex, lar
        mov     efx, mar
        mov     edx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], edx
        mov     [v3], ecx
        ; ---- POP EBX  <- 258 (ultimo PUSH, el de w[v6])
        ; check #29
        push    [v0]
        push    [v6]
        push    29
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #30
        push    eax
        push    ebx
        push    30
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #31
        push    eax
        push    4
        push    31
        call    check
        add     sp, 12
        ; check #32
        push    [v2]
        push    258
        push    32
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        ; check #33
        push    eax
        push    [v16]
        push    33
        call    check
        add     sp, 12
        mov     eax, [v6]
        add     eax, 4
        ; check #34
        push    [v3]
        push    eax
        push    34
        call    check
        add     sp, 12
        mov     [v6], sp        ; SP antes del POP (v6 libre ahora)
        pop     ecx
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- POP ECX  <- 777 (PUSH [v5])
        ; check #35
        push    [v0]
        push    [v6]
        push    35
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #36
        push    eax
        push    ebx
        push    36
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #37
        push    eax
        push    4
        push    37
        call    check
        add     sp, 12
        ; check #38
        push    [v2]
        push    777
        push    38
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        ; check #39
        push    eax
        push    [v15]
        push    39
        call    check
        add     sp, 12
        mov     eax, [v6]
        add     eax, 4
        ; check #40
        push    [v3]
        push    eax
        push    40
        call    check
        add     sp, 12
        mov     [v6], sp        ; SP antes del POP (v6 libre ahora)
        pop     edx
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- POP EDX  <- 0xFFFFABCD (PUSH AX)
        ; check #41
        push    [v0]
        push    [v6]
        push    41
        call    check
        add     sp, 12
        mov     eax, [v0]
        shr     eax, 16
        mov     ebx, ss
        shr     ebx, 16
        ; check #42
        push    eax
        push    ebx
        push    42
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #43
        push    eax
        push    4
        push    43
        call    check
        add     sp, 12
        ; check #44
        push    [v2]
        push    -21555
        push    44
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        ; check #45
        push    eax
        push    [v14]
        push    45
        call    check
        add     sp, 12
        mov     eax, [v6]
        add     eax, 4
        ; check #46
        push    [v3]
        push    eax
        push    46
        call    check
        add     sp, 12
        mov     [v6], sp
        pop     [v7]                ; <- -1 (PUSH -1). Ultimo acceso: escritura de 4 bytes en DS+v7
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- POP [v7]: LAR/MAR/MBR describen la escritura en DS, no la lectura de la pila
        mov     eax, ds
        add     eax, v7
        ; check #47
        push    [v0]
        push    eax
        push    47
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #48
        push    eax
        push    4
        push    48
        call    check
        add     sp, 12
        ; check #49
        push    [v2]
        push    -1
        push    49
        call    check
        add     sp, 12
        mov     eax, [v1]
        shl     eax, 16
        shr     eax, 16
        mov     ebx, [v0]
        shl     ebx, 16
        shr     ebx, 16
        sub     eax, ebx
        mov     [v8], eax   ; base fisica del DS
        mov     eax, [v6]
        add     eax, 4
        ; check #50
        push    [v3]
        push    eax
        push    50
        call    check
        add     sp, 12
        ; check #51
        push    [v7]
        push    -1
        push    51
        call    check
        add     sp, 12
        mov     [v6], sp
        pop     w[v9]               ; <- 0x12345678 (PUSH EAX): se guardan solo los 2 bytes bajos
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- POP w[v9]: escritura de 2 bytes
        mov     eax, ds
        add     eax, v9
        ; check #52
        push    [v0]
        push    eax
        push    52
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #53
        push    eax
        push    2
        push    53
        call    check
        add     sp, 12
        mov     eax, [v6]
        add     eax, 4
        ; check #54
        push    [v3]
        push    eax
        push    54
        call    check
        add     sp, 12
        mov     eax, [v9]
        shr     eax, 16
        ; check #55
        push    eax
        push    22136
        push    55
        call    check
        add     sp, 12
        push    s_mbr
        call    puts
        add     sp, 4
        push    [v2]
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        ; la pila quedo vacia: SP debe ser el inicial de main (ver t06). Se verifica el balance con CALL/RET:
        push    t_c
        call    puts
        add     sp, 4
        mov     [v6], sp            ; SP antes del CALL
        call    t_sub
t_ret1: mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; ---- despues del RET: la ultima operacion fue la lectura de la direccion de retorno
        ; check #56
        push    [v0]
        push    [v24]
        push    56
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #57
        push    eax
        push    4
        push    57
        call    check
        add     sp, 12
        mov     eax, cs
        add     eax, t_ret1
        ; check #58
        push    [v2]
        push    eax
        push    58
        call    check
        add     sp, 12
        ; check #59
        push    [v3]
        push    [v6]
        push    59
        call    check
        add     sp, 12
        ; ---- dentro de la subrutina (justo despues del CALL):
        ; check #60
        push    [v21]
        push    [v24]
        push    60
        call    check
        add     sp, 12
        mov     eax, [v22]
        shr     eax, 16
        ; check #61
        push    eax
        push    4
        push    61
        call    check
        add     sp, 12
        mov     eax, cs
        add     eax, t_ret1
        ; check #62
        push    [v23]
        push    eax
        push    62
        call    check
        add     sp, 12
        mov     eax, [v6]
        sub     eax, 4
        ; check #63
        push    [v24]
        push    eax
        push    63
        call    check
        add     sp, 12
        push    t_d
        call    puts
        add     sp, 4
        push    4242
        mov     eax, [sp]           ; lectura a traves de SP
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v0], eex
        mov     [v1], efx
        mov     [v2], ebx
        mov     [v3], ecx
        ; check #64
        push    [v0]
        push    [v3]
        push    64
        call    check
        add     sp, 12
        mov     eax, [v1]
        shr     eax, 16
        ; check #65
        push    eax
        push    4
        push    65
        call    check
        add     sp, 12
        ; check #66
        push    [v2]
        push    4242
        push    66
        call    check
        add     sp, 12
        pop     eax
        ; instrucciones que no acceden a memoria (ni la busqueda de la instruccion) NO deben cambiar LAR/MAR/MBR
        push    99
        mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     eax, 5
        add     eax, 7
        cmp     eax, 12
        shl     eax, 2
        mov     edx, ecx
        jmp     t_nm
t_nm:   mov     ecx, 1
        cmp     eex, lar
        jz      t_nm1
        mov     ecx, 0
t_nm1: cmp     efx, mar
        jz      t_nm2
        mov     ecx, 0
t_nm2: cmp     ebx, mbr
        jz      t_nm3
        mov     ecx, 0
t_nm3: mov     [v25], ecx
        ; check #67
        push    [v25]
        push    1
        push    67
        call    check
        add     sp, 12
        pop     eax
        call    summary
        mov     sp, bp
        pop     bp
        stop
t_sub:  mov     eex, lar
        mov     efx, mar
        mov     ebx, mbr
        mov     ecx, sp
        mov     [v21], eex
        mov     [v22], efx
        mov     [v23], ebx
        mov     [v24], ecx
        ret
