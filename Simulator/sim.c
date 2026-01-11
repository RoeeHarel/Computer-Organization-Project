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
unsigned int regs[16] = {0};
unsigned int mainMem[MEM_SIZE] = {0}; 
int branch_condition = FALSE; // A global variable used in the functions Execute() and AdvancePC()
Instruction new_inst = {}; // initializing the variable for later use in Fetch()

// --- Function Initialization ---

int inst_is_I_type(Instruction* inst);
// returns TRUE if the instruction is I-type, and FALSE if it's R-type

Instruction Fetch ();
// Fetches each instruction

void Execute (Instruction* inst);
// Executes each instruction

int AdvancePC(Instruction* inst);
// advances pc according to the branch condtiion and type of the current instruction


// --- Function Implementation ---

int inst_is_I_type (Instruction* inst) {
	return (inst->opcode >= 9 && inst->opcode <= 18) ? TRUE : FALSE;
}

Instruction Fetch () {
	regs[1] = inst->imm; // I think this needs to be here, it affects my part of the code (Roee)
}

void Execute(Instruction* inst) {
	int branch_condition = FALSE;
	
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
	    case 7: // sra - Cast to signed int to ensure arithmetic shift (sign extension)
	        regs[inst->rd] = (int)regs[inst->rs] >> regs[inst->rt];
	        break;
	    case 8: // srl - Cast to unsigned int to ensure logical shift (zero fill)
	        regs[inst->rd] = (unsigned int)regs[inst->rs] >> regs[inst->rt];
	        break;
		case 9: // beq
	        if (regs[inst->rs] == regs[inst->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 10: // bne
	        if (regs[inst->rs] != regs[inst->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 11: // blt
	        if ((int)regs[inst->rs] < (int)regs[inst->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 12: // bgt
	        if ((int)regs[inst->rs] > (int)regs[inst->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 13: // ble
	        if ((int)regs[inst->rs] <= (int)regs[inst->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 14: // bge
	        if ((int)regs[inst->rs] >= (int)regs[inst->rt])
	            branch_condition = TRUE;
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

	regs[0] = 0; // making sure $zero is unchanged
	regs[1] = inst->imm; // making sure $imm is unchanged
	}
}

AdvancePC(Instruction* inst) {
	// need to also check for branch condition
	pc += (inst_is_I_type(inst)) ? 2 : 1;
}


int main(int argc, char *argv[])
{
	// PC loop. maybe it's better to first read from memin.txt and finish the while loop at EOF
	while (pc < MEM_SIZE) {
		new_inst = Fetch();
		Execute(&new_inst);
	
		// advance PC by 2 if the instruction is I-type, or by 1 if it's R-type
		AdvancePC(&new_inst);
	}

	//write into all the output files at the end of the program loop

	return 0;
}
