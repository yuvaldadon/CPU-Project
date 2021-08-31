#ifndef Assembler_header
#define Assembler_header

// Params
#define IMEMORY_SIZE 1024
#define DMEMORY_SIZE 4096
#define LABEL_SIZE 50
#define LINE_SIZE 500

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Structs
typedef struct _Label {
	char name[LABEL_SIZE];
	int address;
} Label;

typedef struct _Label_List {
	Label** label_list;
	int list_size;
} Label_List;

// Functions
bool word_command(const char* line, int* memory);
void collect_labels_instructions(const char* line, char temp_line[LINE_SIZE], int memory_index, Label_List* label_list);
bool add_instruction(char* instruction, int* memory, int index, Label_List* label_list);
int convert_imm(char* str, Label_List* label_list);
int convert_opcode(char* str);
int convert_regs(char* str);
bool is_empty(const char* line);
bool is_imm(const char* line);
Label* init_label(int address, char* line);
void append_label(Label_List* label_list, Label* label);
int label_address(char* name, Label_List* label_list);
void clear_label_list(Label_List* label_list);
void print_imemory_file(int* memory, FILE* fp);
void print_dmemory_file(int* memory, FILE* fp);

#endif