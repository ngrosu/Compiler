//
// Created by Nimrod on 02/05/2024.
//



#ifndef COMPILER_CODE_GEN_H
#define COMPILER_CODE_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../definitions.h"
#include "../Shared/error.h"
#include "../Shared/SymbolTableManager.h"
#include <stdarg.h>

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

int create_label(int i);

const char* get_label_name(int i);

const char *get_symbol_code(symbol_item *item, int index);

void generator_output(CodeGen* code_gen, char* format, ...);

void load_token_to_register(ScopeNode* scope, CodeGen* code_gen, int register_num, Token token);

void generate_statement(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast);

void generate_code(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast);

int convert_type_to_bytes(int num); // get a type token, and return the amount of bytes

char convert_type_to_size(int num); // get a type token, and return the size character (d,w,b)

void generate_assignment(CodeGen *code_gen, ScopeNode *scope, symbol_item *item, ASTNode *ast, int index);

void generate_var_dec_ass(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast);

void generate_declaration(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast);

void generate_code_block(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast);

#endif //COMPILER_CODE_GEN_H
