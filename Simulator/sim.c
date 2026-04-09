// Disable Visual Studio security warnings for standard C functions
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include "sim.h"

// Constants
#define MAX_LINE_LENGTH 500

// Global variables initialization
int pc = 0;
unsigned int regs[NUM_REGS] = {0};
unsigned int io_regs[NUM_IO_REGS] = {0};
unsigned int mainMem[MEM_SIZE] = {0};
unsigned int hard_disk[DISK_SIZE] = {0};
uint8_t monitor[MONITOR_SIZE] = {0};
unsigned int *irq2_cycles = NULL;  // Will be allocated dynamically
int irq2_count = 0;
int irq2_index = 0;

int branch_condition = FALSE;
int interrupt_flag = 0;
int clock_cycle = 0;
int halt_condition = 0;

// Disk state
int disk_busy = 0;
int disk_timer = 0;
int disk_cmd_type = 0;
int disk_sector_num = 0;
int disk_buffer_addr = 0;

// File pointers for output
FILE *trace_fp = NULL;
FILE *hwregtrace_fp = NULL;
FILE *leds_fp = NULL;
FILE *display7seg_fp = NULL;

// Last values for change detection
unsigned int last_leds_value = 0;
unsigned int last_display_value = 0;

// Masks
#define PC_MASK 0xFFF      // 12 bits for PC
#define MEM_MASK 0xFFFFF   // 20 bits for memory

// ============================================================================
// SETUP FUNCTIONS
// ============================================================================

int signExtension(int val) {
    // Sign extend 20-bit value to 32-bit
    if (val & 0x80000) { // if bit 19 is 1 (negative)
        return val | 0xFFF00000;
    }
    return val & 0xFFFFF;
}

void FillMainMem(FILE* pmemin) {
    if (!pmemin) return;

    char line[MAX_LINE_LENGTH];
    int addr = 0;

    while (fgets(line, sizeof(line), pmemin) && addr < MEM_SIZE) {
        unsigned int value;
        // Try parsing hex directly
        if (sscanf(line, "%x", &value) == 1) {
            mainMem[addr] = value & MEM_MASK;
            addr++;
        }
        // If failed, check if it's a "[source...]" line and try to find the hex code after it
        else {
            // Look for the closing bracket ']' which usually precedes the code
            char* codeStart = strchr(line, ']');
            if (codeStart && sscanf(codeStart + 1, "%x", &value) == 1) {
                mainMem[addr] = value & MEM_MASK;
                addr++;
            }
        }
    }
}

void FillDiskinArr(FILE *pdiskin) {
    if (!pdiskin) return;
    
    char line[MAX_LINE_LENGTH];
    int addr = 0;
    
    while (fgets(line, sizeof(line), pdiskin) && addr < DISK_SIZE) {
        unsigned int value;
        if (sscanf(line, "%x", &value) == 1) {
            hard_disk[addr] = value & MEM_MASK;
            addr++;
        }
    }
}

void FillIrq2inArr(FILE *pirq2in) {
    if (!pirq2in) {
        // No IRQ2 interrupts - allocate empty array
        irq2_cycles = NULL;
        irq2_count = 0;
        return;
    }
    
    char line[MAX_LINE_LENGTH];
    
    // First pass: count the number of interrupts
    int count = 0;
    while (fgets(line, sizeof(line), pirq2in)) {
        int cycle;
        if (sscanf(line, "%d", &cycle) == 1) {
            count++;
        }
    }
    
    // Allocate array for the actual number of interrupts
    if (count > 0) {
        irq2_cycles = (unsigned int *)malloc(count * sizeof(unsigned int));
        if (!irq2_cycles) {
            printf("Error: Failed to allocate memory for IRQ2 cycles\n");
            exit(1);
        }
        
        // Second pass: read the interrupts
        rewind(pirq2in);
        irq2_count = 0;
        
        while (fgets(line, sizeof(line), pirq2in) && irq2_count < count) {
            int cycle;
            if (sscanf(line, "%d", &cycle) == 1) {
                irq2_cycles[irq2_count] = cycle;
                irq2_count++;
            }
        }
    } else {
        // No valid interrupts in file
        irq2_cycles = NULL;
        irq2_count = 0;
    }
}

