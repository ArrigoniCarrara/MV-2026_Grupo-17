; =====================================================================
; util.asm - biblioteca de apoyo para los tests (NO es un test)
; =====================================================================
; Usa los bytes 0..63 del Data Segment como zona de trabajo.
; Los tests deben usar el Data Segment a partir del offset 64 ("usr").
;
; Subrutinas (todas preservan los registros que modifican, salvo EAX en check):
;   util_init                      pone en cero los contadores de check
;   puts(str)                      imprime un string constante (offset en KS)
;   putc(c)                        imprime un caracter
;   print_int(n)                   imprime n en decimal con signo (sin salto de linea)
;   print_hex(n)                   imprime n como 0x + 8 digitos hexadecimales
;   check(id, esperado, obtenido)  si difieren imprime "FALLA #id esperado=.. obtenido=.."
;   summary                        imprime "RESULTADO: aciertos/total OK"
;   pflags                         imprime "NZCV=xxxx" con los flags del CC al momento del call
;                                  (NO toca CC antes de capturarlo: solo usa LDL/LDH y saltos)
;   pjumps                         imprime NZCV y que saltos condicionales se toman
; Convencion de llamada: parametros por pila (el ultimo se pushea primero),
; el que llama limpia la pila con "add sp, 4*cantidad".
; =====================================================================
scr         equ 0           ; buffer de impresion (bytes 0..31)
chk_pass    equ 32
chk_total   equ 36
usr         equ 64          ; primer byte del Data Segment libre para los tests
; variables de 4 bytes para los tests: v0=DS+64, v1=DS+68, ... v31=DS+188
v0   equ 64
v1   equ 68
v2   equ 72
v3   equ 76
v4   equ 80
v5   equ 84
v6   equ 88
v7   equ 92
v8   equ 96
v9   equ 100
v10  equ 104
v11  equ 108
v12  equ 112
v13  equ 116
v14  equ 120
v15  equ 124
v16  equ 128
v17  equ 132
v18  equ 136
v19  equ 140
v20  equ 144
v21  equ 148
v22  equ 152
v23  equ 156
v24  equ 160
v25  equ 164
v26  equ 168
v27  equ 172
v28  equ 176
v29  equ 180
v30  equ 184
v31  equ 188

u_nl        equ "\n"
u_nzcv      equ "NZCV="
u_jp        equ " JP="
u_jnp       equ " JNP="
u_jnn       equ " JNN="
u_jnz       equ " JNZ="
u_fail      equ "FALLA #"
u_esp       equ " esperado="
u_obt       equ " obtenido="
u_res       equ "RESULTADO: "
u_barra     equ "/"
u_ok        equ " OK\n"
u_mal       equ " <<<<<< HAY ERRORES\n"
u_minint    equ "-2147483648"
u_0x        equ "0x"

util_init:  mov     [chk_pass], 0
            mov     [chk_total], 0
            ret

;----------------------------------------
; puts(+8 offset del string en KS)
;----------------------------------------
puts:       push    bp
            mov     bp, sp
            push    edx
            mov     edx, ks
            add     edx, [bp+8]
            sys     4
            pop     edx
            mov     sp, bp
            pop     bp
            ret

;----------------------------------------
; putc(+8 caracter)
;----------------------------------------
putc:       push    bp
            mov     bp, sp
            push    eax
            push    edx
            mov     eax, [bp+8]
            mov     b[scr], al
            mov     b[1], 0
            mov     edx, ds
            add     edx, scr
            sys     4
            pop     edx
            pop     eax
            mov     sp, bp
            pop     bp
            ret

;----------------------------------------
; print_int(+8 n)  decimal con signo
;----------------------------------------
print_int:  push    bp
            mov     bp, sp
            push    eax
            push    ebx
            push    edx
            mov     eax, [bp+8]
            ldh     ebx, -32768     ; ebx = 0x80000000 (minimo entero)
            ldl     ebx, 0
            cmp     eax, ebx
            jz      pi_min
            mov     edx, ds
            add     edx, 31         ; el string termina en DS+31
            mov     b[edx], 0
            mov     ebx, 0          ; ebx = 1 si el numero es negativo
            cmp     eax, 0
            jnn     pi_loop
            mov     ebx, 1
            mul     eax, -1
pi_loop:    sub     edx, 1
            div     eax, 10         ; eax = cociente, ac = resto
            add     ac, 48          ; '0' + resto
            mov     b[edx], ac
            cmp     eax, 0
            jnz     pi_loop
            cmp     ebx, 0
            jz      pi_print
            sub     edx, 1
            mov     b[edx], 45      ; '-'
pi_print:   sys     4
            jmp     pi_end
pi_min:     push    u_minint
            call    puts
            add     sp, 4
pi_end:     pop     edx
            pop     ebx
            pop     eax
            mov     sp, bp
            pop     bp
            ret

;----------------------------------------
; check(+8 id, +12 esperado, +16 obtenido)
;----------------------------------------
check:      push    bp
            mov     bp, sp
            push    eax
            push    ebx
            add     [chk_total], 1
            mov     eax, [bp+12]
            mov     ebx, [bp+16]
            cmp     eax, ebx
            jnz     ck_fail
            add     [chk_pass], 1
            jmp     ck_fin
ck_fail:    push    u_fail
            call    puts
            add     sp, 4
            push    [bp+8]
            call    print_int
            add     sp, 4
            push    u_esp
            call    puts
            add     sp, 4
            push    eax
            call    print_int
            add     sp, 4
            push    u_obt
            call    puts
            add     sp, 4
            push    ebx
            call    print_int
            add     sp, 4
            push    u_nl
            call    puts
            add     sp, 4
ck_fin:     pop     ebx
            pop     eax
            mov     sp, bp
            pop     bp
            ret

;----------------------------------------
; summary: "RESULTADO: aciertos/total OK"
;----------------------------------------
summary:    push    eax
            push    u_res
            call    puts
            add     sp, 4
            push    [chk_pass]
            call    print_int
            add     sp, 4
            push    u_barra
            call    puts
            add     sp, 4
            push    [chk_total]
            call    print_int
            add     sp, 4
            mov     eax, [chk_pass]
            cmp     eax, [chk_total]
            jz      sm_ok
            push    u_mal
            call    puts
            add     sp, 4
            jmp     sm_fin
sm_ok:      push    u_ok
            call    puts
            add     sp, 4
sm_fin:     pop     eax
            ret

;----------------------------------------
; pflags: imprime "NZCV=" y los 4 flags (0/1) seguido de salto de linea.
; Hay que llamarla justo despues de la instruccion a observar.
; Captura los flags SOLO con LDL/LDH y saltos (que no modifican CC).
;----------------------------------------
pflags:     push    eax
            push    ebx
            push    ecx
            push    edx
            ldh     eax, 0
            ldl     eax, 48         ; '0'  (N)
            ldh     ebx, 0
            ldl     ebx, 48         ; '0'  (Z)
            ldh     ecx, 0
            ldl     ecx, 48         ; '0'  (C)
            ldh     edx, 0
            ldl     edx, 48         ; '0'  (V)
            jnn     pf_n            ; N=0: dejar '0'
            ldl     eax, 49
pf_n:       jnz     pf_z            ; Z=0: dejar '0'
            ldl     ebx, 49
pf_z:       jc      pf_c1
            jmp     pf_c
pf_c1:      ldl     ecx, 49
pf_c:       jv      pf_v1
            jmp     pf_v
pf_v1:      ldl     edx, 49
pf_v:       push    u_nzcv
            call    puts
            add     sp, 4
            mov     b[scr], al
            mov     b[1], bl
            mov     b[2], cl
            mov     b[3], dl
            mov     b[4], 0
            mov     edx, ds
            sys     4
            push    u_nl
            call    puts
            add     sp, 4
            pop     edx
            pop     ecx
            pop     ebx
            pop     eax
            ret

