# triangle.asm - Draw Filled White Right Triangle

.word 0x100 0x1E0A
.word 0x101 0x500A
.word 0x102 0x503C

main:
    # 1. LOAD & UNPACK DATA
    # We load A, B, C into s0, s1, s2 temporarily, then unpack into:
    # A: $t0, $t1
    # B: $t2, $a0 (was t3)
    # C: $a1, $a2 (was t4, t5)
    
    lw $s0, $imm, $zero, 0x100
    lw $s1, $imm, $zero, 0x101
    lw $s2, $imm, $zero, 0x102
    
    # Unpack A
    and $t0, $s0, $imm, 0xFF       # col_A
    srl $t1, $s0, $imm, 8
    and $t1, $t1, $imm, 0xFF       # row_A
    
    # Unpack B
    and $t2, $s1, $imm, 0xFF       # col_B
    srl $a0, $s1, $imm, 8          # row_B (mapped to $a0)
    and $a0, $a0, $imm, 0xFF
    
    # Unpack C
    and $a1, $s2, $imm, 0xFF       # col_C (mapped to $a1)
    srl $a2, $s2, $imm, 8          # row_C (mapped to $a2)
    and $a2, $a2, $imm, 0xFF
    
    add $v0, $t1, $zero, 0         # $v0 = current_row (was $s3)

row_loop:
    bgt $imm, $v0, $a0, done       # if curr_row > row_B
    
    add $gp, $t0, $zero, 0         # $gp = current_col (was $s4)

col_loop:
    # Logic: if numerator >= denominator * (curr_col - col_A)
    # numerator (in $a3) = (curr_row - row_A) * (col_C - col_A)
    # denominator (in $ra) = (row_B - row_A)
    
    sub $a3, $v0, $t1, 0           # curr_row - row_A
    sub $sp, $a1, $t0, 0           # col_C - col_A (using $sp as temp)
    mul $a3, $a3, $sp, 0           # $a3 = numerator
    
    sub $ra, $a0, $t1, 0           # $ra = row_B - row_A (denominator)
    
    # Check threshold
    sub $s0, $gp, $t0, 0           # $s0 = curr_col - col_A
    mul $s0, $ra, $s0, 0           # $s0 = denom * (curr_col - col_A)
    
    blt $imm, $a3, $s0, skip_pixel
    
    # Draw Pixel
    sll $s1, $v0, $imm, 8          # row * 256
    add $s1, $s1, $gp, 0           # + col
    
    out $s1, $imm, $zero, 20       # monitor addr
    add $s2, $zero, $imm, 255      # white
    out $s2, $imm, $zero, 21       # monitor data
    add $s2, $zero, $imm, 1
    out $s2, $imm, $zero, 22       # monitor cmd

skip_pixel:
    add $gp, $gp, $imm, 1          # col++
    ble $imm, $gp, $a1, col_loop   # while col <= col_C
    
    add $v0, $v0, $imm, 1          # row++
    beq $imm, $zero, $zero, row_loop

done:
    halt $zero, $zero, $zero, 0