#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define MEM_SIZE 4096

typedef struct Instruction {
	unsigned int opcode;
	unsigned int rs;
	unsigned int rt;
	unsigned int rd;
	unsigned int imm;
} Instruction;

// --- Variable Initialization ---
int pc = 0;


// --- Function Initialization ---

int inst_is_I_type(Instruction* inst);
// returns TRUE if the instruction is I-type, and FALSE if it's R-type

void Fetch ();
// Fetches each instruction

void Execute (Instruction* inst);
// Executes each instruction


// --- Function Implementation ---

int inst_is_I_type (Instruction* inst) {
	return (inst->opcode >= 9 && inst->opcode <= 18) ? TRUE : FALSE;
}

void Fetch () {
}

void Execute(Instruction* inst) {
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
	    default: // need to implement
	        break;
	}
}


int main(int argc, char *argv[])
{
	// PC loop. maybe it's better to first read from memin.txt and finish the while loop at EOF
	while (pc < MEM_SIZE) {
		Fetch();
		Execute();
	
		// advance PC by 2 if the instruction is I-type, or by 1 if it's R-type
		pc += (inst_is_I_type(Instruction* inst)) ? 2 : 1;
	}

	//write into all the output files at the end of the program loop

	
	return 0;
}
