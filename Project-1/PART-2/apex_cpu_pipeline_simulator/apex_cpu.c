/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apex_cpu.h"
#include "apex_macros.h"

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {
            printf("%s,R%d,R%d,R%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->rs2);
            break;
        }

        case OPCODE_MOVC:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rd, stage->imm);
            break;
        }
        case OPCODE_ADDL:
        case OPCODE_LOAD:
        case OPCODE_SUBL:
        case OPCODE_LOADP:
        case OPCODE_JALR:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
                   stage->imm);
            break;
        }

        case OPCODE_STORE:
        case OPCODE_STOREP:
        {
            printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
                   stage->imm);
            break;
        }
        case OPCODE_BP:
        case OPCODE_BNP:
        case OPCODE_BN:
        case OPCODE_BNN:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            printf("%s,#%d ", stage->opcode_str, stage->imm);
            break;
        }

        case OPCODE_HALT:
        case OPCODE_NOP:
        {
            printf("%s", stage->opcode_str);
            break;
        }
                
        case OPCODE_CML:
        case OPCODE_JUMP:
        {
            printf("%s,R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
            break;            
        }
        case OPCODE_CMP:
        {
            printf("%s,R%d,R%d ", stage->opcode_str, stage->rs1, stage->rs2);
            break;
        }
    }
}

