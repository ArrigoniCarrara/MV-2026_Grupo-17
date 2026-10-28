; TEST 22 - Accesos b[] w[] l[] [] a memoria: lecturas con signo, escrituras truncadas, operaciones con destino en memoria
\\DATA 64
main: MOV l[8], 0
 MOV EAX, 0
 LDH EAX, 0x1122
 LDL EAX, 0x3344
 MOV l[0], EAX
 MOV EBX, 0
 LDH EBX, 0x8899
 LDL EBX, 0xAABB
 MOV l[4], EBX
 SYS 0x0F               ; CP1
 MOV ECX, b[4]
 MOV EDX, b[0]
 MOV EEX, w[4]
 MOV EFX, w[2]
 SYS 0x0F               ; CP2
 MOV EAX, l[1]
 MOV EBX, [5]
 SYS 0x0F               ; CP3
 MOV b[0], 0x7F
 MOV w[1], -2
 MOV l[9], -3
 MOV b[13], AH
 MOV w[14], AX
 SYS 0x0F               ; CP4
 ADD b[0], -1
 SUB w[1], 1
 ADD l[9], 3
 SYS 0x0F               ; CP5
 MUL b[13], -1
 DIV w[14], 4
 SYS 0x0F               ; CP6
 MOV b[4], 0x21
 SHL b[4], 1
 SYS 0x0F               ; CP7
 NOT b[8]
 SYS 0x0F               ; CP8
 OR b[10], 0x55
 SYS 0x0F               ; CP9
 SAR w[1], 1
 SYS 0x0F               ; CP10
 SHR w[14], 4
 SYS 0x0F               ; CP11
 ADD w[2], AX
 SUB b[13], AL
 SYS 0x0F               ; CP12
 MOV EDX, DS
 ADD EDX, 16
 MOV l[EDX], EAX
 MOV BL, b[EDX+2]
 MOV BH, b[EDX]
 MOV BX, w[EDX+1]
 SYS 0x0F               ; CP13
 STOP