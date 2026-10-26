; TEST 06 - PUSH, POP, SP, BP y operando [BP-n]
\\DATA 64
\\STACK 64
main: MOV EEX, SP
 MOV EFX, SP
 PUSH 10
 PUSH 20
 PUSH 30
 SUB EEX, SP
 MOV [0], EEX       ; SP bajo 12
 POP [4]            ; 30
 POP [8]            ; 20
 POP [12]           ; 10
 SUB EFX, SP
 MOV [16], EFX      ; SP volvio al inicio: 0
 MOV EAX, 0
 Sys 15
 MOV AX, 0x1234
 PUSH AX            ; se apila expandido a 4 bytes
 POP EBX
 MOV [20], EBX      ; 0x1234 = 4660
 MOV [40], 99
 PUSH [40]          ; PUSH con operando de memoria
 POP [24]           ; POP a memoria -> 99
 MOV BP, SP
 PUSH 111
 PUSH 222
 MOV EBX, [BP-4]    ; primer push
 MOV [28], EBX      ; 111
 MOV EBX, [BP-8]    ; segundo push
 MOV [32], EBX      ; 222
 POP EBX
 POP EBX
 SUB BP, SP
 MOV [36], BP       ; pila balanceada: 0
 MOV EAX, 0x01
 MOV EDX, DS
 LDL ECX, 10        ; 10 valores
 LDH ECX, 4         ; de 4 bytes
 SYS 0x02
 STOP