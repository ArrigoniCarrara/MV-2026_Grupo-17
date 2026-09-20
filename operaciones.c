#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

#define BIT_N 31
#define BIT_Z 30
#define BIT_C 29
#define BIT_V 28

int buscaDireccionFisica( int32_t valorOp, int32_t cantBytes ){// lo maximo que puede ser son 3 bytes de valorOp
    int8_t codReg = valorOp & 0x00001F;//rescato el codigo de registro
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
        int16_t parteBaja = direFisica;
        registros[MAR] = cantBytes;
        registros[MAR] = registros[MAR] << 16;
        registros[LAR] = 0x00010000;
        registros[LAR] += offset;
        registros[MAR] = (registros[MAR] & 0xFFFF0000 ) | parteBaja;

        return direFisica;
    }
}
void escrituraEnMemoria( int32_t valor, int16_t cantBytes, int32_t valorOp ){

    int direccionEnMemoria = buscaDireccionFisica( valorOp, cantBytes );
    if ( direccionEnMemoria != -1 ){
            registros[MBR] = valor;
            int aux = cantBytes*8;
            printf("[MEMORIA] Escribiendo %d bytes en Dir Física [%04X]: Valor %08X\n", cantBytes, direccionEnMemoria, valor);
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
        printf("[MEMORIA] Leyendo %d bytes desde Dir Física [%04X]: Valor obtenido %08X\n", cantBytes, direccionEnMemoria, valor);
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
            int8_t nroRegistroOpB = valorB & 0x0000001F;
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
        int8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] = valor; 
    }
    registros[CC] &= 0x0FFFFFFF;
    printf("Modifico CC  %d \n", valor);
    if ( valor == 0 ) 
        registros[CC] |= 0x40000000; 

    if ( valor < 0)
        registros[CC] |= 0x80000000;
    printf("Valor modificado CC: %08X\n", registros[CC]);
}


void ADD( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             int8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;

    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria(valorBuscado + valor,4, valorA );
        printf("%d\n", valorBuscado + valor);
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF; // PREGUNTAR A NACHI Q ONDA
         registros[nroRegistroA] += valor; 
         printf("%d\n",  registros[nroRegistroA]);
    }
}

void SUB( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
        int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             int8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorBuscado - valor,4, valorA );
        printf("%d\n", valorBuscado - valor);
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
         registros[nroRegistroA] -= valor; 
         printf("%d\n",  registros[nroRegistroA]);
    }
}

void MUL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             int8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    if( tipoOpA == 0x03 ){
        int valorBuscado = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valorBuscado * valor, 4, valorA );
        printf("%d\n", valorBuscado * valor);
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
         registros[nroRegistroA] *= valor; 
         printf("%d\n",  registros[nroRegistroA]);
    }
}

void DIV (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             int8_t nroRegistroOpB = valorB & 0x0000001F;
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
            printf("%d\n", valorBuscado / valor);
        }
        else{
            int8_t nroRegistroA = valorA & 0x000000FF;
            registros[nroRegistroA] /= valor; 
            printf("%d\n",  registros[nroRegistroA]);
        }
    }
}

void CMP(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor, valorBuscado;
    if ( tipoOpB == 0x03 ){
        valor = lecturaEnMemoria( valorB, 4 );
    }
    else 
        if(tipoOpB == 0x01 ){
             int8_t nroRegistroOpB = valorB & 0x0000001F;
             valor = registros[ nroRegistroOpB ];
        }
        else
            valor = valorB;
    
    if( tipoOpA == 0x03 ){
        valorBuscado = lecturaEnMemoria( valorA, 4 );
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
         valorBuscado = registros[nroRegistroA]; 
    }
    
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
        uint8_t nroRegistroA = valorA & 0x000000FF;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] &= valorGuardadoB;   
    }
    //Modifico el CC
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
        uint8_t nroRegistroA = valorA & 0x000000FF;
        valorGuardadoA = registros[nroRegistroA];
        registros[nroRegistroA] |= valorGuardadoA;  
    }
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
        uint8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] ^= valorGuardadoB;  
    }
    int32_t resultado = valorGuardadoA & valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;
    

}
//solo afecta a N y Z
void SWAP(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor1,valor2;
    if ( tipoOpB == 0x03 )
         valor1 = lecturaEnMemoria(valorB, 4 );
    else{
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valor1 = registros[ nroRegistroOpB ];            
        }
    if ( tipoOpA == 0x03 )
         valor2 = lecturaEnMemoria(valorA, 4 );
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
        valor2 = registros[nroRegistroA];
    }
    escrituraEnMemoria(valor1, 4, valorB);
    escrituraEnMemoria(valor2, 4, valorA);

}

void SHL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor1,valor2;
    if ( tipoOpB == 0x03 ){
        valor1 = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valor1 = registros[ nroRegistroOpB ];        
    }
    else
        valor1 = valorB;
    if ( tipoOpA == 0x03 ){
        valor2 = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valor2 << valor1,4, valorA );
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] <<= valor1;        
    }
}

void SHR(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    uint32_t valor1,valor2;
    if ( tipoOpB == 0x03 ){
        valor1 = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valor1 = registros[ nroRegistroOpB ];        
    }
    else
        valor1 = valorB;
    if ( tipoOpA == 0x03 ){
        valor2 = lecturaEnMemoria( valorA, 4 );
        escrituraEnMemoria( valor2 >> valor1,4, valorA );
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
        registros[nroRegistroA] >>= valor1;        
    }
}

