//
// Created by Nimrod on 29/02/2024.
//

#ifndef COMPILER_FUNCTIONS_H
#define COMPILER_FUNCTIONS_H
#include "DataStructures.h"

void first_helper(int symbol, AVLNode *node, AVLNode *root, intDynArrPtr arr, char* membership); // get a symbol to find the FIRST() of
// get an AVL tree of the production rules, a dynamic array to store the values, and an empty membership array the size
// of the amount of possible symbols

void calculate_first(int symbol, AVLNode *node, intDynArrPtr arr, short numOfSymbols); // set up a membership array for symbols
// and call first_helper with the array to find the FIRST() of the symbol

//return an array of all symbols' FIRST with the index being the ID
intDynArrPtr* calculate_firsts(AVLNode *grammar, short numOfSymbols);


// add all relevant FIRST sets to the appropriate symbols' FOLLOW set in the membership array and 'track' the
// symbols at the end of rules' bodies
void follows_helper_firsts(AVLNode *curr, char* membership_arr, intDynArrPtr* first_sets, short numOfSymbols,
                           intDynArrPtr tracking);

// finish updating the membership array for any tracking symbols that were set in the first helper function
void follows_helper_tracking(char* membership_arr, short numOfSymbols, intDynArrPtr tracking);

// return an array of dynamic arrays where the index corresponds to a symbol ID
intDynArrPtr* calculate_follows(AVLNode *node, short numOfSymbols, intDynArrPtr* first_sets);

// modify the set to be CLOSURE(set)
AVLNode* closure(AVLNode* grammar, AVLNode* set, intDynArrPtr* first_sets);

AVLNode* goto_func(AVLNode* root, AVLNode* set, intDynArrPtr* first_sets, int symbol);

genDynArrPtr generate_items(AVLNode* grammar);

AVLNode* init_grammar();

void generate_parse_tables(unsigned int ***action_table, unsigned int ***goto_table, unsigned int *num_of_states);

#endif //COMPILER_FUNCTIONS_H
