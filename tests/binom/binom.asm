# Initial Memory
.word 0x100 7
.word 0x101 3


# Init and call function
add $sp, $zero, $imm, 2048    # $sp = 2048
lw $a0, $zero, $imm, 0x100    # $a0 = n
lw $a1, $zero, $imm, 0x101    # $a1 = k
jal $imm, $zero, $zero, binom # $v0 = binom(n,k)
sw $v0, $zero, $imm, 0x102    # save output to memory
halt $zero, $zero, $zero, 0

# Perform Binom function
binom:
add $sp, $sp, $imm, -4        # stack 4 items
sw $ra, $sp, $imm, 3          # save $ra
sw $s0, $sp, $imm, 2          # save $s0
sw $a1, $sp, $imm, 1          # save $a1
sw $a0, $sp, $imm, 0          # save $a0

# Handle base case
add $v0, $zero, $imm, 1       # $v0=1
beq $imm, $a0, $a1, return    # leave if k==n
beq $imm, $a1, $zero, return  # leave if k==0

# Call recursive Binom
loop:
add $a0, $a0, $imm, -1        # n--
jal $imm, $zero, $zero, binom # $v0=binom(n-1, k)
add $s0, $v0, $zero, 0        # $s0=binom(n-1, k)
add $a1, $a1, $imm, -1        # k--
jal $imm, $zero, $zero, binom # $v0=binom(n-1, k-1)
add $v0, $v0, $s0, 0          # $v0=binom(n-1, k) + binom(n-1, k-1)

# End of function
return:
lw $a0, $sp, $imm, 0          # restore $a0
lw $a1, $sp, $imm, 1          # restore $a1
lw $s0, $sp, $imm, 2          # restore $s0
lw $ra, $sp, $imm, 3          # restore $ra
add $sp, $sp, $imm, 4         # restore stack
beq $ra, $zero, $zero, 0      # return