#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#define TRUE 1
#define FALSE 0
#define MEM_SIZE 4096
#define DISK_SIZE 16384	//128 sectors * 128 lines
	
//Struct for commands fetched from memin.txt
typedef struct Instruction {
	unsigned int opcode; // 8 bits
	unsigned int rd;     // 4 bits
	unsigned int rs;     // 4 bits
	unsigned int rt;     // 4 bits
	int imm;    // 20 bits (signed)
	int is_itype;
} Instruction;

// --- Variable Initialization ---
int pc = 0;
unsigned int regs[16] = {0};   // R0-R15
unsigned int io_regs[23] = {0};
unsigned int mainMem[MEM_SIZE] = {0}; 
unsigned int hard_disk_arr[DISK_SIZE] = {0};
unsigned int irq2_interrupt_cycles[MEM_SIZE] = { 0 }; // array of all the pc which has irq2in interrupt

int branch_condition = FALSE; // A global variable used in the functions Execute() and AdvancePC()
Instruction current_inst ; // initializing the variable for later use in Fetch()
int interrupt_flag = 0;   // 1 if an interrupt occurred
int mem_mask = 0xfffff; // ensures only the first 20 bits of a 32 bit integer are written into main memory

// --- Function Initialization ---

int inst_is_I_type(Instruction* inst);
// returns TRUE if the instruction is I-type, and FALSE if it's R-type

int signExtension(int var)
// extends a 20 bit signed integer with 12 leading zeros, into a 32 bit signed integer

Instruction Fetch ();
// Fetches each instruction

void Execute (Instruction* inst);
// Executes each instruction

void AdvancePC(Instruction* inst);
// advances pc according to the branch condition and type of the current instruction

//Fills mainMem array, based on memin.txt file
void FillmainMem(FILE * pmemin);

//Fill hard_disk_arr, based on diskin.txt file
void FillDiskinArr(FILE * pdiskin);

//Fills irq2_interrupt_cycles array, based on irq2in.txt file
void FillIrq2inArr(FILE * pirq2in);

// --- Function Implementation ---

// Setup Functions //

int inst_is_I_type (Instruction* inst) {
	return (inst->opcode >= 9 && inst->opcode <= 18) ? TRUE : FALSE;
}

int signExtension(int val) {
    return (val << 12) >> 12; // shifts the 20-bit value to the MSB, and copies the MSB for the first 12 digits to extend the sign
}

void FillmainMem(FILE * pmemin) //TODO
{

}

void FillDiskinArr(FILE * pdiskin)//TODO
{
	
}

void FillIrq2inArr(FILE * pirq2in)//TODO
{
	
}
void SetArrays(FILE * pdiskin, FILE * pmemin, FILE * pirq2in)
{
	FillmainMem(pmemin);
	FillDiskinArr(pdiskin); 
	FillIrq2inArr(pirq2in);
	FillMonitorArr();//should be done in another place?
}
// Main process functions//

Instruction Fetch () {
	regs[1] = inst->imm; // I think this needs to be here, it affects my part of the code (Roee)
}

void Execute(Instruction* inst_ptr) {
	// Reset the global variable branch_condition
	branch_condition = FALSE;
	int addr = regs[inst_ptr->rs] + regs[inst_ptr->rt];
	
	switch (inst_ptr->opcode) {
		case 0: // add
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] + regs[inst_ptr->rt];
	        break;
	    case 1: // sub
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] - regs[inst_ptr->rt];
	        break;
	    case 2: // mul
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] * regs[inst_ptr->rt];
	        break;
	    case 3: // and
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] & regs[inst_ptr->rt];
	        break;
	    case 4: // or
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] | regs[inst_ptr->rt];
	        break;
	    case 5: // xor
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] ^ regs[inst_ptr->rt];
	        break;
	    case 6: // sll - Shifting rs by the value in rt
	        regs[inst_ptr->rd] = regs[inst_ptr->rs] << regs[inst_ptr->rt];
	        break;
	    case 7: // sra - Cast to signed int to ensure arithmetic shift (sign extension)
	        regs[inst_ptr->rd] = (int)regs[inst_ptr->rs] >> regs[inst_ptr->rt];
	        break;
	    case 8: // srl - Cast to unsigned int to ensure logical shift (zero fill)
	        regs[inst_ptr->rd] = (unsigned int)regs[inst_ptr->rs] >> regs[inst_ptr->rt];
	        break;
		case 9: // beq
	        if (regs[inst_ptr->rs] == regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 10: // bne
	        if (regs[inst_ptr->rs] != regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 11: // blt
	        if ((int)regs[inst_ptr->rs] < (int)regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 12: // bgt
	        if ((int)regs[inst_ptr->rs] > (int)regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 13: // ble
	        if ((int)regs[inst_ptr->rs] <= (int)regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	
	    case 14: // bge
	        if ((int)regs[inst_ptr->rs] >= (int)regs[inst_ptr->rt])
	            branch_condition = TRUE;
	        break;
	    case 15: // jal
			regs[inst_ptr->rd] = pc + (inst_is_I_type(inst)) ? 2 : 1; // copied from AdvancePC()
			pc = regs[inst_ptr->rs];
	        break;
		case 16: // lw (Load Word)
            if (inst_ptr->rd > 1) // prevents writing into $zero and $imm
			{
                regs[inst_ptr->rd] = signExtension(mainMem[addr]);
            }
            break;

        case 17: // sw (Store Word)
            mainMem[addr] = regs[inst_ptr->rd] & mem_mask;
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
    
    regs[0] = 0; // making sure $zero is unchanged
    regs[1] = inst_ptr->imm; // making sure $imm is unchanged
}

void AdvancePC(Instruction* inst) {
	if (branch_condition == TRUE)
		pc = regs[inst->rd];
	else pc += (inst_is_I_type(inst)) ? 2 : 1;
}


int main(int argc, char *argv[])
{
	FILE *memin = fopen(argv[1], "r"), *diskin = fopen(argv[2], "r"), *irq2in = fopen(argv[3], "r");//should add a write to all the output files
	SetArrays(diskin, memin, irq2in);// I(Tal) think it should be here

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
