#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "componentes.c"

#define BIT_N 31
#define BIT_Z 30
#define BIT_C 29
#define BIT_V 28

uint16_t IPdirfisica(){
    uint16_t seg = registros[IP] >> 16;
    uint16_t dirfisica = tabla_seg[seg].base;
    dirfisica += registros[IP] & 0x0000FFFF; 
    return dirfisica;
}

void modificoIP(int dirfisica){
    uint32_t seg = registros[IP] >> 16;
    registros[IP] = (seg << 16) | ((dirfisica - tabla_seg[seg].base) & 0xFFFF);
}

int buscaDireccionFisica( int32_t valorOp, int32_t cantBytes ){
    int direFisica = 0;
    uint8_t codReg = valorOp & 0x00001F;
    uint16_t seg = registros[codReg] >> 16; 
    direFisica = tabla_seg[seg].base;
    int offset = (registros[codReg] & 0x0000FFFF) + (int16_t)(valorOp >> 8);
    direFisica += offset;

    if ( seg >= TAM_SEG || (tabla_seg[seg].base == 0xFFFF && tabla_seg[seg].tam == 0xFFFF) || offset < 0 || offset + cantBytes > tabla_seg[seg].tam ){
        error_actual = ERR_FALLO_SEGMENTO;
        return -1;
    }
    registros[LAR] = (seg << 16) | ((uint32_t)offset & 0xFFFF);
    registros[MAR] = ((uint32_t)cantBytes << 16) | (uint16_t)direFisica;

    return direFisica;
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

int lecturaEnMemoria( int32_t valorOp, int32_t cantBytes ){
    int direccionEnMemoria = buscaDireccionFisica( valorOp,cantBytes );
    if ( direccionEnMemoria != -1 ){
        int32_t valor = 0;
        int aux = cantBytes*8;
        for ( int i = direccionEnMemoria; i < direccionEnMemoria + cantBytes; i++ ){
            aux -= 8;
            valor = valor | (RAM[i] << aux);// es un OR acumulativo
        }      
        if ( cantBytes < 4 ){
            int8_t bitMasSignificativo;
            if ( cantBytes == 1 ){
                bitMasSignificativo = valor >> 7;
                if ( bitMasSignificativo == 1 )
                    valor |= 0xFFFFFF00;
            }
            else{
                bitMasSignificativo = valor >> 15;
                if ( bitMasSignificativo == 1 )
                    valor |= 0xFFFF0000;
            }
        }
        registros[MBR] = valor;
        return valor;
    }
}

void escribirBE(FILE *f, uint32_t valor, int n){
    for (int i = n - 1; i >= 0; i--)
        fputc((valor >> (8 * i)) & 0xFF, f);
}

void guardarImagen(){
    if (!hay_vmi || nombre_vmi == NULL)
        return;
 
    FILE *vmi = fopen(nombre_vmi, "wb");
 
    fwrite("VMI26", 1, 5, vmi);
    fputc(1, vmi);                           
    escribirBE(vmi, memoria_vm_kib, 2); // Escribo en big endian el tamaño de la memoria
 
    for (int i = 0; i < CANT_REG; i++)
        escribirBE(vmi, registros[i], 4);
 
    for (int i = 0; i < TAM_SEG; i++){
        escribirBE(vmi, tabla_seg[i].base, 2); // escribo en dos bytes porque la tabla de segmentos tiene por posicion un 4 bytes
        escribirBE(vmi, tabla_seg[i].tam, 2);
    }
 
    fwrite(RAM, 1, memoria_vm_kib * 1024, vmi); // misma manera que en el main
    fclose(vmi);
}

void Breakpoint(){
    char eleccion;
    breakpoint_enter = 0;
    guardarImagen();

    while (1){
        scanf("%c", &eleccion);

        if(eleccion == 'g'){
            return;
        }

        if (eleccion == 'q'){
            registros[IP] = -1;
            return;
        }

        if(eleccion == '\n'){
            breakpoint_enter = 1;
            return;
        }
    }
}

// Devuele el valor del la parte del registro pedida, si se pide el ultimo byte por ejemplo devuelve 4 bytes pero solo maximo en los dos primeros(si no es ek registro completo) tiene datos, los demas son parte del signo
int32_t pseudonimoOperando(int32_t valorOP) {
    uint8_t nroRegistroOpB = valorOP & 0x1F;       // Bits 0-4
    uint8_t pseudonimo = (valorOP >> 5) & 0x03;    // Bits 5-6
    int32_t valor = registros[nroRegistroOpB];
    switch (pseudonimo) {
        case 0x01: // Es el caso de que solo agarro el byte menos significativo del registro
            return (int8_t)(valor & 0xFF);
            
        case 0x02: // Toma el segundo byte menos significativo del registro
            return (int8_t)((valor >> 8) & 0xFF); // Corregido: >> 8 para un byte
            
        case 0x03: // Toma los dos bytes mas significativo
            return (int16_t)(valor & 0xFFFF);
            
        default:   //  devuelve el registro completo
            return valor;
    }
}


void MOV( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valor;
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){// si es un operando de memoria voy a tener que ir a buscar el valor
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valor = lecturaEnMemoria( valorB, cantBytes );
    }
    else
        if( tipoOpB == 0x01 ){
            valor = pseudonimoOperando( valorB );
        }
        else
            valor = valorB;

    if ( tipoOpA == 0x03 ){
        // si es un operando de memoria voy a tener que ir a escribir 
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        escrituraEnMemoria( valor, cantBytes, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = valorA >> 5;// Corro los 5 bits del nro de registro
        pseudonimo = pseudonimo & 0x3;// Me quedo solo con los dos bits menos significativos
        if ( pseudonimo == 0x01 ){
            registros[nroRegistroA] = registros[nroRegistroA] & 0xFFFFFF00;
            registros[nroRegistroA] |= (valor & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                registros[nroRegistroA] &= 0xFFFF00FF;
                int32_t aux = valor & 0x000000FF;
                aux = aux << 8;
                registros[nroRegistroA] |= aux;
            }
        else
            if (pseudonimo == 0x3){
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (valor & 0x0000FFFF);
            }
        else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else 
        if(tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;

    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        escrituraEnMemoria(valorGuardadoA + valorGuardadoB, cantBytes , valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) + valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) + valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) + valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else 
        if(tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        escrituraEnMemoria( valorGuardadoA - valorGuardadoB,cantBytes, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) - valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) - valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) - valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else 
        if(tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        escrituraEnMemoria( valorGuardadoA * valorGuardadoB, cantBytes, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) * valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) * valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) * valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else 
        if(tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;
    if(valorGuardadoB == 0){
        error_actual = ERR_DIVISION_CERO;
        return;
    }
    else{

        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
            escrituraEnMemoria( valorGuardadoA / valorGuardadoB, cantBytes, valorA );
        }
        else{
            valorGuardadoA = pseudonimoOperando(valorA);
            // no nos atajamos si nos viene un opA inmediato.
            int32_t resultado;
            uint8_t nroRegistroA = valorA & 0x000000FF;
            uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
            if ( pseudonimo == 0x01 ){
                resultado = (registros[nroRegistroA] & 0x000000FF) / valorGuardadoB;
                registros[nroRegistroA] &= 0xFFFFFF00; 
                registros[nroRegistroA] |= (resultado & 0x000000FF);
            }
            else
                if (pseudonimo == 0x02){
                    resultado = (((registros[nroRegistroA] >> 8) / valorGuardadoB) & 0xFF) << 8;
                    registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
                }
            else
                if (pseudonimo == 0x3){
                    resultado = ( registros[nroRegistroA] & 0x0000FFFF ) / valorGuardadoB; 
                    registros[nroRegistroA] &= 0xFFFF0000;
                    registros[nroRegistroA] |= (resultado & 0x0000FFFF);
                }
            else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else 
        if(tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
    }
    else{
        valorGuardadoA = pseudonimoOperando( valorA );
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
    int8_t cantBytes;
    registros[CC] &= 0x0FFFFFFF;// registro[CC] = 0x xx xx xx
    if ( tipoOpB ==0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB, cantBytes );
    }
    else
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando( valorB );
        }
        else
            valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA,cantBytes );
        escrituraEnMemoria( valorGuardadoA & valorGuardadoB, cantBytes, valorA);
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) & valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) & valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) & valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
            registros[nroRegistroA] &= valorGuardadoB; 
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
    int8_t cantBytes;
    registros[CC] &= 0x0FFFFFFF;// registro[CC] = 0x xx xx xx
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
    }
    else 
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA,cantBytes );
        escrituraEnMemoria( valorGuardadoB | valorGuardadoA, cantBytes, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) | valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) | valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) | valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
            registros[nroRegistroA] |= valorGuardadoB; 
    }
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    int32_t resultado = valorGuardadoA | valorGuardadoB;
    if ( resultado == 0 ) // 
        registros[CC] |= 0x40000000; // XX XX XX XX 

    if ( resultado < 0 )
        registros[CC] |= 0x80000000;
 

}
//solo afecta a N y Z
void XOR( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
    }
    else 
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando( valorB );
        }
        else
            valorGuardadoB = valorB;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        escrituraEnMemoria( valorGuardadoA ^ valorGuardadoB, cantBytes, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) ^ valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) ^ valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) ^ valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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
     int8_t cantBytes;
     uint8_t nroRegistroA, nroRegistroOpB;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria(valorB, cantBytes );
    }
    else
        valorGuardadoB = pseudonimoOperando(valorB);
    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria(valorA, cantBytes );
    }
    else
        valorGuardadoA = pseudonimoOperando( valorGuardadoA );
    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    if(tipoOpA == 3){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        escrituraEnMemoria(valorGuardadoB, cantBytes, valorA);
    }
    else 
        if (tipoOpA == 2){
            uint8_t nroRegistroA = valorA & 0x000000FF;
            uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro 
            if ( pseudonimo == 0x01 ){
                registros[nroRegistroA] &= 0xFFFFFF00;
                registros[nroRegistroA] |= (valorGuardadoB & 0x000000FF);
            }
            else
                if( pseudonimo == 0x02 ){
                    registros[nroRegistroA] &= 0xFFFF00FF;
                    registros[nroRegistroA] |= ((valorGuardadoB & 0x000000FF ) << 8);
                }
            else
                if ( pseudonimo == 0x03 ){
                    registros[nroRegistroA] &= 0xFFFF0000;
                    registros[nroRegistroA] |= ( valorGuardadoB & 0x0000FFFF );
                }
            else    
                registros[nroRegistroA] = valorGuardadoB;
        }
        if(tipoOpB == 3){
            cantBytes = (valorB >> 7) & 0x00000007;
            escrituraEnMemoria(valorGuardadoA, cantBytes, valorB);
        }
        else{
            uint8_t nroRegistroB = valorB & 0x000000FF;
            uint8_t pseudonimo = (valorB >> 5) & 0x03;// Corro los 5 bits del nro de registro 
            if ( pseudonimo == 0x01 ){
                registros[nroRegistroB] &= 0xFFFFFF00;
                registros[nroRegistroB] |= (valorGuardadoA & 0x000000FF);
            }
            else
                if( pseudonimo == 0x02 ){
                    registros[nroRegistroB] &= 0xFFFF00FF;
                    registros[nroRegistroB] |= ((valorGuardadoA & 0x000000FF ) << 8);
                }
            else
                if ( pseudonimo == 0x03 ){
                    registros[nroRegistroB] &= 0xFFFF0000;
                    registros[nroRegistroB] |= ( valorGuardadoA & 0x0000FFFF );
                }
            else    
                registros[nroRegistroB] = valorGuardadoA;     
        }
            

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    if ( valorGuardadoB == 0 )
        registros[CC] |= 0x40000000;
    if ( valorGuardadoB < 0 )
        registros[CC] |= 0x80000000; // -> 0x xx xx xx limpio los primeros 4 bits

}

