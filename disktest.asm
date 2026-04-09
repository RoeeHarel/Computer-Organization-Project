# disktest.asm - Hard Disk Test

main:
    add $s0, $zero, $imm, 0        # $s0 = current sector
    add $s1, $zero, $imm, 0x200    # $s1 = buffer address
    add $s2, $zero, $imm, 0x300    # $s2 = result buffer

    # Initialize result buffer to zero
    add $t0, $zero, $imm, 0
    add $t2, $zero, $imm, 128      # $t2 = 128 (loop limit)
    
init_loop:
    sw $zero, $s2, $t0, 0
    add $t0, $t0, $imm, 1
    blt $imm, $t0, $t2, init_loop  # if ($t0 < 128) loop

    # Set up sector loop limit
    add $a0, $zero, $imm, 8        # $a0 = 8 (max sectors)

sector_loop:
    bge $imm, $s0, $a0, write_result  # if ($s0 >= 8) exit

wait_disk_free:
    in $t1, $imm, $zero, 17        # $t1 = disk status
    add $a1, $zero, $imm, 1        # $a1 = 1 (busy)
    beq $imm, $t1, $a1, wait_disk_free  # if (status == busy) wait

    # Read from disk
    out $s0, $imm, $zero, 15       # Set sector
    out $s1, $imm, $zero, 16       # Set buffer
    add $t2, $zero, $imm, 1        # Read command
    out $t2, $imm, $zero, 14       # Execute read

wait_disk_done:
    in $t1, $imm, $zero, 17        # Check status
    add $a1, $zero, $imm, 1        # $a1 = 1 (busy)
    beq $imm, $t1, $a1, wait_disk_done  # if (busy) wait

    # Add loop
    add $t0, $zero, $imm, 0        # counter
    add $a2, $zero, $imm, 128      # $a2 = 128 (loop limit)
    
add_loop:
    bge $imm, $t0, $a2, next_sector  # if ($t0 >= 128) exit
    
    lw $t1, $s1, $t0, 0            # buffer[i]
    lw $t2, $s2, $t0, 0            # result[i]
    add $t2, $t2, $t1, 0           # result[i] += buffer[i]
    sw $t2, $s2, $t0, 0            # store back
    
    add $t0, $t0, $imm, 1          # i++
    beq $imm, $zero, $zero, add_loop

next_sector:
    add $s0, $s0, $imm, 1          # sector++
    beq $imm, $zero, $zero, sector_loop

write_result:
wait_disk_free2:
    in $t1, $imm, $zero, 17        # Check status
    add $a1, $zero, $imm, 1        # $a1 = 1 (busy)
    beq $imm, $t1, $a1, wait_disk_free2  # if (busy) wait

    # Write result to sector 8
    add $t2, $zero, $imm, 8        # Sector 8
    out $t2, $imm, $zero, 15       # Set sector
    out $s2, $imm, $zero, 16       # Set buffer
    add $t2, $zero, $imm, 2        # Write command
    out $t2, $imm, $zero, 14       # Execute write

wait_write_done:
    in $t1, $imm, $zero, 17        # Check status
    add $a1, $zero, $imm, 1        # $a1 = 1 (busy)
    beq $imm, $t1, $a1, wait_write_done  # if (busy) wait

    halt $zero, $zero, $zero, 0
