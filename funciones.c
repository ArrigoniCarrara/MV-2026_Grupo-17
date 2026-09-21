#include "operaciones.c"

int32_t ninguno(int32_t *ip){return 0x0;} // ERROR


int32_t registro(int32_t *ip){
        *ip = *ip + 1;
        return RAM[*ip];
}

int32_t inmediato(int32_t *ip){
    *ip = *ip + 1;

    int16_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

int32_t memoria(int32_t *ip){
    *ip = *ip + 1;

    int32_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

 typedef int32_t (*tipo_operando[4])(int32_t *); // Vector de punteros a funciones para rescatar el valor de cada operando
 tipo_operando p_tipo_op = {ninguno, registro, inmediato, memoria}; // y para mover correctamente el IP

void dosOperandos(int32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;

        tipo_opb = (RAM[*ip] >> 6) & 0x03;// obtengo los dos bits mas significativos
        tipo_opa = (RAM[*ip] >> 4) & 0x03; // obtengo el bit 3 y 4 mas significativo
        cod_op = RAM[*ip] & 0x1F;// obtengo los 5 bits menos significativos

        if (cod_op < 0x10) { // los codigos validos de dos operandos van de 0x10 (MOV) a 0x1F (RND)
            error_actual = ERR_INSTRUCCION_INVALIDA;
            return;
        }

        valor_opb = p_tipo_op[tipo_opb](ip);
        valor_opa = p_tipo_op[tipo_opa](ip);

        registros[OPC] = cod_op;//OPC = codigo de operacion
        // Debo poner en el byte mas significativo el codigo del operando y en el resto el valor del operando

        registros[OP1] = tipo_opa; //OP1 = operando A
        registros[OP1] = registros[OP1] << 24;
        registros[OP1] = registros[OP1] | valor_opa;
        registros[OP2] = tipo_opb;// OP2 = operando B
        registros[OP2] = registros[OP2] << 24;
        registros[OP2] = registros[OP2] | valor_opb;
        
        printf("[TRACE] DOS OPERANDOS | CodOp: %02X | OpA (Tipo %d): %08X | OpB (Tipo %d): %08X\n", cod_op, tipo_opa, valor_opa, tipo_opb, valor_opb);
        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 

void unOperando(int32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb = 0; // no hay operando b en un operando
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;

        
        tipo_opa = (RAM[*ip] >> 6) & 0x03;
        cod_op = RAM[*ip] & 0x0F;

        if (cod_op > 0x0A) { // los codigos validos de un operando van de 0x00 (SYS) a 0x0A (NOT)
            error_actual = ERR_INSTRUCCION_INVALIDA;
            return;
        }

        valor_opa = p_tipo_op[tipo_opa](ip);

        registros[OPC] = cod_op; // OPC = codigo de operacion
        registros[OP1] = tipo_opa; //OP1 = operando A
        registros[OP1] = registros[OP1] << 24;
        registros[OP1] = registros[OP1] | valor_opa;
        registros[OP2] = tipo_opb;// OP2 = operando B

        printf("[TRACE] UN OPERANDO | CodOp: %02X | OpA (Tipo %d): %08X\n", cod_op, tipo_opa, valor_opa);
        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 


void ningunOperando(int32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0;
        int32_t valor_opb = 0;

        cod_op = RAM[*ip] & 0x0F;

        if (cod_op != 0x0F) { // el unico codigo valido sin operandos es STOP (0x0F)
            error_actual = ERR_INSTRUCCION_INVALIDA;
            return;
        }
        
        registros[OPC] = cod_op;// codigo de operacion
        
       operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
} 


void buscoOperacion(int32_t *ip){
    printf("\n------------------------------------------------\n");
    printf("[TRACE] Ejecutando IP: %04X | Opcode RAW: %02X\n", *ip, RAM[*ip]);
        if((RAM[*ip] >> 4 & 0x01) == 1){
            dosOperandos(ip);
        }else if(RAM[*ip] >> 6 != 0){
            unOperando(ip);
        }else
             ningunOperando(ip);
}