static void 
print_flags(const APEX_CPU *cpu)
{
    printf("--------\n%s\n--------\n", "FLAGS");
    printf("Zero Flag : %d;  Positive Flag : %d; Negative Flag: %d;", cpu->zero_flag, cpu->p_flag, cpu->n_flag);
    printf("\n\n");
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;

    if (cpu->fetch.has_insn)
    {
        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }

        /* Store current PC in fetch latch */
        cpu->fetch.pc = cpu->pc;

        /* Index into code memory using this pc and copy all instruction fields
         * into fetch latch  */
        current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
        strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
        cpu->fetch.opcode = current_ins->opcode;
        cpu->fetch.rd = current_ins->rd;
        cpu->fetch.rs1 = current_ins->rs1;
        cpu->fetch.rs2 = current_ins->rs2;
        cpu->fetch.imm = current_ins->imm;
        
        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        cpu->decode = cpu->fetch;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Fetch", &cpu->fetch);
        }

        /* Stop fetching new instructions if HALT is fetched */
        if (cpu->fetch.opcode == OPCODE_HALT)
        {
            cpu->fetch.has_insn = FALSE;
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */

static int forwardRs1(APEX_CPU * cpu){
    if( cpu->decode.rs1==cpu->executeStageBufferRegister ){
        cpu->decode.rs1_value=cpu->executeStageBuggerRegisterValue;
    }
    else if( cpu->decode.rs1==cpu->memStageBufferRegister ){
        cpu->decode.rs1_value=cpu->memStageBuggerRegisterValue;
    }
    else if(cpu->register_waiting_flag[cpu->decode.rs1]){

        cpu->fetch_from_next_cycle=TRUE;
        return 1;
    }
    else{
        cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
        
    }

    return 0;

}

static int forwardRs2(APEX_CPU * cpu){
    if( cpu->decode.rs2==cpu->executeStageBufferRegister ){
        cpu->decode.rs2_value=cpu->executeStageBuggerRegisterValue;
    }
    else if( cpu->decode.rs2==cpu->memStageBufferRegister ){
        cpu->decode.rs2_value=cpu->memStageBuggerRegisterValue;
    }
    else if(cpu->register_waiting_flag[cpu->decode.rs2]){

        cpu->fetch_from_next_cycle=TRUE;
        return 1;
    }
    else{
        cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
        
    }

    return 0;
}

// static int forwardRd2(APEX_CPU * cpu){
//     if( cpu->decode.rs2==cpu->executeStageBufferRegister ){
//         cpu->decode.rs2_value=cpu->executeStageBuggerRegisterValue;
//     }
//     else if( cpu->decode.rs2==cpu->memStageBufferRegister ){
//         cpu->decode.rs2_value=cpu->memStageBuggerRegisterValue;
//     }
//     else if(cpu->register_waiting_flag[cpu->decode.rs2]){

//         cpu->fetch_from_next_cycle=TRUE;
//         return 1;
//     }
//     else{
//         cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
//         return 0;
//     }
// }

static void
APEX_decode(APEX_CPU *cpu)
{
    int stall=0;
    if (cpu->decode.has_insn)
    {
        /* Read operands from register file based on the instruction type */
        switch (cpu->decode.opcode)
        {
            case OPCODE_SUB:
            case OPCODE_ADD:
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            case OPCODE_DIV:
            case OPCODE_MUL:
            {
                // if(cpu->register_waiting_flag[cpu->decode.rs1]==1 || cpu->register_waiting_flag[cpu->decode.rs2]==1 || cpu->register_waiting_flag[cpu->decode.rd]==1){
                //     cpu->fetch_from_next_cycle=TRUE;
                //     stall= 1;
                //     break;
                // }
                // cpu->register_waiting_flag[cpu->decode.rd]=1;
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                // break;

                stall = forwardRs1(cpu);
                if(stall==1){
                    break;
                }

                if(! stall ){
                    stall=forwardRs2(cpu);
                }

                if(stall==1){
                    break;
                }

                if(!stall){
                    if ( (cpu->decode.rs2 == cpu->decode.rd) || (cpu->decode.rs1 == cpu->decode.rd) )
                    {
                        stall = FALSE;
                    }
                    else if(cpu->register_waiting_flag[cpu->decode.rd])
                    {
                        stall = 1;
                        cpu->fetch_from_next_cycle = TRUE;
                        break;
                    }
                    else{
                        cpu->register_waiting_flag[cpu->decode.rd]=1;
                    }
                }

                break;

            }
            case OPCODE_STORE:
            {
                stall=forwardRs1(cpu);

                if(stall==1){
                    break;
                }

                if(stall==0){
                    stall=forwardRs2(cpu);
                }

                break;
            }
            case OPCODE_STOREP:
            {
                // if(cpu->register_waiting_flag[cpu->decode.rs1]==1 || cpu->register_waiting_flag[cpu->decode.rs2]==1){
                //     cpu->fetch_from_next_cycle=TRUE;
                //     stall= 1;
                //     break;
                // }
                // // cpu->register_waiting_flag[cpu->decode.rd]=1;
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];

                stall=forwardRs1(cpu);

                if(stall==1){
                    break;
                }
                if(stall==0){
                    stall=forwardRs2(cpu);

                    if(stall==1){
                        break;
                    }

                     cpu->register_waiting_flag[cpu->decode.rs2] = 1;

                }
                    
                break;
            }

            case OPCODE_SUBL:
            case OPCODE_ADDL:
            {
                // if(cpu->register_waiting_flag[cpu->decode.rs1]==1 || cpu->register_waiting_flag[cpu->decode.rd]==1){
                //     cpu->fetch_from_next_cycle=TRUE;
                //     stall= 1;
                //     break;
                // }

                // cpu->register_waiting_flag[cpu->decode.rd]=1;
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                
                // break;

                stall=forwardRs1(cpu);

                if(stall==1){
                    break;
                }

                if (cpu->decode.rs1 == cpu->decode.rd)
                {
                    stall=0;
                }
                else if (cpu->register_waiting_flag[cpu->decode.rd])
                {
                    stall=1;
                    cpu->fetch_from_next_cycle = TRUE;
                    
                    break;
                }
                else
                {
                    cpu->register_waiting_flag[cpu->decode.rd] =  1;
                }

                break;
            }

            case OPCODE_LOAD:
            {
                if (cpu->decode.rs1 == cpu->decode.rd)
                {
                    stall=0;
                }
                else if (cpu->register_waiting_flag[cpu->decode.rd]==1)
                {
                    stall= 1;
                    cpu->fetch_from_next_cycle = TRUE;
                    break;
                }
                else
                {
                    cpu->register_waiting_flag[cpu->decode.rd] =  1;
                }

                if(stall==0){
                    stall=forwardRs1(cpu);

                    
                }
                break;                

            }
            case OPCODE_LOADP:
            {
                stall=forwardRs1(cpu);
                if(stall==1){
                    break;
                }

                if(stall==0){
                if (cpu->register_waiting_flag[cpu->decode.rd]==1)
                    {
                        stall = 1;
                        cpu->fetch_from_next_cycle = TRUE;
                        break;
                    }
                    else
                    {
                        cpu->register_waiting_flag[cpu->decode.rd] = 1;
                    }

                    cpu->register_waiting_flag[cpu->decode.rs1]=1;
                }
                break;
            }
            

            case OPCODE_MOVC:
            {
                /* MOVC doesn't have register operands */
                if( cpu->register_waiting_flag[cpu->decode.rd]==1){
                    cpu->fetch_from_next_cycle=TRUE;
                    stall= 1;
                    break;
                }
                cpu->register_waiting_flag[cpu->decode.rd]=1;
                break;
            }
            case OPCODE_CMP:
            {
                // if (cpu->register_waiting_flag[cpu->decode.rs1] == 1 || cpu->register_waiting_flag[cpu->decode.rs2]== 1)
                // {
                    
                //     cpu->fetch_from_next_cycle = TRUE;
                //     stall=1;
                //     break;
                // }
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                // break;

                stall=forwardRs1(cpu);

                if(stall==1){
                    break;
                }

                if(stall==0){
                    stall=forwardRs2(cpu);
                }
                // stall=forward();

                break;


                
            }

            case OPCODE_CML:
            {
                // if(cpu->register_waiting_flag[cpu->decode.rs1] == 1)
                // {
                //     stall=1;
                //     cpu->fetch_from_next_cycle = TRUE;
                //     break;
                // }
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // break;

                stall = forwardRs1(cpu);

                break;
            }
            case OPCODE_JALR:
            {
                // if (cpu->register_waiting_flag[cpu->decode.rs1] == 1)
                // {
                //     stall=1;
                //     cpu->fetch_from_next_cycle = TRUE;
                //     break;
                // }
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // cpu->register_waiting_flag[cpu->decode.rd] = 1;
                // break;

                if(cpu->decode.rs1==cpu->decode.rd){
                    stall=0;
                }
                else if (cpu->register_waiting_flag[cpu->decode.rd]){
                    stall=1;
                    cpu->fetch_from_next_cycle=TRUE;
                    break;
                }
                else{
                    cpu->register_waiting_flag[cpu->decode.rd]=1;
                }

                if(stall ==0){
                    stall = forwardRs1(cpu);
                }

                break;
            }
            case OPCODE_JUMP:
            {
                // if (cpu->register_waiting_flag[cpu->decode.rs1] == 1)
                // {
                    
                //     cpu->fetch_from_next_cycle = TRUE;
                //     stall=1;
                //     break;
                // }
                // cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                // break;

                stall=forwardRs1(cpu);

                // if(stall==1){
                //     break;
                // }

                break;
            }
        }

        /* Copy data from decode latch to execute latch*/
        if(stall==0){
            cpu->execute = cpu->decode;
            cpu->decode.has_insn = FALSE;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Decode/RF", &cpu->decode);
        }
    }
}

