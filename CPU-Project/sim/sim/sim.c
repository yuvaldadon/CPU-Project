#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define InstLen 6 // Length of each instruction + /0 space
#define MemInstLen 1024 
#define MemLen 4096
#define SECTOR 128


typedef struct _command //structure of command oren project
{
	char comm[6];
	char Op_com[3];
	char rd_com[2];
	char rs_com[2];
	char rt_com[2];
	char immidiate_com[6];
}command;


command comm_arr[MemInstLen] = { NULL }; // RAM
int op, pctemp = 0, imm_dec, mem_idx, pc4trace, InsCount = 0, counter = 0, TRUE = 0, pc = 0, hdlinewrite = 0, counterhw, reti = 0, irq2_time = 0, irq2_finish = 0, valid = 0;
char Mem[MemLen][9];
char reg[16][9] = { "00000000","00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" }; //cache
char ioreg[22][9] = { "00000000", "00000000","00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000" ,"00000000","00000000" ,"00000000" ,"00000000" ,"00000000" }; //cache
char ionames[22][50] = { "irq0enable","irq1enable","irq2enable","irq0status","irq1status","irq2status","irqhandler","irqreturn","clks","leds","reserved","timerenable","timercurrent","timermax","diskcmd","disksector","diskbuffer","diskstatus","monitorcmd","monitorx","monitory","monitordata" };
int mon[288][352];
char R_s[9], R_t[9], R_d[9], Opcode[3], rd[2], rs[2], rt[2], immidiate[6];
long long r_s_dec = 0;

FILE* monitorYUV = NULL;
FILE* imemin = NULL;
FILE* dmemin = NULL;
FILE* trace = NULL;
FILE* irq2in = NULL;
FILE* diskin = NULL;
FILE* dmemout = NULL;
FILE* regout = NULL;
FILE* hwregtrace = NULL;
FILE* cycles = NULL;
FILE* leds = NULL;
FILE* monitor = NULL;
FILE* diskout = NULL;
FILE* ptr = NULL;

////////////////////////////FUNCTIONS///////////////////////////////////
FILE* open_file(char* fname, const char* mode); // opens a file and detects failiure
void write(FILE* file);						    // write to trace.txt
char* decto8Hex(int dec1);						// converts a decimal number to 8 digit Hexa
char* decto3Hex(int dec1);						// converts a decimal number to 3 digit Hexa
char* decto2Hex(int dec1);						// converts a decimal number to 2 digit Hexa
long long convert_hextodec(char* hex);			// converts a Hexa number to decimal
void simulation_process();						// simulates the processor's instructions process 
int chekpsika(int counter);						// checks irq2 type of psika at the current clock cycle 
void hd_rw(int diskcmd, FILE* diskout);			// Recieves diskcmd and a pointer to diskout, according do cmd writes or reads
void interrupts(int counter);					// checks if a psika occured, updating the pc and keeps the previous pc
void open_files(char* argv[]);					// opens a bunch of files using "open_file" function adn return pointers to the files
void close_files();								// closes the files
void halt();									// initiates in case of recieving opcode 0x15, prints data to output files
int out();										// deals with opcode 0x14
void opcode_switch();							// switch of all opcode cases
void initial_Mem2Arr_and_InstMem2Arr();			// converting imemin.txt and dmemin.txt files to arrays
void initial_mon();								// initials monitor to a matrix full of '0'
void copy_diskin(FILE* diskin, FILE* diskout);  // copies diskin to diskout at the beginning of the program

int main(int argc, char* argv[])
{
	open_files(argv);
	initial_mon();
	initial_Mem2Arr_and_InstMem2Arr();
	if (valid == 0)
		simulation_process();

	close_files(); //closing all files together
	return 0;
}

FILE* open_file(char* fname, const char* mode) /*This function unites the file opening and error handling*/ {
	fopen_s(&ptr, fname, mode);
	if (!ptr)
	{
		printf("Error opening %s\n", fname);
		exit(0);
	}
	return ptr;
}

