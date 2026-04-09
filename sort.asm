# sort.asm - Bubble Sort

.word 0x100 45
.word 0x101 23
.word 0x102 87
.word 0x103 12
.word 0x104 67
.word 0x105 34
.word 0x106 91
.word 0x107 56
.word 0x108 78
.word 0x109 9
.word 0x10A 42
.word 0x10B 15
.word 0x10C 73
.word 0x10D 28
.word 0x10E 61
.word 0x10F 50

main:
    add $s0, $zero, $imm, 0x100    # Base address
    add $s1, $zero, $imm, 16       # n
    add $t0, $zero, $imm, 0        # i

outer_loop:
    sub $a0, $s1, $imm, 1          # $a0 (was $t3) = n - 1
    bge $imm, $t0, $a0, outer_done
    
    add $t1, $zero, $imm, 0        # j

inner_loop:
    sub $a1, $s1, $t0, 0           # $a1 (was $t4) = n - i
    sub $a1, $a1, $imm, 1          # n - i - 1
    bge $imm, $t1, $a1, inner_done
    
    add $a2, $s0, $t1, 0           # $a2 (was $t5) = base + j
    lw $v0, $a2, $zero, 0          # $v0 (was $t6) = arr[j]
    
    add $a2, $a2, $imm, 1          # base + j + 1
    lw $a3, $a2, $zero, 0          # $a3 (was $t7) = arr[j+1]
    
    bgt $imm, $v0, $a3, do_swap
    beq $imm, $zero, $zero, skip_swap

do_swap:
    sub $a2, $a2, $imm, 1          # Back to arr[j]
    sw $a3, $a2, $zero, 0          # arr[j] = old arr[j+1]
    add $a2, $a2, $imm, 1
    sw $v0, $a2, $zero, 0          # arr[j+1] = old arr[j]

skip_swap:
    add $t1, $t1, $imm, 1
    beq $imm, $zero, $zero, inner_loop

inner_done:
    add $t0, $t0, $imm, 1
    beq $imm, $zero, $zero, outer_loop

outer_done:
    halt $zero, $zero, $zero, 0