#include "operaciones.c"

int32_t ninguno(){} 

int32_t registro(){
        registros[IP] = registros[IP] + 1;
        return RAM[IPdirfisica()];
}

int32_t inmediato(){
    registros[IP] = registros[IP] + 1;

    int16_t aux = RAM[IPdirfisica()];
    registros[IP] = registros[IP] + 1;

    aux = aux << 8;
    aux = aux | RAM[IPdirfisica()];

    return aux;
}

int32_t memoria(){
    registros[IP] = registros[IP] + 1;

    int32_t aux = RAM[IPdirfisica()];
    registros[IP] = registros[IP] + 1;

    aux = aux << 8;
    aux = aux | RAM[IPdirfisica()];
    registros[IP] = registros[IP] + 1;

    aux = aux << 8;
    aux = aux | RAM[IPdirfisica()];

    return aux;
}

uint32_t empaquetarOperando( unsigned char tipo, int32_t valor ){
    uint32_t mascara;
    if ( tipo == 0x01 )
        mascara = 0x000000FF;
    else
        if ( tipo == 0x02 )
            mascara = 0x0000FFFF;
        else
            if ( tipo == 0x03 )
                mascara = 0x00FFFFFF;
            else
                mascara = 0;
    return ( (uint32_t) tipo << 24 ) | ( (uint32_t) valor & mascara );
}

 typedef int32_t (*tipo_operando[4])(); // Vector de punteros a funciones para rescatar el valor de cada operando
 tipo_operando p_tipo_op = {ninguno, registro, inmediato, memoria}; // y para mover correctamente el IP

void dosOperandos(){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;

        tipo_opb = (RAM[IPdirfisica()] >> 6) & 0x03;// obtengo los dos bits mas significativos
        tipo_opa = (RAM[IPdirfisica()] >> 4) & 0x03; // obtengo el bit 3 y 4 mas significativo
        cod_op = RAM[IPdirfisica()] & 0x1F;// obtengo los 5 bits menos significativos

        //if (cod_op < 0x10) { // los codigos validos de dos operandos van de 0x10 (MOV) a 0x1F (RND)
        //    error_actual = ERR_INSTRUCCION_INVALIDA;
        //    return;
        //}

        valor_opb = p_tipo_op[tipo_opb]();
        valor_opa = p_tipo_op[tipo_opa]();

        registros[OPC] = cod_op;//OPC = codigo de operacion
        // Debo poner en el byte mas significativo el codigo del operando y en el resto el valor del operando

        registros[OP1] = empaquetarOperando( tipo_opa, valor_opa );
        registros[OP2] = empaquetarOperando( tipo_opb, valor_opb );

        if (registros[IP] != -1 && error_actual == ERR_NINGUNO){
            registros[IP] = registros[IP] + 1;
        }

        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 

void unOperando(){
        unsigned char tipo_opa;
        unsigned char tipo_opb = 0; // no hay operando b en un operando
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;

        
        tipo_opa = (RAM[IPdirfisica()] >> 6) & 0x03;
        cod_op = RAM[IPdirfisica()] & 0x0F;

        //if (cod_op > 0x0A) { // los codigos validos de un operando van de 0x00 (SYS) a 0x0A (NOT)
        //    error_actual = ERR_INSTRUCCION_INVALIDA;
        //    return;
        //}

        valor_opa = p_tipo_op[tipo_opa]();

        registros[OPC] = cod_op; // OPC = codigo de operacion
        registros[OP1] = empaquetarOperando( tipo_opa, valor_opa );
        registros[OP2] = 0;

        if (registros[IP] != -1 && error_actual == ERR_NINGUNO){
            registros[IP] = registros[IP] + 1;
        }

        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 


void ningunOperando(){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0;
        int32_t valor_opb = 0;

        cod_op = RAM[IPdirfisica()] & 0x0F;

        //if (cod_op != 0x0F) { // el unico codigo valido sin operandos es STOP (0x0F)
        //    error_actual = ERR_INSTRUCCION_INVALIDA;
        //    return;
        //}
        
        registros[OPC] = cod_op;// codigo de operacion
        registros[OP1] = 0;
        registros[OP2] = 0;
        
        if (registros[IP] != -1 && error_actual == ERR_NINGUNO){
            registros[IP] = registros[IP] + 1;
        }

       operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 


void buscoOperacion(){
        if((RAM[IPdirfisica()] >> 4 & 0x01) == 1){
            dosOperandos();
        }else if(RAM[IPdirfisica()] >> 6 != 0){
            unOperando();
        }else
             ningunOperando();
}