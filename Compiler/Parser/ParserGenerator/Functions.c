//
// Created by Nimrod on 29/02/2024.
//

#include "Functions.h"

void first_helper(int symbol, AVLNode *node, AVLNode *root, intDynArrPtr arr, char* membership)
{
    if (node == NULL)
    {return;}
    if (symbol < TOKEN_COUNT) // check whether the symbol is a terminal by checking if it's a token
    {
        if(membership[symbol]==0) // check whether the symbol already exists in the array
        {
            add_to_int_dyn_array(arr, symbol);
            membership[symbol]=1;
        }
    }
    else // if non-terminal
    {

        AVLNode* temp = find_head(node, symbol); // get the first node which has a production rule's head be the
        // symbol
        if (temp==NULL)
        {return;}
        ProdRule prod = temp->data;

        if (membership[prod->body[0]] == 0) // check if the next symbol has been explored
        {
            if (prod->body[0] >= TOKEN_COUNT)
                membership[prod->body[0]] = 1; // if it's not a terminal symbol, set it as explored
            first_helper(prod->body[0], root, root, arr, membership); // find the FIRST of the production rule
        }
        // look if other production rules have the same head
        first_helper(symbol, temp->right, root, arr, membership);
        first_helper(symbol, temp->left, root, arr, membership);
    }
}

void first(int symbol, AVLNode *node, intDynArrPtr arr, short numOfSymbols)
{
    char *membership;
    membership = calloc(numOfSymbols, sizeof(char)); // create an empty membership array
    if (membership==NULL)
    {
        report_error(ERR_INTERNAL, -1, "Memory allocation failed");
        return;
    }

    first_helper(symbol, node, node, arr, membership); // call the recursive function

    free(membership);

}
