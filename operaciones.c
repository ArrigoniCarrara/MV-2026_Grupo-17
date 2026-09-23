#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

#define BIT_N 31
#define BIT_Z 30
#define BIT_C 29
#define BIT_V 28

int buscaDireccionFisica( int32_t valorOp, int32_t cantBytes ){// lo maximo que puede ser son 3 bytes de valorOp
    uint8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
    int offset = valorOp >> 8;
    int direFisica = registros[codReg];
    direFisica += offset;

    if ( (tabla_seg[1].tam + tabla_seg[1].base < direFisica + cantBytes) || (direFisica < tabla_seg[1].base) ){
        error_actual = ERR_FALLO_SEGMENTO;
        return -1;
    }
    else{
        int16_t parteBaja = direFisica;
        registros[MAR] = cantBytes;
        registros[MAR] = registros[MAR] << 16;
        registros[LAR] = 0x00010000;
        registros[LAR] |= offset;
        registros[MAR] = (registros[MAR] & 0xFFFF0000 ) | parteBaja;

        return direFisica;
    }
}
void escrituraEnMemoria( int32_t valor, int16_t cantBytes, int32_t valorOp ){

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
int lecturaEnMemoria( int32_t valorOp, int32_t cantBytes ){
    int direccionEnMemoria = buscaDireccionFisica( valorOp,cantBytes );
    if ( direccionEnMemoria != -1 ){
        int32_t valor = 0;
        int aux = cantBytes*8;
        for ( int i = direccionEnMemoria; i < direccionEnMemoria + cantBytes; i++ ){
            aux -= 8;
            valor = valor | (RAM[i] << aux);// es un OR acumulativo
        }      
        registros[MBR] = valor;
        return valor;
    }

}


void MOV( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
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

    if ( tipoOpA == 0x03 ){
        // si es un operando de memoria voy a tener que ir a escribir 
        escrituraEnMemoria( valor, 4, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        uint8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] = valor; 
    }
    registros[CC] &= 0x0FFFFFFF;
    if ( valor == 0 ) 
        registros[CC] |= 0x40000000; 

    if ( valor < 0)
        registros[CC] |= 0x80000000;
}


void ADD( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB, valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;

    if( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria(valorGuardadoA + valorGuardadoB,4, valorA );
    }
    else{
         uint8_t nroRegistroA = valorA & 0x0000001F;
         valorGuardadoA = registros[nroRegistroA]; 
         registros[nroRegistroA] += valorGuardadoB; 
    }

    // Aca modifico el CC
     registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoB + valorGuardadoA;
    if ( resultado == 0 )
        registros[CC] |= 0x40000000;
    if ( resultado < 0 )
        registros[CC] |= 0x80000000;
    // Overflow:
    int64_t resul64 = (int64_t) valorGuardadoA  + (int64_t)  valorGuardadoB;
    if ( resul64 > INT32_MAX || resul64 < INT32_MIN )
        registros[CC] |= 0x10000000;
    // Acarreo
    uint64_t resU64 = (uint64_t)(uint32_t)valorGuardadoA + (uint64_t)(uint32_t)valorGuardadoB;
    if ((resU64 >> 32) != 0)// se hace el doble casteo para que no extienda el signo
        registros[CC] |= 0x20000000;
}

void SUB( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
        int valorGuardadoB, valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorGuardadoA - valorGuardadoB,4, valorA );
    }
    else{
         uint8_t nroRegistroA = valorA & 0x0000001F;
         valorGuardadoA = registros[nroRegistroA];
         registros[nroRegistroA] -= valorGuardadoB; 
    }

    // Aca modifico el CC
     registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA - valorGuardadoB;
    if ( resultado == 0 )
        registros[CC] |= 0x40000000;
    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

        // Overflow:
    int64_t resultadoConSigno = (int64_t) valorGuardadoA  - (int64_t)valorGuardadoB;
    if (  resultadoConSigno > INT32_MAX ||  resultadoConSigno < INT32_MIN )
        registros[CC] |= 0x10000000;

    // acarreo
    uint64_t resulSinSigno = (uint64_t)(uint32_t)valorGuardadoA + (uint64_t)(~((uint32_t)valorGuardadoB)+1);
    // tengo que hacer la resta con complemento A2

    if (resulSinSigno>> 32 != 0 )
        registros[CC] |= 0x20000000;
}

