//
// Created by Nimrod on 29/02/2024.
//

#include "Functions.h"

void first(SymbolPtr symbol, AVLNode *root, SymbolArrPtr arr)
{
    if (root==NULL)
    {return;}
    if (symbol->isTerminal)
    {
        add_to_array(arr, symbol);
    }
    else
    {
        AVLNode* temp = find_head(root, symbol);
        if (temp==NULL)
        {return;}
        ProdRule prod = temp->data;
        first(prod->body[0], root, arr);
        first(symbol, temp->right, arr);
        first(symbol, temp->left, arr);
    }
}