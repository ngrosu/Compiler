//
// Created by Nimrod on 29/01/2024.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H


typedef enum
{
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,
    // Literal Tokens
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,
    // Operator Tokens
    TOKEN_PLUS_OP,
    TOKEN_MINUS_OP,
    TOKEN_EQUAL_EQUAL_OP,
    TOKEN_LESS_THAN_OP,
    TOKEN_GREATER_THAN_OP,
    TOKEN_LESS_THAN_EQUAL_OP,
    TOKEN_GREATER_THAN_EQUAL_OP,
        // Bitwise Tokens
        TOKEN_BITWISE_OR,  //|
        TOKEN_BITWISE_AND, //&
        TOKEN_BITWISE_XOR, //^
        TOKEN_BITWISE_NOT, //~
    // Keyword Tokens
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    // Logic Tokens
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_NOT
} TokenType;

#endif //COMPILER_LEXER_H