void MUL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB, valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorGuardadoA * valorGuardadoB, 4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
         registros[nroRegistroA] *= valorGuardadoB; 
    }

     registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA * valorGuardadoB;
    if ( resultado == 0 )
        registros[CC] |= 0x40000000;
    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

        // Overflow:
    int64_t resultadoConSigno = (int64_t) valorGuardadoA  * (int64_t)  valorGuardadoB;
    if ( resultadoConSigno > INT32_MAX || resultadoConSigno < INT32_MIN )
        registros[CC] |= 0x10000000;
        // Acarreo
    uint64_t resul64 = (uint64_t)(uint32_t)valorGuardadoA * (uint64_t)(uint32_t)valorGuardadoB;// Doble caseteo asi no propago el signo
    if (resul64 >> 32 != 0 )
        registros[CC] |= 0x20000000;
}

void DIV (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB, valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;

    if(valorGuardadoB == 0){
        error_actual = ERR_DIVISION_CERO;
        return;
    }
    else{

        if( tipoOpA == 0x03 ){
            valorGuardadoA = lecturaEnMemoria( valorA, 4 );
            escrituraEnMemoria( valorGuardadoA / valorGuardadoB, 4, valorA );
        }
        else{
            uint8_t nroRegistroA = valorA & 0x0000001F;
            valorGuardadoA = registros[nroRegistroA];
            registros[nroRegistroA] /= valorGuardadoB; 
        }
    }

        registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
        registros[AC] = valorGuardadoA % valorGuardadoB;
        int32_t resultado = valorGuardadoA / valorGuardadoB;
        // La operacion DIV no modifica C ni V
        registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
        if ( resultado == 0 )
            registros[CC] |= 0x40000000;
        if ( resultado < 0 )
            registros[CC] |=  0x80000000;
}

void CMP(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB, valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;
    
    if( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA]; 
    }
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA - valorGuardadoB;
    if ( resultado == 0 )
        registros[CC] |= 0x40000000;
    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

        // Overflow:
    int64_t resultadoConSigno = (int64_t) valorGuardadoA  - (int64_t)valorGuardadoB;
    if (  resultadoConSigno > INT32_MAX ||  resultadoConSigno < INT32_MIN )
        registros[CC] |= 0x10000000;

    // acarreo
    uint64_t resultadoSinSigno = (uint64_t)(uint32_t)valorGuardadoA + (uint64_t)(~((uint32_t)valorGuardadoB)+1);
    // tengo que hacer la resta con complemento A2

    if (resultadoSinSigno>> 32 != 0 )
        registros[CC] |= 0x20000000;
}

