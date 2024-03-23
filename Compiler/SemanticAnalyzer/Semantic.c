//
// Created by Nimrod on 23/03/2024.
//

#include "Semantic.h"


char check_type_range(int type, unsigned long value)
{
    char result = 1;
    switch(type)
    {
        case TOKEN_CHAR:
            if (value > 255)
            {
                result = 0;
            }
            break;
        case TOKEN_INT:
            if (value > 2147483647)
            {
                result = 0;
            }
            break;
        case TOKEN_LONG:
            if (value > 9223372036854775807)
            {
                result = 0;
            }
            break;
    }
    return result;
}

int check_var_usage(ASTNode* ast, ScopeNode* scope)
{
    int error = 0;
    symbol_item* item = find_var(scope, ast->token->lexeme);
    if(item == NULL)
    {
        report_error(ERR_SEMANTIC, ast->start_line, "Use of undeclared identifier ", ast->token->lexeme);
        error = 1;
    }
    else if (item->line_of_dec > ast->start_line)
    {
        error = !check_var_usage(ast, scope->parent);
    }
    else if(!item->assigned)
    {
        report_error(ERR_SEMANTIC, ast->start_line, "Use of unassigned identifier ", ast->token->lexeme);
        error = 1;
    }
    return !error;
}

int analyze_expression(ASTNode *ast, ScopeNode *scope)
{
    int result = 1;
    switch (ast->type)
    {
        case TOKEN_PLUS_OP:
        case TOKEN_MINUS_OP:
        case TOKEN_F_SLASH:
        case TOKEN_ASTERISK:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
            result = (analyze_expression(ast->children[0], scope) && analyze_expression(ast->children[1], scope));
            break;
        case TOKEN_BITWISE_NOT:
            result = analyze_expression(ast->children[0], scope);
            break;
        case TOKEN_INT_LITERAL:
            break;
        case TOKEN_IDENTIFIER:
            result = check_var_usage(ast, scope);
            break;
        case SYMBOL_FUNC_CALL + TOKEN_COUNT:
            result = analyze_func_call(ast, scope);
            break;
    }
    return result;
}

int analyze_bool(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    switch (ast->type)
    {
        case TOKEN_AND:
        case TOKEN_OR:
            result = (analyze_bool(ast->children[0], scope) && analyze_bool(ast->children[1], scope));
            break;
        case TOKEN_GREATER_THAN_EQUAL_OP:
        case TOKEN_GREATER_THAN_OP:
        case TOKEN_LESS_THAN_EQUAL_OP:
        case TOKEN_LESS_THAN_OP:
        case TOKEN_EQUAL_EQUAL_OP:
        case TOKEN_NOT_EQUAL_OP:
            result = (analyze_expression(ast->children[0], scope) && analyze_expression(ast->children[1], scope));
            break;
        case TOKEN_NOT:
            result = analyze_bool(ast->children[0], scope);
            break;
        default:
            result = analyze_expression(ast, scope);
            break;
    }
    return result;
}

int analyze_while(ASTNode *ast, ScopeNode *scope)
{
    int result = 1;
    result &= analyze_bool(ast->children[0], scope->parent);
    result &= analyze_statements(ast->children[1], scope);
    return result;
}
int analyze_func(ASTNode *ast, ScopeNode *scope)
{
    if (ast->num_of_children == 4)
    {
        return analyze_statements(ast->children[3], scope);
    }
    return analyze_statements(ast->children[2], scope);
}

int analyze_func_call(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    symbol_item* item = find_var(scope, ast->children[0]->token->lexeme);
    if (item == NULL)
    {
        result = 0;
        report_error(ERR_SEMANTIC, ast->start_line, "Undefined function name | ", ast->children[0]->token->lexeme);
    }
    int num_of_args = ast->num_of_children == 2 ? ast->children[1]->num_of_children : 0;
    if(item->num_of_params != num_of_args)
    {
        result = 0;
        char err[48];
        sprintf(err, " expects %d arguments but received %d arguments", item->num_of_params, num_of_args);
        report_error(ERR_SEMANTIC, ast->start_line, ast->children[0]->token->lexeme, err);
    }
    return result;
}

int analyze_statements(ASTNode *ast, ScopeNode *scope)
{
    if(ast->type == SYMBOL_SCOPE + TOKEN_COUNT)
        return 1;
    unsigned int scope_ind = 0;
    ASTNode* child = NULL;
    int error = 0;
    for(int i = 0; i < ast->num_of_children; i++)
        {
            child = ast->children[i];
            switch(child->type)
            {
                case SYMBOL_RETURN+TOKEN_COUNT:
                    break;
                case SYMBOL_BREAK+TOKEN_COUNT:
                    break;
                case SYMBOL_STATEMENTS + TOKEN_COUNT:
                    break;
                case SYMBOL_FUNC_DEC + TOKEN_COUNT:
                    error |= !analyze_func(child, scope->children[scope_ind++]);
                    break;
                case SYMBOL_FUNC_CALL + TOKEN_COUNT:
                    error |= !analyze_func_call(child, scope);
                    break;
                case SYMBOL_WHILE + TOKEN_COUNT:
                    error |= !analyze_while(child, scope->children[scope_ind++]);
                    break;
                case SYMBOL_FOR + TOKEN_COUNT:
                    break;
            }
        }
    return !error;
}