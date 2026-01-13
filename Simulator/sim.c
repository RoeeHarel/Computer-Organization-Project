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
int pc_mask = 0xfff; // ensures pc remains 12-bits long
unsigned int regs[16] = {0};   // R0-R15
unsigned int io_regs[23] = {0}; // has to be unsigned int because we are using it bitwise
unsigned int mainMem[MEM_SIZE] = {0}; 
unsigned int hard_disk_arr[DISK_SIZE] = {0};
unsigned int irq2_cycles[MEM_SIZE] = { 0 }; // array of all the pc which has irq2in interrupt

int branch_condition = FALSE; // A global variable used in the functions Execute() and AdvancePC()
Instruction current_inst ; // initializing the variable for later use in Fetch()
int interrupt_flag = 0;   // 1 if an interrupt occurred
int clock_cycle = 0;	  // Global clock cycle counter
int halt_condition = 0;  // Flag tp stop the simulator
int mem_mask = 0xfffff; // ensures only the first 20 bits of a 32 bit integer are written into main memory

// --- Function Initialization ---

int inst_is_I_type(Instruction* inst);//should be removed, implemented by Tal on the structure
// returns TRUE if the instruction is I-type, and FALSE if it's R-type

int signExtension(int var)
// extends a 20 bit signed integer with 12 leading zeros, into a 32 bit signed integer

Instruction Fetch ();
// Fetches each instruction

void Execute (Instruction* inst);
// Executes each instruction

void AdvancePC(Instruction* inst);
// advances pc according to the branch condition and type of the current instruction

void AdvanceClock();
// Increases clock by 1, accounts for 32-bit overflow

//Fills mainMem array, based on memin.txt file
void FillmainMem(FILE * pmemin);

//Fill hard_disk_arr, based on diskin.txt file
void FillDiskinArr(FILE * pdiskin);

//Fills irq2_interrupt_cycles array, based on irq2in.txt file
void FillIrq2inArr(FILE * pirq2in);

//Checks if there is an interrupt
int CheckInterrupts();

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

int CheckInterrupts() //need to be called before fetch() on main
{
	//Timer Logic
	if (io_regs[11] == 1) //timerenable
	{
		if (io_regs[12] >= io_regs[13]) // timercurrent >= timermax
		{
			io_regs[3] = 1 //Set irq0status to 1
			io_regs[12] = 0 // Reset timercurrent
		}
		else
			io_regs[12]++  // Increment timercurrent
	}
	// 2. Disk Logic (IRQ1) NEED to be done TODO 
	// --- 2. Disk Logic (Associated with IRQ1) ---
    // Note for Execute Function: 
    // When Disk Command finishes, it must set IOReg[IO_IRQ1_STATUS] = 1
    // (Meaning: IOReg[4] = 1)
    // Safety fallback (if Execute doesn't set it directly, though it should):
    // if (disk_finished_condition) IOReg[IO_IRQ1_STATUS] = 1;

	
	//External Interrupt (IRQ2) Logic
	if (clock_cycle < MEM_size && irq2_cycles[clockcycle])  // Check if an interrupt is scheduled for the current clock cycle
	{
		io_regs[5] = 1 //Set irq2status to 1
	}

	if (interrupt_flag) // If we are already inside an Interrupt Routine, do not interrupt again
		return 0;
	// Check distinct interrupts: (Enable == 1 AND Status == 1)
    
    int irq0_triggered = (io_regs[0] && IOReg[3]); // irq0_enable and irq0status
    int irq1_triggered = (io_regs[1] && IOReg[4]); // irq1_enable and irq1status
    int irq2_triggered = (io_regs[2] && IOReg[5]); // irq2_enable and irq2status
	if (irq0_triggered || irq1_triggered || irq2_triggered) 
	{
		interrupt_flag = 1 //enter Interrupt Routine
		io_regs[7] = pc;// Save Return Address (pc) to the new location
        pc = io_regs[6];// Jump to Handler Address from the new location 
		return 1; // pc changed
		
	}
	return 0
	
}

Instruction Fetch () {
	regs[1] = inst->imm; // I think this needs to be here, it affects my part of the code (Roee)
	if (pc >= MEM_SIZE) exit(1); //handling out of range scenario
	unsigned int line_inst = mainMem[pc];
	current_inst.opcode = (line_inst>>12)& 0xFF; //bits 12-19
	current_inst.rd = (line_inst>>8)& 0xF; //bits 8-11
	current_inst.rs = (line_inst>>4)& 0xF; //bits 4-7
	current_inst.rt = line_inst & 0xF; //bits 0-3
	if (current_inst.rd == 1 || current_inst.rs == 1 || current_inst.rt == 1) //if one of te registers is imm, it is of i type
		current_inst.is_itype = 1;
	else current_inst.is_itype = 0;
	if (current_inst.is_itype = 1) 
	{
		if (pc + 1 >= MEMSIZE)
			exit(1);
		int line_imm = mainMem[pc+1];
		if (line_imm & 0x80000) // sign extension
			current_inst.imm = line_imm || 0xFFF00000 ;
		else
			current_inst.imm = line_imm
		regs[1] = current_inst.imm;
	}
	else
		current_inst.imm = 0;
	return current_inst;
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
                regs[inst_ptr->rd] = signExtension(mainMem[addr]);
            break;
        case 17: // sw (Store Word)
            mainMem[addr] = regs[inst_ptr->rd] & mem_mask;
            break;
		case 18: // reti
	        pc = io_regs[2];
	        break;
	    case 19: // in
	        if (addr >= 0 && addr < 23) {
	            if (inst_ptr->rd != 0)
	                regs[inst_ptr->rd] = io_regs[addr];
			}
			break;
	    case 20: // out
	        if (addr >= 0 && addr < 23)
	            io_regs[addr] = regs[inst_ptr->rd];
	        break;
	    case 21: // halt
			exit(0);
	        break;
	}
    
    regs[0] = 0; // making sure $zero is unchanged
    regs[1] = inst_ptr->imm; // making sure $imm is unchanged
}

void AdvancePC(Instruction* inst) {
	if (branch_condition == TRUE)
		pc = regs[inst->rd] & pc_mask;
	else pc = ((pc + ((inst_is_I_type(inst)) ? 2 : 1)) | pc_mask);
}

void AdvanceClock(){
	io_registers[8]++; // no need for overflow checks, when clock reaches 0xFFFFFFFF and 1 is added, it overflows back to 0.
}


int main(int argc, char *argv[])
{
	FILE *memin = fopen(argv[1], "r"), *diskin = fopen(argv[2], "r"), *irq2in = fopen(argv[3], "r");//should add a write to all the output files
	SetArrays(diskin, memin, irq2in);// I(Tal) think it should be here

	// PC loop. maybe it's better to first read from memin.txt and finish the while loop at EOF
	while (pc < MEM_SIZE) {
		new_inst = Fetch();
		Execute(&new_inst);
	
		AdvancePC(&new_inst);
		AdvanceClock();
	}

	//write into all the output files at the end of the program loop

	return 0;
}
