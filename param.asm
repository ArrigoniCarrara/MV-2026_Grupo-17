basura1 equ "basura"
salto equ "\n"
basura2 equ "basura"

; ESTO NO SE EJECUTA
        stop
        stop
        stop

; ESTO SI SE EJECUTA
main:   sys 0xF
        push bp
        mov bp, sp
        mov eax, [bp+8]
        mov ebx, [bp+12]
otro:   cmp eax, 0
        jz fin
        mov edx, [ebx]
        sys 4
        mov edx, ks
        add edx, salto
        sys 4
        sub eax, 1
        add ebx, 4
        jmp otro

fin:    mov sp, bp
        pop bp
        ret