void SetArrays(FILE *pdiskin, FILE *pmemin, FILE *pirq2in) {
    FillMainMem(pmemin);
    FillDiskinArr(pdiskin);
    FillIrq2inArr(pirq2in);
}

// ============================================================================
// INTERRUPT AND DISK HANDLING
// ============================================================================

void HandleDiskOperation() {
    if (!disk_busy) return;
    
    disk_timer--;
    
    if (disk_timer <= 0) {
        // Disk operation completed
        int buffer_addr = disk_buffer_addr;
        int sector_start = disk_sector_num * DISK_SECTOR_SIZE;
        
        if (disk_cmd_type == 1) {
            // Read from disk to memory
            for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
                int mem_addr = (buffer_addr + i) & PC_MASK;
                if (mem_addr < MEM_SIZE && (sector_start + i) < DISK_SIZE) {
                    mainMem[mem_addr] = hard_disk[sector_start + i];
                }
            }
        } else if (disk_cmd_type == 2) {
            // Write from memory to disk
            for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
                int mem_addr = (buffer_addr + i) & PC_MASK;
                if (mem_addr < MEM_SIZE && (sector_start + i) < DISK_SIZE) {
                    hard_disk[sector_start + i] = mainMem[mem_addr];
                }
            }
        }
        
        // Reset disk state
        disk_busy = 0;
        disk_timer = 0;
        io_regs[14] = 0; // diskcmd = 0
        io_regs[17] = 0; // diskstatus = 0 (free)
        io_regs[4] = 1;  // irq1status = 1 (disk interrupt)
    }
}

int CheckInterrupts() {
    // Timer Logic (IRQ0)
    if (io_regs[11] == 1) { // timerenable
        if (io_regs[12] >= io_regs[13]) { // timercurrent >= timermax
            io_regs[3] = 1;  // Set irq0status to 1
            io_regs[12] = 0; // Reset timercurrent
        } else {
            io_regs[12]++; // Increment timercurrent
        }
    }
    
    // Disk Logic (IRQ1) - handled in HandleDiskOperation
    
    // External Interrupt (IRQ2) Logic
    if (irq2_index < irq2_count && irq2_cycles[irq2_index] == clock_cycle) {
        io_regs[5] = 1; // Set irq2status to 1
        irq2_index++;
    }
    
    // If already in interrupt handler, don't interrupt again
    if (interrupt_flag) {
        return 0;
    }
    
    // Check if any interrupt is triggered
    int irq0_triggered = (io_regs[0] && io_regs[3]); // irq0enable && irq0status
    int irq1_triggered = (io_regs[1] && io_regs[4]); // irq1enable && irq1status
    int irq2_triggered = (io_regs[2] && io_regs[5]); // irq2enable && irq2status
    
    if (irq0_triggered || irq1_triggered || irq2_triggered) {
        interrupt_flag = 1;      // Enter interrupt routine
        io_regs[7] = pc;         // Save return address
        pc = io_regs[6];         // Jump to handler address
        return 1;                // PC changed
    }
    
    return 0;
}

// ============================================================================
// FETCH-DECODE-EXECUTE
// ============================================================================

Instruction Fetch() {
    Instruction current_inst;
    
    if (pc >= MEM_SIZE) {
        printf("Error: PC out of range: %d\n", pc);
        exit(1);
    }
    
    unsigned int line_inst = mainMem[pc];
    
    current_inst.opcode = (line_inst >> 12) & 0xFF; // bits 19-12
    current_inst.rd = (line_inst >> 8) & 0xF;       // bits 11-8
    current_inst.rs = (line_inst >> 4) & 0xF;       // bits 7-4
    current_inst.rt = line_inst & 0xF;              // bits 3-0
    
    // Check if I-type (uses $imm register)
    if (current_inst.rd == 1 || current_inst.rs == 1 || current_inst.rt == 1) {
        current_inst.is_itype = 1;
        
        // Bounds check for PC+1
        if (pc + 1 >= MEM_SIZE || pc + 1 < 0) {
            printf("Error: PC+1 (%d) out of range for I-type instruction\n", pc + 1);
            exit(1);
        }
        
        int line_imm = mainMem[pc + 1];
        current_inst.imm = signExtension(line_imm);
        regs[1] = current_inst.imm; // Set $imm register
    } else {
        current_inst.is_itype = 0;
        current_inst.imm = 0;
    }
    
    return current_inst;
}

