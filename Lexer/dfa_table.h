//
// Created by Nimrod on 29/01/2024.
//

#include "lexer.h"

#ifndef COMPILER_DFA_TABLE_H
#define COMPILER_DFA_TABLE_H
#define DFA_MAX_STATES 256
#define NUM_OF_CHARS 128

typedef struct {

    token_type states[DFA_MAX_STATES];
    short (*transition_matrix)[NUM_OF_CHARS];
    short num_of_transitions;
    short current_state;

} *DFA;

DFA create_DFA(); // Create and return an instance of the state machine

void init_DFA(DFA); // Initialize the state machine

void add_transition(DFA, char, short); // Add a transition to the transition table

void add_token_to_dfa(DFA, char[]); // Set up the transitions for a token in the transition table

void set_state(DFA, short, );

void set_state_alnum_to_identifier(DFA, char);


#endif //COMPILER_DFA_TABLE_H
