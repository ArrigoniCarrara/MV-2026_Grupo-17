#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"
#include <componentes.c>

uint32_t ninguno(uint32_t *ip){return 0x0;} // ERROR

uint32_t registro(uint32_t *ip){
        *ip = *ip + 1;
        return RAM[*ip];
}

uint32_t inmediato(uint32_t *ip){
    *ip = *ip + 1;

    uint32_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

uint32_t memoria(uint32_t *ip){
    *ip = *ip + 1;

    uint32_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}


 typedef uint32_t (*tipo_operando[4])(uint32_t *); // Vector de punteros a funciones para rescatar el valor de cada operando
 tipo_operando p_tipo_op = {ninguno, registro, inmediato, memoria}; // y para mover correctamente el IP

void dosOperandos(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        tipo_opb = (RAM[*ip] >> 6) & 0x03;
        tipo_opa = (RAM[*ip] >> 4) & 0x03;
        cod_op = RAM[*ip] & 0x1F;

        valor_opa = p_tipo_op[tipo_opa](ip);
        valor_opb = p_tipo_op[tipo_opb](ip);

        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 

void unOperando(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        
        tipo_opa = (RAM[*ip] >> 6) & 0x03;
        cod_op = RAM[*ip] & 0x0F;

        valor_opa = p_tipo_op[tipo_opa](ip);
        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 


void ningunOperando(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        cod_op = RAM[*ip] & 0x0F;

        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 



void buscooperacion(uint32_t *ip){
        if((RAM[*ip] >> 4 & 0x01) == 1){
            dosOperandos(ip);
        }else if(RAM[*ip] >> 6 != 0){
            unOperando(ip);
        }else
             ningunOperando(ip);
}