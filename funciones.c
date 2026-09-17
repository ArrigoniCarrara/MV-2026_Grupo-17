#include "operaciones.c"

uint32_t ninguno(uint32_t *ip){return 0x0;} // ERROR



uint32_t registro(uint32_t *ip){
        *ip = *ip + 1;
        return RAM[*ip];
}
//  porque no es de 16 bits? porque tenemos hardware mejor que el de los 80s
uint32_t inmediato(uint32_t *ip){
    *ip = *ip + 1;

    uint32_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

uint32_t memoria(uint32_t *ip){
    *ip = *ip + 1;

    uint32_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

 typedef uint32_t (*tipo_operando[4])(uint32_t *); // Vector de punteros a funciones para rescatar el valor de cada operando
 tipo_operando p_tipo_op = {ninguno, registro, inmediato, memoria}; // y para mover correctamente el IP

void dosOperandos(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa = 0x0;
        uint32_t valor_opb = 0x0;

        tipo_opb = (RAM[*ip] >> 6) & 0x03;// obtengo los dos bits mas significativos
        tipo_opa = (RAM[*ip] >> 4) & 0x03; // obtengo el bit 3 y 4 mas significativo
        cod_op = RAM[*ip] & 0x1F;// obtengo los 5 bits menos significativos

        valor_opa = p_tipo_op[tipo_opa](ip);
        valor_opb = p_tipo_op[tipo_opb](ip);

        registros[OPC] = cod_op;//OPC = codigo de operacion
        
        printf("CODIGO DE OPERACION \n");
        printf( "%02X \n",cod_op  );
        // Debo poner en el byte mas significativo el codigo del operando y en el resto el valor del operando

        registros[OP1] = tipo_opa; //OP1 = operando A
        registros[OP1] = registros[OP1] << 24;
        registros[OP1] = registros[OP1] | valor_opa;
        registros[OP2] = tipo_opb;// OP2 = operando B
        registros[OP2] = registros[OP2] << 24;
        registros[OP2] = registros[OP2] | valor_opb;
        // tipoOperacionDosOperando[cod_op](tipoa, tipob, valora, valorb);
} 

void unOperando(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa = 0x0;
        uint32_t valor_opb = 0x0;

        
        tipo_opa = (RAM[*ip] >> 6) & 0x03;
        cod_op = RAM[*ip] & 0x0F;

        valor_opa = p_tipo_op[tipo_opa](ip);

        printf("CODIGO DE OPERACION  \n");
        printf( "%02X  \n",cod_op  );

        registros[OPC] = cod_op; // OPC = codigo de operacion
        registros[OP1] = tipo_opa; //OP1 = operando A
        registros[OP1] = registros[OP1] << 24;
        registros[OP1] = registros[OP1] | valor_opa;
        registros[OP2] = tipo_opb;// OP2 = operando B
        // tipoOperacionUnOperando[cod_op](tipoa, tipob, valora, valorb);
} 


void ningunOperando(uint32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa = 0;
        uint32_t valor_opb = 0;

        cod_op = RAM[*ip] & 0x0F;
        
        printf("CODIGO DE OPERACION  \n");
        printf( "%02X \n",cod_op  );
        registros[1] = cod_op;// codigo de operacion
       // tipoOperacionMingunOperando[cod_op](tipoa, tipob, valora, valorb);
} 


// preguntar a gian si buscaOperacion deja el IP en la proxima instruccion
void buscoOperacion(uint32_t *ip){
        if((RAM[*ip] >> 4 & 0x01) == 1){
            dosOperandos(ip);
        }else if(RAM[*ip] >> 6 != 0){
            unOperando(ip);
        }else
             ningunOperando(ip);
}