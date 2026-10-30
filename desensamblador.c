#include <stdio.h>
#include "funciones.c"

char *NOMBRE_REG[32] = {
    "IP","OPC","OP1","OP2","LAR","MAR","MBR","SP","BP","-",
    "EAX","EBX","ECX","EDX","EEX","EFX","AC","CC","-","-",
    "-","-","-","-","-","-","CS","DS","ES","SS","KS","PS"
};

char *MNEM_UNO[] = {
    "SYS","JMP","JP","JN","JZ","JC","JV","JNP","JNN","JNZ","NOT","PUSH","POP","CALL"
};

char *MNEM_DOS[] = {
    "MOV","ADD","SUB","MUL","DIV","CMP","AND","OR","XOR","SWAP",
    "SHL","SHR","SAR","LDL","LDH","RND"
};

// Nombre de un registro segun su codigo y el "pseudonimo" (bits 5-6 del operando):
//   0 -> completo (EAX)   1 -> byte bajo (AL)   2 -> byte alto (AH)   3 -> 2 bytes bajos (AX)
// Los pseudonimos solo existen para EAX..EFX (codigos 10 a 15).
void nombreRegistro(char *buffer, uint8_t reg, uint8_t pseudonimo){
    if (reg >= EAX && reg <= EFX && pseudonimo != 0){
        char letra = 'A' + (reg - EAX);
        char sufijo = (pseudonimo == 1) ? 'L' : (pseudonimo == 2) ? 'H' : 'X';
        sprintf(buffer, "%c%c", letra, sufijo);
    }
    else
        sprintf(buffer, "%s", NOMBRE_REG[reg & 0x1F]);
}

void formatearOperando(char *buffer, unsigned char tipo, int32_t valor){
    switch (tipo) {
        case 0x00: // ninguno
            buffer[0] = '\0';
            break;
        case 0x01: { // registro (con pseudonimo)
            nombreRegistro(buffer, valor & 0x1F, (valor >> 5) & 0x03);
            break;
        }
        case 0x02: { // inmediato (o rotulo/constante ya resuelto por el traductor)
            int16_t inm = (int16_t) valor;
            sprintf(buffer, "%d", inm);
            break;
        }
        case 0x03: { // memoria: [REG+offset] con modificador de tamaño b / w / l
            uint8_t reg = valor & 0x1F;
            uint8_t cant = (valor >> 5) & 0x07;
            int16_t offset = (int16_t)(valor >> 8);
            char *modificador = (cant == 1) ? "b" : (cant == 2) ? "w" : ""; // 4 bytes es el valor por defecto
            if (offset > 0)
                sprintf(buffer, "%s[%s+%d]", modificador, NOMBRE_REG[reg], offset);
            else if (offset < 0)
                sprintf(buffer, "%s[%s%d]", modificador, NOMBRE_REG[reg], offset); // el signo va incluido en %d
            else
                sprintf(buffer, "%s[%s]", modificador, NOMBRE_REG[reg]);
            break;
        }
    }
}

// Muestra las cadenas del Const Segment: [direccion] bytes en hexa | "texto"
void mostrarConstantes(){
    if (registros[KS] == 0xFFFFFFFF)       // no hay Const Segment
        return;

    uint32_t i = tabla_seg[registros[KS] >> 16].base;
    uint32_t fin = i + tabla_seg[registros[KS] >> 16].tam;

    while (i < fin){
        uint32_t inicio = i;
        while (i < fin && RAM[i] != 0x00)
            i++;
        if (i < fin)
            i++;                           // incluyo el '\0' final
        uint32_t largo = i - inicio;

        char strBytes[32] = "";
        int pos = 0;
        if (largo > 7){                    // muestro los primeros 6 bytes y ".."
            for (uint32_t k = 0; k < 6; k++)
                pos += snprintf(strBytes + pos, sizeof(strBytes) - pos, "%02X ", RAM[inicio + k]);
            snprintf(strBytes + pos, sizeof(strBytes) - pos, "..");
        }
        else{
            for (uint32_t k = 0; k < largo; k++)
                pos += snprintf(strBytes + pos, sizeof(strBytes) - pos, "%02X ", RAM[inicio + k]);
        }

        printf("  [%04X] %-24s| \"", inicio, strBytes);
        for (uint32_t k = inicio; k < i && RAM[k] != 0x00; k++){
            if (RAM[k] >= 32 && RAM[k] <= 126)
                printf("%c", RAM[k]);
            else
                printf(".");               // no imprimible
        }
        printf("\"\n");
    }
}

void desensamblar(){
    mostrarConstantes();
    if (registros[CS] == 0xFFFFFFFF)       // no hay Code Segment
        return;

    uint32_t ipGuardado = registros[IP];   // el desensamblado usa el IP como cursor: lo restauro al final

    int32_t ipFisico = -1;
    if (ipGuardado != 0xFFFFFFFF)
        ipFisico = tabla_seg[ipGuardado >> 16].base + (ipGuardado & 0xFFFF);

    registros[IP] = registros[CS];         // arranca en el primer byte del segmento de código

    while ((registros[IP] & 0xFFFF) < tabla_seg[registros[IP] >> 16].tam){
        int32_t inicio = IPdirfisica();    // dirección física de la instrucción
        unsigned char primerByte = RAM[inicio];
        unsigned char tipo_opa, tipo_opb, cod_op;
        int32_t valor_opa = 0, valor_opb = 0;
        char strOpA[32] = "", strOpB[32] = "";
        char strBytes[32] = "";
        char strOperandos[72] = "";
        char *mnem;

        if ((primerByte >> 4 & 0x01) == 1){ // dos operandos
            tipo_opb = (primerByte >> 6) & 0x03;
            tipo_opa = (primerByte >> 4) & 0x03;
            cod_op = primerByte & 0x1F;

            valor_opb = p_tipo_op[tipo_opb]();
            valor_opa = p_tipo_op[tipo_opa]();

            mnem = (cod_op >= 0x10) ? MNEM_DOS[cod_op - 0x10] : "??";
            formatearOperando(strOpA, tipo_opa, valor_opa);
            formatearOperando(strOpB, tipo_opb, valor_opb);
            snprintf(strOperandos, sizeof(strOperandos), "%s, %s", strOpA, strOpB);

        } else if (primerByte >> 6 != 0){ // un operando
            tipo_opa = (primerByte >> 6) & 0x03;
            cod_op = primerByte & 0x0F;

            valor_opa = p_tipo_op[tipo_opa]();

            mnem = (cod_op <= 0x0D) ? MNEM_UNO[cod_op] : "??"; // 0x0E y 0x0F no llevan operando
            formatearOperando(strOpA, tipo_opa, valor_opa);
            snprintf(strOperandos, sizeof(strOperandos), "%s", strOpA);

        } else { // sin operandos
            cod_op = primerByte & 0x0F;
            mnem = (cod_op == 0x0E) ? "RET" : (cod_op == 0x0F) ? "STOP" : "??";
            strOperandos[0] = '\0';
        }

        int pos = 0;
        for (int i = inicio; i <= IPdirfisica(); i++)
            pos += snprintf(strBytes + pos, sizeof(strBytes) - pos, "%02X ", RAM[i]);

        printf("%s[%04X] %-24s| %-6s %s\n", (inicio == ipFisico) ? "> " : "  ", inicio, strBytes, mnem, strOperandos);

        registros[IP]++; 
    }
    printf("----------------------------\n\n");

    registros[IP] = ipGuardado;
}