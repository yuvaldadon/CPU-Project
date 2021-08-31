# -----------------
# Peusdo Code
#
# $s0 = 1024 # final result pointer
# $t1 = 2048 # temporary sector pointer
#
# for i in sectors 1:3:
#     read sector i to $t1
#     for j in word 1:sector_len:
#         $s0(j) = xor($s0(j), $t1(j))
#
# write from $s0 to sector 4
#
# -----------------

# irq1enable - 1
# irq1status - 4

# diskcmd - 14 (0 - non, 1 - read sector, 2 - write sector)
# disksector - 15
# diskbuffer - 16
# diskstatus - 17 (0 - free to receive cmd, 1 - busy)


# Init
	add $s0, $zero, $imm, 1024   # $s0 = 1024: final sector in mem
	add $s1, $zero, $imm, 2048   # $s1 = 2048: temporary sector in mem
	add $s2, $zero, $imm, 128    # $s2 = 128: sector length in mem entries

# Handle IO regs
	add $t0, $zero, $imm, 1       # $t0 = 1
	out $t0, $zero, $imm, 1       # set: irq1enable = 1
	add $t0, $zero, $imm, intr    # $t0 = intr
	out $t0, $zero, $imm, 6       # set: irqhandler = intr

# Grab sector 0 to $s0
	out $zero, $zero, $imm, 15    # set: disksector = 0
	out $s0, $zero, $imm, 16      # set: diskbuffer = $s0
	add $t1, $zero, $imm, 0       # $t1 = 0: temporary sector index
	add $t0, $zero, $imm, 1       # $t0 = 1
	out $t0, $zero, $imm, 14      # set: diskcmd = 1
	beq $imm, $zero, $zero, wait  # wait

loop:
	lw $t2, $s0, $t0, 0           # $t2 = final($t0)
	lw $t3, $s1, $t0, 0           # $t3 = temp($t0)
	xor $t2, $t2, $t3, 0          # $t2 = final($t0) XOR temp($t0)
	sw $t2, $s0, $t0, 0           # save $t2 to final($t0)
	add $t0, $t0, $imm, 1         # $t0++
	blt $imm, $t0, $s2, loop      # loop if not done with sector

read_sector:
	add $t1, $t1, $imm, 1         # $t1++
	add $t0, $zero, $imm, 4       # $t0 = 4: max sector
	beq $imm, $t1, $t0, write     # write if finish sectors
	out $t1, $zero, $imm, 15      # set: disksector = $t1
	out $s1, $zero, $imm, 16      # set: diskbuffer = $s1
	add $t0, $zero, $imm, 1       # $t0 = 1
	out $t0, $zero, $imm, 14      # set: diskcmd = 1

# waits for disk interrupt
wait:
	in $t2, $zero, $imm, 17		        # $t2 = diskstatus
	bne $imm, $t2, $zero, wait	        # wait for disk
	add $t0, $zero, $imm, 4             # $t0 = 4: max sector
	beq $imm, $t1, $t0, exit            # exit if done
	beq $imm, $t1, $zero, read_sector   # read next sector if only grabbed sector 0 (first entry)
	add $t0, $zero, $zero, 0            # $t0 = 0: index
	beq $imm, $zero, $zero, loop        # next sector

# write final sector and halt
write:
	out $zero, $zero, $imm, 15    # set: disksector = 0
	out $s0, $zero, $imm, 16      # set: diskbuffer = $s0
	add $t0, $zero, $imm, 2       # $t0 = 2
	out $t0, $zero, $imm, 14      # set: diskcmd = 2
	beq $imm, $zero, $zero, wait  # wait

exit:
	halt $zero, $zero, $zero, 0   # halt

# timer interrupt
intr:
	out $zero, $zero, $imm, 4     # reset irq1status
	reti $zero, $zero, $zero, 0	  # return from interrupt