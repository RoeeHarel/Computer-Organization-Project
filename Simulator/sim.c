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
		case 0: // add
	        regs[inst->rd] = regs[inst->rs] + regs[inst->rt];
	        break;
	    case 1: // sub
	        regs[inst->rd] = regs[inst->rs] - regs[inst->rt];
	        break;
	    case 2: // mul
	        regs[inst->rd] = regs[inst->rs] * regs[inst->rt];
	        break;
	    case 3: // and
	        regs[inst->rd] = regs[inst->rs] & regs[inst->rt];
	        break;
	    case 4: // or
	        regs[inst->rd] = regs[inst->rs] | regs[inst->rt];
	        break;
	    case 5: // xor
	        regs[inst->rd] = regs[inst->rs] ^ regs[inst->rt];
	        break;
	    case 6: // sll - Shifting rs by the value in rt
	        regs[inst->rd] = regs[inst->rs] << regs[inst->rt];
	        break;
	    case 7: // sra (Shift Arithmetic Right) - Cast to signed int to ensure arithmetic shift (sign extension)
	        regs[inst->rd] = (int)regs[inst->rs] >> regs[inst->rt];
	        break;
	    case 8: // srl - Cast to unsigned int to ensure logical shift (zero fill)
	        regs[inst->rd] = (unsigned int)regs[inst->rs] >> regs[inst->rt];
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


int main(int argc, char *argv[])
{

	return 0;
}
