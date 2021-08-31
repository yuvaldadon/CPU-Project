# Initial Matrix A
.word 0x100 0
.word 0x101 3
.word 0x102 0
.word 0x103 1

.word 0x104 4
.word 0x105 0
.word 0x106 0
.word 0x107 1

.word 0x108 1
.word 0x109 0
.word 0x10A 2
.word 0x10B 0

.word 0x10C 2
.word 0x10D 0
.word 0x10E 3
.word 0x10F 0

# Initial Matrix B
.word 0x110 1
.word 0x111 0
.word 0x112 4
.word 0x113 0

.word 0x114 2
.word 0x115 1
.word 0x116 0
.word 0x117 4

.word 0x118 3
.word 0x119 2
.word 0x11A 0
.word 0x11B 2

.word 0x11C 0
.word 0x11D 3
.word 0x11E 0
.word 0x11F 1


# Init
add $sp, $zero, $imm, 2048       # $sp = 2048
jal $imm, $zero, $zero, mulmat   # mulmat
halt $zero, $zero, $zero, 0

# Run Matrix multiply
mulmat:
add $sp, $sp, $imm, -3           # stack 3 items
sw $ra, $sp, $imm, 2             # save $ra
sw $a1, $sp, $imm, 1             # save $a1
sw $a0, $sp, $imm, 0             # save $a0
add $a0, $zero, $zero, 0	     # $a0 = 0: row index

# Loop over Rows
loop_rows:
add $a1, $zero, $zero, 0		 # $a1 = 0: column index

# Loop over Columns
loop_columns:
jal $imm, $zero, $zero, calc    # calculate ($a0, $a1) value and save to memory
add $a1, $a1, $imm, 1            # $a1 = $a1+1
add $t0, $zero, $imm, 4          # $t0 = 4
blt $imm, $a1, $t0, loop_columns # loop column if $a1 < 4

# Check to Loop next Row
add $a0, $a0, $imm, 1            # $a0 = $a0+1
add $t0, $zero, $imm, 4          # $t0 = 4
blt $imm, $a0, $t0, loop_rows    # loop row if $a0 < 4

# End of mulmat
lw $a0, $sp, $imm, 0             # restore $a0
lw $a1, $sp, $imm, 1             # restore $a1
lw $ra, $sp, $imm, 2             # restore $ra
add $sp, $sp, $imm, 3            # restore stack
beq $ra, $zero, $zero, 0         # return


# Calculates (a0,a1) value entry and saves to memory
calc:
add $sp, $sp, $imm, -2           # stack 2 items
sw $s1, $sp, $imm, 1             # save $s1
sw $s0, $sp, $imm, 0             # save $s0

mul $t0, $a0, $imm, 4            # row offset
add $t0, $t0, $imm, 0x100        # $t0 = A(a0,0) address
add $t1, $a1, $imm, 0x110        # $t1 = B(0,a1) address
add $s1, $zero, $imm, 0x11F      # $s1 = 0x11F: to check out of bounds
add $v0, $zero, $zero, 0         # reset $v0

# Loop over summed values
loop:
lw $t2, $t0, $zero, 0            # $t2 = mem($t0)
lw $s0, $t1, $zero, 0            # $s0 = mem($t1)
mul $s0, $s0, $t2, 0             # $s0 = $t2 * $s0
add $v0, $v0, $s0, 0             # $v0 += $s0
add $t0, $t0, $imm, 1            # $t0 += 1: advance in matrix A
add $t1, $t1, $imm, 4            # $t1 += 4: advance in matrix B
ble $imm, $t1, $s1, loop         # add next value if not out of bound

# Found value, save to memory
mul $t0, $a0, $imm, 4            # $t0 = row offset
add $t0, $t0, $a1, 0             # $t0 = C(a0,a1) address without offset
add $t0, $t0, $imm, 0x120        # $t0 = C(a0,a1) address
sw $v0, $t0, $zero, 0            # save $v0

# End of calc
lw $s0, $sp, $imm, 0             # restore $s0
lw $s1, $sp, $imm, 1             # restore $s1
add $sp, $sp, $imm, 2            # restore stack
beq $ra, $zero, $zero, 0         # return