void AND(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    registros[CC] &= 0x0FFFFFFF;// registro[CC] = 0x xx xx xx
    if ( tipoOpB ==0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB, 4 );
    }
    else
        if( tipoOpB == 0x01 ){
            uint8_t nroRegistroOpB = valorB & 0x0000001F;
            valorGuardadoB = registros[ nroRegistroOpB ];
        }
        else
            valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA,4 );
        escrituraEnMemoria( valorGuardadoA & valorGuardadoB, 4, valorA);
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] = registros[nroRegistroA] & valorGuardadoB;   
    }
    //Modifico el CC
    //Modifico el CC
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA & valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;         
        
}
//solo afecta a N y Z
void OR( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    registros[CC] &= 0x0FFFFFFF;// registro[CC] = 0x xx xx xx
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else 
        if( tipoOpB == 0x01 ){
            uint8_t nroRegistroOpB = valorB & 0x0000001F;
            valorGuardadoB = registros[ nroRegistroOpB ];            
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        int valorGuardadoA = lecturaEnMemoria( valorA,4 );
        escrituraEnMemoria( valorGuardadoB | valorGuardadoA, 4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] |= valorGuardadoB;  
    }
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA & valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;
 

}
//solo afecta a N y Z
void XOR( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else 
        if( tipoOpB == 0x01 ){
            uint8_t nroRegistroOpB = valorB & 0x0000001F;
            valorGuardadoB = registros[ nroRegistroOpB ];            
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        int valorGuardadoA = lecturaEnMemoria( valorA,4 );
        escrituraEnMemoria( valorGuardadoA ^ valorGuardadoB, 4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        registros[nroRegistroA] ^= valorGuardadoB;  
    }
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA ^ valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;
    

}
//solo afecta a N y Z
void SWAP(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
     int valorGuardadoB,valorGuardadoA;
     uint8_t nroRegistroA, nroRegistroOpB;
    if ( tipoOpB == 0x03 )
         valorGuardadoB = lecturaEnMemoria(valorB, 4 );
    else{
         nroRegistroOpB = valorB & 0x0000001F;
         valorGuardadoB = registros[ nroRegistroOpB ];
        }
    if ( tipoOpA == 0x03 )
         valorGuardadoA = lecturaEnMemoria(valorA, 4 );
    else{
        nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
    }
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    if(tipoOpA == 3)
        escrituraEnMemoria(valorGuardadoB, 4, valorA);
    else if (tipoOpA == 2)
             registros[nroRegistroA] = valorGuardadoB;
            
    if(tipoOpB == 3)
        escrituraEnMemoria(valorGuardadoA, 4, valorB);
        else
            registros[nroRegistroOpB] = valorGuardadoA;

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    if ( valorGuardadoB == 0 )
        registros[CC] |= 0x40000000;
    if ( valorGuardadoB < 0 )
        registros[CC] |= 0x80000000; // -> 0x xx xx xx limpio los primeros 4 bits

}

void SHL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valorGuardadoB = registros[ nroRegistroOpB ];        
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorGuardadoA << valorGuardadoB,4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] <<= valorGuardadoB;        
    }

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA << valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

    // Overflow
    int64_t resultadoConSigno = (int64_t)(int32_t)valorGuardadoA << (int64_t)(int32_t)valorGuardadoB;
    if ( resultadoConSigno > INT32_MAX || resultadoConSigno < INT32_MIN)
        registros[CC] |= 0x10000000;

    // Acarreo
    uint64_t resultadoSinSigno = (uint64_t) valorGuardadoA << (uint64_t)valorGuardadoB;
    if( resultadoSinSigno >> 32 != 0 || (valorGuardadoB >= 32) && valorGuardadoA != 0 )
        registros[CC] |= 0x20000000;
}

void SHR(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int32_t valorGuardadoB,valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valorGuardadoB = registros[ nroRegistroOpB ];        
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorGuardadoA >> valorGuardadoB,4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] >>= valorGuardadoB;        
    }

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits

    uint32_t resultado = valorGuardadoA >> valorGuardadoB;

    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

    uint32_t a    = (uint32_t)valorGuardadoA;
    uint32_t cant = (uint32_t)valorGuardadoB;

    int c;
    if (cant >= 32)
        c = (a != 0);
    else {
        uint64_t wide = (uint64_t)a << 32;   // 
        wide >>= cant;                       // 
        c = ((uint32_t)wide != 0);           // 
    }
    if (c)
        registros[CC] |= 0x20000000;
}

