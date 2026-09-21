#include "operaciones.c"
/*
char *mnemonicos[] = {
    "SYS", "JMP", "JP", "JN", "JZ", "JC", "JV", "JNP", "JNN", "JNZ", 
    "NOT", "ERROR", "ERROR", "ERROR", "ERROR", "STOP", "MOV", "ADD", 
    "SUB", "MUL", "DIV", "CMP", "AND", "OR", "XOR", "SWAP", "SHL", 
    "SHR", "SAR", "LDL", "LDH", "RND"
};
char *registrosNombres[] = {
    
    "IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "", "", "",
    "EAX", "EBX", "ECX", "EDX", "EEX", "EFX",
    "AC", "CC", "", "", "", "", "", "", "", "",
    "CS", "DS", "", "", "", ""
};
*/
int32_t ninguno(int32_t *ip){return 0x0;} // ERROR


int32_t registro(int32_t *ip){
        
        
        *ip = *ip + 1;
        //printf( "  %02X  ", RAM[*ip] );
        return RAM[*ip];
}

int32_t inmediato(int32_t *ip){
        //for (int i=*ip; i<=2; i++)
           // printf( "  %02X  ", RAM[i] );
    *ip = *ip + 1;

    int16_t aux = RAM[*ip];
    *ip = *ip + 1;

    aux = aux << 8;
    aux = aux | RAM[*ip];

    return aux;
}

int32_t memoria(int32_t *ip){
    //for (int i=*ip; i<=3; i++)
        //printf( "  %02X  ", RAM[i] );
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
        //printf( "[%04X]", *ip  );
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;
        tipo_opb = (RAM[*ip] >> 6) & 0x03;// obtengo los dos bits mas significativos
        tipo_opa = (RAM[*ip] >> 4) & 0x03; // obtengo el bit 3 y 4 mas significativo
        cod_op = RAM[*ip] & 0x1F;// obtengo los 5 bits menos significativos
        
        //printf("\n");
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
        
        //printf("[TRACE] DOS OPERANDOS | CodOp: %02X | OpA (Tipo %d): %08X | OpB (Tipo %d): %08X\n", cod_op, tipo_opa, valor_opa, tipo_opb, valor_opb);
        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);

     /*      
        int8_t auxCodOp = cod_op & 0b00011111;

        printf( "%s", mnemonicos[auxCodOp]);
        if ( tipo_opa == 1 ){
            printf( "    %s   ,",registrosNombres[valor_opa & 0x00001F] );

        }
        else{// son de memoria, tengo que ver si es un registro o es un inmediato, valor_opb trae 00 nn nn RR
            int8_t auxReg = valor_opa & 0x00001F; // auxReg = -- -- -- RR
            int16_t auxNro = valor_opa >> 8;// auxNro = -- -- nn nn
            if ( auxReg == DS )
                printf(" [%d] ,", auxNro);
            else
                 printf(" [%d] ,", auxReg);

        }    
        if ( tipo_opb == 1 )
            printf("%s", registrosNombres[valor_opb & 0x00001F]);
        else
            if ( tipo_opb == 2 )
                printf("%d", valor_opb);
        else 
            {// son de memoria, tengo que ver si es un registro o es un inmediato, valor_opb trae 00 nn nn RR
            int8_t auxReg = valor_opb & 0x00001F; // aux = -- -- -- RR
            int16_t auxNro = valor_opb >> 8;
            if ( auxReg == DS )
                printf(" [%d]\n ", auxNro);
            else    
                printf(" [%d] \n", auxReg);
        }
    */

} 

void unOperando(int32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb = 0; // no hay operando b en un operando
        unsigned char cod_op;
        int32_t valor_opa = 0x0;
        int32_t valor_opb = 0x0;

        
        tipo_opa = (RAM[*ip] >> 6) & 0x03;
        cod_op = RAM[*ip] & 0x0F;

        valor_opa = p_tipo_op[tipo_opa](ip);

        registros[OPC] = cod_op; // OPC = codigo de operacion
        registros[OP1] = tipo_opa; //OP1 = operando A
        registros[OP1] = registros[OP1] << 24;
        registros[OP1] = registros[OP1] | valor_opa;
        registros[OP2] = tipo_opb;// OP2 = operando B

        printf("[TRACE] UN OPERANDO | CodOp: %02X | OpA (Tipo %d): %08X\n", cod_op, tipo_opa, valor_opa);
        operaciones[cod_op](tipo_opa, tipo_opb, valor_opa, valor_opb);
     /*  
        int8_t auxCodOp = cod_op & 0b00011111;

        printf( "%s", mnemonicos[auxCodOp]);
        if ( tipo_opa == 1 ){
            printf( "    %s   ,",registrosNombres[valor_opa & 0x00001F] );

        }
        else{// son de memoria, tengo que ver si es un registro o es un inmediato, valor_opb trae 00 nn nn RR
            int8_t auxReg = valor_opa & 0x00001F; // auxReg = -- -- -- RR
            int16_t auxNro = valor_opa >> 8;// auxNro = -- -- nn nn
            if ( auxReg == DS )
                printf(" [%d] ,", auxNro);
            else
                 printf(" [%d] ,", auxReg);

        }  
    */  
} 


void ningunOperando(int32_t *ip){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        int32_t valor_opa = 0;
        int32_t valor_opb = 0;

        cod_op = RAM[*ip] & 0x0F;
        registros[1] = cod_op;// codigo de operacion
        
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