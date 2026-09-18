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
int buscaDireccionFisica( uint32_t valorOp, uint32_t cantBytes ){// lo maximo que puede ser son 3 bytes de valorOp
    uint8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
    int offset = valorOp >> 8 ;
    if ( codReg != DS ){
        offset += registros[codReg];
    }
    int direFisica = tabla_seg[1].base + offset;
    if ( (tabla_seg[1].tam + tabla_seg[1].base < direFisica + cantBytes) || (direFisica < tabla_seg[1].base) ){
        printf( "Te fuiste del segmento capo\n" );
        return -1;
    }
    else{
        uint16_t parteBaja = direFisica;
        registros[MAR] = cantBytes;
        registros[MAR] = registros[MAR] << 16;
        registros[LAR] = 0x00010000;
        registros[LAR] += offset;
        registros[MAR] = (registros[MAR] & 0xFFFF0000 ) | parteBaja;

        printf("Aca se muestra la direccion fisica a la que se accedio: \n", direFisica, "\n");
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
                printf( " se escribe el valor: %02X\n", RAM[i] );
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
    printf(" Valor que se va a asignar en memoria o en registro \n");
    printf("%d al valor que se accedio es:  \n", valor );
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

void ( *operacionesDosOperando[ 16 ] )(unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ) 
= { MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };
