#include "main.c"

uint32_t ninguno(){return 0x0} // ERROR

uint32_t registro(){
        registros[0] = registros[0] + 1;
        return RAM[registros[0]];
}

uint32_t inmediato(){
    registros[0] = registros[0] + 1;

    uint32_t aux = RAM[registros[0]];
    registros[0] = registros[0] + 1;

    aux = aux << 8;
    aux = aux | RAM[registros[0]];

    return aux;
}

uint32_t memoria(){
    registros[0] = registros[0] + 1;

    uint32_t aux = RAM[registros[0]];
    registros[0] = registros[0] + 1;

    aux = aux << 8;
    aux = aux | RAM[registros[0]];
    registros[0] = registros[0] + 1;

    aux = aux << 8;
    aux = aux | RAM[registros[0]];

    return aux;
}


 typedef uint32_t (*tipo_operando[2])(); // Vector de punteros a funciones para rescatar el valor de cada operando
 tipo_operando p_tipo_op = {ninguno, registro, inmediato, memoria} // y para mover correctamente el IP

void dosOperandos(uint32_t registros[0]){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        tipo_opb = RAM[registros[0]] >> 6;
        tipo_opa = RAM[registros[0]] >> 4;
        cod_op = RAM[registros[0]] & 0x1F;

        valor_opa = p_tipo_op[tipo_opa]
        valor_opb = p_tipo_op[tipo_opb]

        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 

void unOperando(uint32_t registros[0]){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        
        tipo_opa = RAM[registros[0]] >> 6;
        cod_op = RAM[registros[0]] & 0x0F;

        valor_opa = p_tipo_op[tipo_opa]
        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 


void ningunOperando(uint32_t registros[0]){
        unsigned char tipo_opa;
        unsigned char tipo_opb;
        unsigned char cod_op;
        uint32_t valor_opa;
        uint32_t valor_opb;

        cod_op = RAM[registros[0]] & 0x0F;

        valor_opa = p_tipo_op[tipo_opa]
        // TipoOperacion[cod_op](tipoa, tipob, valora, valorb);
} 



void buscooperacion(uint32_t registros[0]){
        if((RAM[registros[0]] >> 4 & 0x01) == 1){
            dosOperandos(registros[0]);
        }else if(RAM[registros[0]] >> 6 != 0){
            unOperando();
        }else
             ningunOperando();
}