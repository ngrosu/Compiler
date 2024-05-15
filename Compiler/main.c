#include <stdio.h>
#include "Lexer/dfa_table.h"
#include "Lexer/lexer.h"
#include "Shared/token.h"
#include "Shared/SymbolTableManager.h"
#include "Parser/ParserGenerator/DataStructures.h"
#include "Parser/Parser.h"
#include "Parser/ParserGenerator/Functions.h"
#include "SemanticAnalyzer/Semantic.h"
#include "CodeGenerator/code_gen.h"


int main(int argc, char **argv)
{
    char* file_name;
    if(argc != 2)
    {printf("Please make sure to provide the source code name");
    return 1;}
    else
    {
        file_name = argv[1];
    }
    Lexer lexer = init_lexer(file_name);
    int result = (int)tokenize(lexer);
    if(result == 0)
    {
        printf("\nCompiler terminated after lexing due to lexical error\n");
        exit(1);
    }
    printf("Passed lexer");


    Parser parser = init_parser(lexer->tokens, lexer->num_of_tokens,
                                convert_AVL_to_array(init_grammar()), 1, 1, "action.b", "goto.b");
    result = parse(parser);
    if(result == 0)
    {
        printf("\nCompiler terminated after parsing due to syntax error\n");
        exit(1);
    }
    printf("Passed parser");
    pop(parser->stack); // remove the state that is above the AST node on the stack

    ScopeNode* global = init_scope_node(SCOPE_GLOBAL, TOKEN_ERROR, 0);
    result = construct_symbol_table_rec((ASTNode *) (parser->stack->content->data), global);
    if (result == 0)
    {
        printf("\nCompiler terminated during semantic analysis due to semantic error\n");
        exit(1);
    }

    result = analyze_statements(parser->stack->content->data, global, 0);
    if (result == 0)
    {
        printf("\nCompiler terminated after semantic analysis due to semantic error\n");
        exit(1);
    }
    printf("Passed semantic");
    generate_code(init_code_gen(), global, (ASTNode *) (parser->stack->content->data));
    return 0;
}