void SAR(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int32_t valor1,valor2;
    if ( tipoOpB == 0x03 ){
        valor1 = lecturaEnMemoria( valorB,4 );
    }
    else
        if( tipoOpB == 0x01 ){
        uint8_t nroRegistroOpB = valorB & 0x0000001F;
        valor1 = registros[ nroRegistroOpB ];        
    }
    else
        valor1 = valorB;
    if ( tipoOpA == 0x03 ){
        valor2 = lecturaEnMemoria( valorA, 4 );
        int32_t resultado = valor2 >> valor1;
        escrituraEnMemoria( resultado, 4, valorA );
    }
    else{
        int8_t nroRegistroA = valorA & 0x000000FF;
        int32_t resultado = valor2 >> valor1;
        registros[nroRegistroA] = resultado;        
    }
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
        int8_t nroRegistroA = valorA & 0x000000FF;
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
        int8_t nroRegistroA = valorA & 0x000000FF;
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
    if( tipoOpA == 0x03 ){
        escrituraEnMemoria( rand() %( valor + lecturaEnMemoria( valorA, 4 )+1 )+lecturaEnMemoria( valorA,4 ), 4, valorA);
    }

}

void SYS(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int32_t dire_memoria = registros[EDX]; // Donde empiezo a leer o escribir
    int16_t cant_valores = registros[ECX] & 0x00FF;
    int16_t cantbytes = registros[ECX] >> 16;
    int8_t modo_lectura = registros[EAX];
    if(valorA == 1){

        for(int i = 0; i < cant_valores; i++){
            printf("[%d]", dire_memoria);
            int32_t valor_usuario;
            if(modo_lectura == 1){
                scanf("%d",&valor_usuario);
            }
            else if(modo_lectura == 2){
                    scanf("%c",&valor_usuario); 
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
                printf("[MEMORIA] Escribiendo %d bytes en Dir Física [%04X]: Valor %08X\n", cantbytes, dire_memoria, valor_usuario);
                for ( int i = dire_memoria; i < dire_memoria + cantbytes; i++  ){
                    aux -= 8;
                    RAM[i] = valor_usuario >> (aux) & 0xFF;// dado el valor viene en 32 bits debo hacer determinada logica
                }
                dire_memoria += cantbytes;
            }        
        }


    }else if (valorA == 2){
        printf("==========ZONA SYSCAL==========\n");
       for(int i = 0; i < cant_valores; i++){

        if ( dire_memoria != -1 ){
            int32_t valor = 0;
            int aux = cantbytes*8;
            for ( int i = dire_memoria; i < dire_memoria + cantbytes; i++ ){
                aux -= 8;
                valor = valor | (RAM[i] << aux);// es un OR acumulativo
            }      

            dire_memoria += cantbytes;

            printf("[%d]: ", dire_memoria);
            if((modo_lectura & 0x01) == 1){
                printf(" %d", valor);
            }

            if(((modo_lectura >> 1) & 0x01) == 1){
                char c = (char)(valor & 0xFF);
                if (c >= 32 && c <= 126) {
                  printf(" %c", c);
                } else 
                    printf(" .");   
            }

            if(((modo_lectura >> 2) & 0x01) == 1){
                printf(" 0o");
                printf("%o", valor);
            }

            if(((modo_lectura >> 3) & 0x01) == 1){
                printf(" 0x");
                printf("%08X", valor);
            }

            if(((modo_lectura >> 4) & 0x01) == 1){ // NO FUNCIONA ARREGLAR
                printf(" 0b"); 
                for (int x = 31; x >= 0; x--) {
                    printf("%d", (valor >> x) & 1); 
                }
            }
            printf("\n");
        }

       }
       printf("=======================================\n");
    }
}

void JMP( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
         int8_t nroRegistroA = valorA & 0x000000FF;
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

void JP( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 0 && z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 1){
        
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  z = (registros[CC] >> 30) & 1;

    if(z == 1){
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JC (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int c = (registros[CC] >> 29) & 1;
    if(c == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JV (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int v = (registros[CC] >> 28) & 1;
    if(v == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JNP (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;

    if(n == 1 || z == 1){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JNN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;

    if(n == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void JNZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
     int  z = (registros[CC] >> 30) & 1;

    if(z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, 4 );
    }
    else if (tipoOpA == 0x01){
        int8_t nroRegistroA = valorA & 0x000000FF;
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

void NOT ( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
        int valor;
        if( tipoOpA == 0x03 ){
            valor = lecturaEnMemoria( valorA, 4 );
            valor = ~valor;
            escrituraEnMemoria(valor, 4, valorA);
        }
        else if (tipoOpA == 0x01){
            int8_t nroRegistroA = valorA & 0x000000FF;
            valor = registros[nroRegistroA]; 
        }else
            valor = valorA;
        
}

void ERROR (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){}// ver manejos de errores despues

void STOP( unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ){
    registros[IP] = -1;
}

void ( *operaciones[ 32 ] )(unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ) 
= { SYS, JMP, JP, JN, JZ, JC, JV, JNP, JNN, JNZ, NOT, ERROR, ERROR, ERROR, ERROR, STOP, MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };