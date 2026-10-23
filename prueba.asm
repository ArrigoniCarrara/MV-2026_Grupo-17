\\DATA 8                 ; [0] = x (leído), [4] = resultado
\\STACK 256
\\EXTRA 20

NULL EQU -1
texto EQU "JUAN"

main:   ; leer x (decimal, 1 valor de 4 bytes) en [0]
        mov eax, 0x01
        mov edx, ds
        ldh ecx, 4
        ldl ecx, 1
        sys 0x1

        push [0]
        call f
        add sp, 4
        mov [4], eax

        ; mostrar el resultado
        mov eax, 0x01
        mov edx, ds
        add edx, 4
        ldh ecx, 4
        ldl ecx, 1
        sys 0x2
        sys 0
        stop
        

; f(x) = cubo(x) + (cuadrado(x) + doble(x))
; parámetro: x en [bp+8]. Devuelve en EAX.
f:      push bp
        mov bp, sp
        sub sp, 12              ; [bp-4]=cubo, [bp-8]=cuadrado, [bp-12]=doble

        push [bp+8]
        call cubo
        add sp, 4
        mov [bp-4], eax

        push [bp+8]
        call cuadrado
        add sp, 4
        mov [bp-8], eax

        push [bp+8]
        call doble
        add sp, 4
        mov [bp-12], eax

        push [bp-8]             ; a = cuadrado
        push [bp-12]            ; b = doble
        call suma
        add sp, 8               ; eax = cuadrado + doble

        push [bp-4]             ; a = cubo
        push eax                ; b = (cuadrado + doble)
        call suma
        add sp, 8               ; eax = resultado final

        mov sp, bp              ; libero los locales
        pop bp
        ret

; suma(a, b): a en [bp+12], b en [bp+8]
suma:   push bp
        mov bp, sp
        mov eax, [bp+12]
        add eax, [bp+8]
        SYS 15
        mov sp, bp
        pop bp
        ret

; doble(x) = suma(x, x)
doble:  push bp
        mov bp, sp
        push [bp+8]
        push [bp+8]
        call suma
        add sp, 8
        mov sp, bp
        pop bp
        ret

; cuadrado(x) = x * x
cuadrado: push bp
          mov bp, sp
          mov eax, [bp+8]
          mul eax, [bp+8]
          mov sp, bp
          pop bp
          ret

; cubo(x) = cuadrado(x) * x
cubo:   push bp
        mov bp, sp
        push [bp+8]
        call cuadrado
        add sp, 4               ; eax = x^2
        mul eax, [bp+8]         ; eax = x^3
        mov sp, bp
        pop bp
        ret