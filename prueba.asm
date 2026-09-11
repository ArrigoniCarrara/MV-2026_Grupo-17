mov edx, DS
add edx, 4
mov eax, 0b01
ldh ecx, 4
ldl ecx, 1
sys 1
mov eax, [edx] ; hola
stop 