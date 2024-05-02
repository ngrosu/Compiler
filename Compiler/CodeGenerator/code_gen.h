//
// Created by Nimrod on 02/05/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../definitions.h"
#include "../Shared/error.h"

#ifndef COMPILER_CODE_GEN_H
#define COMPILER_CODE_GEN_H

typedef struct Register
{
    char inuse;
    char name[REGISTER_NAME_MAX_SIZE];
} Register;

typedef struct CodeGen
{
    Register registers[NUM_OF_REGISTERS];
    FILE* output;
}CodeGen;

void init_registers(CodeGen* code_gen);

CodeGen* init_code_gen();

int allocate_register(CodeGen* code_gen);

const char* get_register_name(CodeGen* code_gen, int i);

void free_register(CodeGen* code_gen, int i);



#endif //COMPILER_CODE_GEN_H