void Execute(Instruction *inst) {
    // Reset branch condition
    branch_condition = FALSE;
    
    // Calculate address for memory operations
    int addr = regs[inst->rs] + regs[inst->rt];
    
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
        case 6: // sll
            regs[inst->rd] = regs[inst->rs] << regs[inst->rt];
            break;
        case 7: // sra - arithmetic shift (sign extension)
            regs[inst->rd] = ((int)regs[inst->rs]) >> regs[inst->rt];
            break;
        case 8: // srl - logical shift (zero fill)
            regs[inst->rd] = ((unsigned int)regs[inst->rs]) >> regs[inst->rt];
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
            if (((int)regs[inst->rs]) < ((int)regs[inst->rt]))
                branch_condition = TRUE;
            break;
        case 12: // bgt
            if (((int)regs[inst->rs]) > ((int)regs[inst->rt]))
                branch_condition = TRUE;
            break;
        case 13: // ble
            if (((int)regs[inst->rs]) <= ((int)regs[inst->rt]))
                branch_condition = TRUE;
            break;
        case 14: // bge
            if (((int)regs[inst->rs]) >= ((int)regs[inst->rt]))
                branch_condition = TRUE;
            break;
        case 15: // jal
            regs[inst->rd] = pc + (inst->is_itype ? 2 : 1);
            pc = regs[inst->rs] & PC_MASK;
            break;
        case 16: // lw (Load Word)
            if (inst->rd > 1) { // Don't write to $zero or $imm
                addr = addr & PC_MASK;
                if (addr < MEM_SIZE) {
                    regs[inst->rd] = signExtension(mainMem[addr]);
                }
            }
            break;
        case 17: // sw (Store Word)
            addr = addr & PC_MASK;
            if (addr < MEM_SIZE) {
                mainMem[addr] = regs[inst->rd] & MEM_MASK;
            }
            break;
        case 18: // reti
            pc = io_regs[7]; // Return from interrupt
            interrupt_flag = 0;
            break;
        case 19: // in
            addr = addr & 0xFF;
            if (addr >= 0 && addr < NUM_IO_REGS) {
                if (inst->rd > 1) { // Don't write to $zero or $imm
                    regs[inst->rd] = io_regs[addr];
                }
                WriteHwRegTrace("READ", addr, io_regs[addr]);
            }
            break;
        case 20: // out
            addr = addr & 0xFF;
            if (addr >= 0 && addr < NUM_IO_REGS) {
                io_regs[addr] = regs[inst->rd];
                WriteHwRegTrace("WRITE", addr, regs[inst->rd]);
                
                // Handle special IO registers
                if (addr == 9) { // leds
                    if (io_regs[9] != last_leds_value) {
                        WriteLeds(clock_cycle, io_regs[9]);
                        last_leds_value = io_regs[9];
                    }
                } else if (addr == 10) { // display7seg
                    if (io_regs[10] != last_display_value) {
                        WriteDisplay7seg(clock_cycle, io_regs[10]);
                        last_display_value = io_regs[10];
                    }
                } else if (addr == 14) { // diskcmd
                    if (io_regs[14] != 0 && io_regs[17] == 0) { // Command given and disk is free
                        disk_busy = 1;
                        disk_timer = DISK_RW_TIME;
                        disk_cmd_type = io_regs[14];
                        disk_sector_num = io_regs[15] & 0x7F; // 7 bits
                        disk_buffer_addr = io_regs[16] & PC_MASK; // 12 bits
                        io_regs[17] = 1; // Set diskstatus to busy
                    }
                } else if (addr == 22) { // monitorcmd
                    if (io_regs[22] == 1) {
                        int monitor_addr = io_regs[20] & 0xFFFF; // 16 bits
                        int monitor_data = io_regs[21] & 0xFF;   // 8 bits
                        if (monitor_addr < MONITOR_SIZE) {
                            monitor[monitor_addr] = (uint8_t)monitor_data;
                        }
                        io_regs[22] = 0; // Reset monitorcmd
                    }
                }
            }
            break;
        case 21: // halt
            halt_condition = 1;
            break;
        default:
            printf("Error: Unknown opcode %d at PC %d\n", inst->opcode, pc);
            break;
    }
    
    // Ensure $zero and $imm are not modified
    regs[0] = 0;
    regs[1] = inst->imm;
}

