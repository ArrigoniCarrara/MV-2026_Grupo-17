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
    int n = (int32_t)valor < 0;   // reinterpretar como signed
    int z = (valor == 0);

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


void ADD( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria(valorBuscado + valor,4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x000000FF;
         registros[nroRegistroA] += valor; 
    }
}

void SUB( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
        int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorBuscado - valor,4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x000000FF;
         registros[nroRegistroA] -= valor; 
    }
}

void MUL(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorBuscado * valor, 4, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x000000FF;
         registros[nroRegistroA] *= valor; 
    }
}

void DIV (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;

    if(valor == 0)
        printf("Division por cero flaco");
    else{
        if( tipoOpA == 0x03 ){
            int valorBuscado = lecturaEnMemoria( valorA, 4 );
            escrituraEnMemoria( valorBuscado / valor, 4, valorA );
        }
        else{
            uint8_t nroRegistroA = valorA & 0x000000FF;
            registros[nroRegistroA] /= valor; 
        }
    }
}

void CMP(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor, valorBuscado;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             uint8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    
    if( tipoOpA == 0x03 ){
        valorBuscado = lecturaEnMemoria( valorA, 4 );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valorBuscado = registros[nroRegistroA]; 
    }

    uint32_t opB = ~valor + 1;                       // complemento a 2 de B
    uint64_t resta = (uint64_t)valorBuscado + (uint64_t)opB;   // suma, no resta directa
    uint32_t resultado = (uint32_t)resta;
    int c = resta > 0xFFFFFFFFULL;
    int v = ((~(valorBuscado ^ opB)) & (valorBuscado ^ resultado)) >> 31 & 1;

    actualizarCC(resta, c, v);
}

void AND(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    if ( tipoOpB ==0x03 ){
        

    }

}

void OR(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void XOR( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void SWAP(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void SHL(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void SHR(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void SAR(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void LDL(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void LDH(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void RND(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}

void SYS(  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    uint32_t dire_memoria = registros[EDX]; // Donde empiezo a leer o escribir
    uint16_t cant_valores = registros[ECX] & 0x00FF;
    uint16_t cantbytes = registros[ECX] >> 16;
    uint8_t modo_lectura = registros[EAX];
    if(valorA == 1){
    }
}

void JMP( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int valor;
    if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
         uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    } else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }
}

void JP( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 0 && z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
     
}

void JN (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 1){
        
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JZ (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int  z = (registros[CC] >> 30) & 1;

    if(z == 1){
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JC (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int c = (registros[CC] >> 29) & 1;
    if(c == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }
    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JV (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int v = (registros[CC] >> 28) & 1;
    if(v == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JNP (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 1 || z == 1){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JNN (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void JNZ (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
     int  z = (registros[CC] >> 30) & 1;

    if(z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x000000FF;
         valor = registros[nroRegistroA]; 
    }else{
        valor = valorA;
    }

    if(valor <= tabla_seg[0].tam)
        registros[IP] = valor;
    else{
        printf("Te fuiste al DATA SEGMENT PEDAZO DE GIL");
    }

    }
}

void NOT ( unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){
        int valor;
        if( tipoOpA == 0x03 ){
            valor = lecturaEnMemoria( valorA, 4 );
            valor = ~valor;
            escrituraEnMemoria(valor, 4, valorA);
        }
        else if (tipoOpA == 0x01){
            uint8_t nroRegistroA = valorA & 0x000000FF;
            valor = registros[nroRegistroA]; 
        }else
            valor = valorA;
        
}

void ERROR (  unsigned char tipoOpA, unsigned char tipoOpB, uint32_t valorA, uint32_t valorB ){}// ver manejos de errores despues

void STOP( unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ){
    registros[IP] = -1;
}

void ( *operaciones[ 32 ] )(unsigned char opA, unsigned char opB, uint32_t valorA, uint32_t valorB ) 
= { SYS, JMP, JP, JN, JZ, JC, JV, JNP, JNN, JNZ, NOT, ERROR, ERROR, ERROR, ERROR, STOP, MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };