# ------------------------------
# monitorcmd - 18 (0 - no command, 1 - write pixel to monitor)
# monitorx - 19 (X : 0-351)
# monitory - 20 (Y : 0-287)
# monitordata - 21 (lum : 0-255)
# ----------------------------------

.word 0x100 15 # Radius

# Init
	add $a1, $zero, $imm, 175     # $a1 = 175: mid x
	add $a0, $zero, $imm, 143     # $a0 = 143: mid y
	lw $s2, $zero, $imm, 0x100    # $s2 = Radius
	add $s1, $a1, $s2, 0          # $s1 = mid x + R
	add $s0, $a0, $s2, 0          # $s0 = mid y + R

# Handle IO regs
	add $t0, $zero, $imm, 255     # $t0 = 255
	out $t0, $zero, $imm, 21      # set: monitordata = 255
	sub $t0, $a0, $s2, 0          # $t0 = $a0 - $s2: starting y index

# loop over y: (x, y) = ($t1, $t0)
loop_y:

	# loop over x
	sub $t1, $a1, $s2, 0          # $t1 = $a1 - $s2: starting x index
	Loop_x:

		# y distance
		sub $t2, $a0, $t0, 0         # $t2 = mid(y) - current(y)
		jal $imm, $zero, $zero, abs  # absolute value
		add $t3, $t2, $zero, 0       # $t3 = Y_distance

		# x distance
		sub $t2, $a1, $t1, 0         # $t2 = mid(x) - current(x)
		jal $imm, $zero, $zero, abs  # absolute value
		add $t3, $t3, $t2, 0         # $t3 = Y_distance + X_distance

		# light if d < R
		jal $imm, $zero, $zero, light # check light
		add $t1, $t1, $imm, 1         # $t1++
		blt $imm, $t1, $s1, Loop_x    # loop if not done with x pixels

	add $t0, $t0, $imm, 1            # $t0++
	blt $imm, $t0, $s0, loop_y       # loop if not done with y pixels
	beq $imm, $zero, $zero, exit     # exit if done

# light pixel of in radius
light:
	bgt $ra, $t3, $s2, 0          # stay if $t3 <= Radius
	out $t0, $zero, $imm, 20      # set: monitory = $t0
	out $t1, $zero, $imm, 19      # set: monitorx = $t1
	add $t2, $zero, $imm, 1       # $t2 = 1
	out $t2, $zero, $imm, 18      # set: monitorcmd = 1
	beq $ra, $zero, $zero, 0      # return

# converts neg value to pos
abs:
	bge $ra, $t2, $zero, 0        # return if $t2 >= 0
	mul $t2, $t2, $imm, -1        # $t2 <- positive
	beq $ra, $zero, $zero, 0      # return

# exit if done
exit:
	halt $zero, $zero, $zero, 0   # halt