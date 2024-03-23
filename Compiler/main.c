#include <stdio.h>
#include "Lexer/dfa_table.h"
#include "Lexer/lexer.h"
#include "Shared/token.h"
#include "Shared/SymbolTableManager.h"
#include "Parser/ParserGenerator/DataStructures.h"
#include "Parser/Parser.h"
#include "Parser/ParserGenerator/Functions.h"
#include "SemanticAnalyzer/Semantic.h"

int main1()
{
//    Lexer lexer = init_lexer("../../test.chad");
//    tokenize(lexer);
//    print_tokens(lexer);

    AVLNode *root = NULL;
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_START_TAG, TOKEN_COUNT + SYMBOL_START, 0));
//    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_COUNT + SYMBOL_EXPRESSION};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_START, arr1, 2, 0));
//    int arr2[MAX_RULE_SIZE] = {TOKEN_IDENTIFIER, TOKEN_COUNT + SYMBOL_EXPRESSION};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr2, 2, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_INT_LITERAL, 0));


    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_PLUS_OP, TOKEN_COUNT+SYMBOL_TERM};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr1, 3, 0, FUNC_DEFAULT));

    int arr2[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_ASTERISK, TOKEN_COUNT+SYMBOL_FACTOR};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, arr2, 3, 0, FUNC_DEFAULT));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TERM, TOKEN_COUNT + SYMBOL_FACTOR, 0));

    int arr3[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, arr3, 3, 0, FUNC_DEFAULT));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_INT_LITERAL, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_IDENTIFIER, 0));
    intDynArrPtr *test = calculate_firsts(root, TOKEN_COUNT + SYMBOL_COUNT - 1);
//
    int temp[1];
    temp[0] = TOKEN_COUNT + SYMBOL_START;
    AVLNode *clos = closure(root, insert(NULL, init_LR1_item(TOKEN_COUNT + SYMBOL_START_TAG, temp, 1, 0, TOKEN_EOF)),
                            test);
    AVLNode *got = goto_func(root, clos, test, TOKEN_COUNT + SYMBOL_EXPRESSION);

    //printf("%d", trees_is_equal(got, got));

    genDynArrPtr testarr = init_gen_dynamic_array();
//    printf("%d", got==NULL);
    //in_order(got);
//    add_to_gen_dyn_array(testarr, got);
//    add_to_gen_dyn_array(testarr, got);
//    printf("%d", testarr->array[0]->data->head);


//    for(int j=TOKEN_COUNT; j<TOKEN_COUNT+SYMBOL_COUNT-1; j++)
//    {
//        if(test[j]->array_size!=0) {
//            printf("%s: ", get_symbol_name(j));
//            for (int i = 0; i < test[j]->array_size; i++) {
//                printf("%s, ", get_symbol_name(test[j]->array[i]));
//            }
//            printf("\n");
//        }
//    }

    genDynArrPtr item_sets = generate_items(root);

    for (int i = 0; i < item_sets->array_size; i++)
    {
        printf("STATE %d:\n", i);
        in_order(item_sets->array[i]);
    }
//    printf("\n");
//    in_order(got);
    return 1;
}

int main()
{

    Lexer lexer = init_lexer("../../test.chad");
//    print_transition_matrix(lexer->dfa);
    if(!tokenize(lexer))
    {exit(0);}
    //in_order(init_grammar());
//    in_order(init_grammar());
    //generate_items(init_grammar());
    Parser parser = init_parser(lexer->tokens, lexer->num_of_tokens,
                                convert_AVL_to_array(init_grammar()), 1, 1, "action.b", "goto.b");
    printf("%d", parser->num_of_states);
//    print_tokens(lexer);



//    printf(" i |  c     d     $  | S C\n");
//    printf(" _________________________\n");
//    for (int i = 0; i < parser->num_of_states; i++)
//    {
//        unsigned int *action = parser->action_table[i];
//        unsigned int *GOTO = parser->goto_table[i];
//        printf(" %d:  %d %d   %d %d   %d %d | %d %d\n", i,
//               action[TOKEN_IDENTIFIER * 2], action[TOKEN_IDENTIFIER * 2 + 1],
//               action[TOKEN_INT_LITERAL * 2], action[TOKEN_INT_LITERAL * 2 + 1],
//               action[TOKEN_EOF * 2], action[TOKEN_EOF * 2 + 1],
//               GOTO[SYMBOL_START], GOTO[SYMBOL_EXPRESSION]
//        );
//    }

//    in_order((init_grammar()));
//    printf("  %d  ", parser->num_of_states);
    parse(parser);
    pop(parser->stack);
    printf("\nprinting:\n");
    char arr[50] = {0};
    ScopeNode* t = init_scope_node(SCOPE_GLOBAL, TOKEN_ERROR);
    construct_symbol_table_rec((ASTNode *) (parser->stack->content->data), t);
    print_scope_tree(t, 0, arr);
    printAST((ASTNode *) (parser->stack->content->data), 0, arr);
    analyze_statements(parser->stack->content->data, t);



}