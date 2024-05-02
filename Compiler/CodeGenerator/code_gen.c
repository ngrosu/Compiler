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
            printf("allocated %s\n", get_register_name(code_gen, r));
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
    printf("freed %s\n", get_register_name(code_gen, i));
}

int create_label(int i)
{
    static int label_count = 0;
    label_count++;
    return label_count;
}

const char* get_label_name(int i) // value will be changed after each call, so must call every time
{
    static char label_name[LABEL_NAME_MAX_SIZE];
    if(snprintf(label_name, LABEL_NAME_MAX_SIZE, LABEL_FORMAT, i) >= LABEL_NAME_MAX_SIZE)
    {
        report_error(ERR_INTERNAL, -1, "Label name became too large", NULL);
        return NULL;
    }
    return label_name;
}

const char *get_symbol_code(symbol_item *item, int index)
{
    // static to avoid having to malloc every time
    static char result[TOKEN_MAXSIZE];
    if(item->scope->scope == SCOPE_GLOBAL)
    {
        snprintf(result, TOKEN_MAXSIZE, GLOBAL_ADDRESS_FORMAT, item->name);
        return result;
    }

    int offset = item->scope->parent->bytes_since_last_frame + item->offset
            - convert_type_to_bytes(item->type[0])*index; // subtract the index to access specific indices
    snprintf(result, TOKEN_MAXSIZE, STACK_OFFSET_FORMAT, offset);
    return result;
}

void generator_output(CodeGen* code_gen, char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(code_gen->output, format, args);
    va_end(args);
}

void load_token_to_register(ScopeNode* scope, CodeGen* code_gen, int register_num, Token token)
{
    if (token->type == TOKEN_IDENTIFIER)
    {
        symbol_item* item = find_var(scope, token->lexeme);
        generator_output(code_gen, mov, get_register_name(code_gen, register_num), get_symbol_code(item, 0));
    }
    else
    {
        generator_output(code_gen, mov, get_register_name(code_gen, register_num), token->lexeme);
    }
}

int convert_type_to_bytes(int num)
{
    int res = 1;
    switch(num)
    {
        case TOKEN_INT:
            res= 4;
            break;
        case TOKEN_SHORT:
            res= 2;
            break;
        case TOKEN_CHAR:
            res= 1;
            break;
    }
    return res;
}
char convert_type_to_size(int num)
{
    char res;
    switch(num)
    {
        case TOKEN_INT:
            res= 'd';
            break;
        case TOKEN_SHORT:
            res= 'w';
            break;
        case TOKEN_CHAR:
            res= 'b';
            break;
    }
    return res;
}