void SHL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
    }
    else
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando( valorB );
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        uint32_t resultado = valorGuardadoA << valorGuardadoB;
        escrituraEnMemoria( resultado,4, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) << valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) << valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) << valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
            registros[nroRegistroA] <<= valorGuardadoB;     
    }

    registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
    uint32_t resultado = valorGuardadoA << valorGuardadoB;
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes);
    }
    else
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        uint32_t resultado = valorGuardadoA << valorGuardadoB;
        escrituraEnMemoria( resultado,4, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        uint32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){// AL, BL, CL, DL, EL, FL
            resultado = (registros[nroRegistroA] & 0x000000FF) >> valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){// AH, BH, CH, DH, EH, FH
                resultado = (((registros[nroRegistroA] >> 8) >> valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){// AX, BX, CX, DX, EX, FX
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) >> valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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
    int8_t cantBytes;
    if ( tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
    }
    else
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando( valorB );
    }
    else
        valorGuardadoB = valorB;
    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valorGuardadoA = lecturaEnMemoria( valorA, cantBytes );
        int32_t resultado = valorGuardadoA >> valorGuardadoB;
        escrituraEnMemoria( resultado, 4, valorA );
    }
    else{
        // no nos atajamos si nos viene un opA inmediato.
        valorGuardadoA = pseudonimoOperando(valorA);
        int32_t resultado;
        uint8_t nroRegistroA = valorA & 0x000000FF;
        uint8_t pseudonimo = (valorA >> 5) & 0x03;// Corro los 5 bits del nro de registro
        if ( pseudonimo == 0x01 ){
            resultado = (registros[nroRegistroA] & 0x000000FF) >> valorGuardadoB;
            registros[nroRegistroA] &= 0xFFFFFF00; 
            registros[nroRegistroA] |= (resultado & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                resultado = (((registros[nroRegistroA] >> 8) >> valorGuardadoB) & 0xFF) << 8;
                registros[nroRegistroA] = (registros[nroRegistroA] & 0xFFFF00FF) | resultado;
            }
        else
            if (pseudonimo == 0x3){
                resultado = ( registros[nroRegistroA] & 0x0000FFFF ) >> valorGuardadoB; 
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (resultado & 0x0000FFFF);
            }
        else
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

void LDL(  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int valorGuardadoB,valorGuardadoA;
    int8_t cantBytes;
    if (tipoOpB == 0x03){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
    }
    else
        if( tipoOpB == 0x01 ){
            valorGuardadoB = pseudonimoOperando(valorB);   
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
    int8_t cantBytes;
    if (tipoOpB == 0x03){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valorGuardadoB = lecturaEnMemoria( valorB,cantBytes );
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
    int8_t cantBytes;
    if (  tipoOpB == 0x03 ){
        cantBytes = ( valorB >> 5 ) & 0x00000007;
        valor = lecturaEnMemoria( valorB,cantBytes );
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
    
    if (valorA == 0xF){ //Faltan breakpoints
        if (hay_vmi)
         Breakpoint();
        return;
    }

    uint32_t dire_memoria = tabla_seg[ registros[EDX]>>16 ].base + ( registros[EDX] & 0x0000FFFF );
    uint16_t cant_valores = registros[ECX] & 0x00FF;
    uint16_t cantbytes = registros[ECX] >> 16;
    uint8_t modo_lectura = registros[EAX];
    if(valorA == 1){

        for(int i = 0; i < cant_valores; i++){
            printf("[%04X]: ", dire_memoria);
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

            printf("[%04X]: ", dire_memoria_imprimir);
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
                printf("0b");
                int comenzo = 0;
                for (int x = 31; x >= 0; x--) {
                    int bit = ((uint32_t)valor >> x) & 1;
                    if (bit) comenzo = 1;
                    if (comenzo || x == 0)   // x == 0: el 0 se imprime como "0"
                    printf("%d", bit);
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
    int8_t cantBytes;
    if( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valor = lecturaEnMemoria( valorA, cantBytes );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valor = pseudonimoOperando(valorA); 
    } else
        valor = valorA;
    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
    else
        error_actual = ERR_FALLO_SEGMENTO;
}

void JP( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;
    int8_t cantBytes;
    if(n == 0 && z == 0){
        int valor;
        if( tipoOpA == 0x03 ){        
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            uint8_t nroRegistroA = valorA & 0x0000001F;
            valor = registros[nroRegistroA]; 
        }else
            valor = valorA;
    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
    else
        error_actual = ERR_FALLO_SEGMENTO;
    }
}

void JN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int8_t cantBytes;
    if(n == 1){
        int valor;
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        if( tipoOpA == 0x03 ){
        valor = lecturaEnMemoria( valorA, cantBytes );
    }
    else if (tipoOpA == 0x01){
        uint8_t nroRegistroA = valorA & 0x0000001F;
        valor = registros[nroRegistroA]; 
    }else
        valor = valorA;
    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  z = (registros[CC] >> 30) & 1;
    int8_t cantBytes;
    if(z == 1){
        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando(valorA);
        }else{
            valor = valorA;
        }

        if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JC (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int c = (registros[CC] >> 29) & 1;
    int8_t cantBytes;
    if(c == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando(valorA);
        }else{
            valor = valorA;
        }
    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JV (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int v = (registros[CC] >> 28) & 1;
    int8_t cantBytes;
    if(v == 1){
    
        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;            
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando(valorA);
        }else{
            valor = valorA;
        }

    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JNP (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int  z = (registros[CC] >> 30) & 1;
    int8_t cantBytes;
    if(n == 1 || z == 1){

        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando(valorA);
        }else{
            valor = valorA;
        }

    if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JNN (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  n = (registros[CC] >> 31) & 1;
    int8_t cantBytes;
    if(n == 0){

        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando(valorA);
        }else{
            valor = valorA;
        }

        if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
         modificoIP(valor);
        else{
          error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void JNZ (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
    int  z = (registros[CC] >> 30) & 1;
    int8_t cantBytes;
    if(z == 0){

        int valor;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
        }
        else if (tipoOpA == 0x01){
            valor = pseudonimoOperando( valorA );
        }else{
           valor = valorA;
        }

        if(valor <= (tabla_seg[registros[IP] >> 16].tam + tabla_seg[registros[IP] >> 16].base)  && valor >= tabla_seg[registros[IP] >> 16].base)
            modificoIP(valor);
        else{
            error_actual = ERR_FALLO_SEGMENTO;
        }

    }
}

void NOT ( unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
        int valor;
        int8_t cantBytes;
        if( tipoOpA == 0x03 ){
            cantBytes = ( valorA >> 5 ) & 0x00000007;
            valor = lecturaEnMemoria( valorA, cantBytes );
            valor = ~valor;
            escrituraEnMemoria(valor, 4, valorA);
        }
        else if (tipoOpA == 0x01){
            valor = ~pseudonimoOperando(valorA);
        }else
            valor = ~valorA;

        registros[CC] &= 0x0FFFFFFF; // -> 0x xx xx xx limpio los primeros 4 bits
        if ( valor < 0 )
            registros[CC] |= 0x80000000;
        if ( valor == 0 )
            registros[CC] |= 0x40000000;     
}

//void ERROR (  unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB ){
//    error_actual = ERR_INSTRUCCION_INVALIDA;
//}

void STOP( unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ){
    registros[IP] = -1;
}

void POP(unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB){
    uint16_t seg = registros[SP] >> 16;

    if (seg != registros[SS] >> 16 || (registros[SP] & 0xFFFF) > tabla_seg[registros[SS] >> 16].tam){
        error_actual = ERR_FALLO_SEGMENTO;
        return;
    }

    if((registros[SP] & 0xFFFF) + 4 > tabla_seg[seg].tam){
        error_actual = ERR_UNDERFLOW;
        return;
    }

    uint32_t dirfisica = tabla_seg[seg].base + (registros[SP] & 0xFFFF);
    uint32_t valor = 0;
    for (int i = 0; i < 4; i++){
        valor = valor << 8;
        valor = valor | RAM[dirfisica + i];
    }
    registros[SP] += 4;

    uint8_t cantBytes = 0;

    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        escrituraEnMemoria( valor, cantBytes, valorA );
    }
    else{
        uint8_t nroRegistroA = valorA & 0x0000001F;
        uint8_t pseudonimo = valorA >> 5;
        pseudonimo = pseudonimo & 0x3;
        if ( pseudonimo == 0x01 ){
            registros[nroRegistroA] = registros[nroRegistroA] & 0xFFFFFF00;
            registros[nroRegistroA] |= (valor & 0x000000FF);
        }
        else
            if (pseudonimo == 0x02){
                registros[nroRegistroA] &= 0xFFFF00FF;
                int32_t aux = valor & 0x000000FF;
                aux = aux << 8;
                registros[nroRegistroA] |= aux;
            }
        else
            if (pseudonimo == 0x3){
                registros[nroRegistroA] &= 0xFFFF0000;
                registros[nroRegistroA] |= (valor & 0x0000FFFF);
            }
        else
            registros[nroRegistroA] = valor; 
    }
}


void RET(unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB){
    POP(1, 0, 0, 0); // tercer parametro es el IP da la casualidad que todo el valor es 0
}

void PUSH(unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB){
    uint16_t seg = registros[SP] >> 16;

    if (seg != registros[SS] >> 16 || (registros[SP] & 0xFFFF) > tabla_seg[registros[SS] >> 16].tam){
        error_actual = ERR_FALLO_SEGMENTO;
        return;
    }

    if((registros[SP] & 0xFFFF) < 4){ 
        error_actual = ERR_OVERFLOW;
        return;
    }

    uint32_t valor = 0;
    uint8_t cantBytes = 0;

    if ( tipoOpA == 0x03 ){
        cantBytes = ( valorA >> 5 ) & 0x00000007;
        valor = lecturaEnMemoria( valorA, cantBytes );
    }
    else if ( tipoOpA == 0x01 )
        valor = pseudonimoOperando( valorA );
    else
        valor = valorA; 

    registros[SP] -= 4;
    uint32_t dirfisica = tabla_seg[seg].base + (registros[SP] & 0xFFFF);
    for (int i = 0; i < 4; i++)
        RAM[dirfisica + i] = (valor >> (24 - 8 * i)) & 0xFF;
}

void CALL(unsigned char tipoOpA, unsigned char tipoOpB, int32_t valorA, int32_t valorB){
    PUSH(1, 0, 0, 0); // tercer parametro es el IP da la casualidad que todo el valor es 0
    JMP(tipoOpA, 0, valorA, 0); // llamo a JMP para que salte a la direccion inmediata valorA
}

void ( *operaciones[ 32 ] )(unsigned char opA, unsigned char opB, int32_t valorA, int32_t valorB ) 
= { SYS, JMP, JP, JN, JZ, JC, JV, JNP, JNN, JNZ, NOT, PUSH, POP, CALL, RET, STOP, MOV, ADD, SUB, MUL, DIV, CMP, AND, OR, XOR, SWAP, SHL, SHR, SAR, LDL, LDH, RND };