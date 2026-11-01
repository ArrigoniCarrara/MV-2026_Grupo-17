; t06 - sectores de registro (AL/AH/AX), LDL/LDH, modificadores b/w/l de memoria,
;       orden big-endian, registros de segmento y estado inicial de la pila
; ======================================================================
; PARA EJECUTAR:  vmt t06_registros_memoria.asm   y despues   vmx t06_registros_memoria.vmx
; SALIDA ESPERADA:
; | --- A: estado inicial (registros de segmento y pila) ---
; | KS = 0x00000000
; | CS = 0x00010000
; | DS = 0x00020000
; | ES = 0x00030000
; | SS = 0x00040000
; | PS = 0xFFFFFFFF
; | --- B: sectores de registro ---
; | --- C: LDL / LDH ---
; | --- D: memoria: modificadores y big-endian ---
; | (informativo) w[]=0xFFFF leido en registro de 32 bits = -1   (-1 si se extiende el signo; 65535 si no)
; | --- E: PUSH / POP con sectores (ejemplo del enunciado) ---
; | --- F: constantes string en el Const Segment ---
; | RESULTADO: 47/47 OK
; ======================================================================
\\include "util.asm"

t_a     equ "--- A: estado inicial (registros de segmento y pila) ---\n"
t_b     equ "--- B: sectores de registro ---\n"
t_c     equ "--- C: LDL / LDH ---\n"
t_d     equ "--- D: memoria: modificadores y big-endian ---\n"
t_e     equ "--- E: PUSH / POP con sectores (ejemplo del enunciado) ---\n"
t_f     equ "--- F: constantes string en el Const Segment ---\n"
s_ks    equ "KS = "
s_cs    equ "CS = "
s_ds    equ "DS = "
s_es    equ "ES = "
s_ss    equ "SS = "
s_ps    equ "PS = "
s_ext   equ "(informativo) w[]=0xFFFF leido en registro de 32 bits = "
s_ext2  equ "   (-1 si se extiende el signo; 65535 si no)\n"
t_str   equ "ABC"

