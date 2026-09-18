#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

#define BIT_N 31
#define BIT_Z 30
#define BIT_C 29
#define BIT_V 28


void actualizarCC(uint32_t valor, int c, int v){
    registros[CC] = 0;
    int n = 0;
    int z = 0;

    if (valor < 1)
        n = 1;
    if (valor == 0)
        n = 1;

    registros[CC] |= (uint32_t)(n!=0) << BIT_N;
    registros[CC] |= (uint32_t)(z!=0) << BIT_Z;
    registros[CC] |= (uint32_t)(c!=0) << BIT_C;
    registros[CC] |= (uint32_t)(v!=0) << BIT_V;
}

int buscaDireccionFisica( uint32_t valorOp, uint32_t cantBytes ){// lo maximo que puede ser son 3 bytes de valorOp
    uint8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
    int offset = valorOp >> 8 ;
    if ( codReg != DS ){
        offset += registros[codReg];
    }
    int direFisica = tabla_seg[1].base + offset;
    if ( (tabla_seg[1].tam + tabla_seg[1].base < direFisica + cantBytes) || (direFisica < tabla_seg[1].base) ){
        printf( "Te fuiste del segmento capo" );
        return -1;
    }
    else{
        uint16_t parteBaja = direFisica;
        registros[MAR] = cantBytes;
        registros[MAR] = registros[MAR] << 16;
        registros[LAR] = 0x00010000;
        registros[LAR] += offset;
        registros[MAR] = (registros[MAR] & 0xFFFF0000 ) | parteBaja;

        printf(" Aca se muestra la direccion fisica a la que se accedio \n");
        printf("%d\n", direFisica);
        return direFisica;
    }
}
void escrituraEnMemoria( uint32_t valor, uint16_t cantBytes, uint32_t valorOp ){

    int direccionEnMemoria = buscaDireccionFisica( valorOp, cantBytes );
    if ( direccionEnMemoria != -1 ){
            registros[MBR] = valor;
            int aux = cantBytes*8;
            for ( int i = direccionEnMemoria; i < direccionEnMemoria + cantBytes; i++  ){
                aux -= 8;
                RAM[i] = valor >> (aux) & 0xFF;// dado el valor viene en 32 bits debo hacer determinada logica
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


void MOV( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){// si es un operando de memoria voy a tener que ir a buscar el valor
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else
        if( tipoOpB == 0x01 ){
            uint8_t nroRegistroOpB = valorB & 0x0000001F;
            valor = registros[nroRegistroOpB];
        }
        else
            valor = valorB;

    if ( tipoOpA == 0x03 ){// si es un operando de memoria voy a tener que ir a escribir 
        escrituraEnMemoria( valor, 4, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        uint8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] = valor; 
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

void SHL( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SHR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SAR( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void LDL( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void LDH( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void RND( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void SYS( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JMP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JN ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JZ ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JC ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JV ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JNP ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JNN ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void JNZ ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void NOT ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void ERROR ( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){}

void STOP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){
    registros[IP] = -1;
}

void ( *operaciones[ 32 ] )(unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ) 
= { SYS, JMP, JP, JN, JZ, JC, JV, JNP, JNN, JNZ, NOT, ERROR, ERROR, ERROR, ERROR, STOP, MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };