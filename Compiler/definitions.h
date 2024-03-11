//
// Created by Nimrod on 04/02/2024.
//

#ifndef COMPILER_DEFINITIONS_H
#define COMPILER_DEFINITIONS_H

// DFA_TABLE
#define ID_DELIMITERS " +=\n;,()[]/&|!"
#define NUM_OF_CHARS 128
#define START_STATE 1
#define DELIMITER_STATE 2
#define IDENTIFIER_STATE 3


// LEXER
#define TOKEN_MAXSIZE 64

// TOKEN
typedef enum
{
    TOKEN_ERROR,
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_COMMENT,
    // Literal Tokens
    TOKEN_INT_LITERAL,
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,
    // Operator Tokens
    TOKEN_PLUS_OP,
    TOKEN_MINUS_OP,
    TOKEN_NOT_EQUAL_OP,
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
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_LONG,
    TOKEN_UINT,
    TOKEN_ULONG,
    TOKEN_VOID,
    // Logic Tokens
    TOKEN_OR,
    TOKEN_AND,
    TOKEN_NOT,
    // Symbol Tokens
    TOKEN_L_CURLY_B,
    TOKEN_R_CURLY_B,
    TOKEN_L_PAREN,
    TOKEN_R_PAREN,
    TOKEN_L_BRACKET,
    TOKEN_R_BRACKET,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_EQUAL,
    TOKEN_ASTERISK,
    TOKEN_F_SLASH,
    TOKEN_COUNT // stores the amount of TokenTypes in the enum
} TokenType;

// SYMBOLS
typedef enum {
    SYMBOL_STATEMENTS,
    SYMBOL_STATEMENT,
    SYMBOL_DECLARATION,
    SYMBOL_IF,
    SYMBOL_IF_ELSE,
    SYMBOL_FOR,
    SYMBOL_WHILE,
    SYMBOL_RETURN,
    SYMBOL_SCOPE,
    SYMBOL_RET_SCOPE,
    SYMBOL_PARAMETERS,
    SYMBOL_PARAMETER,
    SYMBOL_ASSIGNMENT,
    SYMBOL_ARGUMENTS,
    SYMBOL_ARGUMENTS_TAIL,
    SYMBOL_VAR_DEC,
    SYMBOL_ARR_DEC,
    SYMBOL_FUNC_DEC,
    SYMBOL_FUNC_CALL,
    SYMBOL_EXPRESSION,
    SYMBOL_BOOL_EXPR,
    SYMBOL_BOOL_TERM,
    SYMBOL_BOOL_FACTOR,
    SYMBOL_RELAT_EXPR,
    SYMBOL_RELAT_OP,
    SYMBOL_TERM,
    SYMBOL_FACTOR,
    SYMBOL_TYPE,
    SYMBOL_NUMBER,
    SYMBOL_END_STATEMENT,
    SYMBOL_START,
    SYMBOL_START_TAG,
    SYMBOL_COUNT // stores the amount of array in SymbolType
} SymbolType;

typedef enum {
    ACTION_ERROR,
    ACTION_SHIFT,
    ACTION_REDUCE,
    ACTION_ACCEPT
} Actions;
// PARSER
#define MAX_RULE_SIZE 10


// ERROR
typedef enum {
    ERR_LEXICAL,        // Lexical analysis error
    ERR_SYNTAX,         // Syntax error
    ERR_SEMANTIC,       // Semantic error
    ERR_UNDEFINED_VAR,  // Undefined variable
    ERR_TYPE_MISMATCH,  // Type mismatch in expressions
    ERR_INTERNAL        // Internal compiler error
} ErrorCode;

#endif //COMPILER_DEFINITIONS_H