void AdvancePC(Instruction* inst) {
    if (inst->opcode == 15 || inst->opcode == 18) {
        return;
    }

    if (branch_condition == TRUE) {
        pc = regs[inst->rd] & PC_MASK;
    }
    else {
        // If I-type (including beq with rd=$imm), skip the immediate word
        pc = (pc + (inst->is_itype ? 2 : 1)) & PC_MASK;
    }
}

void AdvanceClock() {
    io_regs[8]++; // clks - will overflow naturally at 0xFFFFFFFF
    clock_cycle++;
}

// ============================================================================
// OUTPUT FUNCTIONS
// ============================================================================

void WriteMemout(FILE *fp) {
    if (!fp) return;
    for (int i = 0; i < MEM_SIZE; i++) {
        fprintf(fp, "%05X\n", mainMem[i] & MEM_MASK);
    }
}

void WriteRegout(FILE *fp) {
    if (!fp) return;
    // Write R2-R15 (skip $zero and $imm)
    for (int i = 2; i < NUM_REGS; i++) {
        fprintf(fp, "%08X\n", regs[i]);
    }
}

void WriteTrace(FILE *fp, Instruction *inst) {
    if (!fp) return;
    
    // Format: PC INST R0 R1 R2 ... R15
    fprintf(fp, "%03X %05X ", pc & PC_MASK, mainMem[pc] & MEM_MASK);
    
    // R0 (always 0)
    fprintf(fp, "%08X ", 0);
    
    // R1 ($imm - 0 for R-type, sign-extended imm for I-type)
    if (inst->is_itype) {
        fprintf(fp, "%08X ", (unsigned int)inst->imm);
    } else {
        fprintf(fp, "%08X ", 0);
    }
    
    // R2-R15
    for (int i = 2; i < NUM_REGS; i++) {
        fprintf(fp, "%08X", regs[i]);
        if (i < NUM_REGS - 1) fprintf(fp, " ");
    }
    fprintf(fp, "\n");
}

void WriteHwRegTrace(const char *operation, int addr, unsigned int value) {
    if (!hwregtrace_fp) return;
    
    // IO register names
    const char *names[] = {
        "irq0enable", "irq1enable", "irq2enable",
        "irq0status", "irq1status", "irq2status",
        "irqhandler", "irqreturn", "clks",
        "leds", "display7seg", "timerenable",
        "timercurrent", "timermax", "diskcmd",
        "disksector", "diskbuffer", "diskstatus",
        "reserved1", "reserved2", "monitoraddr",
        "monitordata", "monitorcmd"
    };
    
    if (addr >= 0 && addr < NUM_IO_REGS) {
        fprintf(hwregtrace_fp, "%d %s %s %08X\n", 
                clock_cycle, operation, names[addr], value);
    }
}

void WriteCycles(FILE *fp) {
    if (!fp) return;
    fprintf(fp, "%d\n", clock_cycle);
}

void WriteLeds(int cycle, unsigned int value) {
    if (!leds_fp) return;
    fprintf(leds_fp, "%d %08X\n", cycle, value);
}

void WriteDisplay7seg(int cycle, unsigned int value) {
    if (!display7seg_fp) return;
    fprintf(display7seg_fp, "%d %08X\n", cycle, value);
}

void WriteDiskout(FILE *fp) {
    if (!fp) return;
    for (int i = 0; i < DISK_SIZE; i++) {
        fprintf(fp, "%05X\n", hard_disk[i] & MEM_MASK);
    }
}