/*
 * Execute Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_execute(APEX_CPU *cpu)
{
    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {   
            case OPCODE_SUB:
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_AND:
            case OPCODE_XOR:
            case OPCODE_OR:
            case OPCODE_DIV:
            case OPCODE_MUL:
            {
                if(cpu-> execute.opcode==OPCODE_ADD){
                    cpu->execute.result_buffer
                        = cpu->execute.rs1_value + cpu->execute.rs2_value;
                }
                else if(cpu-> execute.opcode==OPCODE_SUB){
                    cpu->execute.result_buffer
                        = cpu->execute.rs1_value - cpu->execute.rs2_value;
                }
                else if (cpu->execute.opcode==OPCODE_ADDL){
                    cpu->execute.result_buffer
                        = cpu->execute.rs1_value + cpu->execute.imm;                    
                }
                else if (cpu->execute.opcode==OPCODE_SUBL){
                    cpu->execute.result_buffer
                        = cpu->execute.rs1_value - cpu->execute.imm;                    
                }
                else if (cpu->execute.opcode==OPCODE_AND){
                    cpu->execute.result_buffer
                        = cpu->execute.rs1_value && cpu->execute.imm; 
                }
                else if (cpu->execute.opcode == OPCODE_XOR){
                    cpu->execute.result_buffer = cpu->execute.rs1_value ^ cpu->execute.rs2_value;
                }
                else if (cpu->execute.opcode == OPCODE_OR){
                    cpu->execute.result_buffer = cpu->execute.rs1_value | cpu->execute.rs2_value;
                }
                else if (cpu->execute.opcode == OPCODE_DIV){
                    cpu->execute.result_buffer = cpu->execute.rs1_value / cpu->execute.rs2_value;
                }
                else if (cpu->execute.opcode == OPCODE_MUL){
                    cpu->execute.result_buffer = cpu->execute.rs1_value * cpu->execute.rs2_value;
                }

                cpu->executeStageBufferRegister = cpu->execute.rd;
                cpu->executeStageBuggerRegisterValue = cpu->execute.result_buffer;         

                /* Set the zero flag based on the result buffer */
                // if (cpu->execute.result_buffer == 0)
                // {
                //     cpu->zero_flag = TRUE;
                // } 
                // else 
                // {
                //     cpu->zero_flag = FALSE;
                // }
                // break;
            }

            case OPCODE_LOAD:
            {
                cpu->execute.memory_address
                    = cpu->execute.rs1_value + cpu->execute.imm;
                break;
            }
            case OPCODE_LOADP:
            {
                cpu->execute.memory_address = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.aux_buffer = cpu->execute.rs1_value + 4;
                cpu->executeStageBufferRegister = cpu->execute.rs1;
                cpu->executeStageBuggerRegisterValue = cpu->execute.aux_buffer;
                break;
            }

            case OPCODE_BZ:
            {
                if (cpu->zero_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            
            case OPCODE_BNZ:
            {
                if (cpu->zero_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BP:
            {
                if (cpu->p_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNP:
            {
                if (cpu->p_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }
            case OPCODE_BN:
            {
                if (cpu->n_flag == TRUE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_BNN:
            {
                if (cpu->n_flag == FALSE)
                {
                    /* Calculate new PC, and send it to fetch unit */
                    cpu->pc = cpu->execute.pc + cpu->execute.imm;
                    
                    /* Since we are using reverse callbacks for pipeline stages, 
                     * this will prevent the new instruction from being fetched in the current cycle*/
                    cpu->fetch_from_next_cycle = TRUE;

                    /* Flush previous stages */
                    cpu->decode.has_insn = FALSE;

                    /* Make sure fetch stage is enabled to start fetching from new PC */
                    cpu->fetch.has_insn = TRUE;
                }
                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->execute.result_buffer = cpu->execute.imm;
                cpu->executeStageBufferRegister=cpu->execute.rd;
                cpu->executeStageBuggerRegisterValue=cpu->execute.result_buffer;

            }
            case OPCODE_STORE:
            {
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                break;
            }
            case OPCODE_STOREP:
            {
                cpu->execute.memory_address = cpu->execute.rs2_value + cpu->execute.imm;
                cpu->execute.aux_buffer = cpu->execute.rs2_value + 4;
                cpu->executeStageBufferRegister = cpu->execute.rs2;
                cpu->executeStageBuggerRegisterValue = cpu->execute.aux_buffer;
                break;
            }
            case OPCODE_JALR:
            {
                int program_counter = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->execute.jump_buffer = cpu->execute.pc + 4;

                cpu->pc = program_counter;
                cpu->executeStageBuggerRegisterValue=cpu->execute.jump_buffer;
                cpu->executeStageBufferRegister=cpu->execute.rd;
                cpu->fetch_from_next_cycle = TRUE;
                cpu->decode.has_insn = FALSE;
                cpu->fetch.has_insn = TRUE;
                break;
            }
            
            case OPCODE_JUMP:
            {
                cpu->pc = cpu->execute.rs1_value + cpu->execute.imm;
                cpu->fetch_from_next_cycle = TRUE;
                cpu->fetch.has_insn = TRUE;
                cpu->decode.has_insn = FALSE;
                
                break;
            }
            
        }


    if (cpu->execute.has_insn)
    {
        /* Execute logic based on instruction type */
        switch (cpu->execute.opcode)
        {   
                case OPCODE_ADD: //
                case OPCODE_SUB: //
                case OPCODE_MUL: //
                case OPCODE_ADDL: //
                case OPCODE_SUBL: //
                case OPCODE_AND: // 
                case OPCODE_OR: //
                case OPCODE_XOR: // 
                case OPCODE_DIV:
                {
                    cpu->zero_flag = cpu->execute.result_buffer == 0;
                    cpu->p_flag = cpu->execute.result_buffer > 0;
                    cpu->n_flag = cpu->execute.result_buffer < 0;
                    break;
                }
                case OPCODE_CMP:
                {
                    cpu->zero_flag = cpu->execute.rs1_value == cpu->execute.rs2_value;
                    cpu->p_flag = cpu->execute.rs1_value > cpu->execute.rs2_value;
                    cpu->n_flag = cpu->execute.rs1_value < cpu->execute.rs2_value;
                    break;
                }
                case OPCODE_CML:
                {
                    cpu->zero_flag = cpu->execute.rs1_value == cpu->execute.imm;
                    cpu->p_flag = cpu->execute.rs1_value > cpu->execute.imm;
                    cpu->n_flag = cpu->execute.rs1_value < cpu->execute.imm;
                    break;
                }
        }
    }  

        /* Copy data from execute latch to memory latch*/
        cpu->memory = cpu->execute;
        cpu->execute.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Execute", &cpu->execute);
        }
    }
}

/*
 * Memory Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_memory(APEX_CPU *cpu)
{
    if (cpu->memory.has_insn)
    {
        switch (cpu->memory.opcode)
        {
            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_DIV:
            case OPCODE_MOVC:
            case OPCODE_SUBL:
            case OPCODE_MUL:
            case OPCODE_AND:
            case OPCODE_ADDL:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                /* No work for ADD */
                cpu->memStageBuggerRegisterValue = cpu->memory.result_buffer;
                cpu->memStageBufferRegister = cpu->memory.rd;
                
                break;
            }

            case OPCODE_LOAD:
            case OPCODE_LOADP:
            {
                /* Read from data memory */

                cpu->memory.result_buffer
                    = cpu->data_memory[cpu->memory.memory_address];
                cpu->memStageBufferRegister = cpu->memory.rd;
                cpu->memStageBuggerRegisterValue = cpu->memory.result_buffer;
                break;
            }
            case OPCODE_STORE:
            case OPCODE_STOREP:
            {
                cpu->data_memory[cpu->memory.memory_address] = cpu->memory.rs1_value;
                cpu->memStageBufferRegister = cpu->memory.rd;
                cpu->memStageBuggerRegisterValue = cpu->memory.result_buffer;
                break;
            }

        }

        /* Copy data from memory latch to writeback latch*/
        cpu->writeback = cpu->memory;
        cpu->memory.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Memory", &cpu->memory);
        }
    }
}

