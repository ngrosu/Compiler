//
// Created by Nimrod on 23/03/2024.
//

#ifndef COMPILER_SEMANTIC_H
#define COMPILER_SEMANTIC_H

#include "../Shared/SymbolTableManager.h"

typedef struct
{
    ASTNode* ast;
    ScopeNode* scope;
    ScopeNode* global;
} SemAnalyzer;

SemAnalyzer* init_sem_analyzer(ASTNode* ast);

int analyze_statements(ASTNode *ast, ScopeNode *scope, int scope_index);

int check_num_type(ASTNode *ast);

int analyze_func(ASTNode *ast, ScopeNode *scope);
int analyze_while(ASTNode *ast, ScopeNode *scope);
int analyze_for(ASTNode *ast, ScopeNode *scope);
int analyze_if(ASTNode* ast, ScopeNode* scope);
int analyze_else(ASTNode* ast, ScopeNode* scope);
int analyze_func_call(ASTNode* ast, ScopeNode* scope);
int analyze_input(ASTNode* ast, ScopeNode* scope);
int analyze_bool(ASTNode* ast, ScopeNode* scope);
int analyze_expression(ASTNode* ast, ScopeNode* scope);
int resolve_types(int type1, int type2);
int get_expression_type(ASTNode *ast, ScopeNode *scope);
int analyze_assignment(ASTNode* ast, ScopeNode* scope);
int analyze_arr_acc(ASTNode* ast, ScopeNode* scope);
int analyze_arr_dec(ASTNode* ast, ScopeNode* scope);
int analyze_var_acc(ASTNode* ast, ScopeNode* scope);
symbol_item *check_var_declared(ASTNode *ast, ScopeNode *scope);
int check_break(ASTNode* ast, ScopeNode* scope);
int check_return(ASTNode* ast, ScopeNode* scope);

#endif //COMPILER_SEMANTIC_H
