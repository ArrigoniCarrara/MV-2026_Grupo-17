#include "main.c"

void dosOperandos(uint32_t registros[0]){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        
        tipo_opa = registros[0] >> 6;
        tipo_opb = registros[0] >> 4;
} 


void buscooperacion(uint32_t registros[0]){
        if((RAM[registros[0]] >> 4 & 0x01) == 1){
            dosOperandos(registros[0]);
        }else if(RAM[registros[0]] >> 6 != 0){
            unOperando();
        }else
             ningunoperando();
}