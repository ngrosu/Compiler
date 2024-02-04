//
// Created by Nimrod on 29/01/2024.
//


#include "../definitions.h"
#include <stdlib.h>
#include <stdbool.h>

#ifndef COMPILER_DFA_TABLE_H
#define COMPILER_DFA_TABLE_H

typedef unsigned short StatesInt;

typedef struct {

    TokenType *states;
    StatesInt (*transition_matrix)[NUM_OF_CHARS];
    StatesInt num_of_states;

} *DFA, DFAStruct;

DFA create_DFA(); // Create and return an instance of the state machine

void init_DFA(DFA); // Initialize the state machine

bool allocate_states(DFA, StatesInt) ; // Allocate the memory required to the
//states array and the transition table, return false if failed, else true

void set_transition(DFA, char, StatesInt, StatesInt); // set a transition in the transition table

void add_token_to_DFA(DFA, char*, TokenType, StatesInt, const char*); // Set up the transitions for a token in the transition table
// and the state array

void add_alnum_token_to_DFA(DFA, char*, TokenType, StatesInt, const char*); // Set up the transitions for a token in the transition table
// and the state array, with progression to identifier token.

void add_symbol_token_to_DFA(DFA, char*, TokenType, StatesInt);

void set_state(DFA, StatesInt, TokenType); // Set an accepting state in the states array to the appropriate token

void set_state_alnum_to_identifier(DFA, StatesInt); // Set transitions to all alnum characters
// except for one to the identifier state

void add_delimiters(DFA, StatesInt, char*); // add transition to delimiter state

void add_all_delimiters(DFA, StatesInt);

void print_transition_matrix(DFA); // Print the transition matrix


#endif //COMPILER_DFA_TABLE_H
