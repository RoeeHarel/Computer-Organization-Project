add $v0, $zero, $zero, 0   # R-type normal
add $t0, $zero, $imm, 255 # I-type with $imm at rt
add $t1, $imm, $zero, 400 # I-type with $imm at rs
L1:
beq $t0, $t1, L1          # Branch with label
.word 200 0xABCDE         # Directive at address 200
halt $zero, $zero, $zero, 0
