//
// Created by Nimrod on 29/01/2024.
//

#include "../Shared/error.h"
#include "../definitions.h"
#include "dfa_table.h"
#include "../Shared/token.h"

#include <stdio.h>

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

typedef struct {
    DFA dfa;
    FILE* file;
    Token* tokens;
    unsigned int tokens_allocated_size;
    unsigned int num_of_tokens;
    unsigned int curr_line;
    char token_buffer[TOKEN_MAXSIZE];
} *Lexer, LexerStruct;

Lexer init_lexer(char*); // create a lexer and init it with the source code's fp

Token get_next_token(Lexer);

void skip_whitespaces(Lexer); // advance the file until the next non-whitespace

void print_tokens(Lexer);

char tokenize(Lexer);

void add_token(Lexer, Token); // add a token to the lexers tokens array

#endif //COMPILER_LEXER_H
