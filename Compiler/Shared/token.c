//
// Created by Nimrod on 04/02/2024.
//

#include "token.h"
#include <stdlib.h>
#include <string.h>

Token init_token(TokenType type, const char *lexeme, unsigned int line)
{
    Token token = malloc(sizeof(TokenStruct));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    return token;
}

void delete_token(Token token)
{
    free(token->lexeme);
    free(token);
}

char* get_token_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_ERROR:
            return "ERROR";
        case TOKEN_IDENTIFIER:
            return "ID";
        case TOKEN_INT_LITERAL:
            return "INT_LITERAL";
        case TOKEN_STRING_LITERAL:
            return "STRING_LITERAL";
        case TOKEN_PLUS_OP:
            return "PLUS";
        case TOKEN_MINUS_OP:
            return " - ";
        case TOKEN_EQUAL_EQUAL_OP:
            return " == ";
        case TOKEN_LESS_THAN_OP:
            return " < ";
        case TOKEN_GREATER_THAN_OP:
            return " > ";
        case TOKEN_LESS_THAN_EQUAL_OP:
            return " <= ";
        case TOKEN_GREATER_THAN_EQUAL_OP:
            return " >= ";
        case TOKEN_BITWISE_OR:
            return " | ";
        case TOKEN_BITWISE_AND:
            return " & ";
        case TOKEN_BITWISE_XOR:
            return " ^ ";
        case TOKEN_BITWISE_NOT:
            return " ~ ";
        case TOKEN_IF:
            return "IF";
        case TOKEN_ELSE:
            return "ELSE";
        case TOKEN_WHILE:
            return "WHILE";
        case TOKEN_FOR:
            return "FOR";
        case TOKEN_OR:
            return "OR";
        case TOKEN_AND:
            return "AND";
        case TOKEN_NOT:
            return "NOT";
        case TOKEN_L_CURLY_B:
            return "L_CURLY_B";
        case TOKEN_R_CURLY_B:
            return "R_CURLY_B";
        case TOKEN_RETURN:
            return "RETURN";
        case TOKEN_SEMICOLON:
            return "SEMICOLON";
        case TOKEN_INT:
            return "INT";
        case TOKEN_L_PAREN:
            return "L_PAREN";
        case TOKEN_R_PAREN:
            return "R_PAREN";
        case TOKEN_L_BRACKET:
            return "L_BRACKET";
        case TOKEN_R_BRACKET:
            return "R_BRACKET";
        case TOKEN_EOF:
            return "EOF";
        case TOKEN_EQUAL:
            return "EQUAL";
        case TOKEN_COMMENT:
            return "COMMENT";
        case TOKEN_COUNT:
            return "ERROR";
        case TOKEN_ASTERISK:
            return "ASTERISK";
        case TOKEN_F_SLASH:
            return "F_SLASH";
        case TOKEN_NOT_EQUAL_OP:
            return "NOT EQUAL";
        case TOKEN_CHAR:
            return "CHAR";
        case TOKEN_LONG:
            return "LONG";
        case TOKEN_UINT:
            return "UINT";
        case TOKEN_ULONG:
            return "ULONG";
        case TOKEN_VOID:
            return "VOID";
        case TOKEN_COLON:
            return "COLON";
        case TOKEN_COMMA:
            return "COMMA";
        case TOKEN_CHAR_LITERAL:
            return "CHAR LITERAL";
        case TOKEN_OUTPUT:
            return "OUTPUT";
    }
}