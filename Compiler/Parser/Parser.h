//
// Created by Nimrod on 05/03/2024.
//

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "../Shared/token.h"
#include "../Shared/hashtable.h"
#include "ParserGenerator/DataStructures.h"
#include "ParserGenerator/Functions.h"

typedef struct ASTNode{
    struct ASTNode** children;
    int num_of_children;
    int type;
    unsigned long start_line;
    Token token;
} ASTNode;

// Print out the AST neatly.
void printAST(ASTNode *node, int depth, char *finals);

typedef struct parser{
    Token* tokens;
    unsigned int num_of_tokens;
    genDynArrPtr grammar;
    unsigned int num_of_states;
    unsigned int** action_table;
    unsigned int** goto_table;
    Stack* stack;

} *Parser, parserStruct;

// helper functions for AST construction functions
void ast_skip_items(Parser parser, int num);
void ast_keep_items(Parser parser, ASTNode** children, int curr_index, int num);
ASTNode* peak_ast_symbol(Parser parser);

void init_AST_funcs();

ASTNode *init_AST_node(int type, ASTNode **children, int num_of_children, Token token, unsigned long line);

void delete_AST(ASTNode* node);

Parser
init_parser(Token *tokens, unsigned int num_of_tokens, genDynArrPtr grammar, char load, char save, char *action_fp,
            char *goto_fp);

char parse(Parser parser);


#endif //COMPILER_PARSER_H
