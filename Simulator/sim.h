#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Constants
#define TRUE 1
#define FALSE 0
#define MEM_SIZE 4096
#define DISK_SECTORS 128
#define DISK_SECTOR_SIZE 128
#define DISK_SIZE (DISK_SECTORS * DISK_SECTOR_SIZE) // 16384
#define DISK_RW_TIME 1024
#define NUM_REGS 16
#define NUM_IO_REGS 23
#define MONITOR_SIZE (256 * 256)

// Instruction structure
typedef struct Instruction {
    unsigned int opcode; // 8 bits
    unsigned int rd;     // 4 bits
    unsigned int rs;     // 4 bits
    unsigned int rt;     // 4 bits
    int imm;             // 20 bits (signed)
    int is_itype;        // flag: 1 if I-type, 0 if R-type
} Instruction;

// Global variables
extern int pc;
extern unsigned int regs[NUM_REGS];
extern unsigned int io_regs[NUM_IO_REGS];
extern unsigned int mainMem[MEM_SIZE];
extern unsigned int hard_disk[DISK_SIZE];
extern uint8_t monitor[MONITOR_SIZE];
extern unsigned int *irq2_cycles;  // Dynamic array
extern int irq2_count;
extern int irq2_index;

extern int branch_condition;
extern int interrupt_flag;
extern int clock_cycle;
extern int halt_condition;

// Disk state
extern int disk_busy;
extern int disk_timer;
extern int disk_cmd_type;
extern int disk_sector_num;
extern int disk_buffer_addr;

// File pointers for output
extern FILE *trace_fp;
extern FILE *hwregtrace_fp;
extern FILE *leds_fp;
extern FILE *display7seg_fp;

// Last values for change detection
extern unsigned int last_leds_value;
extern unsigned int last_display_value;

// Function declarations

// Setup functions
void FillMainMem(FILE *pmemin);
void FillDiskinArr(FILE *pdiskin);
void FillIrq2inArr(FILE *pirq2in);
void SetArrays(FILE *pdiskin, FILE *pmemin, FILE *pirq2in);
int signExtension(int val);

// Main process functions
int CheckInterrupts();
void HandleDiskOperation();
Instruction Fetch();
void Execute(Instruction *inst_ptr);
void AdvancePC(Instruction *inst);
void AdvanceClock();

// Output functions
void WriteMemout(FILE *fp);
void WriteRegout(FILE *fp);
void WriteTrace(FILE *fp, Instruction *inst);
void WriteHwRegTrace(const char *operation, int addr, unsigned int value);
void WriteCycles(FILE *fp);
void WriteLeds(int cycle, unsigned int value);
void WriteDisplay7seg(int cycle, unsigned int value);
void WriteDiskout(FILE *fp);
void WriteMonitor(FILE *monitor_txt, FILE *monitor_yuv);

#endif // SIM_H