void SAR(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int32_t valorGuardadoB,valorGuardadoA;
    if ( tipoOpB == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valorGuardadoB = registros[ nroRegistroOpB ];        
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        valorGuardadoB = lecturaEnMemoria( valorA, 4 );
        int32_t resultado = valorGuardadoA >> valorGuardadoB;
        escrituraEnMemoria( resultado, 4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        int32_t resultado = valorGuardadoA >> valorGuardadoB;
        registros[nroRegistroA] = resultado;        
    }

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits

    uint32_t resultado = valorGuardadoA >> valorGuardadoB;

    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;

    uint32_t a    = (uint32_t)valorGuardadoA;
    uint32_t cant = (uint32_t)valorGuardadoB;

    int c;
    if (cant >= 32)
        c = (a != 0);
    else {
        uint64_t wide = (uint64_t)a << 32;   // 
        wide >>= cant;                       // 
        c = ((uint32_t)wide != 0);           // 
    }
    if (c)
        registros[CC] |= 0x20000000;

}

void LDL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    if (tipoOpB == 0x03){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valorGuardadoB = registros[ nroRegistroOpB ];   
        }
        else{
            valorGuardadoB = valorB;  
        }
    valorGuardadoB &= 0x0000FFFF;
    if ( tipoOpA == 0x03  ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        valorGuardadoA &= 0xFFFF0000; 
        escrituraEnMemoria( valorGuardadoB | valorGuardadoA, 4,valorA );  
    }
    else{
        uint8_t nroRegistroA = valorA & 0x000000FF;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] = valorGuardadoB | valorGuardadoA;
    }
    
}

void LDH(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    if (tipoOpB == 0x03){
        valorGuardadoB = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valorGuardadoB = registros[ nroRegistroOpB ];   
        }
        else{
            valorGuardadoB = valorB;  
        }
    valorGuardadoB &= 0x0000FFFF;
    if ( tipoOpA == 0x03  ){
        valorGuardadoA = lecturaEnMemoria( valorA, 4 );
        valorGuardadoA &= 0x0000FFFF; 
        escrituraEnMemoria( (valorGuardadoB << 16) | valorGuardadoA, 4,valorA );  
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] = (valorGuardadoB << 16) | valorGuardadoA;
    }
    
}

void RND(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;    
    if (  tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
            uint8_t nroRegistroOpB = valorB & 0x0000001F;            
            valor = registros[nroRegistroOpB];
        }
        else  
            valor = valorB;
    uint32_t num_aleatorio =  rand() % (valor + 1) ; // BUSCAR UNA FUNCION RANDOM QUE FUNCIONE
    if( tipoOpA == 0x03 ){
        escrituraEnMemoria(num_aleatorio, 4, valorA);
    }else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        registros[nroRegistroA] = num_aleatorio;
    }

}

