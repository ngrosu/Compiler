//
// Created by Nimrod on 29/02/2024.
//

#ifndef COMPILER_FUNCTIONS_H
#define COMPILER_FUNCTIONS_H
#include "DataStructures.h"

void first_helper(int symbol, AVLNode *node, AVLNode *root, intDynArrPtr arr, char* membership); // get a symbol to find the FIRST() of
// get an AVL tree of the production rules, a dynamic array to store the values, and an empty membership array the size
// of the amount of possible symbols

void first(int symbol, AVLNode *node, intDynArrPtr arr, short numOfSymbols); // set up a membership array for symbols
// and call first_helper with the array to find the FIRST() of the symbol

#endif //COMPILER_FUNCTIONS_H
