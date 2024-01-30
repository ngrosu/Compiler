//
// Created by Nimrod on 29/01/2024.
//

#include "dfa_table.h"


DFA create_DFA()
{
    DFA dfa = malloc(sizeof(_DFA));
    dfa->num_of_states=1;
    dfa->states=calloc(dfa->num_of_states,sizeof(token_type));
    dfa->transition_matrix=calloc(dfa->num_of_states,sizeof(short)*NUM_OF_CHARS);
    return dfa;
}

void init_DFA(DFA dfa)
{
}

void allocate_states(DFA dfa, short state_pos)
{

}

void add_transition(DFA dfa, char input, short state, short transition)
{
    if (!(dfa->num_of_states>state && dfa->num_of_states>transition))
    {
        dfa->num_of_states = (state>transition ? state : transition);
        dfa->states = realloc(dfa->states, (dfa->num_of_states+1)*sizeof(token_type));
        dfa->transition_matrix = realloc(dfa->transition_matrix,
                                         (dfa->num_of_states+1)*sizeof(short)*NUM_OF_CHARS)
    }
}
