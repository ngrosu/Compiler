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
#define NUMBER_STATE 4


// LEXER
#define TOKEN_MAXSIZE 64

// HASH TABLE
#define HASH_START_SIZE 10
#define DISABLE_HASH_ADD_WARNING 1

// SYMBOL TABLE
typedef enum
{
    SCOPE_GLOBAL,
    SCOPE_CONDITIONAL,
    SCOPE_LOOP,
    SCOPE_FUNCTION
} ScopeType;


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
    TOKEN_BREAK,
    TOKEN_OUTPUT,
    TOKEN_INPUT,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_SHORT,
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
    SYMBOL_BREAK,
    SYMBOL_SCOPE,
    SYMBOL_RET_SCOPE,
    SYMBOL_PARAMETERS,
    SYMBOL_PARAMETER,
    SYMBOL_ASSIGNMENT,
    SYMBOL_ARR_ACC,
    SYMBOL_ARGUMENTS,
    SYMBOL_VAR_DEC,
    SYMBOL_ARR_DEC,
    SYMBOL_FUNC_DEC,
    SYMBOL_OUTPUT,
    SYMBOL_INPUT,
    SYMBOL_FUNC_CALL,
    SYMBOL_EXPRESSION,
    SYMBOL_EXPRESSION_STATEMENT,
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

typedef enum {
    FUNC_DEFAULT,
    FUNC_BINARY_INFIX,
    FUNC_PARENTHESES,
    FUNC_DIRECT_PASS,
    FUNC_CHAIN,
    FUNC_REM_SECOND_L_CHAIN,
    FUNC_REMOVE_END,
    FUNC_REM_F_TWO_AND_SECOND_L,
    FUNC_REM_L_AND_THIRD_L,
    FUNC_REM_L_THIRD_L_FOURTH_L_SIXTH_L,
    FUNC_REM_LAST_TWO_AND_FOURTH_L,
    FUNC_REM_SECOND_L,
    FUNC_KEEP_FIRST,
    FUNC_KEEP_FIRST_INDIRECT,
    FUNC_KEEP_SECOND,
    FUNC_REMOVE_EDGES,
    FUNC_REMOVE_THIRD,
    FUNC_FOR,
    FUNC_CHAR,
    FUNCS_COUNT
} ASTFuncs;
// PARSER
#define MAX_RULE_SIZE 10


// CODE GENERATOR

#define REGISTER_NAME_MAX_SIZE 8
#define NUM_OF_REGISTERS 4
#define LABEL_NAME_MAX_SIZE 10
#define LABEL_FORMAT ".L%d"
#define STACK_ALLIGNMENT_SIZE 16
#define STACK_ENTRY_SIZE 8
#define STACK_OFFSET_FORMAT "[rbp%c%d]"
#define STACK_FUNC_START_PUSHES 2
#define STD_OUTPUT_HANDLE (-11)
#define STD_INPUT_HANDLE (-10)
#define INPUT_BUFFER "_ReadChar"
#define OUTPUT_BUFFER "_Message"
#define GLOBAL_ADDRESS_FORMAT "[REL %s]"
// REGISTERS
#define ax "rax"
#define cx "rcx"
#define dx "rdx"
#define bp "rbp"
#define sp "rsp"
#define R8 "r8"
#define R9 "r9"
#define R10 "r10"
#define R11 "r11"
// INSTRUCTIONS
#define mov "\tmov %s, %s\n"
#define movx "\tmov %s %s, %s\n"
#define movsx "\tmovsx %s, %s\n"
#define add "\tadd %s, %s\n"
#define sub "\tsub %s, %s\n"
#define imul "\timul %s, %s\n"
#define idiv "\tidiv %s\n"
#define xor "\txor %s, %s\n"
#define and "\tand %s, %s\n"
#define or "\tor %s, %s\n"


// ERROR
typedef enum {
    ERR_LEXICAL,        // Lexical analysis error
    ERR_SYNTAX,         // Syntax error
    ERR_SEMANTIC,       // Semantic error
    ERR_UNDEFINED_VAR,  // Undefined variable
    ERR_TYPE_MISMATCH,  // Type mismatch in expressions
    ERR_INTERNAL,        // Internal compiler error
    ERR_WARNING
} ErrorCode;

#endif //COMPILER_DEFINITIONS_H
