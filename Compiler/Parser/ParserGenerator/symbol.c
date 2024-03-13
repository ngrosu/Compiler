//
// Created by Nimrod on 02/03/2024.
//

#include "symbol.h"



const char* get_symbol_name(int symbol)
{
    if(symbol < TOKEN_COUNT)
    {
        return get_token_name(symbol);
    }
    SymbolType tempID = symbol-TOKEN_COUNT;
    switch (tempID)
    {
        case SYMBOL_EXPRESSION:
            return "Expr";
        case SYMBOL_TERM:
            return "Term";
        case SYMBOL_FACTOR:
            return "Fact";
        case SYMBOL_COUNT:
            return "ERROR(COUNT SYMBOL)";
        case SYMBOL_START:
            return "START";
        case SYMBOL_START_TAG:
            return "START_TAG";
        case SYMBOL_STATEMENTS:
            return "Statements";
        case SYMBOL_STATEMENT:
            return "Statement";
        case SYMBOL_DECLARATION:
            return "Declaration";
        case SYMBOL_IF:
            return "If";
        case SYMBOL_IF_ELSE:
            return "If-Else";
        case SYMBOL_FOR:
            return "For";
        case SYMBOL_WHILE:
            return "While";
        case SYMBOL_RETURN:
            return "Return Statement";
        case SYMBOL_SCOPE:
            return "Scope";
        case SYMBOL_RET_SCOPE:
            return "Return Scope";
        case SYMBOL_PARAMETERS:
            return "Parameters";
        case SYMBOL_PARAMETER:
            return "Parameter";
        case SYMBOL_VAR_DEC:
            return "Variable Declaration";
        case SYMBOL_ARR_DEC:
            return "Array Declaration";
        case SYMBOL_FUNC_DEC:
            return "Function Declaration";
        case SYMBOL_FUNC_CALL:
            return "Function Call";
        case SYMBOL_BOOL_EXPR:
            return "Boolean Expression";
        case SYMBOL_BOOL_TERM:
            return "Boolean Term";
        case SYMBOL_BOOL_FACTOR:
            return "Boolean Factor";
        case SYMBOL_RELAT_EXPR:
            return "Relational Expression";
        case SYMBOL_RELAT_OP:
            return "Relational Operator";
        case SYMBOL_TYPE:
            return "Type";
        case SYMBOL_NUMBER:
            return "Number";
        case SYMBOL_END_STATEMENT:
            return "End Statement";
        default:
            return "INVALID";
        case SYMBOL_ASSIGNMENT:
            return "Assignment";
        case SYMBOL_ARGUMENTS:
            return "Arguments";
        case SYMBOL_OUTPUT:
            return "Output";
        case SYMBOL_ARR_ACC:
            return "Array Access";
        case SYMBOL_INPUT:
            return "Input";
    }

}

