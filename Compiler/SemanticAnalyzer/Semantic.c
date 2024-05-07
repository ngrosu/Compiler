//
// Created by Nimrod on 23/03/2024.
//

#include "Semantic.h"


int check_num_type(ASTNode *ast)
{
    int result;
    unsigned long long value;
    value = strtoull(ast->token->lexeme, NULL, 10);
    if(value <= 255)
    {
        result = TOKEN_CHAR;
    }
    else if(value <= 32767)
    {
        result = TOKEN_SHORT;
    }

    else if(value <= 2147483647)
    {
        result = TOKEN_INT;
    }
    else
    {
        result = TOKEN_ERROR;
        report_error(ERR_SEMANTIC, ast->start_line, "Number literal is too large | ", ast->token->lexeme);
    }
    return result;
}

int check_return(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    ScopeNode* func = is_in_scope(scope, SCOPE_FUNCTION);
    if(func != NULL)
    {
        if(func->return_type == TOKEN_VOID)
        {
            if(ast->num_of_children == 1)
            {
                result = 0;
                report_error(ERR_SEMANTIC, ast->start_line, "Void function returns a value", NULL);
            }
        }
        else
        {
            if(ast->num_of_children == 0)
            {
                result = 0;
                report_error(ERR_SEMANTIC, ast->start_line, "Non void function returns void", NULL);
            }
            else
            {
                int temp_type = get_expression_type(ast->children[0], scope);
                if(func->return_type != resolve_types(func->return_type, temp_type))
                {
                    result = 0;
                    char err[41];
                    sprintf(err, "function of type %s returns type %s ", get_token_name(func->return_type),
                            get_token_name(temp_type));
                    report_error(ERR_SEMANTIC, ast->start_line, err, NULL);
                }
            }
        }
        func->returning = 1;
    }
    else
        result = 0;
    return result;
}

int check_break(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    if(is_in_scope(scope, SCOPE_LOOP) == NULL)
    {
        result = 0;
        report_error(ERR_SEMANTIC, ast->start_line, "Break statement outside of loop", NULL);
    }
    return result;
}

symbol_item *check_var_declared(ASTNode *ast, ScopeNode *scope)
{
    symbol_item* item = find_var(scope, ast->token->lexeme);
    if(item == NULL)
    {
        report_error(ERR_SEMANTIC, ast->start_line, "Use of undeclared identifier ", ast->token->lexeme);
    }
    else if (item->line_of_dec > ast->start_line)
    {
        item = check_var_declared(ast, scope->parent);
    }
    return item;
}

int analyze_var_dec(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    if(ast->num_of_children == 3)
    {
        if(scope->scope == SCOPE_GLOBAL && ast->children[2]->type != TOKEN_INT_LITERAL)
        {
            result = 0;
            report_error(ERR_SEMANTIC, ast->children[1]->start_line, "Invalid assignment in global scope"
                                                                     " to variable ", ast->children[1]->token->lexeme);
        }
        int temp_type = get_expression_type(ast->children[2], scope);
        if(ast->children[0]->type != resolve_types(temp_type, ast->children[0]->type))
        {
            char err[39 + TOKEN_MAXSIZE];
            sprintf(err, "variable %s of type %s assigned type %s", ast->children[1]->token->lexeme,
                    get_symbol_name(ast->children[0]->type), get_symbol_name(temp_type));
            report_error(ERR_SEMANTIC, ast->start_line, err, NULL);
            result = 0;
        }
    }
    return result;
}