void WriteMonitor(FILE *monitor_txt, FILE *monitor_yuv) {
    if (monitor_txt) {
        for (int i = 0; i < MONITOR_SIZE; i++) {
            fprintf(monitor_txt, "%02X\n", monitor[i]);
        }
    }
    
    if (monitor_yuv) {
        fwrite(monitor, sizeof(uint8_t), MONITOR_SIZE, monitor_yuv);
    }
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc != 14) {
        printf("Usage: sim.exe memin.txt diskin.txt irq2in.txt memout.txt regout.txt trace.txt hwregtrace.txt cycles.txt leds.txt display7seg.txt diskout.txt monitor.txt monitor.yuv\n");
        return 1;
    }
    
    // Open input files
    FILE *memin = fopen(argv[1], "r");
    FILE *diskin = fopen(argv[2], "r");
    FILE *irq2in = fopen(argv[3], "r");
    
    if (!memin || !diskin || !irq2in) {
        printf("Error: Could not open input files\n");
        if (memin) fclose(memin);
        if (diskin) fclose(diskin);
        if (irq2in) fclose(irq2in);
        return 1;
    }
    
    // Load input data
    SetArrays(diskin, memin, irq2in);
    fclose(memin);
    fclose(diskin);
    fclose(irq2in);
    
    // Open output files
    trace_fp = fopen(argv[6], "w");
    hwregtrace_fp = fopen(argv[7], "w");
    leds_fp = fopen(argv[9], "w");
    display7seg_fp = fopen(argv[10], "w");
    
    if (!trace_fp || !hwregtrace_fp || !leds_fp || !display7seg_fp) {
        printf("Error: Could not open output files\n");
        return 1;
    }
    // Set maximum cycles to prevent infinite loops
    // Reasonable limit: 100,000 cycles for simple programs
    // For complex programs, increase this value
    #define MAX_CYCLES 1000000
    
    // Main simulation loop
    while (!halt_condition && pc < MEM_SIZE && clock_cycle < MAX_CYCLES) {
        // Handle disk operations
        HandleDiskOperation();
        
        // Check for interrupts
        int irq_occurred = CheckInterrupts();
        
        // Fetch instruction
        Instruction current_inst = Fetch();
        
        // Write trace before execution
        WriteTrace(trace_fp, &current_inst);
        
        // Execute instruction
        Execute(&current_inst);
        
        // Advance PC
        if (current_inst.opcode != 15 && current_inst.opcode != 18) { // not jal or reti
            AdvancePC(&current_inst);
        }
        
        // Advance clock
        AdvanceClock();
    }
    
    // Check if stopped due to max cycles (infinite loop protection)
    if (clock_cycle >= MAX_CYCLES && !halt_condition) {
        printf("WARNING: Simulation stopped after %d cycles (possible infinite loop)\n", MAX_CYCLES);
        printf("Last PC: 0x%03X\n", pc);
        printf("If this is intentional, increase MAX_CYCLES in sim.c\n");
    }
    
    // Close trace files
    fclose(trace_fp);
    fclose(hwregtrace_fp);
    fclose(leds_fp);
    fclose(display7seg_fp);
    
    // Write final output files
    FILE *memout = fopen(argv[4], "w");
    FILE *regout = fopen(argv[5], "w");
    FILE *cycles_fp = fopen(argv[8], "w");
    FILE *diskout = fopen(argv[11], "w");
    FILE *monitor_txt = fopen(argv[12], "w");
    FILE *monitor_yuv = fopen(argv[13], "wb");
    
    if (memout) {
        WriteMemout(memout);
        fclose(memout);
    }
    
    if (regout) {
        WriteRegout(regout);
        fclose(regout);
    }
    
    if (cycles_fp) {
        WriteCycles(cycles_fp);
        fclose(cycles_fp);
    }
    
    if (diskout) {
        WriteDiskout(diskout);
        fclose(diskout);
    }
    
    if (monitor_txt || monitor_yuv) {
        WriteMonitor(monitor_txt, monitor_yuv);
        if (monitor_txt) fclose(monitor_txt);
        if (monitor_yuv) fclose(monitor_yuv);
    }
    
    // Free dynamically allocated memory
    if (irq2_cycles) {
        free(irq2_cycles);
    }
    
    return 0;
}