/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {   
            case OPCODE_SUB:
            case OPCODE_ADD:
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            case OPCODE_AND:
            case OPCODE_MUL:
            case OPCODE_DIV:
            case OPCODE_XOR:
            case OPCODE_OR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                cpu->register_waiting_flag[cpu->writeback.rd]=0;
                break;
            }

            case OPCODE_LOAD:
            {
                cpu->register_waiting_flag[cpu->writeback.rd]=0;
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_LOADP:
            {             
                cpu->register_waiting_flag[cpu->writeback.rd] = 0;
                cpu->register_waiting_flag[cpu->writeback.rs1] = 0;
                cpu->regs[cpu->writeback.rs1] = cpu->writeback.aux_buffer;
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;

                break;
            }

            case OPCODE_MOVC: 
            {
                cpu->register_waiting_flag[cpu->writeback.rd]=0;
                cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
                break;
            }
            case OPCODE_JALR:
            {
                cpu->regs[cpu->writeback.rd] = cpu->writeback.jump_buffer;
                cpu->register_waiting_flag[cpu->writeback.rd] = 0;
                break;
            }
            case OPCODE_STOREP:
            {             
                cpu->regs[cpu->writeback.rs2] = cpu->writeback.aux_buffer;
                cpu->register_waiting_flag[cpu->writeback.rs2] = 0;
                break;
            }
            case OPCODE_NOP:
            case OPCODE_HALT:
            case OPCODE_BNN:
            case OPCODE_BNP:
            case OPCODE_BN:
            case OPCODE_BP:
            case OPCODE_BZ:
            case OPCODE_BNZ:
            case OPCODE_JUMP:
            {
                break;
            }
        }

        cpu->insn_completed++;
        cpu->writeback.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES)
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}
static void print_data_memory(const APEX_CPU *cpu)
{
    printf("----------\n%s\n----------\n", "NON-ZERO MEMORY VALUES");
    for (int i = 0; i < DATA_MEMORY_SIZE; i++)
    {
        if (cpu->data_memory[i] != 0)
        {
            printf("MEM[%d] = %d\n", i, cpu->data_memory[i]);
        }
    }
    printf("\n");
}
/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if(cpu->maxCycles!=0 && cpu->maxCycles<cpu->clock+1){
            break;
        }
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock+1);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock+1, cpu->insn_completed);
            break;
        }

        APEX_memory(cpu);
        APEX_execute(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);
        print_data_memory(cpu);
        print_flags(cpu);
        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void
APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}