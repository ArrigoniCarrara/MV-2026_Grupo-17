mov ebx, 100;
mov edx, [2]; en el registro EDX, voy a leer desde la celda 2 hasta la celda 5, o sea 4 bytes
mov edx, eax;
mov [2], [6]; lo que hay en la celda 6 pasa a la celda 2, un byte
mov [2], 100; en la celda 2 (relativa a ds) poneme el 100 (que ocupa 2 byte), vas a escribir dos bytes;
stop 