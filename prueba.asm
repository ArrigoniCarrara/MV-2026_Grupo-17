         MOV EAX, 12345      ; número a analizar
        MOV ECX, 0          ; contador de dígitos
cuenta: CMP EAX, 0
        JZ fin
        DIV EAX, 10         ; EAX = EAX/10, resto queda en AC
        ADD ECX, 1
        JMP cuenta
fin:    STOP