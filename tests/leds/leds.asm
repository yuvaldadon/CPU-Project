
# Init
	add $s0, $zero, $imm, 0xFFFFFFFF

# Handle IO regs
	add $t0, $zero, $imm, 1       # $t0 = 1
	out $t0, $zero, $imm, 0       # set: irq0enable = 1
	add $t0, $zero, $imm, intr    # $t0 = intr
	out $t0, $zero, $imm, 6       # set: irqhandler = intr
	add $t0, $zero, $imm, 1024    # $t0 = 1024
	out $t0, $zero, $imm, 13      # set: timermax = 1024 cycles (1 sec)
	add $t0, $zero, $imm, 1       # $t0 = 1
	out $t0, $zero, $imm, 11      # set: timerenable = 1
	add $t1, $zero, $zero, 0      # $t1=0: leds indicator

loop:
	sll $t1, $t1, $imm, 1        # prepare next leds(1)
	add $t1, $t1, $imm, 1		 # prepare next leds(2)	
	out $t1, $zero, $imm, 9      # activate leds

# waits for timer interrupt
wait:
	in $t0, $zero, $imm, 3       # $t0 = irq0status
	beq $imm, $t0, $zero, wait   # wait if not intr
	out $zero, $zero, $imm, 3    # reset irq0status
	bne $imm, $s0, $t1, loop     # loop if not done

# done
exit:
	out $zero, $zero, $imm, 0     # set: timerenable = 0
	halt $zero, $zero, $zero, 0   # halt

# timer interrupt
intr:
	reti $zero, $zero, $zero, 0	  # return from interrupt