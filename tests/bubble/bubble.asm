# Initial Memory
.word 1024 12
.word 1025 25
.word 1026 64
.word 1027 130
.word 1028 255
.word 1029 20
.word 1030 7
.word 1031 13
.word 1032 14
.word 1033 458
.word 1034 18
.word 1035 1
.word 1036 3
.word 1037 9
.word 1038 8
.word 1039 4


# Init and call function
add $a0, $zero, $imm, 1024     # $a0 = 1024: start of array
add $a1, $zero, $imm, 1039     # $a1 = 1039: end of array
add $s0, $zero, $imm, 1024     # $s0 = 1024: higher in pair
add $s1, $zero, $imm, 1025     # $s1 = 1025: lower in pair
jal $imm, $zero, $zero, bubble # bubble sort
halt $zero, $zero, $zero, 0

# Switch pair of values
switch:
sw $t0, $s1, $zero, 0         # s0 <- s1
sw $t1, $s0, $zero, 0         # s1 <- s0

# Perform Bubble Sort
bubble:
lw $t0, $s0, $zero, 0          # $t0 = mem($s0)
lw $t1, $s1, $zero, 0          # $t1 = mem($s1)
bgt $imm, $t1, $t0, switch     # s0 <-> s1
add $s0, $s0, $imm, 1          # $s0 = $s0++
add $s1, $s1, $imm, 1          # $s1 = $s1++
blt $imm, $s0, $a1, bubble     # move to next pair if not reached bottom

# Finished full iteration, decreasing array size by 1
add $a1, $a1, $imm, -1         # $a1 = $a1 - 1
beq $ra, $a0, $a1, 0           # finish if $a0 reached bottom of array
add $s0, $a0, $zero, 0         # $s0 = $a0
add $s1, $a0, $imm, 1          # $s1 = $a0++
beq $imm, $zero, $zero, bubble # loop with new array