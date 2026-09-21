MOV EAX, 3               ; 0: N = 3
MOV [DS], EAX            ; 1
MOV EAX, 10              ; 2: arreglo[0]
MOV [DS+4], EAX          ; 3
MOV EAX, 5               ; 4: arreglo[1]
MOV [DS+8], EAX          ; 5
MOV EAX, 20              ; 6: arreglo[2]
MOV [DS+12], EAX         ; 7
MOV EAX, 0               ; 8
LDH EAX, 0x7FFF          ; 9
LDL EAX, 0xFFFF          ; 10: EAX = 0x7FFFFFFF (entero maximo)
MOV [DS+4000], EAX       ; 11: minimo provisorio
MOV EBX, 0               ; 12
LDH EBX, 0x8000          ; 13
LDL EBX, 0x0000          ; 14: EBX = 0x80000000 (entero minimo)
MOV [DS+4004], EBX       ; 15: maximo provisorio
MOV EAX, 0               ; 16: suma acumulada
MOV EBX, 0               ; 17: indice i
MOV ECX, [DS]            ; 18: ECX = N
CMP EBX, ECX             ; 19
JNN 36                   ; 20: si i >= N, salta a fin_estadisticas (línea 36)
MOV EEX, EBX             ; 21
SHL EEX, 2               ; 22
MOV EFX, DS              ; 23
ADD EFX, 4               ; 24
ADD EFX, EEX             ; 25: EFX -> direccion del elemento i
MOV EDX, [EFX]           ; 26: EDX = arreglo[i] (¡Corregido: usamos EDX en vez de AC!)
ADD EAX, EDX             ; 27: suma += arreglo[i]
CMP EDX, [DS+4000]       ; 28: compara con minimo
JNN 31                   ; 29: si EDX >= min, salta a no_es_min (línea 31)
MOV [DS+4000], EDX       ; 30: nuevo minimo
CMP EDX, [DS+4004]       ; 31: no_es_min: compara con maximo                 
STOP                     ; 102