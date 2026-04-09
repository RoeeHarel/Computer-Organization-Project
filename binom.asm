# binom.asm - Binomial Coefficient Calculator
# Computes C(n, k) = n! / (k! * (n-k)!)

# Test data
.word 0x100 5    # n = 5
.word 0x101 2    # k = 2

main:
    add $sp, $zero, $imm, 0x0FF    # Stack starts at 0xFF
    lw $a0, $imm, $zero, 0x100     # $a0 = n
    lw $a1, $imm, $zero, 0x101     # $a1 = k
    jal $ra, $imm, $zero, binom
    sw $v0, $imm, $zero, 0x102     # Store result
    halt $zero, $zero, $zero, 0

binom:
    # Save $ra, $a0, $a1
    # Stack Frame:
    # 1. RA
    # 2. n ($a0)
    # 3. k ($a1)
    
    sw $ra, $sp, $zero, 0
    sub $sp, $sp, $imm, 1
    sw $a0, $sp, $zero, 0
    sub $sp, $sp, $imm, 1
    sw $a1, $sp, $zero, 0
    sub $sp, $sp, $imm, 1
    
    # Base cases
    beq $imm, $a1, $zero, base_case # if k==0
    beq $imm, $a0, $a1, base_case   # if n==k
    
    # Recursive step
    sub $a0, $a0, $imm, 1          # n-1
    sub $a1, $a1, $imm, 1          # k-1
    jal $ra, $imm, $zero, binom
    
    # SAVE RESULT 1 TO STACK
    sw $v0, $sp, $zero, 0          # push result1
    sub $sp, $sp, $imm, 1          # sp-- (SP is now 4 slots down from RA)

    # -------------------------------------------------------
    # Stack Map relative to current SP:
    # SP + 1 = result1
    # SP + 2 = k ($a1)
    # SP + 3 = n ($a0)
    # -------------------------------------------------------

    # 1. Restore n (offset 3)
    add $t0, $sp, $imm, 3          
    lw $a0, $t0, $zero, 0          # Reload original n
    
    # Prepare for second call: binom(n-1, k)
    sub $a0, $a0, $imm, 1          # n = n - 1
    
    # 2. Restore k (offset 2)
    add $t0, $sp, $imm, 2          
    lw $a1, $t0, $zero, 0          # Reload original k
    
    jal $ra, $imm, $zero, binom
    
    # Load Result 1 from stack
    add $sp, $sp, $imm, 1          # pop result1 (SP points to Result1)
    lw $t0, $sp, $zero, 0          # $t0 = result1
    
    add $v0, $t0, $v0, 0           # result = result1 + result2
    beq $imm, $zero, $zero, binom_return

base_case:
    add $v0, $zero, $imm, 1

binom_return:
    # Restore registers in reverse order
    add $sp, $sp, $imm, 1
    lw $a1, $sp, $zero, 0          # Restore k
    add $sp, $sp, $imm, 1
    lw $a0, $sp, $zero, 0          # Restore n
    add $sp, $sp, $imm, 1
    lw $ra, $sp, $zero, 0          # Restore ra
    
    beq $ra, $zero, $zero, 0
