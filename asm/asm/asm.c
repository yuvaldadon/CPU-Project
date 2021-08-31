#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#include "env.h"

int main(int argc, char** argv) {
	
	//Load files
	FILE* file_asm = fopen(argv[1], "r");
	if (file_asm == NULL)
		exit(1);
	FILE* file_imem = fopen(argv[2], "w");
	if (file_imem == NULL)
		exit(1);
	FILE* file_dmem = fopen(argv[3], "w");
	if (file_dmem == NULL)
		exit(1);

	//Initialize memories and label list
	int imemory[IMEMORY_SIZE] = { 0 };
	int dmemory[DMEMORY_SIZE] = { 0 };

	Label_List label_list;
	label_list.list_size = 0;
	label_list.label_list = NULL;

	// First Run - Handle .word, Collect lables, Store instructions in desired format on list
	char instruction_list[IMEMORY_SIZE][LINE_SIZE];
	char line[LINE_SIZE];
	int instructions_cnt = 0, imemory_cnt = 0;

	while (NULL != fgets(line, LINE_SIZE, file_asm)) {

		// Handle .word commands
		if (word_command(line, dmemory))
			continue;

		// Collect labels and Formats instructions
		char new_instruction[LINE_SIZE];
		collect_labels_instructions(line, new_instruction, imemory_cnt, &label_list);
		if (is_empty(new_instruction))
			continue;

		// Add formatted instructions to list
		strcpy(instruction_list[instructions_cnt], new_instruction);

		// Save additional memory line for imm instructions (to keep track of label addresses)
		if (is_imm(new_instruction))
			imemory_cnt++;

		imemory_cnt++;
		instructions_cnt++;
	}

	// Second Run - Insert instructions to memory
	int instruction_index = 0, imemory_index = 0;
	while (instruction_index < instructions_cnt) {
		bool imm_check = false;
		char* instruction = instruction_list[instruction_index];
		imm_check = add_instruction(instruction ,imemory, imemory_index, &label_list);
		if (imm_check)
			imemory_index++;

		imemory_index++;
		instruction_index++;
	}

	// Save instructions and data to memory files
	print_imemory_file(imemory, file_imem);
	print_dmemory_file(dmemory, file_dmem);

	// Clears objects
	fclose(file_asm);
	fclose(file_imem);
	fclose(file_dmem);
	clear_label_list(&label_list);

	return 0;
}

// Execute .word commands on data memory
bool word_command(const char* line, int* memory) {
	int i = 0;
	// skip to start of command
	while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0' && i < LINE_SIZE) {
		if (!(line[i] == ' ' || line[i] == '\t'))
			break;
		i++;
	}
	// checks if .word line
	if (line[i] == '.') {
		char* dummy;
		while (!(line[i] == ' ' || line[i] == '\t')) // skips 'word'
			i++;
		while (line[i] == ' ' || line[i] == '\t') // skips whitespaces after '.word'
			i++;

		// handles address of command
		int address;
		if (line[i + 1] == 'x')
			address = strtol(&line[i], &dummy, 16);
		else
			address = strtol(&line[i], &dummy, 10);

		while (!(line[i] == ' ' || line[i] == '\t'))
			i++;
		while (line[i] == ' ' || line[i] == '\t')
			i++;

		// handles data of command
		int data;
		if (line[i + 1] == 'x')
			data = strtol(&line[i], &dummy, 16);
		else
			data = strtol(&line[i], &dummy, 10);

		memory[address] = data;

		return true;
	}
	return false;
}

// Collects labels to list and Instructions in desired format
void collect_labels_instructions(const char* line, char temp_line[LINE_SIZE], int memory_index, Label_List* label_list) {
	int temp_index = 0, line_index = 0;
	bool found_opcode = false, detected_word = false;
	while (line_index < LINE_SIZE && line[line_index] != '\r' && line[line_index] != '\n' && line[line_index] != '\0') {

		// Look for labels and add to list
		if (line[line_index] == ':') {
			temp_line[temp_index] = '\0';
			Label* label = init_label(memory_index, temp_line);
			append_label(label_list, label);
			line_index++;
			// reset to find opcode again
			detected_word = false;
			temp_index = 0;
			temp_line[temp_index] = '\0';
			continue;
		}

		// Look for instruction, if found copies line without spaces to convert to desired format
		if (line[line_index] == ' ' || line[line_index] == '\t') {
			// Enters once after finding opcode
			if (detected_word && !found_opcode) {
				temp_line[temp_index] = line[line_index];
				detected_word = false;
				found_opcode = true;
				temp_index++;
			}
			line_index++;
			continue;
		}

		if (line[line_index] == '#')
			break;

		// Found word
		temp_line[temp_index] = line[line_index];
		detected_word = true;
		line_index++;
		temp_index++;
	}
	temp_line[temp_index] = '\0';
}

