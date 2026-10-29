; TEST 25 - SHR (corrimiento logico) en 8, 16 y 32 bits, con cantidad 0, mayor o igual al ancho, y vecinos intactos
\\DATA 32
main: MOV EAX, 0
 LDH EAX, 0xF000
 SHR EAX, 4
 SYS 0x0F               ; CP1
 MOV EAX, 0
 LDH EAX, 0xF000
 SHR EAX, 0
 SYS 0x0F               ; CP2
 SHR EAX, 40
 SYS 0x0F               ; CP3
 MOV EBX, 0
 LDL EBX, 0x00F0
 SHR BL, 1
 SYS 0x0F               ; CP4
 SHR BL, 3
 SYS 0x0F               ; CP5
 SHR BL, 8
 SYS 0x0F               ; CP6
 MOV b[0], -2
 SHR b[0], 1
 MOV ECX, b[0]
 SYS 0x0F               ; CP7
 MOV l[4], -2
 SHR l[4], 31
 MOV EDX, l[4]
 SYS 0x0F               ; CP8
 MOV w[8], 0x7FFF
 SHR w[8], 16
 MOV EEX, w[8]
 SYS 0x0F               ; CP9
 MOV l[12], -1
 SHR w[12], 4
 MOV EFX, l[12]
 SYS 0x0F               ; CP10
 STOP