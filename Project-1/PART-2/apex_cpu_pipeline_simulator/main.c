/*
 * main.c
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "apex_cpu.h"
#include <string.h>
int
main(int argc, char const *argv[])
{
    APEX_CPU *cpu;

    fprintf(stderr, "APEX CPU Pipeline Simulator v%0.1lf\n", VERSION);

    if (argc < 2)
    {
        fprintf(stderr, "APEX_Help: Usage %s <input_file>\n", argv[0]);
        exit(1);
    }

    cpu = APEX_cpu_init(argv[1]);
    if (!cpu)
    {
        fprintf(stderr, "APEX_Error: Unable to initialize CPU\n");
        exit(1);
    }
    if(argc==2){
        APEX_cpu_run(cpu);
    }

    else if(argc>2){
        if( strcmp(argv[2], "simulate") == 0 && argc == 4){
            int numCycles=atoi(argv[3]);
            cpu->maxCycles=numCycles;
            cpu->single_step=0;
            APEX_cpu_run(cpu);

        }
        else if (strcmp(argv[2], "single_step") == 0)
        {
            cpu->single_step = 1;
            APEX_cpu_run(cpu);
        }
        else{
            fprintf(stderr, "APEX_Error: Invalid args\n");
            exit(1);
        }
    }

    
    APEX_cpu_stop(cpu);
    return 0;
}