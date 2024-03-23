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

int analyze_statements(ASTNode *ast, ScopeNode *scope);

char check_type_range(int type, unsigned long value);

int analyze_func(ASTNode *ast, ScopeNode *scope);
int analyze_while(ASTNode *ast, ScopeNode *scope);
int analyze_for(ASTNode *ast, ScopeNode *scope);
int analyze_func_call(ASTNode* ast, ScopeNode* scope);
int analyze_bool(ASTNode* ast, ScopeNode* scope);
int analyze_expression(ASTNode* ast, ScopeNode* scope);
int check_var_usage(ASTNode* ast, ScopeNode* scope);

#endif //COMPILER_SEMANTIC_H
