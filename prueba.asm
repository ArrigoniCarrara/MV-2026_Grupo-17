; TEST 05 - SYS 3 (STRING READ) con limite y sin limite, SYS 4
\\DATA 64
P1  EQU "Texto 1: "
P2  EQU "Texto 2 (max 3): "
P3  EQU "Texto 3: "
OUT EQU "Leido -> "
NL  EQU "\n"
main: MOV EDX, KS
 ADD EDX, P1
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 10
 MOV ECX, 50        ; lectura 1: hasta 50 caracteres en DS+10
 SYS 0x03
 MOV EDX, KS
 ADD EDX, OUT
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 10
 SYS 0x04
 MOV EDX, KS
 ADD EDX, NL
 SYS 0x04
 MOV EDX, KS
 ADD EDX, P2
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 20
 MOV ECX, 3         ; lectura 2: maximo 3 caracteres en DS+20
 SYS 0x03
 MOV EDX, KS
 ADD EDX, OUT
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 20
 SYS 0x04
 MOV EDX, KS
 ADD EDX, NL
 SYS 0x04
 MOV EDX, KS
 ADD EDX, P3
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 30
 MOV ECX, -1        ; lectura 3: sin limite en DS+30
 SYS 0x03
 MOV EDX, KS
 ADD EDX, OUT
 SYS 0x04
 MOV EDX, DS
 ADD EDX, 30
 SYS 0x04
 MOV EDX, KS
 ADD EDX, NL
 SYS 0x04
 ; dump hexa de las 4 celdas de la lectura 2 (DS+20 .. DS+23)
 MOV EAX, 0x08
 MOV EDX, DS
 ADD EDX, 20
 LDL ECX, 4
 LDH ECX, 1
 SYS 0x02
 STOP