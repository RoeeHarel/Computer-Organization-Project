#ifndef ASM_H
#define ASM_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 500
#define MAX_LABELS 500
#define MEMORY_SIZE 4096

// global memory array
extern int memory[MEMORY_SIZE];

typedef struct {
  char name[51]; // 50 chars + 1 null terminator
  int address;
} Symbol;

// symbol table (label mapping)
extern Symbol symbol_table[MAX_LABELS];
extern int symbol_count;

// functions
int get_opcode_id(char *name);
int get_register_id(char *name);

void trim_whitespace(char *str);
int parse_int(char *str);
int parse_line(char *line, char *opcode, char *rd, char *rs, char *rt,
               char *imm);

void first_pass(FILE *input);
void second_pass(FILE *input);
void add_symbol(char *label, int address);
int get_symbol_address(char *label);

#endif