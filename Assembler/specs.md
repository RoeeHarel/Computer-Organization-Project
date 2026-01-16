# SIMP Assembler Specifications

## 1. Memory & Architecture
- **Word Width:** 20 bits.
- **Memory Depth:** 4096 lines.
- **Registers:** 16 registers (32-bit wide).
- **PC:** 12 bits.

## 2. Instruction Encoding
The assembler reads a text line and outputs 1 or 2 lines of hex (20-bit width) to `memin.txt`.

### Format R (1 Word)
Used when the instruction **does not** use the `$imm` register.
- Bits 19:12 (8 bits): Opcode
- Bits 11:8  (4 bits): rd (Destination)
- Bits 7:4   (4 bits): rs (Source 1)
- Bits 3:0   (4 bits): rt (Source 2)

### Format I (2 Words)
Used when `rs` OR `rt` is the `$imm` register (Register index 1).
- **Word 1:** Same as Format R (Opcode | rd | rs | rt).
- **Word 2:** The Immediate value (20 bits).
    - Note: The CPU sign-extends this 20-bit value to 32 bits at runtime.

## 3. Register Map
| Name | Number | Note |
| :--- | :--- | :--- |
| $zero | 0 | Constant 0 |
| $imm | 1 | **Triggers 2-word instruction format** |
| $v0 | 2 | Result value |
| $a0 - $a3 | 3 - 6 | Arguments |
| $t0 - $t2 | 7 - 9 | Temporaries |
| $s0 - $s2 | 10 - 12 | Saved |
| $gp | 13 | Global Pointer |
| $sp | 14 | Stack Pointer |
| $ra | 15 | Return Address |

## 4. Opcode Map
| Opcode | Name | Type | Note |
| :--- | :--- | :--- | :--- |
| 0 | add | Arithmetic | |
| 1 | sub | Arithmetic | |
| 2 | mul | Arithmetic | |
| 3 | and | Logical | |
| 4 | or | Logical | |
| 5 | xor | Logical | |
| 6 | sll | Shift | |
| 7 | sra | Shift | Arithmetic shift |
| 8 | srl | Shift | Logical shift |
| 9 | beq | Branch | if (rs == rt) pc = rd |
| 10 | bne | Branch | if (rs != rt) pc = rd |
| 11 | blt | Branch | if (rs < rt) pc = rd |
| 12 | bgt | Branch | if (rs > rt) pc = rd |
| 13 | ble | Branch | if (rs <= rt) pc = rd |
| 14 | bge | Branch | if (rs >= rt) pc = rd |
| 15 | jal | Jump | rd = next_pc, pc = rs |
| 16 | lw | Memory | rd = MEM[rs+rt] |
| 17 | sw | Memory | MEM[rs+rt] = rd |
| 18 | reti | Interrupt | Return from interrupt |
| 19 | in | IO | rd = IOReg[rs+rt] |
| 20 | out | IO | IOReg[rs+rt] = rd |
| 21 | halt | System | Stop execution |

## 5. Input Syntax (parser logic)
Each line in `.asm` has exactly 5 fields (comma separated) + optional comment.
Format: `Opcode, rd, rs, rt, imm`

**Handling `imm` field:**
- Can be decimal, hex (0x...), or a Label.
- If `rs` or `rt` is `$imm` (reg 1), the assembler must parse the `imm` field value and output it in the next memory line.
- If neither `rs` nor `rt` is `$imm`, the `imm` field is ignored (usually 0).

## 6. Directives
- `.word address data`: Writes `data` directly to memory `address`.
- Labels: Ends with `:`, e.g., `L1:`.

## 7. Assembler Logic (2-Pass)
1. **Pass 1:** Scan code. Count PC (increment by 1 for R-type, 2 for I-type). Save Labels and their PC addresses in a symbol table.
2. **Pass 2:** Re-scan. translate instructions to Hex. Replace Label names in the `imm` field with addresses from the symbol table.