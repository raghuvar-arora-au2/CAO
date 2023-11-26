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
    int iq_idx;
    int has_insn;
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
    int fetch_from_next_cycle;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage execute;
    CPU_Stage execute_int_fu;
    CPU_Stage memory;
    CPU_Stage writeback;
} APEX_CPU;

typedef struct IQ_ENTRY
{
    /* data */
    int valid;
    int op_code;
    int literal;
    int src1_ready;
    int src1_tag;
    int src1_value;
    int src2_ready;
    int src2_tag;
    int src2_value;
    int dest;
    int cycle;
    int instruction;
    int lsqIdx;
} IQ_ENTRY;

typedef struct IQ{
    IQ_ENTRY iq[REG_FILE_SIZE];
} IQ;


typedef struct physical_register
{
    int arc_reg;
    int allocation;
    int status;
    int value;
    int renamed;
    int z;
    int p;

} physical_register;

typedef struct physical_register_file
{
    /* data */
    physical_register registers[REG_FILE_SIZE];

} physical_register_file;



APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
static void int_fu();
#endif