void SYS(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    uint32_t dire_memoria = registros[EDX]; // Donde empiezo a leer o escribir
    uint16_t cant_valores = registros[ECX] & 0x00FF;
    uint16_t cantbytes = registros[ECX] >> 16;
    uint8_t modo_lectura = registros[EAX];
    if(valorA == 1){

        for(int i = 0; i < cant_valores; i++){
            printf("[%d]: ", dire_memoria);
            int32_t valor_usuario;
            if(modo_lectura == 1){
                scanf("%d",&valor_usuario);
            }
            else if(modo_lectura == 2){
                    scanf(" %c",&valor_usuario); 
            }else if(modo_lectura == 4){
                    scanf("%o",&valor_usuario);
            }else if(modo_lectura == 8){
                    scanf("%x",&valor_usuario);
            }else if(modo_lectura == 16){
                char bin_str[33]; // Arreglo para almacenar la cadena de ceros y unos
                scanf("%32s", bin_str); // Se lee el texto ingresado por el usuario
                valor_usuario = strtol(bin_str, NULL, 2);
            }

            if ( dire_memoria != -1 ){
                registros[MBR] = valor_usuario;
                int aux = cantbytes*8;
                for ( int i = dire_memoria; i < dire_memoria + cantbytes; i++  ){
                    aux -= 8;
                    RAM[i] = valor_usuario >> (aux) & 0xFF;// dado el valor viene en 32 bits debo hacer determinada logica
                }
                dire_memoria += cantbytes;
            }else
                return;        
        }


    }else if (valorA == 2){
       for(int i = 0; i < cant_valores; i++){

        if ( dire_memoria != -1 ){
            int32_t valor = 0;
            int aux = cantbytes*8;
            for ( int i = dire_memoria; i < dire_memoria + cantbytes; i++ ){
                aux -= 8;
                valor = valor | (RAM[i] << aux);// es un OR acumulativo
            }
            uint32_t dire_memoria_imprimir = dire_memoria;    
            dire_memoria += cantbytes;

            printf("[%d]: ", dire_memoria_imprimir);
            if((modo_lectura & 0x01) == 1){
                printf(" %d ", valor);
            }

            if(((modo_lectura >> 1) & 0x01) == 1){
                unsigned char c = (valor & 0xFF);
                if (c >= 32 && c <= 126) {
                  printf(" %c ", c);
                } else 
                    printf(" . ");   
            }

            if(((modo_lectura >> 2) & 0x01) == 1){
                printf(" 0o");
                printf("%o ", valor);
            }

            if(((modo_lectura >> 3) & 0x01) == 1){
                printf(" 0x");
                printf("%08X ", valor);
            }

            if(((modo_lectura >> 4) & 0x01) == 1){ 
               int comenzo = 0;
               printf("0b");
                for (int x = 31; x >= 0; x--) {
                    int bit = (valor >> x) & 1;
                    if (bit) comenzo = 1;
                    if (comenzo) printf("%d", bit);
                }
                printf(" ");
            }
            printf("\n");
        }else
             return;

       }
    }
}

void JMP( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
         uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    } else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }
}

void JP( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 0 && z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
     
}

void JN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 1){
        
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void JZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  z = (registros[CC] >> 30) & 1;

    if(z == 1){
        int valor;
        if( tipoOpA == 0x03 ){
            valor = lecturaEnMemoria( valorA, 4 );
        }
        else if (tipoOpA == 0x01){
            uint8_t nroRegistroA = valorA & 0x0000001F;
            valor = registros[nroRegistroA]; 
        }else{
            valor = valorA;
        }

        if(valor <= tabla_seg[0].tam)
            registros[IP] = valor - 1;
        else{
            error_actual = ERR_FALLO_SEGMENTO;
        }   

    }
}

void JC (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int c = (registros[CC] >> 29) & 1;
    if(c == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }
    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void JV (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int v = (registros[CC] >> 28) & 1;
    if(v == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void JNP (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 1 || z == 1){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void JNN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void JNZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
     int  z = (registros[CC] >> 30) & 1;

    if(z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor - 1;
    else{
        error_actual = ERR_FALLO_SEGMENTO;
    }

    }
}

void NOT ( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
        int valor;
        if( tipoOpA == 0x03 ){
            valor = lecturaEnMemoria( valorA, 4 );
            valor = ~valor;
            escrituraEnMemoria(valor, 4, valorA);
        }
        else if (tipoOpA == 0x01){
            uint8_t nroRegistroA = valorA & 0x0000001F;
            valor = registros[nroRegistroA]; 
        }else
            valor = valorA;

        registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
        if ( valor < 0 )
            registros[CC] |= 0x80000000;
        if ( valor == 0 )
            registros[CC] |= 0x40000000;
        
}

void ERROR (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    error_actual = ERR_INSTRUCCION_INVALIDA;
}

void STOP( unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ){
    registros[IP] = -1;
}

void ( *operaciones[ 32 ] )(unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ) 
= { SYS, JMP, JP, JN, JZ, JC, JV, JNP, JNN, JNZ, NOT, ERROR, ERROR, ERROR, ERROR, STOP, MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };