#include <stdio.h>
#include "Lexer/dfa_table.h"
#include "Lexer/lexer.h"
#include "Shared/token.h"
#include "Parser/ParserGenerator/DataStructures.h"
#include "Parser/ParserGenerator/Functions.h"

int main()
{
    //Lexer lexer = init_lexer("../../test.chad");
    //tokenize(lexer);
    //print_tokens(lexer);
    AVLNode* root = NULL;


    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_COUNT+SYMBOL_FACTOR, 0));

    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_PLUS_OP, TOKEN_COUNT+SYMBOL_TERM};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr1, 3, 0));

    int arr2[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_ASTERISK, TOKEN_COUNT+SYMBOL_FACTOR};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, arr2, 3, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TERM, TOKEN_COUNT+SYMBOL_FACTOR, 0));

    int arr3[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, arr3, 3, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_INT_LITERAL, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_IDENTIFIER, 0));
    intDynArrPtr test = init_array();
    first(TOKEN_COUNT+SYMBOL_EXPRESSION, root, test, TOKEN_COUNT+SYMBOL_COUNT);
    for (int i=0; i<test->array_size; i++)
    {
        printf("%s, ", get_symbol_name(test->array[i]));
    }
    printf("\n");
    pre_order(root);
}