// Add instruction to memory
bool add_instruction(char* instruction, int* memory, int index, Label_List* label_list) {
	int op, rd, rs, rt, imm;
	int entry = 0;
	bool imm_inst = is_imm(instruction); // check here before breaking instruction

	// grab opcode
	char* str = strtok(instruction, " ");
	op = convert_opcode(str);
	// grab instruction parts
	str = strtok(NULL, ",");
	rd = convert_regs(str);
	str = strtok(NULL, ",");
	rs = convert_regs(str);
	str = strtok(NULL, ",");
	rt = convert_regs(str);
	str = strtok(NULL, ",");
	// saves imm incase imm instruction
	imm = convert_imm(str, label_list);
	// add fields to memory
	entry = op<<4;
	entry = (entry|rd)<<4;
	entry = (entry|rs)<<4;
	entry = (entry|rt);
	memory[index] = entry;

	// handle adding imm
	if (imm_inst)
	{
		memory[index + 1] = imm;
		return true;
	}
	return false;
}

// Decode imm label or constant
int convert_imm(char* str, Label_List* label_list) {
	char* dummy;
	int val;
	if ((str[0] >= '0' && str[0] <= '9') || str[0] == '-')
	{
		if (str[1] == 'x') //is hexa
			val = strtol(str, &dummy, 16);
		else
			val = strtol(str, &dummy, 10);
	}
	else
		val = label_address(str, label_list);
	return val;
}

// Converts opcode to value
int convert_opcode(char* str) {
	if (strcmp(str, "add") == 0) return 0;
	if (strcmp(str, "sub") == 0) return 1;
	if (strcmp(str, "and") == 0) return 2;
	if (strcmp(str, "or") == 0) return 3;
	if (strcmp(str, "xor") == 0) return 4;
	if (strcmp(str, "mul") == 0) return 5;
	if (strcmp(str, "sll") == 0) return 6;
	if (strcmp(str, "sra") == 0) return 7;
	if (strcmp(str, "srl") == 0) return 8;
	if (strcmp(str, "beq") == 0) return 9;
	if (strcmp(str, "bne") == 0) return 10;
	if (strcmp(str, "blt") == 0) return 11;
	if (strcmp(str, "bgt") == 0) return 12;
	if (strcmp(str, "ble") == 0) return 13;
	if (strcmp(str, "bge") == 0) return 14;
	if (strcmp(str, "jal") == 0) return 15;
	if (strcmp(str, "lw") == 0) return 16;
	if (strcmp(str, "sw") == 0) return 17;
	if (strcmp(str, "reti") == 0) return 18;
	if (strcmp(str, "in") == 0) return 19;
	if (strcmp(str, "out") == 0) return 20;
	if (strcmp(str, "halt") == 0) return 21;
	return -1;
}

// Converts regs to value
int convert_regs(char* str) {
	if (strcmp(str, "$zero") == 0) return 0;
	if (strcmp(str, "$imm") == 0) return 1;
	if (strcmp(str, "$v0") == 0) return 2;
	if (strcmp(str, "$a0") == 0) return 3;
	if (strcmp(str, "$a1") == 0) return 4;
	if (strcmp(str, "$t0") == 0) return 5;
	if (strcmp(str, "$t1") == 0) return 6;
	if (strcmp(str, "$t2") == 0) return 7;
	if (strcmp(str, "$t3") == 0) return 8;
	if (strcmp(str, "$s0") == 0) return 9;
	if (strcmp(str, "$s1") == 0) return 10;
	if (strcmp(str, "$s2") == 0) return 11;
	if (strcmp(str, "$gp") == 0) return 12;
	if (strcmp(str, "$sp") == 0) return 13;
	if (strcmp(str, "$fp") == 0) return 14;
	if (strcmp(str, "$ra") == 0) return 15;
	return -1;
}

// Checks if string is empty
bool is_empty(const char* line) {
	return (line[0] == '\0');
}

// Checks if instruction is imm type
bool is_imm(const char* line) {
	char imm_str[] = "$imm";
	if (strstr(line, imm_str) != NULL)
		return true;
	return false;
}

// Allocate new label
Label* init_label(int address, char* line) {
	Label* label = (Label*)malloc(sizeof(Label));
	label->address = address;
	strcpy(label->name, line);
	return label;
}

// Add label to label list
void append_label(Label_List* label_list, Label* label) {
	int list_size = label_list->list_size;
	Label** new_list = (Label**)realloc(label_list->label_list, sizeof(Label*) * (list_size + 1));
	new_list[list_size] = label;

	label_list->list_size++;
	label_list->label_list = new_list;
}

// Return label's address
int label_address(char* name, Label_List* label_list) {
	int i;
	for (i = 0; i < label_list->list_size; i++) {
		if (strcmp(label_list->label_list[i]->name, name) == 0)
			return label_list->label_list[i]->address;
	}
	return NULL;
}

// Clears allocated memory of label list
void clear_label_list(Label_List* label_list) {
	int i;
	int list_size = label_list->list_size;
	for (i = 0; i < list_size; i++) {
		free(label_list->label_list[i]);
	}
}

// Adds instructions to memory file
void print_imemory_file(int* memory, FILE* fp) {
	int i;
	for (i = 0; i < IMEMORY_SIZE; i++)
		fprintf(fp, "%.5X\n", (0xFFFFF & memory[i]));
}

// Adds data to memory file
void print_dmemory_file(int* memory, FILE* fp) {
	int i;
	for (i = 0; i < DMEMORY_SIZE; i++)
		fprintf(fp, "%.8X\n", (0xFFFFFFFF & memory[i]));
}