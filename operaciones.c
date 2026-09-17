#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

//Debo calcular direcciones logicas y fisicas cuando se haga una operacion en memoria
  
int buscaDireccionFisica( uint32_t valorOp ){
    uint8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
    int offset = valorOp >> 8 ;
    if ( codReg != DS ){
        offset += registros[codReg];
    }
    registros[LAR] = 0x00010000;
    registros[LAR] += offset;// lo que pone literalmente en el pdf
    // MAR = la cantidad de bytes que vamos a leer, nos pasamos a esta funcion el valor del opA y opB
    int direFisica = tabla_seg[1].base + offset;
    int aux = registros[MAR] >> 16;

    if ( (tabla_seg[1].tam + tabla_seg[1].base < direFisica + aux) || (direFisica < tabla_seg[1].tam) ){
        printf( "Te fuiste del segmento capo" );
        return -1;
    }
    else{
        return direFisica;
    }

    

}

void MOV( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){
    if ( opA == 0x01 && (opB == 0b01  || opB == 0b10) ){ // si donde voy a guardar es un dato y lo que guardo es un registro o inmediato entro:
        uint8_t nroRegistro = opA & 0b00011111;         
        if ( opB == 0x02 )
            registros[ nroRegistro ] = valorB;
        else{// el dato que voy a guardar viene en un registro
            // en valorB deberia venir el nro del registro?? preguntar a gian
            uint8_t nroRegistroOpB = valorB & 0x0000001F; // valorB al tener 32 bits y como solo me importa el primer byte ya que le debo sacar los 3 bits mas significativos aplico una mascara
            registros[ nroRegistro ] = nroRegistroOpB;
        }
    }
    else{ // es una operacion en memoria
        if ( opB == 0b11 ){
            // voy a buscar donde esta opB            

        }
        else{
            // voy a buscar donde esta opA

        }
            

    }
    


}

void ADD( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SUB( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void MUL( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void DIV ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void CMP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void AND( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void OR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void XOR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SWAP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SWAP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SHL( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SHR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SAR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void LDL( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void LDH( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void RND( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void ( *operacionesDosOperanDosOperando[ 16 ] )(unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ) 
= { MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };