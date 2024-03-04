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
        default:
            return "INVALID";
        case SYMBOL_START:
            return "START";
        case SYMBOL_START_TAG:
            return "START_TAG";
    }
}

