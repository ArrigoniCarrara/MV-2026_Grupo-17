\\EXTRA 1024
null        equ     -1
size_es     equ     1024
; inicializar HEAP
; sin parámetros
; asume que todo el extra segment es para uso de memoria dinámica    
heap_init:  mov [es], es
            add [es], 4
            ret
; invocacion: 
; call heap_init

; solicita memoria dinámica 
; parámetros: +8 cantidad de bytes a solicitar
; devuelve en EAX dirección de memoria o NULL si no hay memoria suficiente
alloc:      push    bp
            mov     bp, sp
            push    bx
            mov     eax, null
            mov     bx, [es]
            add     bx, [bp+8]
            cmp     bx, size_es
            jp      alloc_end
            mov     eax, [es]
            add     [es], [bp+8]
alloc_end:  pop     bx
            mov     sp, bp  
            pop     bp
            ret
; invocacion: 
; push <cantidad de bytes a solicitar>
; call alloc
; add  sp, 4
; << eax dirección de memoria