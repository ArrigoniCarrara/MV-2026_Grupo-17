; PRUEBA 8D - registros internos del procesador: OPC, OP1, OP2, LAR, MAR, MBR
; Traducir: vmt t8d_internos.asm t8d.vmx v=2
; Ejecutar: vmx t8d.vmx -d     (el -d te muestra que el codigo mide 73 bytes = 0x49)
;
; Como leer un registro requiere ejecutar un MOV, OPC/OP1/OP2 muestran los de ESE MOV.
; Salida: 6 celdas en hexadecimal (DS arranca en 0x0049, la primera celda es DS+16 = [0059]):
;   [0059]  0x00000010   OPC  = 0x10 (codigo de MOV)
;   [005D]  0x0100000B   OP1  = tipo registro (01) + codigo de EBX (0x0B)
;   [0061]  0x01000003   OP2  = tipo registro (01) + codigo de OP2 (0x03)
;   [0065]  0x00010008   LAR  = segmento 1 (DS), offset 8
;   [0069]  0x00040051   MAR  = 4 bytes en la parte alta + direccion fisica 0x49 + 8 = 0x51
;   [006D]  0x00001234   MBR  = valor escrito por "mov [8], 0x1234"
; Ademas, entre el "mov [8], 0x1234" y las lecturas hay varias instrucciones: la lectura de
; instrucciones NO debe modificar LAR, MAR ni MBR.
; Si cambias el codigo, MAR bajo = (tamaño del codigo) + 8.
\\DATA 64
\\STACK 32

main:   mov [8], 0x1234         ; unico acceso a memoria antes de leer LAR/MAR/MBR
        mov eax, opc
        mov ebx, op1
        mov ecx, op2
        mov edx, lar
        mov eex, mar
        mov efx, mbr

        mov [16], eax
        mov [20], ebx
        mov [24], ecx
        mov [28], edx
        mov [32], eex
        mov [36], efx

        mov edx, DS
        ldh ecx, 4
        ldl ecx, 6
        mov eax, 0x08
        sys 2
        ret
