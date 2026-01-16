#define _CRT_SECURE_NO_WARNINGS
#include "asm.h"

// global arrays for mapping
const char *opcodes[] = {"add",  "sub", "mul", "and", "or",  "xor",
                         "sll",  "sra", "srl", "beq", "bne", "blt",
                         "bgt",  "ble", "bge", "jal", "lw",  "sw",
                         "reti", "in",  "out", "halt"};

// register names. imm is at index 1
const char *registers[] = {"$zero", "$imm", "$v0", "$a0", "$a1", "$a2",
                           "$a3",   "$t0",  "$t1", "$t2", "$s0", "$s1",
                           "$s2",   "$gp",  "$sp", "$ra"};

int get_opcode_id(char *name) {
  if (!name)
    return -1;
  for (int i = 0; i <= 21; i++) {
    if (strcmp(name, opcodes[i]) == 0) {
      return i;
    }
  }
  return -1;
}

int get_register_id(char *name) {
  if (!name)
    return -1;
  for (int i = 0; i <= 15; i++) {
    if (strcmp(name, registers[i]) == 0) {
      return i;
    }
  }
  return -1;
}

// helper functions

void trim_whitespace(char *str) {
  if (!str)
    return;

  // 1. Find the first non-whitespace char
  char *start = str;
  while (*start && isspace((unsigned char)*start))
    start++;

  // 2. Shift the string to the beginning of the buffer (str)
  char *d = str;
  while (*start) {
    *d++ = *start++;
  }
  *d = '\0'; // Null terminate the new shorter string

  // 3. Trim trailing whitespace
  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  *(end + 1) = 0;
}

int parse_int(char *str) {
  if (!str)
    return 0;
  // handle hex format
  if (strstr(str, "0x") == str || strstr(str, "0X") == str) {
    return (int)strtol(str, NULL, 16);
  }
  return atoi(str);
}

// split line into parts. returns 1 if valid instruction, 0 otherwise
int parse_line(char *line, char *opcode, char *rd, char *rs, char *rt,
               char *imm) {
  char buf[MAX_LINE_LENGTH];
  strncpy(buf, line, MAX_LINE_LENGTH);

  // split by comma, space or tabs
  char *delims = ", \t\n\r";
  char *token = strtok(buf, delims);

  if (!token)
    return 0; // empty or just comment
  strcpy(opcode, token);

  token = strtok(NULL, delims);
  if (!token)
    return 0;
  strcpy(rd, token);

  token = strtok(NULL, delims);
  if (!token)
    return 0;
  strcpy(rs, token);

  token = strtok(NULL, delims);
  if (!token)
    return 0;
  strcpy(rt, token);

  token = strtok(NULL, delims);
  if (!token)
    return 0;
  strcpy(imm, token);

  return 1;
}

// pass 1 - find labels

Symbol symbol_table[MAX_LABELS];
int symbol_count = 0;

void add_symbol(char *label, int address) {
  if (symbol_count >= MAX_LABELS) {
    printf("Error: Max labels reached\n");
    exit(1);
  }
  char buf[51];
  strcpy(buf, label);

  // remove colon
  char *colon = strchr(buf, ':');
  if (colon)
    *colon = '\0';

  strcpy(symbol_table[symbol_count].name, buf);
  symbol_table[symbol_count].address = address;
  symbol_count++;
}

int get_symbol_address(char *label) {
  for (int i = 0; i < symbol_count; i++) {
    if (strcmp(symbol_table[i].name, label) == 0) {
      return symbol_table[i].address;
    }
  }
  return -1;
}

void first_pass(FILE *input) {
  char line[MAX_LINE_LENGTH];
  int PC = 0;
  symbol_count = 0;

  while (fgets(line, sizeof(line), input)) {
    char buf[MAX_LINE_LENGTH];
    strcpy(buf, line);

    // remove comments
    char *comment = strchr(buf, '#');
    if (comment)
      *comment = '\0';

    trim_whitespace(buf);
    if (strlen(buf) == 0)
      continue;

    // is it a label?
    char *colon = strchr(buf, ':');
    char *instruction_start = buf;

    if (colon) {
      *colon = '\0';
      add_symbol(buf, PC); // save label address
      instruction_start = colon + 1;
      trim_whitespace(instruction_start);
    }

    if (strlen(instruction_start) == 0)
      continue;

    // check for .word
    if (strncmp(instruction_start, ".word", 5) == 0) {
      continue; // .word doesn't increase PC
    }

    char opcode[20], rd[20], rs[20], rt[20], imm[MAX_LINE_LENGTH];
    if (parse_line(instruction_start, opcode, rd, rs, rt, imm)) {
      int rs_id = get_register_id(rs);
      int rt_id = get_register_id(rt);

      // if using $imm (index 1), we need 2 words
      if (rs_id == 1 || rt_id == 1) {
        PC += 2;
      } else {
        PC += 1;
      }
    }
  }
  rewind(input);
}