void open_files(char* argv[]) {
	imemin = open_file(argv[1], "r");
	dmemin = open_file(argv[2], "r");
	diskin = open_file(argv[3], "r");
	irq2in = open_file(argv[4], "r");
	dmemout = open_file(argv[5], "w");
	regout = open_file(argv[6], "w");
	trace = open_file(argv[7], "w");
	hwregtrace = open_file(argv[8], "w");
	cycles = open_file(argv[9], "w");
	leds = open_file(argv[10], "w");
	monitor = open_file(argv[11], "w");
	monitorYUV = fopen(argv[12], "wb");
	diskout = open_file(argv[13], "w+");
}
void initial_Mem2Arr_and_InstMem2Arr()
{
	//////////////////////////////////convert dmemin to array
	char line[9];
	for (int j = 0; j < MemLen; j++)
	{
		fscanf(dmemin, " %[^, \t\n] ", line); // copy the word
		strcpy(Mem[j], line);
	}
	////////////////////////////////////convert imemin to array
	int i = 0;
	char line2[InstLen];
	char imm[InstLen];
	int j = 0;
	for (j; j < MemInstLen; j++)
	{
		strcpy(comm_arr[j].comm, "00000000");
		strcpy(comm_arr[j].Op_com, "00");
		strcpy(comm_arr[j].rd_com, "0");
		strcpy(comm_arr[j].rs_com, "0");
		strcpy(comm_arr[j].rt_com, "0");
		strcpy(comm_arr[j].immidiate_com, "000");
	}
	while (!feof(imemin))
	{
		char* ptr = NULL;
		fscanf(imemin, " %[^, \t\n] ", line2); // copy the word

		strcpy(comm_arr[i].comm, line2);
		ptr = line2;
		strncpy(Opcode, line2, 2);
		strcpy(comm_arr[i].Op_com, Opcode);//Updating Command structure parameters
		ptr = ptr + 2;

		strncpy(rd, ptr, 1);//Updating Command structure parameters
		strcpy(comm_arr[i].rd_com, rd);
		ptr = ptr + 1;
		strncpy(rs, ptr, 1);//Updating Command structure parameters
		strcpy(comm_arr[i].rs_com, rs);
		ptr = ptr + 1;

		strncpy(rt, ptr, 1);//Updating Command structure parameters
		strcpy(comm_arr[i].rt_com, rt);
		ptr = ptr + 1;

		char imm[InstLen];
		if (strcmp(comm_arr[i].rd_com, "1") == 0 || strcmp(comm_arr[i].rs_com, "1") == 0 || strcmp(comm_arr[i].rt_com, "1") == 0)// //Updating immidiate parameter if needed
		{
			fscanf(imemin, " %[^, \t\n] ", line2);
			strcpy(imm, line2);
			strcpy(comm_arr[i].immidiate_com, imm);
			strcpy(comm_arr[i + 1].comm, imm);
			i += 2;
		}
		else
			i++;
	}
}
void initial_mon()
{
	for (int i = 0; i < 288; i++)
		for (int j = 0; j < 352; j++)
			mon[i][j] = 0;
}
void copy_diskin(FILE* diskin, FILE* diskout)
{
	char c = fgetc(diskin);
	while (c != EOF)
	{
		fputc(c, diskout);
		if (c == 10) hdlinewrite++;
		c = fgetc(diskin);
	}
	if (hdlinewrite != 0) hdlinewrite++;
}

