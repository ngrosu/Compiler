//
// Created by Nimrod on 14/03/2024.
//

#ifndef COMPILER_SYMBOLTABLEMANAGER_H
#define COMPILER_SYMBOLTABLEMANAGER_H
#include "../Parser/Parser.h"
#include "hashtable.h"

typedef struct ScopeNode{
    struct ScopeNode** children;
    struct ScopeNode* parent;
    int num_of_children;
    struct hash_table* table;
    ScopeType scope;
} ScopeNode;

ScopeNode* init_scope_node(ScopeType scope);

void add_scope_child(ScopeNode* parent, ScopeNode *child);

typedef struct param
{
    int type;
    char* name;
} Param;

typedef struct SymbolItem
{
    char* name;
    int type[2];
    unsigned int offset;
    unsigned int size;
    struct param* parameters;
    int num_of_params;
    unsigned int line_of_dec;
} symbol_item;

symbol_item *
init_symbol_item(char *name, int data_type, int symbol_type, Param *parameters, int num_of_params, int length,
                 unsigned int line);

Param* init_params(ASTNode* params);

void construct_symbol_table_rec(ASTNode *ast, ScopeNode *scope, ScopeNode *global);

void print_scope_tree(ScopeNode *node, int depth, char *finals);

ScopeNode* construct_symbol_table(ASTNode* ast_root, ScopeType scope);
#endif //COMPILER_SYMBOLTABLEMANAGER_H
