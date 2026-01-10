#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

typedef struct Instruction {
	unsigned int opcode;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	unsigned int imm;
} Instruction;

// Executes each instruction
void Execute (Instruction* inst)


void Execute(Instruction* inst){
	switch (inst->opcode) {
	switch (opcode) {
    case 0: // add
        break;
    case 1: // sub
        break;
    case 2: // mul
        break;
    case 3: // and
        break;
    case 4: // or
        break;
    case 5: // xor
        break;
    case 6: // sll
        break;
    case 7: // sra
        break;
    case 8: // srl
        break;
    case 9: // beq
        break;
    case 10: // bne
        break;
    case 11: // blt
        break;
    case 12: // bgt
        break;
    case 13: // ble
        break;
    case 14: // bge
        break;
    case 15: // jal
        break;
    case 16: // lw
        break;
    case 17: // sw
        break;
    case 18: // reti
        break;
    case 19: // in
        break;
    case 20: // out
        break;
    case 21: // halt
        break;
    default:
        break;
}
	}
}


int main(int argc, char *argv[])
{

	return 0;
}