void simulation_process()
{
	copy_diskin(diskin, diskout); //copying diskin to diskout
	fclose(diskin);

	while (TRUE == 0) //run till we get to halt command
	{
		counter++;
		if (pc == 1023) //pc has limit of 1024
			pc = 0;
		imm_dec = convert_hextodec(comm_arr[pc].immidiate_com);//Converts hex immidiate to dec
		strcpy(reg[1], decto8Hex(imm_dec)); //First we update the immidiate
		write(trace); // Then we write to the file
		op = (int)strtol(comm_arr[pc].Op_com, 0, 16);
		if (op < 0 || op>21) // Illegal opcode gets the value 22
			op = 22;
		strcpy(R_s, reg[(int)strtol(comm_arr[pc].rs_com, 0, 16)]);//Fetch Registers Values
		strcpy(R_t, reg[(int)strtol(comm_arr[pc].rt_com, 0, 16)]);
		strcpy(R_d, reg[(int)strtol(comm_arr[pc].rd_com, 0, 16)]);
		pctemp = pc;

		opcode_switch(); //Executing the relevant command


/////////////////////////////////////////////////

		if ((op < 9) || ((op > 15) && (op != 18))) //updating the PC if it wasn't updated by other commands
			if (op != 22 && strcmp(rd, "1") != 0) //If opcpde=22 we want to do nothing
			{
				strcpy(reg[(int)strtol(comm_arr[pc].rs_com, 0, 16)], R_s);
				strcpy(reg[(int)strtol(comm_arr[pc].rt_com, 0, 16)], R_t);
				strcpy(reg[(int)strtol(comm_arr[pc].rd_com, 0, 16)], R_d);
			}
		if (convert_hextodec(ioreg[11]) == 1) //timer
		{
			if ((convert_hextodec(ioreg[12]) < convert_hextodec(ioreg[13])))
			{
				if ((strcmp(comm_arr[pc].rd_com, "1") == 0 || strcmp(comm_arr[pc].rs_com, "1") == 0 || strcmp(comm_arr[pc].rt_com, "1") == 0))
					strcpy(ioreg[12], decto8Hex(convert_hextodec(ioreg[12]) + 2)); // timercurrent++2
			}
			else
				strcpy(ioreg[12], decto8Hex(convert_hextodec(ioreg[12]) + 1)); // timercurrent++
		}
		////////////////////////////////////////////////////


		if ((op < 9 || op>15) & op != 18)
		{
			if (strcmp(comm_arr[pc].rd_com, "1") == 0 || strcmp(comm_arr[pc].rs_com, "1") == 0 || strcmp(comm_arr[pc].rt_com, "1") == 0)// //Updating immidiate parameter if needed
			{

				pc += 2;
				counter++;
				InsCount++;
			}
			else
			{

				pc += 1;
				InsCount++;
			}
		}
		else
		{

			if (strcmp(comm_arr[pctemp].rd_com, "1") == 0 || strcmp(comm_arr[pctemp].rs_com, "1") == 0 || strcmp(comm_arr[pctemp].rt_com, "1") == 0)// //Updating immidiate parameter if needed
			{
				counter++;
				InsCount += 2;

			}
			else
			{
				InsCount++;
			}
		}
		if (reti == 0) //handling interrupts only if we are not in an interrupt event
			interrupts(counter);
	}

}
void opcode_switch() {
	if ((op < 9 || op == 16 || op == 19) && (((int)strtol(comm_arr[pc].rd_com, 0, 16) == 1) || ((int)strtol(comm_arr[pc].rd_com, 0, 16) == 0))) op = 22; //if it tries to write to $imm or $0, no command is executed
	switch (op) {
	case 0: //add: R[rd]=R[rs]+R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) + convert_hextodec(R_t)));
		break;
	case 1: //sub: R[rd]=R[rs]-R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) - convert_hextodec(R_t)));
		break;
	case 2: //and: R[rd]=R[rs]&R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) & convert_hextodec(R_t)));
		break;
	case 3: //or: R[rd] = R[rs] | R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) | convert_hextodec(R_t)));
		break;
	case 4: //xor: R[rd] = R[rs] ^ R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) ^ convert_hextodec(R_t)));
		break;
	case 5: //mul: R[rd] = R[rs] * R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) * convert_hextodec(R_t)));
		break;
	case 6: //sll: R[rd] = R[rs] << R[rt]
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) << convert_hextodec(R_t)));
		break;
	case 7: //sra: R[rd] = R[rs] >> R[rt], arithmetic shift with sign extension. The sign bit is shifted in.
		strcpy(R_d, decto8Hex(convert_hextodec(R_s) >> convert_hextodec(R_t)));
		break;
	case 8: //srl: R[rd] = R[rs] >> R[rt], logical shift. Zeroes are shifted in.
		if (convert_hextodec(R_t) > 0 && convert_hextodec(R_s) < 0)
		{
			r_s_dec = convert_hextodec(R_s) + pow(2, 31);
			r_s_dec = r_s_dec >> 1;
			r_s_dec += pow(2, 30);
			strcpy(R_d, decto8Hex(r_s_dec >> convert_hextodec(R_t) - 1));
		}
		else
			strcpy(R_d, decto8Hex(convert_hextodec(R_s) >> convert_hextodec(R_t)));
		break;


	case 9: //beq :if (R[rs] == R[rt]) pc = R[rd][low bits 9:0]
			// comparing two registers in the converted 16-base index:
		if (strcmp(R_s, R_t) == 0)
			pc = (int)strtol((R_d), 0, 16) & 1023;
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}
		break;


	case 10: //bne: if (R[rs] != R[rt]) pc = R[rd] [low bits 9:0]
			// comparing two registers in the converted 16-base index:
		if (strcmp(R_s, R_t) != 0)
			pc = (int)strtol((R_d), 0, 16) & 1023;
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}
		break;


	case 11: //blt : if (R[rs] < R[rt]) pc = R[rd] [low bits 9:0]
			// comparting the *actual value* of two registers in the converted 16-base index:
		if (convert_hextodec(R_s) < convert_hextodec(R_t))
		{
			pc = (int)strtol((R_d), 0, 16) & 1023;
		}
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}
		break;


	case 12: //bgt: if (R[rs] > R[rt]) pc = R[rd] [low bits 9:0]
		if (convert_hextodec(R_s) > convert_hextodec(R_t))
		{
			pc = (int)strtol((R_d), 0, 16) & 1023;
		}
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}

		break;


	case 13://ble: if (R[rs] <= R[rt]) pc = R[rd] [low bits 9:0]
		if (convert_hextodec(R_s) <= convert_hextodec(R_t))
		{
			pc = (int)strtol((R_d), 0, 16) & 1023;

		}
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}
		break;


	case 14: //bge:if (R[rs] >= R[rt]) pc = R[rd] [low bits 9:0]
		if (convert_hextodec(R_s) >= convert_hextodec(R_t))
		{
			pc = (int)strtol((R_d), 0, 16) & 1023;
		}
		else
		{
			if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
				pc += 2;
			else
				pc++;
		}
		break;


	case 15: //jal:R[15] = pc + 1 (next instruction address), pc = R[rd][9:0]

		if ((strcmp(comm_arr[pc].rd_com, "1") == 0))
		{
			pc += 2;
			strcpy(reg[15], decto8Hex(pc)); //saving our PC as a Return Address
			pc = (int)strtol((R_d), 0, 16) & 1023; //updating the PC;
		}
		else
		{
			pc++;
			strcpy(reg[15], decto8Hex(pc)); //saving our PC as a Return Address
			pc = (int)strtol((R_d), 0, 16) & 1023; //updating the PC;
		}

		break;
	case 16: //lw:R[rd] = MEM[R[rs]+R[rt]]
		mem_idx = convert_hextodec(R_s) + convert_hextodec(R_t);
		if (mem_idx >= 0 && mem_idx < MemLen)
			strcpy(R_d, Mem[mem_idx]); // validating the mem_idx is legal
		break;
	case 17://sw:MEM[R[rs]+R[rt]] = R[rd]
		mem_idx = convert_hextodec(R_s) + convert_hextodec(R_t);
		if (mem_idx >= 0 && mem_idx < MemLen)
			strcpy(Mem[mem_idx], R_d); // validating the mem_idx is legal

		break;
	case 18: //reti:PC = ioregister[7]
		pc = (int)strtol(((ioreg[7])), 0, 16);
		reti = 0;
		break;
	case 19: //in:R[rd] = ioregister[R[rs] + R[rt]]
		mem_idx = convert_hextodec(R_s) + convert_hextodec(R_t);
		if (mem_idx < 0 || mem_idx>21) break;  // validating the mem_idx is legal
		if (mem_idx == 18) strcpy(R_d, ioreg[0]);// return 0 if trying to read monitorcmd
		else strcpy(R_d, ioreg[mem_idx]);
		fprintf(hwregtrace, "%d READ %s %s\n", counter, ionames[mem_idx], ioreg[mem_idx]);
		break;
	case 20: //out:ioregister [R[rs]+R[rt]] = R[rd]
		out();
		break;
	case 21: //halt
		halt();
		break;
	}
}