int analyze_assignment(ASTNode* ast, ScopeNode* scope)
{
    symbol_item* item;
    int result = 1;
    if (ast->children[0]->type == TOKEN_IDENTIFIER) // check if assigning to an identifier
    {
        item = check_var_declared(ast->children[0], scope);
        if (item != NULL)
        {
            if(item->type[1] != SYMBOL_VAR_DEC + TOKEN_COUNT)
            {
                result = 0;
                report_error(ERR_SEMANTIC, ast->start_line, "Use of non-variable identifier as variable | ", ast->children[0]->token->lexeme);
            } // TODO fix bug where if u do a variable = to itself it thinks its assigned
            int run = 1;
            if(item->assigned == 0)
            {
                while (scope != NULL && run)
                {
                    if (get_item(scope->table, ast->children[0]->token->lexeme) == NULL)
                    {
                        if (scope->scope == SCOPE_LOOP || scope->scope == SCOPE_CONDITIONAL)
                        {
                            report_error(ERR_WARNING, ast->start_line,
                                         "Assignment in conditional may lead to use of unassigned variable | ",
                                         ast->children[0]->token->lexeme);
                            item->assigned = 2;
                            run = 0;
                        } else
                            scope = scope->parent;
                    } else
                    {
                        run = 0;
                        item->assigned = 1;
                    }
                }
            }
            else
                item->assigned = 1;
        }
        else
            result = 0;
    }
    else // if not identifier, it's an array
    {
        item = check_var_declared(ast->children[0]->children[0], scope);
        result &= analyze_arr_acc(ast->children[0], scope);
    }
    result &= analyze_expression(ast->children[1], scope);
    int type = get_expression_type(ast->children[1], scope);
    if (type == TOKEN_ERROR)
        result = 0;
    else if (resolve_types(item->type[0], type) != item->type[0])
    {
        result = 0;
        char err[63];
        sprintf(err, "Tried assigning expression of type %s to item of type %s, ", get_token_name(type),
                get_token_name(item->type[0]));
        report_error(ERR_SEMANTIC, ast->start_line, err, ast->children[0]->token->lexeme);
    }
    return result;
}

int analyze_arr_dec(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    result &= (check_num_type(ast->children[2]) != 0);
    if(ast->num_of_children == 4)
    {
        if(scope->scope == SCOPE_GLOBAL && ast->children[3]->type != TOKEN_STRING_LITERAL)
        {
            result = 0;
            report_error(ERR_SEMANTIC, ast->children[1]->start_line, "Invalid assignment in global scope"
                                                                     " to variable ", ast->children[1]->token->lexeme);
        }
        if(strlen(ast->children[3]->token->lexeme)  > strtol(ast->children[2]->token->lexeme, NULL, 10))
        {
            result = 0;
            report_error(ERR_SEMANTIC, ast->start_line, "Initialization string for array is too large | ",
                         ast->children[2]->token->lexeme);
        }
    }
    return result;
}
int analyze_arr_acc(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    symbol_item* item = check_var_declared(ast->children[0], scope);
    if (item != NULL)
    {
        if (item->type[1] != SYMBOL_ARR_DEC + TOKEN_COUNT)
        {
            result = 0;
            report_error(ERR_SEMANTIC, ast->start_line, "Tried to access non-array identifier as array | ",
                         ast->children[0]->token->lexeme);
        }
    }
    else
        result = 0;
    result &= analyze_expression(ast->children[1], scope);
    return result;
}

int analyze_var_acc(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    symbol_item* item = check_var_declared(ast, scope);
    if(item != NULL)
    {
        if(item->type[1] != SYMBOL_VAR_DEC + TOKEN_COUNT)
        {
            report_error(ERR_SEMANTIC, ast->start_line, "Use of non-variable identifier as variable | ", ast->token->lexeme);
            result = 0;
        }
        else if (item->assigned == 0)
        {
            report_error(ERR_SEMANTIC, ast->start_line, "Use of unassigned variable | ", ast->token->lexeme);
            result = 0;
        }
    } else result = 0;
    return result;
}

int resolve_types(int type1, int type2)
{
    if(type1 == TOKEN_ERROR || type2 == TOKEN_ERROR)
        return TOKEN_ERROR;
    if(type1 == TOKEN_INT || type2 == TOKEN_INT)
        return TOKEN_INT;
    if(type1 == TOKEN_SHORT || type2 == TOKEN_SHORT)
        return TOKEN_SHORT;
    if(type1 == TOKEN_CHAR || type2 == TOKEN_CHAR)
        return TOKEN_CHAR;
    printf("resolve_types unexpected code path :O");
    return TOKEN_ERROR;
}

