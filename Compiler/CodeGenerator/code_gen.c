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
            const char* temp = get_register_name(code_gen, r);
            printf("allocated %s\n", temp);
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

int create_label()
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

void load_array_ax(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast, int scratch_reg)
{
    symbol_item* item;
    item = find_var(scope, ast->children[0]->token->lexeme);
    load_token_to_register(scope, code_gen, scratch_reg, ast->children[1]);
    if(convert_type_to_bytes(item->type[0]) != 1)
    {   // multiply the index by the amount of bytes
        generator_output(code_gen, "\tmov %s, %d\n", ax, convert_type_to_bytes(item->type[0]));
        generator_output(code_gen, "\tmul %s\n", get_register_name(code_gen, scratch_reg));
        generator_output(code_gen, mov, get_register_name(code_gen, scratch_reg), ax);
    }
    // load the array start address and subtract the index
    generator_output(code_gen, "\tlea %s, %s ;array start address\n", ax, get_symbol_code(item));
    generator_output(code_gen, item->scope->scope != SCOPE_GLOBAL ?sub : add, // if global array, flip index addition
                     ax, get_register_name(code_gen, scratch_reg));
}

const char *get_symbol_code(symbol_item *item)
{
    // static to avoid having to malloc every time
    static char result[TOKEN_MAXSIZE];
    if(item->scope->scope == SCOPE_GLOBAL)
    {
        snprintf(result, TOKEN_MAXSIZE, GLOBAL_ADDRESS_FORMAT, item->name);
        return result;
    }

    if(item->offset >= 0)
    {
        // if the offset is positive, the argument is under the base pointer
        int offset = item->scope->parent->bytes_used_since_last_frame + item->offset ; // subtract the index to access specific indices
        snprintf(result, TOKEN_MAXSIZE, STACK_OFFSET_FORMAT, '-', offset);
    }
    else
    {  // if the offset is negative, the variable is an argument to a function, and is above the
        //base pointer
        snprintf(result, TOKEN_MAXSIZE, STACK_OFFSET_FORMAT, '+',-item->offset + 8);
    }
    return result;
}

void generator_output(CodeGen* code_gen, char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(code_gen->output, format, args);
    va_end(args);
}