;----------------------------------------
; pjumps: imprime NZCV y si se toman JP, JNP, JNN, JNZ (1 = salta).
; (JN, JZ, JC y JV coinciden con los flags N, Z, C, V.)
;----------------------------------------
pjumps:     push    eax
            push    ebx
            push    ecx
            push    edx
            push    efx
            ldh     eax, 0
            ldl     eax, 48
            ldh     ebx, 0
            ldl     ebx, 48
            ldh     ecx, 0
            ldl     ecx, 48
            ldh     edx, 0
            ldl     edx, 48
            jp      pj_p1           ; eax = JP
            jmp     pj_p
pj_p1:      ldl     eax, 49
pj_p:       jnp     pj_np1          ; ebx = JNP
            jmp     pj_np
pj_np1:     ldl     ebx, 49
pj_np:      jnn     pj_nn1          ; ecx = JNN
            jmp     pj_nn
pj_nn1:     ldl     ecx, 49
pj_nn:      jnz     pj_nz1          ; edx = JNZ
            jmp     pj_nz
pj_nz1:     ldl     edx, 49
pj_nz:      push    eax             ; guardo los 4 resultados en la pila
            push    ebx
            push    ecx
            push    edx
            ldh     eax, 0
            ldl     eax, 48
            ldh     ebx, 0
            ldl     ebx, 48
            ldh     ecx, 0
            ldl     ecx, 48
            ldh     edx, 0
            ldl     edx, 48
            jnn     pj_n            ; N
            ldl     eax, 49
pj_n:       jnz     pj_z            ; Z
            ldl     ebx, 49
pj_z:       jc      pj_c1
            jmp     pj_c
pj_c1:      ldl     ecx, 49
pj_c:       jv      pj_v1
            jmp     pj_v
pj_v1:      ldl     edx, 49
pj_v:       push    u_nzcv          ; ---- recien ahora imprimo ----
            call    puts
            add     sp, 4
            mov     b[scr], al
            mov     b[1], bl
            mov     b[2], cl
            mov     b[3], dl
            mov     b[4], 0
            mov     edx, ds
            sys     4
            pop     edx             ; recupero JNZ, JNN, JNP, JP
            pop     ecx
            pop     ebx
            pop     eax
            mov     efx, edx        ; JNZ queda en efx (edx se usa como puntero)
            push    u_jp
            call    puts
            add     sp, 4
            mov     b[scr], al
            mov     b[1], 0
            mov     edx, ds
            sys     4
            push    u_jnp
            call    puts
            add     sp, 4
            mov     b[scr], bl
            sys     4
            push    u_jnn
            call    puts
            add     sp, 4
            mov     b[scr], cl
            sys     4
            push    u_jnz
            call    puts
            add     sp, 4
            mov     b[scr], fl
            sys     4
            push    u_nl
            call    puts
            add     sp, 4
            pop     efx
            pop     edx
            pop     ecx
            pop     ebx
            pop     eax
            ret

;----------------------------------------
; print_hex(+8 n): imprime "0x" y 8 digitos hexadecimales (mayusculas)
;----------------------------------------
print_hex:  push    bp
            mov     bp, sp
            push    eax
            push    ebx
            push    ecx
            push    u_0x
            call    puts
            add     sp, 4
            mov     eax, [bp+8]
            mov     ecx, 28
ph_loop:    mov     ebx, eax
            shr     ebx, ecx
            and     ebx, 15
            cmp     ebx, 10
            jn      ph_dig
            add     ebx, 55         ; 'A' - 10
            jmp     ph_out
ph_dig:     add     ebx, 48
ph_out:     push    ebx
            call    putc
            add     sp, 4
            sub     ecx, 4
            cmp     ecx, 0
            jnn     ph_loop
            pop     ecx
            pop     ebx
            pop     eax
            mov     sp, bp
            pop     bp
            ret
