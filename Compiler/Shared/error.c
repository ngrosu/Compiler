//
// Created by Nimrod on 04/02/2024.
//

#include "error.h"

void report_error(ErrorCode code, unsigned int line, const char *message, const char *extra)
{  // print out an error
    const char* errorString = "Unknown error";

    switch (code) {
        case ERR_LEXICAL:
            errorString = "Lexical Error";
            break;
        case ERR_SYNTAX:
            errorString = "Syntax Error";
            break;
        case ERR_SEMANTIC:
            errorString = "Semantic Error";
            break;
        case ERR_UNDEFINED_VAR:
            errorString = "Undefined Variable";
            break;
        case ERR_TYPE_MISMATCH:
            errorString = "Type Mismatch";
            break;
        case ERR_INTERNAL:
            errorString = "Internal Error";
            break;
        default:
            break;  // No action needed for no error or default case
    }

    fprintf(stderr, "[%s] at line %d: %s", errorString, line, message);
    if(extra!=NULL)
        fprintf(stderr, "%s", extra);
    fprintf(stderr, "\n");
}