void load_token_to_register(ScopeNode* scope, CodeGen* code_gen, int register_num, ASTNode *ast)
{
    int r;
    symbol_item* item;
    switch(ast->type)
    {
        case TOKEN_COUNT + SYMBOL_ARR_ACC:
            item = find_var(scope, ast->children[0]->token->lexeme);
            load_array_ax(code_gen, scope, ast, register_num);
            generator_output(code_gen, "\tmovsx %s, %s [%s]\n",
                             get_register_name(code_gen, register_num),
                             convert_type_to_size_full(item->type[0]),
                             ax);
//            load_token_to_register(scope, code_gen, register_num, ast->children[1]);
//            if(convert_type_to_bytes(item->type[0]) != 1)
//            {
//                generator_output(code_gen, "\tmov %s, %d\n", ax, convert_type_to_bytes(item->type[0]));
//                generator_output(code_gen, "\tmul %s\n", get_register_name(code_gen, register_num));
//                generator_output(code_gen, mov, get_register_name(code_gen, register_num), ax);
//            }
//            addr_reg = allocate_register(code_gen);
//            generator_output(code_gen, "\tlea %s, %s\n", get_register_name(code_gen, addr_reg), get_symbol_code(item));
//            generator_output(code_gen, sub, get_register_name(code_gen, addr_reg), get_register_name(code_gen, register_num));
//            generator_output(code_gen, "\tmovsx %s, %s [%s]\n", get_register_name(code_gen, register_num),
//                             convert_type_to_size_full(item->type[0]), get_register_name(code_gen, addr_reg));
//            free_register(code_gen, addr_reg);
            break;
        case TOKEN_IDENTIFIER:
            item = find_var(scope, ast->token->lexeme);
            generator_output(code_gen, "\tmovsx %s, %s %s\n", get_register_name(code_gen, register_num),
                             convert_type_to_size_full(item->type[0]), get_symbol_code(item));
            break;
        case TOKEN_PLUS_OP:
        case TOKEN_MINUS_OP:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
        case TOKEN_ASTERISK:
        case TOKEN_F_SLASH:
        case TOKEN_COUNT + SYMBOL_FUNC_CALL:
            r= generate_expression(code_gen, scope, ast);
            generator_output(code_gen, mov, get_register_name(code_gen, register_num), get_register_name(code_gen, r));
            break;
        case TOKEN_INT_LITERAL:
            generator_output(code_gen, mov, get_register_name(code_gen, register_num), ast->token->lexeme);
            break;
        default:
            printf("NON INTEGER OR IDENTIFIER TOKEN PASSED TO LOAD TOKEN TO REGISTER\n");
            printf("-------------------------------|   %s   |-----------------------------\n",
                   ast->token->lexeme);
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

const char* convert_type_to_size_full(int num)
{
    char* res;
    switch(num)
    {
        case TOKEN_INT:
            res= "dword";
            break;
        case TOKEN_SHORT:
            res= "word";
            break;
        case TOKEN_CHAR:
            res= "byte";
            break;
    }
    return res;
}

int generate_comparison_expressions(CodeGen *code_gen, ASTNode *ast, int r1, int r2)
{
    int true_label = create_label();
    int end_label = create_label();
    generator_output(code_gen, "\tcmp %s, %s\n", get_register_name(code_gen, r1), get_register_name(code_gen, r2));
    switch(ast->type)
    {
        case TOKEN_EQUAL_EQUAL_OP:
            generator_output(code_gen, "\tje %s\n", get_label_name(true_label));
            break;
        case TOKEN_NOT_EQUAL_OP:
            generator_output(code_gen, "\tjne %s\n", get_label_name(true_label));
            break;
        case TOKEN_GREATER_THAN_OP:
            generator_output(code_gen, "\tjg %s\n", get_label_name(true_label));
            break;
        case TOKEN_GREATER_THAN_EQUAL_OP:
            generator_output(code_gen, "\tjge %s\n", get_label_name(true_label));
            break;
        case TOKEN_LESS_THAN_OP:
            generator_output(code_gen, "\tjl %s\n", get_label_name(true_label));
            break;
        case TOKEN_LESS_THAN_EQUAL_OP:
            generator_output(code_gen, "\tjle %s\n", get_label_name(true_label));
            break;
    }

    generator_output(code_gen, mov, get_register_name(code_gen, r1), "0");
    generator_output(code_gen, "\tjmp %s\n", get_label_name(end_label));
    generator_output(code_gen, "%s:\n", get_label_name(true_label));
    generator_output(code_gen, mov, get_register_name(code_gen, r1), "1");
    generator_output(code_gen, "%s:\n", get_label_name(end_label));
    free_register(code_gen, r2);
    return r1;
}

int generate_and_or(CodeGen *code_gen, ASTNode *ast, int r1, int r2)
{
    int end_label = create_label();
    int true_label = create_label();
    int false_label = create_label();
    generator_output(code_gen, "\tcmp %s, 0; start and/or\n", get_register_name(code_gen, r1));
    if(ast->type == TOKEN_AND){
        generator_output(code_gen, "\tje %s; first check and\n", get_label_name(false_label));
    }
    else{
        generator_output(code_gen, "\tjne %s; first check or\n", get_label_name(true_label));}

    generator_output(code_gen, "\tcmp %s, 0\n", get_register_name(code_gen, r2));

    generator_output(code_gen, "\tjne %s; second check both\n", get_label_name(true_label));
    generator_output(code_gen, "%s: ;false label\n", get_label_name(false_label));
    generator_output(code_gen, mov, get_register_name(code_gen, r1), "0");
    generator_output(code_gen, "\tjmp %s\n", get_label_name(end_label));
    generator_output(code_gen, "%s:\n", get_label_name(true_label));
    generator_output(code_gen, mov, get_register_name(code_gen, r1), "1");
    generator_output(code_gen, "%s: ; end label and/or\n", get_label_name(end_label));
    free_register(code_gen, r2);
    return r1;
}

void generate_if_else(CodeGen *code_gen, ScopeNode* scope, ASTNode *ast)
{
    int end_label = create_label();
    int if_label = create_label();
    ScopeNode* tempScope;

    int r1 = generate_expression(code_gen, scope, ast->children[0]); // evaluate the condition

    generator_output(code_gen, "\tcmp %s, 0  ;if check\n", get_register_name(code_gen, r1));
    free_register(code_gen, r1);
    generator_output(code_gen, "\tjne %s ;jmp to if\n", get_label_name(if_label)); // if true, go to the if label
    // otherwise continues down to the else section
    if(ast->type == SYMBOL_IF_ELSE+TOKEN_COUNT)
    {
        tempScope = scope->children[scope->curr_index+1]; // get the else scope
        tempScope->bytes_used_since_last_frame = scope->bytes_used_since_last_frame; // transfer variable info
        tempScope->bytes_cleared_since_last_frame = scope->bytes_cleared_since_last_frame;

        generate_code_block(code_gen, tempScope, ast->children[2]); // generate code block

        generator_output(code_gen, "\tadd %s, %d\n", sp, // set the stack pointer back to what it was
                         tempScope->bytes_cleared_since_last_frame - scope->bytes_cleared_since_last_frame);
    }
    // if block
    generator_output(code_gen, "\tjmp %s\n", get_label_name(end_label));
    tempScope = scope->children[scope->curr_index]; // get the if scope
    tempScope->bytes_used_since_last_frame = scope->bytes_used_since_last_frame; // transfer variable info
    tempScope->bytes_cleared_since_last_frame = scope->bytes_cleared_since_last_frame;

    generator_output(code_gen, "%s:\n", get_label_name(if_label));
    generate_code_block(code_gen, tempScope, ast->children[1]); // generate code block
    generator_output(code_gen, "\tadd %s, %d\n", sp, // set the stack pointer back to what it was
                     tempScope->bytes_cleared_since_last_frame - scope->bytes_cleared_since_last_frame);

    generator_output(code_gen, "%s:  ; if end\n", get_label_name(end_label));
    if(ast->type == SYMBOL_IF_ELSE+TOKEN_COUNT)
    {scope->curr_index += 2;}
    else
    {scope->curr_index++;}
}

int generate_binary_expressions(CodeGen *code_gen, ASTNode *ast, int r1, int r2)
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
        case TOKEN_BITWISE_AND:
            generator_output(code_gen, and, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
        case TOKEN_BITWISE_OR:
            generator_output(code_gen, or, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
        case TOKEN_BITWISE_XOR:
            generator_output(code_gen, xor, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
        case TOKEN_ASTERISK:
            generator_output(code_gen, imul, get_register_name(code_gen, r1), get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            break;
        case TOKEN_F_SLASH:
            generator_output(code_gen, mov, ax, get_register_name(code_gen, r1));
            generator_output(code_gen, "\txor rdx, rdx\n");
            generator_output(code_gen, idiv, get_register_name(code_gen, r2));
            free_register(code_gen, r2);
            generator_output(code_gen, mov, get_register_name(code_gen, r1), ax);
            break;
        case TOKEN_EQUAL_EQUAL_OP:
        case TOKEN_NOT_EQUAL_OP:
        case TOKEN_LESS_THAN_EQUAL_OP:
        case TOKEN_LESS_THAN_OP:
        case TOKEN_GREATER_THAN_EQUAL_OP:
        case TOKEN_GREATER_THAN_OP:
            generate_comparison_expressions(code_gen, ast, r1, r2);
            break;
        case TOKEN_OR:
        case TOKEN_AND:
            generate_and_or(code_gen, ast, r1, r2);
            break;

    }
    return r1;
}

int generate_expression(CodeGen *code_gen, ScopeNode* scope, ASTNode* ast)
{
    int r;
    int r_temp;
    if(ast->type == TOKEN_COUNT + SYMBOL_FUNC_CALL)
    {
        return generate_func_call(code_gen, scope, ast);
    }
    else if(ast->type == TOKEN_COUNT + SYMBOL_ARR_ACC)
    {
        r = allocate_register(code_gen);
        load_token_to_register(scope, code_gen, r, ast);
        return r;
    }
    else if(ast->num_of_children==0)
    {
        r = allocate_register(code_gen);
        load_token_to_register(scope, code_gen, r, ast);
        return r;
    }
    else if(ast->num_of_children==2)
    {
        r = generate_expression(code_gen, scope, ast->children[0]);
        r_temp = generate_expression(code_gen, scope, ast->children[1]);
        return generate_binary_expressions(code_gen, ast, r, r_temp);
    }
    else
    {
        return 0; // TODO UNARY EXPRESSION
    }
}

void generate_assignment(CodeGen *code_gen, ScopeNode* scope, ASTNode* ast)
{
    if(ast->children[1]->type == TOKEN_INT_LITERAL)
        load_number_to_symbol(code_gen, scope, ast->children[0], ast->children[1]->token->lexeme);
    else
    {
        int r = generate_expression(code_gen, scope, ast->children[1]);
        load_register_to_symbol(code_gen, scope, ast->children[0], r);
        free_register(code_gen, r);
    }
}

void generate_arr_dec_ass(CodeGen* code_gen, symbol_item* item, const char* s)
{
    const char* size = convert_type_to_size_full(item->type[0]);
    char buffer[4] = "\"0\"";
    int index = 0;
    while(*s)
    {
        buffer[1] = *s;
        generator_output(code_gen, "\tmov %s [%s-%d], %s\n", size, bp,
                         item->scope->parent->bytes_used_since_last_frame + item->offset +
                         index*(convert_type_to_bytes(item->type[0])),
                         buffer);
        index++;
        s++;
    }

}

int calc_stack_size_diff(int bytes_used, int bytes_cleared)
{
    int diff = 0;
    if(bytes_used > bytes_cleared)
    {
        diff = (bytes_used / STACK_ALLIGNMENT_SIZE + 1) *
               STACK_ALLIGNMENT_SIZE - bytes_cleared;
    }
    return diff;
}

void generate_declaration(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    if (scope->scope != SCOPE_GLOBAL)
    {
        char buffer[100];

        symbol_item* item = find_var(scope, ast->children[1]->token->lexeme);
        // update the offset of the variable

        int size_diff; //
        int size = convert_type_to_bytes(ast->children[0]->type);
        // keep stack allignment
        if (ast->type == TOKEN_COUNT + SYMBOL_VAR_DEC)
        {
            scope->bytes_used_since_last_frame += size;
            item->offset = scope->bytes_used_since_last_frame;
            size_diff = calc_stack_size_diff(scope->bytes_used_since_last_frame, scope->bytes_cleared_since_last_frame);
            if(size_diff)
            {
                sprintf(buffer, "%d", size_diff);
                generator_output(code_gen, sub, sp, buffer);
                scope->bytes_cleared_since_last_frame += size_diff;
            }
            if (ast->num_of_children == 3)
            {
                if(ast->children[2]->type == TOKEN_INT_LITERAL)
                    load_number_to_symbol(code_gen, scope, ast->children[1], ast->children[2]->token->lexeme);
                else
                {
                    int r = allocate_register(code_gen);
                    r = generate_expression(code_gen, scope, ast->children[2]);
                    load_register_to_symbol(code_gen, scope, ast->children[1], r);
                    free_register(code_gen, r);
                }
                //generate_assignment(code_gen, scope, item, ast->children[2], 0);
            }
        } else if (ast->type == TOKEN_COUNT + SYMBOL_ARR_DEC)
        {
            item->offset = scope->bytes_used_since_last_frame + size;
            size = size * (int) strtol(ast->children[2]->token->lexeme, NULL, 10);
            size_diff = calc_stack_size_diff(scope->bytes_used_since_last_frame + size, scope->bytes_cleared_since_last_frame);
            if(size_diff)
            {
                sprintf(buffer, "%d", size_diff);
                generator_output(code_gen, sub, sp, buffer);
                scope->bytes_cleared_since_last_frame += size_diff;
            }
            if (ast->num_of_children == 4)
            {
                generate_arr_dec_ass(code_gen, item, ast->children[3]->token->lexeme);
            }
            scope->bytes_used_since_last_frame += size;
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

int generate_func_call(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    // load arguments
    int curr_size = 0;
    int allocated_size = 0;
    symbol_item* func = find_var(scope, ast->children[0]->token->lexeme);
    Param* params = func->parameters;


    generator_output(code_gen, "\tsub %s, %d\n", sp, STACK_ENTRY_SIZE);
    allocated_size+=STACK_ENTRY_SIZE;
    if(ast->num_of_children == 2)
    {
        ASTNode *args = ast->children[1];
        for (int i = 0; i < args->num_of_children; i++)
        {
            curr_size += convert_type_to_bytes(params[i].type);
            if (curr_size > allocated_size)
            {
                generator_output(code_gen, "\tsub %s, %d\n", sp, STACK_ENTRY_SIZE * 2);
                allocated_size += STACK_ENTRY_SIZE * 2;
            }
            int r1 = generate_expression(code_gen, scope, args->children[i]);
            generator_output(code_gen, "\tmov %s [%s-%d], %s%c\n", convert_type_to_size_full(params[i].type),
                             bp, curr_size + scope->bytes_cleared_since_last_frame,
                             get_register_name(code_gen, r1),
                             convert_type_to_size(params[i].type));
            free_register(code_gen, r1);
        }
    }
    // call function
    generator_output(code_gen, "\tcall _%s\n", func->name);

    // reset the stack
    generator_output(code_gen, "\tadd %s, %d\n", sp, allocated_size);

    // return value
    int r = allocate_register(code_gen);
    generator_output(code_gen, "\tmov %s, %s\n", get_register_name(code_gen, r), ax);


    return r;
}

void generate_return(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    int r = generate_expression(code_gen, scope, ast->children[0]);
    scope = is_in_scope(scope, SCOPE_FUNCTION);
    generator_output(code_gen, "\txor rax, rax\n");
    generator_output(code_gen, mov, ax, get_register_name(code_gen, r));
    generator_output(code_gen, "\tjmp _%s_epilogue\n", scope->name);
    free_register(code_gen, r);
}

void generate_function(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    char* func_name = ast->children[1]->token->lexeme;

    generator_output(code_gen, "_%s:\n", func_name);
    update_params(scope, ast); // account for parameters on stack
    // push rbp
    generator_output(code_gen, "\tpush %s\n", bp);
    generator_output(code_gen, "\tmov %s, %s\n", bp, sp);

    generate_code_block(code_gen, scope, ast->children[ast->num_of_children-1]);

    generator_output(code_gen, "_%s_epilogue:\n", func_name);
    generator_output(code_gen, mov, sp, bp);
    generator_output(code_gen, "\tpop %s\n", bp);
    generator_output(code_gen, "\tret\n\n", bp);
}


void update_params(ScopeNode *scope, ASTNode *ast)
{
    symbol_item* item;
    int total_size = 0;

    if (ast->num_of_children == 4)
    {
        ASTNode* params = ast->children[2];
        for (int i = 0; i < params->num_of_children; i++)
        {
            total_size += convert_type_to_bytes(ast->children[i]->type);
        }
        total_size = ((total_size+7)/16+1)*16;
        for (int i = 0; i < params->num_of_children; i++)
        {
            item = find_var(scope, params->children[i]->children[0]->token->lexeme);
            total_size -= convert_type_to_bytes(item->type[0]);
            item->offset = -total_size;
        }
//        for (int i = 0; i < ast->num_of_children; i++)
//        {
//            size = STACK_ENTRY_SIZE; //convert_type_to_bytes(ast->children[i]->type);
//            item = find_var(scope, ast->children[i]->children[0]->token->lexeme);
//            item->scope->bytes_used_since_last_frame += size;
//            item->scope->bytes_cleared_since_last_frame += size;
//            item->offset = item->scope->bytes_used_since_last_frame; // -(i+STACK_FUNC_START_PUSHES)*STACK_ENTRY
//            // (need to check the correctness of this change first)
//
//        }

    }
}

void generate_statement(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    switch (ast->type)
    {
        case TOKEN_COUNT + SYMBOL_FUNC_DEC:
            generate_function(code_gen, scope->children[scope->curr_index], ast);
            scope->curr_index++;
            break;
        case TOKEN_COUNT + SYMBOL_IF:
        case TOKEN_COUNT + SYMBOL_IF_ELSE:
            generate_if_else(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_WHILE:
            generate_while(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_FOR:
            generate_for(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_ASSIGNMENT:
            generate_assignment(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_VAR_DEC:
        case TOKEN_COUNT + SYMBOL_ARR_DEC:
            generate_declaration(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_OUTPUT:
            generate_output_stmt(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_INPUT:
            generate_input_stmt(code_gen, scope, ast);
            break;
        case TOKEN_COUNT + SYMBOL_RETURN:
            generate_return(code_gen, scope, ast);
            break;
        case TOKEN_PLUS_OP:
        case TOKEN_MINUS_OP:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
        case TOKEN_ASTERISK:
        case TOKEN_F_SLASH:
        case TOKEN_COUNT + SYMBOL_FUNC_CALL:
            free_register(code_gen, generate_expression(code_gen, scope, ast));
            break;
        default:
            break;
    }
}

void generate_init_io(CodeGen* code_gen)
{
    generator_output(code_gen, "\tsub %s, 8\n" // align stack
                               "\tsub %s, 32\n"  // clear shadow space
                               "\tmov ECX, %d\n"
                               "\tcall GetStdHandle\n"
                               "\tmov qword [REL _StandardOHandle], %s\n"
                               "\tmov ECX, %d\n"
                               "\tcall GetStdHandle\n"
                               "\tmov qword [REL _StandardIHandle], %s\n"
                               "\tadd %s, 40\n", sp, sp, STD_OUTPUT_HANDLE, ax, STD_INPUT_HANDLE, // revert stack
                               ax, sp);

}
void generate_while(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    int start_label = create_label();
    int end_label = create_label();
    ScopeNode* tempScope;
    tempScope = scope->children[scope->curr_index];
    tempScope->bytes_cleared_since_last_frame = scope->bytes_cleared_since_last_frame;
    tempScope->bytes_used_since_last_frame = scope->bytes_used_since_last_frame;
    scope->curr_index++;
    generator_output(code_gen, "%s:\n", get_label_name(start_label));

    int r1 = generate_expression(code_gen, tempScope, ast->children[0]); // evaluate the condition

    generator_output(code_gen, "\tcmp %s, 0  ;if check\n", get_register_name(code_gen, r1));
    free_register(code_gen, r1);
    generator_output(code_gen, "\tje %s ;jmp to end\n", get_label_name(end_label));
    generate_code_block(code_gen, tempScope, ast->children[1]);
    generator_output(code_gen, "\tjmp %s ;jmp to start\n", get_label_name(start_label));
    generator_output(code_gen, "%s:\n", get_label_name(end_label));
    generator_output(code_gen, "\tadd %s, %d\n", sp, // set the stack pointer back to what it was
                     tempScope->bytes_cleared_since_last_frame - scope->bytes_cleared_since_last_frame);
}
void generate_for(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    int start_label = create_label();
    int end_label = create_label();
    ScopeNode* tempScope;
    tempScope = scope->children[scope->curr_index];
    tempScope->bytes_cleared_since_last_frame = scope->bytes_cleared_since_last_frame;
    tempScope->bytes_used_since_last_frame = scope->bytes_used_since_last_frame;
    scope->curr_index++;

    generate_statement(code_gen, tempScope, ast->children[0]); // do the setup
    generator_output(code_gen, "%s:\n", get_label_name(start_label));

    int r1 = generate_expression(code_gen, tempScope, ast->children[1]); // evaluate the condition

    generator_output(code_gen, "\tcmp %s, 0  ;if check\n", get_register_name(code_gen, r1));
    free_register(code_gen, r1);
    generator_output(code_gen, "\tje %s ;jmp to end\n", get_label_name(end_label));
    generate_code_block(code_gen, tempScope, ast->children[3]);
    generate_statement(code_gen, tempScope, ast->children[2]);
    generator_output(code_gen, "\tjmp %s ;jmp to start\n", get_label_name(start_label));
    generator_output(code_gen, "%s:\n", get_label_name(end_label));
    generator_output(code_gen, "\tadd %s, %d\n", sp, // set the stack pointer back to what it was
                     tempScope->bytes_cleared_since_last_frame - scope->bytes_cleared_since_last_frame);
}

void generate_output_stmt(CodeGen *code_gen, ScopeNode *scope, ASTNode *ast)
{

    int r1 = generate_expression(code_gen, scope, ast->children[0]);
    const char* r1_n = get_register_name(code_gen, r1);
    int r2 = allocate_register(code_gen);
    const char* r2_n = get_register_name(code_gen, r2);
    generator_output(code_gen, mov, ax, r1_n);
    generator_output(code_gen, "\tlea %s, [REL _Message+9]\n", r2_n);

    symbol_item* item=NULL;
    if(ast->children[0]->type == TOKEN_IDENTIFIER || ast->children[0]->type == TOKEN_COUNT + SYMBOL_ARR_ACC)
    {
        if(ast->children[0]->type == TOKEN_IDENTIFIER)
        {
            item = find_var(scope, ast->children[0]->token->lexeme);
        }
        else
        {
            item = find_var(scope, ast->children[0]->children[0]->token->lexeme);
        }

    }
    if(item == NULL || item->type[0] != TOKEN_CHAR)
    {
        int label = create_label();
        generator_output(code_gen, "\tmov %sd, 10\n", r1_n);
        generator_output(code_gen, "%s:\n", get_label_name(label));
        generator_output(code_gen, "\txor rdx, rdx\n"
                                   "\tdiv %sd\n"
                                   "\tadd dl, '0'\n", r1_n);

        generator_output(code_gen, "\tmov byte [%s], dl\n"
                                   "\tdec %s\n", r2_n, r2_n);

        generator_output(code_gen, "\ttest rax, rax\n"
                                   "\tjnz %s\n", get_label_name(label));

        generator_output(code_gen, "\tinc %s\n", r2_n);
        free_register(code_gen, r1);
    }
    else
    {
        generator_output(code_gen, "\tmov byte [%s], al\n", r2_n);
    }

    generator_output(code_gen, "\tpush r8\n"
                               "\tpush r9\n");

    generator_output(code_gen, sub, sp, "40"); // shadow space + 5th parameter

    generator_output(code_gen, "\tmov rcx, qword [REL _StandardOHandle]\n" // file handle
                               "\tmov rdx, %s\n" // load message location
                               "\tlea r9, [REL _Written]\n" // save the amount of chars printed
                               "\tmov qword [rsp+32], 0\n", r2_n, 10); // pass NULL to 5th param
    generator_output(code_gen, "\tlea r8, [REL _Message+10]\n" // the amount of characters to print
                               "\tsub r8, rdx\n");

    free_register(code_gen, r2);
    generator_output(code_gen, "\tcall WriteFile\n");
    generator_output(code_gen, add, sp, "40"); // restore stack

    generator_output(code_gen, "\tpop r9\n"
                               "\tpop r8\n");
}

void load_register_to_symbol(CodeGen* code_gen, ScopeNode* scope, ASTNode* symbol_ast, int source_reg)
{
    if(symbol_ast->type == TOKEN_COUNT + SYMBOL_ARR_ACC)
    {
        symbol_item* item = find_var(scope, symbol_ast->children[0]->token->lexeme);
        int r = allocate_register(code_gen);
        load_array_ax(code_gen, scope, symbol_ast->children[0], r);
        free_register(code_gen, r);
        generator_output(code_gen, "\tmov %s [%s], %s%c\n", convert_type_to_size_full(item->type[0]),
                         ax, get_register_name(code_gen, source_reg), convert_type_to_size(item->type[0]));
    } else
    {
        symbol_item* item = find_var(scope, symbol_ast->token->lexeme);
        generator_output(code_gen, "\tmov %s %s, %s%c\n", convert_type_to_size_full(item->type[0]),
                         get_symbol_code(item), get_register_name(code_gen, source_reg),
                         convert_type_to_size(item->type[0]));
    }
}

void load_number_to_symbol(CodeGen* code_gen, ScopeNode* scope, ASTNode* symbol_ast, char* num)
{
    if(symbol_ast->type == TOKEN_COUNT + SYMBOL_ARR_ACC)
    {
        symbol_item* item = find_var(scope, symbol_ast->children[0]->token->lexeme);
        int r = allocate_register(code_gen);
        load_array_ax(code_gen, scope, symbol_ast, r);
        free_register(code_gen, r);
        generator_output(code_gen, "\tmov %s [%s], %s\n", convert_type_to_size_full(item->type[0]),
                         ax, num);
    } else
    {
        symbol_item* item = find_var(scope, symbol_ast->token->lexeme);
        generator_output(code_gen, "\tmov %s %s, %s\n", convert_type_to_size_full(item->type[0]),
                         get_symbol_code(item), num);
    }
}

void generate_input_stmt(CodeGen *code_gen, ScopeNode *scope, ASTNode *ast)
{
    generator_output(code_gen, sub, sp, "40"); // shadow space + 5th parameter
    generator_output(code_gen, mov, cx, "[REL _StandardIHandle]"); // handle
    generator_output(code_gen, "\tlea %s, [REL %s]\n", dx, INPUT_BUFFER); // input buffer
    generator_output(code_gen, mov, R8, "1"); // number of chars to read
    generator_output(code_gen, "\tlea %s, %s\n", R9, "_Written"); // unused buffer of how many chars were read
    generator_output(code_gen, "\tmov qword [%s + %d], %s\n", sp, 32, "0"); // set input control parameter to off

    generator_output(code_gen, "\tcall ReadConsoleA\n");
    generator_output(code_gen, add, sp, "40"); // revert stack

    int reg = allocate_register(code_gen);
    generator_output(code_gen, "\txor %s, %s\n", get_register_name(code_gen, reg), get_register_name(code_gen, reg));
    generator_output(code_gen, "\tmovzx %s, byte [REL %s]\n", get_register_name(code_gen, reg), INPUT_BUFFER);
    load_register_to_symbol(code_gen, scope, ast->children[0], reg);
    free_register(code_gen, reg);
}

void generate_code(CodeGen* code_gen, ScopeNode* scope, ASTNode* ast)
{
    generator_output(code_gen, "extern ExitProcess\n");
    generator_output(code_gen, "extern GetStdHandle\n");
    generator_output(code_gen, "extern WriteFile\n");
    generator_output(code_gen, "extern ReadConsoleA\n\n");
    generator_output(code_gen, "extern GetConsoleMode\n\n");
    generator_output(code_gen, "extern SetConsoleMode\n\n");
    generator_output(code_gen, "default rel\n\n");

    generator_output(code_gen, "SECTION .bss\n");
    generator_output(code_gen, "\talignb 8\n");
    generator_output(code_gen, "\t_StandardOHandle resq 1\n");
    generator_output(code_gen, "\t_StandardIHandle resq 1\n");
    generator_output(code_gen, "\t_Written resq 1\n");


    //-----Creation of data segment-----
    generator_output(code_gen, "SECTION .data\n");
    generator_output(code_gen, "\t_Message db 10 dup(0)\n"); // reserve 10 bytes for output
    generator_output(code_gen, "\t_ReadChar dw 0\n"); // reserve 10 bytes for output
    generator_output(code_gen, "\t_ConsoleMode dw 0\n"); // reserve 10 bytes for output
    for(int i = 0; i< ast->num_of_children; i++)
    {
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
                generator_output(code_gen, "%s:\td%c 1\n", child->children[1]->token->lexeme, size);
            }
        }
        if (child->type == TOKEN_COUNT + SYMBOL_ARR_DEC)
        {
            int leftover = strtol(child->children[2]->token->lexeme, NULL, 10);
            if (child->num_of_children == 4)
            {
                generator_output(code_gen, "%s: d%c \"%c\"\n", child->children[1]->token->lexeme, size, child->children[3]->token->lexeme[0]);
                leftover -= strlen(child->children[3]->token->lexeme);
                int len = (int)strlen(child->children[3]->token->lexeme);
                for(int j = 1; j < len; j++)
                {
                    generator_output(code_gen, "\td%c \"%c\"\n", size, child->children[3]->token->lexeme[j]);
                }
            }
            if(leftover != 0)
            {
                generator_output(code_gen, "\td%c %d dup (?)\n", size, leftover);
            }
        }
    }
    //-----Start of code segment-----
    generator_output(code_gen, "\nSECTION .text\n");

    //call main
    generator_output(code_gen, "_start:\n");
    generate_init_io(code_gen);
    generator_output(code_gen, "\tcall _main\n"
                               "\tcall ExitProcess\n");

    generate_code_block(code_gen, scope, ast);
    for(int i = 0; i< ast->num_of_children; i++)
    {
        if(ast->children[i]->type == TOKEN_COUNT+SYMBOL_FUNC_DEC)
        {
            generate_code_block(code_gen, scope, ast->children[i]);
        }
    }

}