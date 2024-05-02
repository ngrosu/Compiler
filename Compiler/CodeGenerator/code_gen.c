//
// Created by Nimrod on 02/05/2024.
//

#include "code_gen.h"



CodeGen* init_code_gen()
{
    CodeGen* result = calloc(sizeof(CodeGen), 1);
    result->output = fopen("out.asm", "w");
    init_registers(result);
    return result;
}

void init_registers(CodeGen* code_gen)
{
    int ri = 0;
    strncpy(code_gen->registers[ri++].name, R8, REGISTER_NAME_MAX_SIZE);
    strncpy(code_gen->registers[ri++].name, R9, REGISTER_NAME_MAX_SIZE);
    strncpy(code_gen->registers[ri++].name, R10, REGISTER_NAME_MAX_SIZE);
    strncpy(code_gen->registers[ri++].name, R11, REGISTER_NAME_MAX_SIZE);
}

int allocate_register(CodeGen* code_gen)
{
    for (int r = 0; r < NUM_OF_REGISTERS; r++)
    { // look for an unused register
        if (!code_gen->registers[r].inuse)
        {
            code_gen->registers[r].inuse = 1;
            return r;
        }
    }

    // if no register is free
    report_error(ERR_INTERNAL, -1, "Ran out of available registers", NULL);
    return -1;
}
const char* get_register_name(CodeGen* code_gen, int i)
{
    return code_gen->registers[i].name;
}
void free_register(CodeGen* code_gen, int i)
{
    code_gen->registers[i].inuse = 0;
}

