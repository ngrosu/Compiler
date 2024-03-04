#include <stdio.h>
#include "Lexer/dfa_table.h"
#include "Lexer/lexer.h"
#include "Shared/token.h"
#include "Parser/ParserGenerator/DataStructures.h"
#include "Parser/ParserGenerator/Functions.h"

int main()
{
//    Lexer lexer = init_lexer("../../test.chad");
//    tokenize(lexer);
//    print_tokens(lexer);

    AVLNode* root = NULL;
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_START_TAG, TOKEN_COUNT+SYMBOL_START, 0));
    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_COUNT+SYMBOL_EXPRESSION};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_START, arr1, 2, 0));
    int arr2[MAX_RULE_SIZE] = {TOKEN_IDENTIFIER, TOKEN_COUNT+SYMBOL_EXPRESSION};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr2, 2, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_INT_LITERAL, 0));


//    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_PLUS_OP, TOKEN_COUNT+SYMBOL_TERM};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr1, 3, 0));
//
//    int arr2[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_ASTERISK, TOKEN_COUNT+SYMBOL_FACTOR};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, arr2, 3, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TERM, TOKEN_COUNT+SYMBOL_FACTOR, 0));
//
//    int arr3[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, arr3, 3, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_INT_LITERAL, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_IDENTIFIER, 0));
    intDynArrPtr *test = calculate_firsts(root, TOKEN_COUNT+SYMBOL_COUNT-1);
//
    int temp[1];
    temp[0] = TOKEN_COUNT+SYMBOL_START;
    AVLNode* clos = closure(root, insert(NULL, init_LR1_item(TOKEN_COUNT+SYMBOL_START_TAG, temp, 1, 0, TOKEN_EOF)),
                            test);

//    intDynArrPtr *test2 = calculate_follows(root, TOKEN_COUNT+SYMBOL_COUNT-1, test);
    for(int j=TOKEN_COUNT; j<TOKEN_COUNT+SYMBOL_COUNT-1; j++)
    {
        if(test[j]->array_size!=0) {
            printf("%s: ", get_symbol_name(j));
            for (int i = 0; i < test[j]->array_size; i++) {
                printf("%s, ", get_symbol_name(test[j]->array[i]));
            }
            printf("\n");
        }
    }

    in_order(clos);
}