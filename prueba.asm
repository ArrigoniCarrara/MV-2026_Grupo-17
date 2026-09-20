MOV [10], 50     ; Almacena el valor inmediato 50 en la dirección lógica 10
MOV EDX, 5       ; Carga el valor 5 en el registro EDX
MUL [10], EDX    ; Multiplica el valor de la memoria por EDX (50 * 5 = 250)
SUB [10], 50     ; Resta 50 directamente al valor almacenado en la memoria (200)
MOV EAX, [10]    ; Extrae el resultado final de la memoria hacia el registro EAX
MOV [15], [10]   ; preguntar a nachi que onda con esto
MOV EAX, -20
STOP             ; Termina la ejecución