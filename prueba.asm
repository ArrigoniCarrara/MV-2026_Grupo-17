; TEST 21 - AND/OR/XOR/NOT/SHL/SHR/SAR/SWAP/CMP + saltos condicionales sobre alias de registro
\\DATA 16
main: MOV EAX, 0
 LDH EAX, 0x00F0
 LDL EAX, 0x0F0F
 SYS 0x0F               ; CP1
 AND AL, 0x3C
 OR AH, 0x30
 XOR AX, 0x00FF
 SYS 0x0F               ; CP2
 NOT AL
 SYS 0x0F               ; CP3
 NOT AH
 SYS 0x0F               ; CP4
 NOT AX
 SYS 0x0F               ; CP5
 SHL AL, 2
 SYS 0x0F               ; CP6
 SHR AL, 4
 SYS 0x0F               ; CP7
 SAR AX, 4
 SYS 0x0F               ; CP8
 SAR AH, 1
 SYS 0x0F               ; CP9
 MOV AL, -100
 SAR AL, 2
 SYS 0x0F               ; CP10
 MOV AH, -128
 SAR AH, 7
 SYS 0x0F               ; CP11
 SHR AH, 4
 SYS 0x0F               ; CP12
 MOV AX, -4096
 SAR AX, 4
 SYS 0x0F               ; CP13
 SHR AX, 4
 SYS 0x0F               ; CP14
 MOV AX, 0x0FE7
 SWAP AL, AH
 SYS 0x0F               ; CP15
 MOV EBX, 0
 LDH EBX, 0xAAAA
 LDL EBX, 0x1234
 SWAP AX, BX
 SYS 0x0F               ; CP16
 SWAP EAX, EBX
 SYS 0x0F               ; CP17
 MOV ECX, 0
 MOV EDX, 0
 MOV CL, 10
 CMP CL, 10
 JZ l1
 ADD EDX, 100
l1: ADD EDX, 1
 CMP CL, 20
 JN l2
 ADD EDX, 100
l2: ADD EDX, 2
 CMP CL, 5
 JP l3
 ADD EDX, 100
l3: ADD EDX, 4
 CMP CH, 0
 JNP l4
 ADD EDX, 100
l4: ADD EDX, 8
 CMP CL, 10
 JNN l5
 ADD EDX, 100
l5: ADD EDX, 16
 CMP CL, 11
 JNZ l6
 ADD EDX, 100
l6: ADD EDX, 32
 SYS 0x0F               ; CP18
 STOP