int get_expression_type(ASTNode *ast, ScopeNode *scope)
{
    int result = TOKEN_CHAR;
    switch (ast->type)
    {
        case TOKEN_PLUS_OP:
        case TOKEN_MINUS_OP:
        case TOKEN_F_SLASH:
        case TOKEN_ASTERISK:
        case TOKEN_BITWISE_AND:
        case TOKEN_BITWISE_OR:
        case TOKEN_BITWISE_XOR:
            result = resolve_types(get_expression_type(ast->children[0], scope), get_expression_type(ast->children[1], scope));
            break;
        case TOKEN_BITWISE_NOT:
            result = get_expression_type(ast->children[0], scope);
            break;
        case TOKEN_INT_LITERAL:
            result = check_num_type(ast);
            break;
        case TOKEN_IDENTIFIER:
            if(check_var_declared(ast, scope))
            {
                result = find_var(scope, ast->token->lexeme)->type[0];
            }
            else
                result = TOKEN_ERROR;
            break;

        case SYMBOL_ARR_ACC + TOKEN_COUNT:
            if(analyze_arr_acc(ast, scope))
                result = find_var(scope, ast->children[0]->token->lexeme)->type[0];
            else
                result = TOKEN_ERROR;
            break;

        case SYMBOL_FUNC_CALL + TOKEN_COUNT:
            if(analyze_func_call(ast, scope))
                result = find_var(scope, ast->children[0]->token->lexeme)->type[0];
            else
                result = TOKEN_ERROR;
            break;
    }
    return result;
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
            result = check_num_type(ast) != TOKEN_ERROR;
            break;
        case TOKEN_IDENTIFIER:
            result = analyze_var_acc(ast, scope);
            break;
        case SYMBOL_ARR_ACC + TOKEN_COUNT:
            result = analyze_arr_acc(ast, scope);
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

int analyze_if(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    result &= analyze_bool(ast->children[0], scope->parent);
    result &=analyze_statements(ast->children[1], scope, 0);
    return result;
}
int analyze_else(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    result &= analyze_statements(ast->children[2], scope, 0);
    return result;
}

int analyze_for(ASTNode *ast, ScopeNode *scope)
{
    int result = 1;
    result &= analyze_bool(ast->children[1], scope);
    if(ast->children[2]->type == SYMBOL_ASSIGNMENT)
    {
        analyze_assignment(ast->children[2], scope);
//        result &= check_var_usage(ast->children[2]->children[0], scope, 0);
//        result &= analyze_expression(ast->children[2]->children[1], scope);
    }
    else
    {
        result &= analyze_expression(ast->children[2], scope);
    }
    result &= analyze_statements(ast->children[3], scope, 0);
    return result;
}

int analyze_while(ASTNode *ast, ScopeNode *scope)
{
    int result = 1;
    result &= analyze_bool(ast->children[0], scope->parent);
    result &= analyze_statements(ast->children[1], scope, 0);
    return result;
}
int analyze_func(ASTNode *ast, ScopeNode *scope)
{
    if(scope->parent->scope != SCOPE_GLOBAL)
    {
        report_error(ERR_SEMANTIC, ast->start_line, "Function declared outside of global scope", NULL);
        return 0;
    }
    if (ast->num_of_children == 4)
    {
        return analyze_statements(ast->children[3], scope, 0);
    }
    return analyze_statements(ast->children[2], scope, 0);
}

int analyze_func_call(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    symbol_item* item = check_var_declared(ast->children[0], scope);
    if ( item == NULL)
    {
        result = 0;
    }
    else if (item->type[1] != SYMBOL_FUNC_DEC + TOKEN_COUNT)
    {
        report_error(ERR_SEMANTIC, ast->start_line, "Non-func identifier used as func | ", ast->children[0]->token->lexeme);
    }
    else
    {
        int num_of_args = ast->num_of_children == 2 ? ast->children[1]->num_of_children : 0;
        if (item->num_of_params != num_of_args)
        {
            result = 0;
            char err[48];
            sprintf(err, " expects %d arguments but received %d arguments", item->num_of_params, num_of_args);
            report_error(ERR_SEMANTIC, ast->start_line, ast->children[0]->token->lexeme, err);
        }
        else
        {
            for(int i = 0; i < num_of_args; i++)
            {
                int temp_type = get_expression_type(ast->children[1]->children[i], scope);
                if(resolve_types(temp_type, item->parameters[i].type) != item->parameters[i].type)
                {
                    result = 0;
                    char err[61 + TOKEN_MAXSIZE];
                    sprintf(err, " received argument of type %s for parameter \"%s\" of type %s", get_token_name(temp_type),
                            item->parameters[i].name, get_token_name(item->parameters[i].type));
                    report_error(ERR_SEMANTIC, ast->start_line, ast->children[0]->token->lexeme, err);

                }
            }
        }
    }
    return result;
}

int analyze_input(ASTNode* ast, ScopeNode* scope)
{
    int result = 1;
    symbol_item* item;
    switch (ast->type)
    {
        case TOKEN_COUNT + SYMBOL_ARR_ACC:
            item = find_var(scope, ast->children[0]->token->lexeme);
            if(item!=NULL)
                item->assigned=1;
            result &= analyze_arr_acc(ast, scope);
            break;
        case TOKEN_IDENTIFIER:
            item = find_var(scope, ast->token->lexeme);
            if(item!=NULL)
                item->assigned=1;
            result &= analyze_var_acc(ast, scope);
            break;
        default:
            result = 0;
            report_error(ERR_SEMANTIC, ast->start_line, "Input must be into array or variable", NULL);
            break;
    }
    return result;
}

int analyze_statements(ASTNode *ast, ScopeNode *scope, int scope_index)
{
    if(ast->type == SYMBOL_SCOPE + TOKEN_COUNT)
        return 1;
    int scope_ind = scope_index;
    ASTNode* child = NULL;
    int error = 0;
    for(int i = 0; i < ast->num_of_children; i++)
        {
            child = ast->children[i];
            switch(child->type)
            {
                case SYMBOL_RETURN+TOKEN_COUNT:
                    error |= !check_return(child, scope);
                    break;
                case SYMBOL_BREAK+TOKEN_COUNT:
                    error |= !check_break(child, scope);
                    break;
                case SYMBOL_STATEMENTS + TOKEN_COUNT:
                    error |= !analyze_statements(child, scope, scope_ind);
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
                    error |= !analyze_for(child, scope->children[scope_ind++]);
                    break;
                case SYMBOL_IF_ELSE + TOKEN_COUNT:
                    error |= !analyze_if(child, scope->children[scope_ind++]);
                    error |= !analyze_else(child, scope->children[scope_ind++]);
                    break;
                case SYMBOL_IF + TOKEN_COUNT:
                    error |= !analyze_if(child, scope->children[scope_ind++]);
                    break;
                case SYMBOL_ASSIGNMENT + TOKEN_COUNT:
                    error |= !analyze_assignment(child, scope);
                    break;
                case SYMBOL_VAR_DEC + TOKEN_COUNT:
                    error |= !analyze_var_dec(child, scope);
                    break;
                case SYMBOL_ARR_DEC + TOKEN_COUNT:
                    error |= !analyze_arr_dec(child, scope);
                    break;
                case TOKEN_IDENTIFIER:
                case TOKEN_INT_LITERAL:
                case SYMBOL_ARR_ACC + TOKEN_COUNT:
                case TOKEN_PLUS_OP:
                case TOKEN_MINUS_OP:
                case TOKEN_F_SLASH:
                case TOKEN_ASTERISK:
                case TOKEN_BITWISE_AND:
                case TOKEN_BITWISE_OR:
                case TOKEN_BITWISE_XOR:
                case TOKEN_BITWISE_NOT:
                    error |= !analyze_expression(child, scope);
                    break;
                case TOKEN_COUNT + SYMBOL_OUTPUT:
                    error |= !analyze_expression(child->children[0], scope);
                    break;
                case TOKEN_COUNT + SYMBOL_INPUT:
                    error |= !analyze_input(child->children[0], scope);
                    break;
                default:
                    printf("\nNo handling for statement type %s\n", get_symbol_name(child->type));
            }
        }
    if(scope->scope == SCOPE_FUNCTION && scope->returning == 0 && scope->return_type != TOKEN_VOID)
    {
        error = 1;
        report_error(ERR_SEMANTIC, scope->start_line, "Non void function missing return statement", NULL);
    }
    return !error;
}