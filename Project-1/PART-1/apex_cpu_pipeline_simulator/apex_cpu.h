/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
#define BTB_SIZE 4
#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;
typedef struct BTB_Entry{
    int address;
    int calculated_address;
    int taken;
    int valid;
    int outcome_bits;
    int resolved;
} BTB_Entry;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int imm;
    int rs1_value;
    int rs2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int aux_buffer;
    int jump_buffer;
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                        /* Current program counter */
    int clock;                     /* Clock cycles elapsed */
    int insn_completed;            /* Instructions retired */
    int regs[REG_FILE_SIZE];       /* Integer register file */
    int code_memory_size;          /* Number of instruction in the input file */
    APEX_Instruction *code_memory; /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;               /* Wait for user input after every cycle */
    int zero_flag;                 /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int n_flag;
    int p_flag;
    int fetch_from_next_cycle;
    int register_waiting_flag[REG_FILE_SIZE];
    int maxCycles;
    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage memory;
    CPU_Stage writeback;
    int BTB_head;
    BTB_Entry BTB[BTB_SIZE];
} APEX_CPU;



APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
void flushAndFetchNext(APEX_CPU* cpu);
void branch_BNZ_BP(APEX_CPU* cpu);
void branch_BZ_BNP(APEX_CPU* cpu);
int searchBTB(APEX_CPU* cpu, int instruction_address);
void addToBTB(APEX_CPU * cpu, int instruction_address, int calculated_address);
void initBTB(APEX_CPU * cpu);
int increment(int bits);
int decrement(int bits);
#endif
