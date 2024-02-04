//
// Created by Nimrod on 04/02/2024.
//

#include "../definitions.h"

#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

typedef struct {
    TokenType type;
    char* lexeme;
} *Token, TokenStruct;

Token init_token(TokenType, const char*);

void delete_token(Token); // delete a token

char* get_token_name(TokenType); // get a string of the token

#endif //COMPILER_TOKEN_H