main:   mov     eax, sp             ; antes de cualquier push: SP inicial
        mov     ebx, [sp]
        mov     ecx, [sp+4]
        mov     edx, [sp+8]
        push    bp
        mov     bp, sp
        call    util_init

        push    t_a
        call    puts
        add     sp, 4
        mov     eex, ss
        add     eex, 1012           ; SS + 1024 (tamanio de la pila) - 12 (ret, argc, argv)
        ; check #1
        push    eax
        push    eex
        push    1
        call    check
        add     sp, 12
        ; check #2
        push    ebx          ; direccion de retorno de main
        push    -1
        push    2
        call    check
        add     sp, 12
        ; check #3
        push    ecx           ; argc (sin -p)
        push    0
        push    3
        call    check
        add     sp, 12
        ; check #4
        push    edx          ; argv (sin -p)
        push    -1
        push    4
        call    check
        add     sp, 12
        ; check #5
        push    ps           ; no hay Param Segment
        push    -1
        push    5
        call    check
        add     sp, 12
        push    s_ks
        call    puts
        add     sp, 4
        push    ks
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_cs
        call    puts
        add     sp, 4
        push    cs
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_ds
        call    puts
        add     sp, 4
        push    ds
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_es
        call    puts
        add     sp, 4
        push    es
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_ss
        call    puts
        add     sp, 4
        push    ss
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4
        push    s_ps
        call    puts
        add     sp, 4
        push    ps
        call    print_hex
        add     sp, 4
        push    u_nl
        call    puts
        add     sp, 4

        push    t_b
        call    puts
        add     sp, 4
        ldh     eax, 4660           ; 0x1234
        ldl     eax, 22136          ; 0x5678
        mov     ebx, 0
        mov     bl, al              ; ebx = 0x00000078
        ; check #10
        push    ebx
        push    120
        push    10
        call    check
        add     sp, 12
        mov     ebx, 0
        mov     bh, al              ; ebx = 0x00007800
        ; check #11
        push    ebx
        push    30720
        push    11
        call    check
        add     sp, 12
        mov     ecx, 0
        mov     cx, ax              ; ecx = 0x00005678
        ; check #12
        push    ecx
        push    22136
        push    12
        call    check
        add     sp, 12
        mov     bl, ah              ; ebx = 0x00007856
        ; check #13
        push    ebx
        push    30806
        push    13
        call    check
        add     sp, 12
        mov     ax, 1               ; eax = 0x12340001 (no toca los 2 bytes altos)
        ldh     efx, 4660
        ldl     efx, 1
        ; check #14
        push    eax
        push    efx
        push    14
        call    check
        add     sp, 12
        mov     al, 255             ; eax = 0x123400FF
        ldl     efx, 255
        ; check #15
        push    eax
        push    efx
        push    15
        call    check
        add     sp, 12
        add     al, 1               ; 0xFF + 1 = 0x100 -> se trunca a 0x00, sin acarreo hacia AH
        ldl     efx, 0
        ; check #16
        push    eax
        push    efx
        push    16
        call    check
        add     sp, 12
        mov     ah, 127             ; eax = 0x12347F00
        ldl     efx, 32512
        ; check #17
        push    eax
        push    efx
        push    17
        call    check
        add     sp, 12
        mov     eex, 0
        mov     ex, -1              ; eex = 0x0000FFFF
        ldh     efx, 0
        ldl     efx, -1
        ; check #18
        push    eex
        push    efx
        push    18
        call    check
        add     sp, 12
        mov     eex, -1
        mov     el, 0               ; eex = 0xFFFFFF00
        ldh     efx, -1
        ldl     efx, -256
        ; check #19
        push    eex
        push    efx
        push    19
        call    check
        add     sp, 12

        push    t_c
        call    puts
        add     sp, 4
        ldh     eax, 4660
        ldl     eax, 22136          ; 0x12345678
        ldh     ebx, -21555         ; 0xABCD
        ldl     ebx, 1              ; 0xABCD0001
        mov     ecx, eax
        ldl     ecx, ebx            ; ecx = 0x12340001
        ldh     efx, 4660
        ldl     efx, 1
        ; check #20
        push    ecx
        push    efx
        push    20
        call    check
        add     sp, 12
        mov     ecx, eax
        ldh     ecx, ebx            ; los 2 bytes altos de ecx = los 2 bytes bajos de ebx -> 0x00015678
        ldh     efx, 1
        ldl     efx, 22136
        ; check #21
        push    ecx
        push    efx
        push    21
        call    check
        add     sp, 12
        mov     ecx, 0
        ldh     ecx, 7              ; 0x00070000
        ldl     ecx, 9              ; 0x00070009
        ldh     efx, 7
        ldl     efx, 9
        ; check #22
        push    ecx
        push    efx
        push    22
        call    check
        add     sp, 12

        push    t_d
        call    puts
        add     sp, 4
        mov     ebx, ds
        add     ebx, v0             ; ebx = &v0
        ldh     eax, 4660
        ldl     eax, 22136          ; 0x12345678
        mov     [v0], eax
        ; check #30
        push    b[ebx]      ; 0x12  <- byte mas significativo en la direccion mas baja
        push    18
        push    30
        call    check
        add     sp, 12
        ; check #31
        push    b[ebx+1]    ; 0x34
        push    52
        push    31
        call    check
        add     sp, 12
        ; check #32
        push    b[ebx+2]    ; 0x56
        push    86
        push    32
        call    check
        add     sp, 12
        ; check #33
        push    b[ebx+3]   ; 0x78
        push    120
        push    33
        call    check
        add     sp, 12
        ; check #34
        push    w[ebx]    ; 0x1234
        push    4660
        push    34
        call    check
        add     sp, 12
        ; check #35
        push    w[ebx+2] ; 0x5678
        push    22136
        push    35
        call    check
        add     sp, 12
        mov     b[ebx+1], 127       ; 0x127F5678
        ldh     efx, 4735
        ldl     efx, 22136
        ; check #36
        push    [v0]
        push    efx
        push    36
        call    check
        add     sp, 12
        mov     w[ebx+2], 0         ; 0x127F0000
        ldl     efx, 0
        ; check #37
        push    l[v0]
        push    efx
        push    37
        call    check
        add     sp, 12
        mov     w[ebx], -1          ; 0xFFFF0000
        ; check #38 (esperado -65536)
        ldh     efx, -1
        ldl     efx, 0
        push    [v0]
        push    efx
        push    38
        call    check
        add     sp, 12
        mov     eax, w[ebx]         ; w[] = 0xFFFF
        push    s_ext
        call    puts
        add     sp, 4
        push    eax
        call    print_int
        add     sp, 4
        push    s_ext2
        call    puts
        add     sp, 4
        ; ejemplo del enunciado: MOV w[2], l[10] copia los 2 bytes bajos del valor de 4 bytes
        ldh     eax, 4660
        ldl     eax, 22136
        mov     [v1], eax           ; 12 34 56 78
        mov     [v2], 0
        mov     w[v2], l[v1]        ; v2 = 56 78 00 00
        ; check #39
        push    w[v2]
        push    22136
        push    39
        call    check
        add     sp, 12
        mov     [v3], 0
        mov     b[v3], w[v1]        ; solo el byte bajo de 0x1234 -> 0x34
        mov     ebx, ds
        add     ebx, v3
        ; check #40
        push    b[ebx]
        push    52
        push    40
        call    check
        add     sp, 12
        ; check #41
        push    b[ebx+1]
        push    0
        push    41
        call    check
        add     sp, 12
        ; aritmetica de punteros
        mov     [v4], 111
        mov     [v5], 222
        mov     ebx, ds
        add     ebx, v4
        ; check #42
        push    [ebx+4]
        push    222
        push    42
        call    check
        add     sp, 12
        add     ebx, 8
        ; check #43
        push    [ebx-4]
        push    222
        push    43
        call    check
        add     sp, 12
        mov     [ebx], 333          ; v6
        ; check #44
        push    [v6]
        push    333
        push    44
        call    check
        add     sp, 12
        mov     ecx, ebx
        sub     ecx, ebx
        mov     [ds+v7], 444        ; [DS+desplazamiento] equivale a [desplazamiento]
        ; check #45
        push    [v7]
        push    444
        push    45
        call    check
        add     sp, 12

        push    t_e
        call    puts
        add     sp, 4
        ldh     eax, 4660           ; 0x1234
        ldl     eax, -21555         ; 0xABCD  -> EAX = 0x1234ABCD
        mov     ebx, sp
        push    ax                  ; se expande a 4 bytes con signo: 0xFFFFABCD
        mov     ecx, sp
        sub     ebx, ecx
        ; check #50
        push    ebx          ; PUSH siempre mueve SP 4 bytes
        push    4
        push    50
        call    check
        add     sp, 12
        ; check #51
        push    l[sp]   ; 0xFFFFABCD
        push    -21555
        push    51
        call    check
        add     sp, 12
        mov     ebx, sp
        mov     [v8], 0
        pop     w[v8]               ; guarda 0xABCD en los 2 bytes de v8
        ldh     efx, -21555
        ldl     efx, 0
        ; check #52
        push    l[v8]      ; 0xABCD0000
        push    efx
        push    52
        call    check
        add     sp, 12
        mov     ecx, sp
        sub     ecx, ebx
        ; check #53
        push    ecx          ; POP siempre mueve SP 4 bytes
        push    4
        push    53
        call    check
        add     sp, 12
        push    ax
        pop     ebx                 ; 0xFFFFABCD
        ; check #54
        push    ebx
        push    -21555
        push    54
        call    check
        add     sp, 12
        push    eax
        ; check #55
        push    b[sp]       ; 0x12
        push    18
        push    55
        call    check
        add     sp, 12
        pop     ecx
        push    -1
        push    5
        pop     eax
        pop     ebx
        ; check #57
        push    eax
        push    5
        push    57
        call    check
        add     sp, 12
        ; check #58
        push    ebx
        push    -1
        push    58
        call    check
        add     sp, 12
        mov     [v9], 99
        push    [v9]                ; PUSH con operando de memoria
        pop     [v10]               ; POP a memoria
        ; check #59
        push    [v10]
        push    99
        push    59
        call    check
        add     sp, 12

        push    t_f
        call    puts
        add     sp, 4
        mov     ebx, ks
        add     ebx, t_str
        ; check #60
        push    b[ebx]      ; 'A'
        push    65
        push    60
        call    check
        add     sp, 12
        ; check #61
        push    b[ebx+1]    ; 'B'
        push    66
        push    61
        call    check
        add     sp, 12
        ; check #62
        push    b[ebx+2]    ; 'C'
        push    67
        push    62
        call    check
        add     sp, 12
        ; check #63
        push    b[ebx+3]     ; '\0'
        push    0
        push    63
        call    check
        add     sp, 12

        call    summary
        mov     sp, bp
        pop     bp
        stop
