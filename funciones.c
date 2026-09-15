#include "main.c"

void buscooperacion(uint32_t registros[0]){
        if((RAM[registros[0]] >> 4 & 0x01) == 1){
            dosOperandos();
        }else if(RAM[registros[0]] >> 6 != 0){
            unOperando();
        }else
             ningunoperando();
}