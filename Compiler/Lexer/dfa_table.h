//
// Created by Nimrod on 29/01/2024.
//

#include "lexer.h"
#include <stdlib.h>

#ifndef COMPILER_DFA_TABLE_H
#define COMPILER_DFA_TABLE_H
#define NUM_OF_CHARS 128

typedef struct {

    token_type *states;
    short (*transition_matrix)[NUM_OF_CHARS];
    short num_of_states;
    short current_state;

} *DFA, _DFA;

DFA create_DFA(); // Create and return an instance of the state machine

void init_DFA(DFA); // Initialize the state machine

void allocate_states(DFA, short)  // Allocate the memory required to the

void add_transition(DFA, char, short, short); // Add a transition to the transition table

void add_token_to_DFA(DFA, char[]); // Set up the transitions for a token in the transition table

void set_state(DFA, short, token_type); // Set a state in the states array to the appropriate token

void set_state_alnum_to_identifier(DFA, char); // Set transitions to all alnum characters
// except for one to the identifier state


#endif //COMPILER_DFA_TABLE_H