int generate_binary_expressions(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast, int r1, int r2)
{
    switch(ast->type)
    {
        case TOKEN_PLUS_OP:
            generator_output(code_gen, add, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
        case TOKEN_MINUS_OP:
            generator_output(code_gen, sub, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
    }
    return r1;
}

int generate_expression(CodeGen *code_gen, ScopeNode* scope, ASTNode* ast)
{
    int r;
    int r_temp;
    if(ast->num_of_children==0)
    {
        r = allocate_register(code_gen);
        load_token_to_register(scope, code_gen, r, ast->token);
        return r;
    }
    else if(ast->num_of_children==2)
    {
        r = generate_expression(code_gen, scope, ast->children[0]);
        r_temp = generate_expression(code_gen, scope, ast->children[1]);
        return generate_binary_expressions(code_gen, scope, ast, r, r_temp);
    }
}

void generate_assignment(CodeGen *code_gen, ScopeNode* scope, symbol_item *item, ASTNode *ast, int index)
{
    //symbol_item* item = find_var(scope, ast->children[0]->token->lexeme);
    if(ast->type == TOKEN_INT_LITERAL)
    {
        generator_output(code_gen, movx,
                         convert_type_to_size(item->type[0])
                         , get_symbol_code(item, 0), ast->token->lexeme);
    }
    else
    {
        int r = generate_expression(code_gen, scope, ast);
        generator_output(code_gen, movx,
                         convert_type_to_size(item->type[0])
                , get_symbol_code(item, 0), get_register_name(code_gen, r));
        free_register(code_gen, r);
    }
}

void generate_arr_dec_ass(CodeGen* code_gen, symbol_item* item, const char* s)
{
    char size = convert_type_to_size(item->type[0]);
    char buffer[4] = "\"0\"";
    int index = 0;
    while(*s)
    {
        buffer[1] = *s;
        generator_output(code_gen, movx, size, get_symbol_code(item, index), buffer);
        index++;
        s++;
    }
}

void generate_declaration(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    if (scope->scope != SCOPE_GLOBAL)
    {
        char buffer[100];

        symbol_item* item = find_var(scope, ast->children[1]->token->lexeme);
        // update the offset of the variable

        int size = convert_type_to_bytes(ast->children[0]->type);
        if (ast->type == TOKEN_COUNT + SYMBOL_VAR_DEC)
        {
            scope->bytes_since_last_frame += size;
            item->offset = scope->bytes_since_last_frame;
            sprintf(buffer, "%d", size);
            generator_output(code_gen, sub, sp, buffer);
            if (ast->num_of_children == 3)
            {
                generate_assignment(code_gen, scope, item, ast->children[2], 0);
            }
        } else if (ast->type == TOKEN_COUNT + SYMBOL_ARR_DEC)
        {
            size = size * (int) strtol(ast->children[2]->token->lexeme, NULL, 10);
            scope->bytes_since_last_frame += size;
            item->offset = scope->bytes_since_last_frame;
            sprintf(buffer, "%d", size);
            generator_output(code_gen, sub, sp, buffer);
            if (ast->num_of_children == 4)
            {
                generate_arr_dec_ass(code_gen, item, ast->children[3]->token->lexeme);
            }
        }
    }
}

void generate_code_block(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    for(int i = 0; i < ast->num_of_children; i++)
    {
        generate_statement(code_gen, scope, ast->children[i]);
    }
}

void update_params(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    int size;
    symbol_item* item;

    if (ast->num_of_children == 4)
    {
        ast = ast->children[2];
        for (int i = 0; i < ast->num_of_children; i++)
        {
            size = convert_type_to_bytes(ast->children[i]->type);
            item = find_var(scope, ast->children[i]->children[0]->token->lexeme);
            item->scope->bytes_since_last_frame += size;
            item->offset = item->scope->bytes_since_last_frame;
        }

    }
}

void generate_statement(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    switch (ast->type)
    {
        case TOKEN_COUNT + SYMBOL_FUNC_DEC:
            update_params(code_gen, scope->children[scope->curr_index], ast);
            generate_code_block(code_gen, scope->children[scope->curr_index], ast->children[ast->num_of_children-1]);
            scope->curr_index++;
            break;
        case TOKEN_COUNT + SYMBOL_ASSIGNMENT:
            generate_assignment(code_gen, scope, find_var(scope, ast->children[0]->token->lexeme), ast->children[1], 0);
            break;
        case TOKEN_COUNT + SYMBOL_VAR_DEC:
        case TOKEN_COUNT + SYMBOL_ARR_DEC:
            generate_declaration(code_gen, scope, ast);
            break;
    }
}

void generate_code(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    //-----Creation of data segment-----
    generator_output(code_gen, "SECTION .data\n");
    for(int i = 0; i< ast->num_of_children; i++)
    {
        char* data;
        char size;
        ASTNode* child = ast->children[i];
        size = convert_type_to_size(child->children[0]->type);
        if (child->type == TOKEN_COUNT + SYMBOL_VAR_DEC)
        {
            if (child->num_of_children == 3)
            {
                generator_output(code_gen, "%s:\td%c %s\n", child->children[1]->token->lexeme, size, child->children[2]->token->lexeme);
            }
            else
            {
                generator_output(code_gen, "%s:\tres%c %s\n", child->children[1]->token->lexeme, size, "1");
            }
        }
        if (child->type == TOKEN_COUNT + SYMBOL_ARR_DEC)
        {
            int leftover = strtol(child->children[2]->token->lexeme, NULL, 10);
            if (child->num_of_children == 4)
            {
                generator_output(code_gen, "%s:\td%c \"%s\"\n", child->children[1]->token->lexeme, size, child->children[3]->token->lexeme);
                leftover -= strlen(child->children[3]->token->lexeme);
            }
            if(leftover != 0)
            {
                generator_output(code_gen, "\tres%c %d\n", size, leftover);
            }
        }
    }
    //-----Start of code segment-----
    generator_output(code_gen, "\nSECTION .text\n");

    generate_code_block(code_gen, scope, ast);
    for(int i = 0; i< ast->num_of_children; i++)
    {
        if(ast->children[i]->type == TOKEN_COUNT+SYMBOL_FUNC_DEC)
        {
            generate_code_block(code_gen, scope, ast->children[i]);
        }
    }

}