; PRUEBA 8A - registros de proposito general y pseudonimos (EAX..EFX, AL/AH/AX, EL/EH/EX, FX)
; Traducir: vmt t8a_generales.asm t8a.vmx v=2
; Ejecutar: vmx t8a.vmx
; Salida: 9 celdas de 4 bytes en hexadecimal. Las direcciones [XXXX] dependen del tamaño
; del codigo (DS arranca justo despues del codigo); lo que importa son los valores:
;   celda 0  0x12347F01   EAX final
;   celda 1  0x12345678   EBX (copia de EAX original)
;   celda 2  0xFFFFFF99   ECX = AL extendido con signo
;   celda 3  0x00000056   EDX = AH
;   celda 4  0xFFFF8001   EEX = AX extendido con signo
;   celda 5  0x12347F01   EFX (copia de EAX)
;   celda 6  0xFFFF0000   EDX tras DL=0 y DH=0
;   celda 7  0xFFFF3412   EEX tras EL=0x12 y EH=0x34
;   celda 8  0x0000FFFF   EFX tras FX=-1
\\DATA 64
\\STACK 64

main:   mov eax, 0
        ldl eax, 0x5678
        ldh eax, 0x1234         ; EAX = 0x12345678
        mov ebx, eax            ; EBX = 0x12345678
        mov al, 0x99            ; EAX = 0x12345699
        mov ecx, al             ; ECX = 0xFFFFFF99
        mov edx, ah             ; AH = 0x56 -> EDX = 0x00000056
        mov ax, 0x8001          ; EAX = 0x12348001
        mov eex, ax             ; EEX = 0xFFFF8001
        mov ah, 0x7F            ; EAX = 0x12347F01
        mov efx, eax            ; EFX = 0x12347F01

        mov [0], eax
        mov [4], ebx
        mov [8], ecx
        mov [12], edx
        mov [16], eex
        mov [20], efx

        mov edx, -1
        mov dl, 0
        mov dh, 0               ; EDX = 0xFFFF0000
        mov [24], edx
        mov eex, -1
        mov el, 0x12
        mov eh, 0x34            ; EEX = 0xFFFF3412
        mov [28], eex
        mov efx, 0
        mov fx, -1              ; EFX = 0x0000FFFF
        mov [32], efx

        mov edx, DS
        ldh ecx, 4
        ldl ecx, 9
        mov eax, 0x08           ; hexadecimal
        sys 2
        ret