void halt() {
	TRUE = 1;

	char monData_HEX[3];

	int j = 0;
	for (int i = 0; i < 288; i++)   // copy mon matrix to monitor.txt
		for (int j = 0; j < 352; j++)
		{
			strcpy(monData_HEX, decto2Hex(mon[i][j]));
			fprintf(monitor, "%s\n", monData_HEX);
			fprintf(monitorYUV, "%s", monData_HEX);
		}
	for (int j = 0; j < MemLen; j++) // copy Mem to the output Mem.txt.
		fprintf(dmemout, "%s\n", Mem[j]);
	for (j = 2; j < 16; j++)				 // copy registers table to the output regout.txt.
		fprintf(regout, "%s\n", reg[j]);
	fprintf(cycles, "%d\n%d\n ", counter + 1, InsCount + 1);

}
int out() {

	int rd;

	mem_idx = convert_hextodec(R_s) + convert_hextodec(R_t);
	if (mem_idx < 0 || mem_idx>21 || (((mem_idx == 14) || (mem_idx == 15) || (mem_idx == 16)) && convert_hextodec(ioreg[17]))) return 0; //if mem_idx is not legal,or if we try to update harddisk regs before time, we do nothing
	strcpy(ioreg[mem_idx], R_d);//Transferring R_d to the relavent I/O_register
	int ioreg16 = convert_hextodec(ioreg[16]), ioreg15 = convert_hextodec(ioreg[15]);
	fprintf(hwregtrace, "%d WRITE %s %s\n", counter, ionames[mem_idx], ioreg[mem_idx]);
	if (mem_idx == 9)
		fprintf(leds, "%d %s\n", counter, ioreg[mem_idx]);

	if (((int)strtol(ioreg[18], 0, 16) == 1)) //monitorcmd==1, that means the data value is copied to mon matrix
		mon[(int)strtol(ioreg[19], 0, 16)][(int)strtol(ioreg[20], 0, 16)] = (int)strtol(ioreg[21], 0, 16);

	if (mem_idx == 14 && (convert_hextodec(R_d) != 0)) { // if we're updating diskcmd to value different than '0'
		if (((int)strtol(ioreg[16], 0, 16) + SECTOR) > (MemLen)) return 0; //if the RAM address is too high to be read or to be written in, this opcode won't work
		strcpy(ioreg[17], decto8Hex(1)); //diskstatus = 1 (busy)
		counterhw = counter + 1024;
		hd_rw(convert_hextodec(R_d), diskout); //reading/writing to disk at the beginning
	}
	return 1;
}
void write(FILE* file)   //write to file trace.txt
{
	char PC_HEX[4];
	strcpy(PC_HEX, decto3Hex(pc));
	fprintf(file, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s\n", PC_HEX, comm_arr[pc].comm, reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7], reg[8], reg[9], reg[10], reg[11], reg[12], reg[13], reg[14], reg[15]);
}
void hd_rw(int diskcmd, FILE* diskout) {
	int source_idx, dest_idx, i;
	switch (diskcmd) {
	case 1:
		//case 1: //Reading from harddisk to mem
		source_idx = convert_hextodec(ioreg[15]) * SECTOR * 10; //Sector starting point multiply by line's length (10)
		dest_idx = (long)strtol(ioreg[16], 0, 16); //Destination line in RAM
		if (fseek(diskout, source_idx, SEEK_SET) != 0) { //Moves the pointer to the relevant point, handling error if occurs
			printf("Error reading from disk, Please check your disk content");
			exit(0);
		}

		for (i = 0; i < SECTOR; i++)
			fscanf(diskout, " %[^, \n]", Mem[dest_idx + i]);
		break;
	case 2://Writing to harddisk from mem
		source_idx = (long)strtol(ioreg[16], 0, 16); //Source line in RAM
		dest_idx = convert_hextodec(ioreg[15]) * SECTOR * 10; //Sector starting point multiply by line's length (10)
		if (hdlinewrite < (dest_idx / 10)) //if we want to write beyond the last line we stopped at we fill the gap with zeros
		{
			if (hdlinewrite != 0)
			{
				fseek(diskout, hdlinewrite * 10 - 2, SEEK_SET);
				fputc('\n', diskout);
			}
			else fseek(diskout, hdlinewrite, SEEK_SET); {

				for (hdlinewrite; hdlinewrite < (dest_idx / 10); hdlinewrite++)  //Padding with zeros between lines
					fprintf(diskout, "%s\n", reg[0]); }
			hdlinewrite += SECTOR; //updating the last line index
		}
		else if (hdlinewrite == (dest_idx / 10)) //if we want to write at the line we stopped at
		{
			hdlinewrite += SECTOR; //updating the last line index
			fseek(diskout, dest_idx - 2, SEEK_SET);
			fputc('\n', diskout);
		}
		else // if we want to write before the line we stopped at
		{
			fseek(diskout, dest_idx, SEEK_SET);
		}
		for (i = 0; i < SECTOR - 1; i++) { // actual writing process
			fprintf(diskout, "%s\n", Mem[source_idx + i]);

		}
		fprintf(diskout, "%s", Mem[source_idx + i]); //last line writing is a special case
		break;
	}

}

long long convert_hextodec(char* hex) // for converting ONLY immidiate hexadecimal string to decimal number
{
	char first[2] = "";
	long long dec, limit;
	int len = strlen(hex);
	if (len == 8)
	{
		first[0] = hex[0];
		++hex;
		dec = (long)strtol(hex, 0, 16) + (long)strtol(first, 0, 16) * pow(16, 7);
	}
	else
	{
		dec = (long)strtol(hex, 0, 16); //built-in function -> converts array to int with given base (in this case base=16)
	}
	limit = pow(2, len * 4 - 1);
	if (dec < limit)// if dec<limit it represets a positive number
	{
		return dec;
	}
	dec = dec - (2 * limit); // otherwise it is a negative number and need to be recalculated
	return dec;

}
char* decto8Hex(int dec1)
{
	long long dec = dec1;
	if (dec < 0) {
		dec = pow(2, 32) + dec;
	}
	int i = 0, j, temp;
	char hex[9];

	while (dec != 0) {
		temp = dec % 16;
		//To convert integer into character
		if (temp < 10)
			temp = temp + 48; else
			temp = temp + 55;
		hex[i++] = temp;
		dec = dec / 16;
	}

	while (i < 8)
	{
		hex[i] = '0';
		i++;
	}

	hex[8] = '\0';
	strrev(hex);
	hex[8] = '\0';
	return hex;
}
char* decto3Hex(int dec1)
{
	long long dec = dec1;
	if (dec < 0) {
		dec = pow(2, 32) + dec;
	}
	int i = 0, j, temp;
	char hex[4];

	while (dec != 0) {
		temp = dec % 16;
		//To convert integer into character
		if (temp < 10)
			temp = temp + 48; else
			temp = temp + 55;
		hex[i++] = temp;
		dec = dec / 16;
	}

	while (i < 3)
	{
		hex[i] = '0';
		i++;
	}

	hex[3] = '\0';
	strrev(hex);
	hex[3] = '\0';
	return hex;
}
char* decto2Hex(int dec1)
{
	long long dec = dec1;
	if (dec < 0) {
		dec = pow(2, 32) + dec;
	}
	int i = 0, j, temp;
	char hex[3];

	while (dec != 0) {
		temp = dec % 16;
		//To convert integer into character
		if (temp < 10)
			temp = temp + 48; else
			temp = temp + 55;
		hex[i++] = temp;
		dec = dec / 16;
	}

	while (i < 2)
	{
		hex[i] = '0';
		i++;
	}

	hex[2] = '\0';
	strrev(hex);
	hex[2] = '\0';
	return hex;
}
void interrupts(int counter)
{
	int irq, irq0status = 0, irq1status = 0, irq2status = 0;

	if ((convert_hextodec(ioreg[12]) != 0) && (convert_hextodec(ioreg[12]) == convert_hextodec(ioreg[13]))) //checking for interrupt #0
	{
		irq0status = 1;
		strcpy(ioreg[3], decto8Hex(1)); //Setting irq0status=1
		strcpy(ioreg[12], decto8Hex(0)); //Setting timercurrent=0
	}
	if (counter >= counterhw && (convert_hextodec(ioreg[17]) == 1)) {//Checking for interrupt #1 - True if 1024 cycles were completed

		strcpy(ioreg[14], decto8Hex(0)); //Setting diskcmd=0
		strcpy(ioreg[17], decto8Hex(0));//Setting diskstatus=0
		irq1status = 1;
		strcpy(ioreg[4], decto8Hex(1));//Setting irq1status=1
	}
	if ((irq2_finish != 1) && ((chekpsika(counter) == 1)) && counter > 1) //Checking for interrupt #2
	{
		irq2status = 1;
		strcpy(ioreg[5], decto8Hex(1));//Setting irq2status=1
	}

	irq = (convert_hextodec(ioreg[0]) && irq0status) || ((convert_hextodec(ioreg[1]) && irq1status) || (convert_hextodec(ioreg[2]) && irq2status));

	if (irq == 1)
	{ //PC saving & updating
		strcpy(ioreg[7], decto8Hex(pc));
		pc = (int)strtol((ioreg[6]), 0, 16);
		reti = 1;
	}
}
int chekpsika(int counter)
{
	char interrupt_time[33] = { NULL };
	if (counter == 0) {// for first time-checking
		fscanf(irq2in, " %[^, \t\n] ", interrupt_time);
		irq2_time = atoi(interrupt_time);
	}

	if (irq2_time < counter) { //we always want the irq2_time to be bigger than our counter
		fscanf(irq2in, " %[^, \t\n] ", interrupt_time);
		irq2_time = atoi(interrupt_time);
	}

	if (feof(irq2in)) { //we want to know if we finished with interrupt #2
		irq2_finish = 1; // flag for finishing with interrupt #2
		return 0;
	}

	if (irq2_time == counter)
		return 1;

	else
		return 0;
}
void close_files()
{ //closing all files together
	fclose(dmemin);
	fclose(imemin);
	fclose(irq2in);
	fclose(diskin);
	fclose(dmemout);
	fclose(regout);
	fclose(trace);
	fclose(hwregtrace);
	fclose(leds);
	fclose(monitor);
	fclose(diskout);
}