// pass 2 - generate machine code

int memory[MEMORY_SIZE];

void second_pass(FILE *input) {
  char line[MAX_LINE_LENGTH];
  int PC = 0;

  // clear memory
  for (int i = 0; i < MEMORY_SIZE; i++)
    memory[i] = 0;

  while (fgets(line, sizeof(line), input)) {
    char buf[MAX_LINE_LENGTH];
    strcpy(buf, line);

    char *comment = strchr(buf, '#');
    if (comment)
      *comment = '\0';

    trim_whitespace(buf);
    if (strlen(buf) == 0)
      continue;

    // skip labels this time
    char *colon = strchr(buf, ':');
    char *instruction_start = buf;
    if (colon) {
      instruction_start = colon + 1;
      trim_whitespace(instruction_start);
    }
    if (strlen(instruction_start) == 0)
      continue;

    // handle .word
    if (strncmp(instruction_start, ".word", 5) == 0) {
      int addr, data;
      char *ptr = instruction_start + 5;
      char word_addr_str[50], word_val_str[50];

      // parse manual args
      while (isspace((unsigned char)*ptr))
        ptr++;

      int k = 0;
      while (*ptr && !isspace((unsigned char)*ptr) && k < 49)
        word_addr_str[k++] = *ptr++;
      word_addr_str[k] = 0;

      while (isspace((unsigned char)*ptr))
        ptr++;

      k = 0;
      while (*ptr && !isspace((unsigned char)*ptr) && k < 49)
        word_val_str[k++] = *ptr++;
      word_val_str[k] = 0;

      addr = parse_int(word_addr_str);
      data = parse_int(word_val_str);

      if (addr >= 0 && addr < MEMORY_SIZE) {
        memory[addr] = data;
      }
      continue;
    }

    // handle instructions
    char opcode_str[20], rd_str[20], rs_str[20], rt_str[20],
        imm_str[MAX_LINE_LENGTH];
    if (parse_line(instruction_start, opcode_str, rd_str, rs_str, rt_str,
                   imm_str)) {

      int opcode = get_opcode_id(opcode_str);
      int rd = get_register_id(rd_str);
      int rs = get_register_id(rs_str);
      int rt = get_register_id(rt_str);

      if (opcode == -1 || rd == -1 || rs == -1 || rt == -1) {
        printf("Error: Invalid instruction at PC %d\n", PC);
        exit(1);
      }

      int imm_val = 0;
      int label_addr = get_symbol_address(imm_str);

      if (label_addr != -1) {
        imm_val = label_addr;
      } else {
        // assume it's a number since input is valid
        imm_val = parse_int(imm_str);
      }

      // build first word: opcode, rd, rs, rt
      int word1 = (opcode << 12) | (rd << 8) | (rs << 4) | (rt);
      word1 &= 0xFFFFF;

      memory[PC] = word1;

      // check if we need extra word for imm
      if (rs == 1 || rt == 1) {
        PC++;
        memory[PC] = imm_val & 0xFFFFF;
        PC++;
      } else {
        PC++;
      }
    }
  }
}

// main

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: asm.exe <input.asm> <memin.txt>\n");
    return 1;
  }

  FILE *input = fopen(argv[1], "r");
  if (!input) {
    printf("Error: Can't open input file %s\n", argv[1]);
    return 1;
  }

  FILE *output = fopen(argv[2], "w");
  if (!output) {
    printf("Error: Can't open output file %s\n", argv[2]);
    fclose(input);
    return 1;
  }

  first_pass(input);
  second_pass(input);

  // write output (5 hex digits)
  for (int i = 0; i < MEMORY_SIZE; i++) {
    fprintf(output, "%05X\n", memory[i] & 0xFFFFF);
  }

  fclose(input);
  fclose(output);
  return 0;
}