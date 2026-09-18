#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

//Debo calcular direcciones logicas y fisicas cuando se haga una operacion en memoria
/*
Cada vez que se realiza una operación en la memoria, se debe cargar en el registro LAR la dirección
lógica a la que se quiere acceder y la cantidad de bytes en la parte alta del registro MAR (los 2 bytes más
significativos). Luego de realizar la traducción a una dirección física, el resultado debe almacenarse en la
parte baja del registro MAR (los 2 bytes menos significativos). En el registro MBR debe quedar el valor con
el cual se está operando, ya sea el valor que se desea almacenar en el caso de una escritura o el que se
obtuvo después de la lectura. La lectura de la instrucción no debe modificar ninguno de estos registros.
*/ 

void escrituraEnMemoria( uint32_t valor, uint16_t cantBytes, uint32_t valorOp ){

    int direccionEnMemoria = buscaDireccionFisica( valorOp, cantBytes );
    if ( direccionEnMemoria != -1 ){
            registros[MBR] = valor;
            int aux = cantBytes*8;
            for ( int i = direccionEnMemoria; i < direccionEnMemoria + cantBytes; i++  ){
                aux -= 8;
                RAM[i] = valor >> (aux-8) & 0xFF;
            }
    }        
}
// solo sirve para operanciones de dos operando, ya que si recupero dos valores siempre son de 4 bytes ( variable valor )
int lecturaEnMemoria( uint32_t valorOp, uint32_t cantBytes ){
    int direccionEnMemoria = buscaDireccionFisica( valorOp,cantBytes );
    if ( direccionEnMemoria != -1 ){
        int valor = 0;
        int aux = cantBytes*8;
        for ( int i = direccionEnMemoria; i < direccionEnMemoria + cantBytes; i++ ){
            aux -= 8;
            valor = valor | (RAM[i] << aux);// es un OR acumulativo

        }       
        registros[MBR] = valor;
        return valor;
    }

}

int buscaDireccionFisica( uint32_t valorOp, uint32_t cantBytes ){// lo maximo que puede ser son 3 bytes de valorOp
    uint8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
    int offset = valorOp >> 8 ;
    if ( codReg != DS ){
        offset += registros[codReg];
    }
    registros[LAR] = 0x00010000;
    registros[LAR] += offset;// lo que pone literalmente en el pdf
    // la operacion al ser MOV se lee o se escribe de a 4 bytes
    registros[MAR] = cantBytes;
    registros[MAR] = registros[MAR] << 16;
    int direFisica = tabla_seg[1].base + offset;
    int aux = registros[MAR] >> 16;
    uint16_t parteBaja = direFisica;
    registros[MAR] = (registros[MAR] & 0xFFFF0000 ) | parteBaja;
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