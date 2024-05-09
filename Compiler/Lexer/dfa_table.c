//
// Created by Nimrod on 29/01/2024.
//

#include "dfa_table.h"
#include <stdio.h>
#include <string.h>


DFA create_DFA()
{
    DFA dfa = malloc(sizeof(DFAStruct));
    dfa->num_of_states=1; // default size
    dfa->states=calloc(dfa->num_of_states,sizeof(TokenType));
    dfa->transition_matrix=calloc(dfa->num_of_states, sizeof(StatesInt) * NUM_OF_CHARS);
    return dfa;
}

void init_DFA(DFA dfa)
{
    int i;
    const char* stddelimiter = " /:;,\n(){}[]=-+<>*!^~&|";

    // state 0 is error
    // state 1 is the starting state
    // state 2 is a token delimiter state
    allocate_states(dfa, 5);

    // identifier
    set_state_alnums_to_identifier(dfa, START_STATE);
    set_state_alnums_to_identifier(dfa, IDENTIFIER_STATE);
    set_state(dfa, IDENTIFIER_STATE, TOKEN_IDENTIFIER);
    add_delimiters(dfa, IDENTIFIER_STATE, stddelimiter);

    // int literal
    for(i='0'; i<='9'; i++)
    {
        set_transition(dfa, (char)i, START_STATE, NUMBER_STATE);
        set_transition(dfa, (char)i, NUMBER_STATE, NUMBER_STATE);
    }
    set_state(dfa, 4, TOKEN_INT_LITERAL);
    add_delimiters(dfa, 4, stddelimiter);

    add_symbol_token_to_DFA(dfa, "/", TOKEN_F_SLASH, START_STATE);
    add_token_to_DFA(dfa, "/", TOKEN_COMMENT, dfa->num_of_states-1, "\n");
    for(i=0; i<NUM_OF_CHARS; i++)
    {
        if(i!='\n') {set_transition(dfa, (char)i, dfa->num_of_states-1, dfa->num_of_states-1);}
    }

    add_token_to_DFA(dfa, "\"\n", TOKEN_STRING_LITERAL, START_STATE, "");
    for(i=0; i<NUM_OF_CHARS; i++)
    {
        if(i!='\n') {set_transition(dfa, (char)i, dfa->num_of_states-2, dfa->num_of_states-2);}
    }
    set_transition(dfa, '\"', dfa->num_of_states-2, dfa->num_of_states-1);
    add_all_delimiters(dfa, dfa->num_of_states-1);

    // char literal
    add_token_to_DFA(dfa, "\'\'\'", TOKEN_CHAR_LITERAL, START_STATE, stddelimiter);
    for(i=0; i<NUM_OF_CHARS; i++)
    {
        if(i!='\n') {set_transition(dfa, (char)i, dfa->num_of_states-3, dfa->num_of_states-2);}
    }


    // symbols
    add_symbol_token_to_DFA(dfa, "(", TOKEN_L_PAREN, START_STATE);
    add_symbol_token_to_DFA(dfa, ")", TOKEN_R_PAREN, START_STATE);
    add_symbol_token_to_DFA(dfa, "{", TOKEN_L_CURLY_B, START_STATE);
    add_symbol_token_to_DFA(dfa, "}", TOKEN_R_CURLY_B, START_STATE);
    add_symbol_token_to_DFA(dfa, ";", TOKEN_SEMICOLON, START_STATE);
    add_symbol_token_to_DFA(dfa, ":", TOKEN_COLON, START_STATE);
    add_symbol_token_to_DFA(dfa, ",", TOKEN_COMMA, START_STATE);
    add_symbol_token_to_DFA(dfa, "=", TOKEN_EQUAL, START_STATE);
    add_symbol_token_to_DFA(dfa, "=", TOKEN_EQUAL_EQUAL_OP, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "+", TOKEN_PLUS_OP, START_STATE);
    add_symbol_token_to_DFA(dfa, "-", TOKEN_MINUS_OP, START_STATE);
    add_symbol_token_to_DFA(dfa, "*", TOKEN_ASTERISK, START_STATE);
    // f_slash is next to comment
    add_symbol_token_to_DFA(dfa, ">", TOKEN_GREATER_THAN_OP, START_STATE);
    add_symbol_token_to_DFA(dfa, "=", TOKEN_GREATER_THAN_EQUAL_OP, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "<", TOKEN_LESS_THAN_OP, START_STATE);
    add_symbol_token_to_DFA(dfa, "=", TOKEN_LESS_THAN_EQUAL_OP, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "[", TOKEN_L_BRACKET, START_STATE);
    add_symbol_token_to_DFA(dfa, "]", TOKEN_R_BRACKET, START_STATE);
    add_symbol_token_to_DFA(dfa, "&", TOKEN_BITWISE_AND, START_STATE);
    add_symbol_token_to_DFA(dfa, "&", TOKEN_AND, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "|", TOKEN_BITWISE_OR, START_STATE);
    add_symbol_token_to_DFA(dfa, "|", TOKEN_OR, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "!", TOKEN_NOT, START_STATE);
    add_symbol_token_to_DFA(dfa, "=", TOKEN_NOT_EQUAL_OP, dfa->num_of_states-1);
    add_symbol_token_to_DFA(dfa, "~", TOKEN_BITWISE_NOT, START_STATE);
    add_symbol_token_to_DFA(dfa, "^", TOKEN_BITWISE_XOR, START_STATE);





    // starting with b
    // add_alnum_token_to_DFA(dfa, "break", TOKEN_BREAK, START_STATE, stddelimiter);


    // starting with i
    add_alnum_token_to_DFA(dfa, "if", TOKEN_IF, START_STATE, stddelimiter);
    add_alnum_token_to_DFA(dfa, "nt", TOKEN_INT, dfa->num_of_states-2, stddelimiter);
    // starting with I
    add_alnum_token_to_DFA(dfa, "INPUT", TOKEN_INPUT, START_STATE, stddelimiter);

    // starting with r
    add_alnum_token_to_DFA(dfa, "return", TOKEN_RETURN, START_STATE, stddelimiter);

    // starting with v
    add_alnum_token_to_DFA(dfa, "void", TOKEN_VOID, START_STATE, stddelimiter);

    // starting with c
    add_alnum_token_to_DFA(dfa, "char", TOKEN_CHAR, START_STATE, stddelimiter);

    // starting with e
    add_alnum_token_to_DFA(dfa, "else", TOKEN_ELSE, START_STATE, stddelimiter);

    // starting l
    add_alnum_token_to_DFA(dfa, "short", TOKEN_SHORT, START_STATE, stddelimiter);

    // starting with w
    add_alnum_token_to_DFA(dfa, "while", TOKEN_WHILE, START_STATE, stddelimiter);

    // starting with f
    add_alnum_token_to_DFA(dfa, "for", TOKEN_FOR, START_STATE, stddelimiter);

    // starting with o
    add_alnum_token_to_DFA(dfa, "OUTPUT", TOKEN_OUTPUT, START_STATE, stddelimiter);

}

bool allocate_states(DFA dfa, StatesInt new_amount)
{
    void* temp;
    if (dfa->num_of_states<new_amount) // check that you are adding MORE states
    {

        temp = realloc(dfa->states, (new_amount)*sizeof(TokenType)); // re-alloc states array
        if (!temp)  // check for successful allocation
        { return false; }
        dfa->states = temp;
        memset(dfa->states+dfa->num_of_states, 0,  // set all the newly allocated memory to 0
               (new_amount-dfa->num_of_states)*(sizeof(TokenType)));


        temp = realloc(dfa->transition_matrix, new_amount *
                                    NUM_OF_CHARS * sizeof(dfa->transition_matrix[0][0]));
        if (!temp)
        { return false; }
        dfa->transition_matrix = temp;
        memset(dfa->transition_matrix[dfa->num_of_states], 0, // set all the newly allocated memory to 0
               (new_amount-dfa->num_of_states)*NUM_OF_CHARS*(sizeof(StatesInt)));

        dfa->num_of_states=new_amount; // update the amount of states in the DFA
        return true;
    }
    return true;

}

void set_transition(DFA dfa, char input, StatesInt curr_state, StatesInt dest_state)
{
    dfa->transition_matrix[curr_state][input] = dest_state;
}



void set_state_alnums_to_identifier(DFA dfa, StatesInt state)
{
    if (state>=dfa->num_of_states)
    {
        printf("\033[0;31m"); //Set the text to the color red
        printf("----WARNING----\nTRIED TO SET ALNUM TO IDENTIFIER FROM A NONEXISTENT STATE %d\n----WARNING----\n",
               state);
        printf("\033[0m"); //Resets the text to default color
        return;}  //
    StatesInt* state_ptr = dfa->transition_matrix[state]; // To avoid unnecessary ptr dereferences
    int i;
    // iterate over each alnum character
    // hardcoded- alnums will never change.
    for (i='0'; i<='9';i++)
    {
        state_ptr[i] = IDENTIFIER_STATE;
    }
    for (i='A'; i<='Z'; i++)
    {
        state_ptr[i] = IDENTIFIER_STATE;
    }
    for (i='a'; i<='z'; i++)
    {
        state_ptr[i] = IDENTIFIER_STATE;
    }
    state_ptr['_'] = IDENTIFIER_STATE;
}

void set_state(DFA dfa, StatesInt state, TokenType token_type)
{
    dfa->states[state] = token_type;
}

void add_token_to_DFA(DFA dfa, char* token, TokenType token_type, StatesInt start, const char* delimiters)
{
    StatesInt curr_state = dfa->num_of_states; // save the current number of initialized states
    if (!allocate_states(dfa, dfa->num_of_states+ strlen(token)))
    {   // Check that the memory allocated properly.
        printf("Failed to allocate memory for token %s, state %d", token, dfa->num_of_states);
        exit(0);
    }
    set_transition(dfa, *token, start, curr_state);
    token++; // set a transition from the chosen start state to the first uninitialized state and advance the token
    while (*token)
    {
        set_transition(dfa, *token, curr_state, curr_state+1);
        // set a transition from the first uninitialized state to the next one
        token++;
        curr_state++;
    }
    set_state(dfa, curr_state, token_type); // mark the final state added as an accepting state for the token type
    add_delimiters(dfa, curr_state, delimiters);
}

void add_alnum_token_to_DFA(DFA dfa, char* token, TokenType token_type, StatesInt start, const char* delimiters)
{
    StatesInt curr_state = dfa->num_of_states; // save the current number of initialized states

    if (!allocate_states(dfa, dfa->num_of_states+ strlen(token)))
    {   // Check that the memory allocated properly.
        printf("Failed to allocate memory for token %s, state %d", token, dfa->num_of_states);
        exit(0);
    }
    set_transition(dfa, *token, start, curr_state);
    token++; // set a transition from the chosen start state to the first uninitialized state and advance the token
    while (*token)
    {
        set_state_alnums_to_identifier(dfa, curr_state); // set transitions alnum->identifier for the state
        add_delimiters(dfa, curr_state, delimiters);
        set_transition(dfa, *token, curr_state, curr_state+1);
        set_state(dfa, curr_state, TOKEN_IDENTIFIER);
        // set a transition from the first uninitialized state to the next one
        token++;
        curr_state++;
    }
    set_state_alnums_to_identifier(dfa, curr_state); // add transitions alnum->identifier from the accepting state
    set_state(dfa, curr_state, token_type); // mark the accepting state as the appropriate token
    add_delimiters(dfa, curr_state, delimiters);
}

void add_symbol_token_to_DFA(DFA dfa, char* token, TokenType token_type, StatesInt start)
{
    add_token_to_DFA(dfa, token, token_type, start, "");
    add_all_delimiters(dfa, dfa->num_of_states-1);
}

void add_delimiters(DFA dfa, StatesInt state, const char *delimiters)
{
    while (*delimiters)
    {
        set_transition(dfa, *delimiters, state, DELIMITER_STATE);
        delimiters++;
    }
}

void add_all_delimiters(DFA dfa, StatesInt state)
{
    StatesInt* state_ptr = dfa->transition_matrix[state]; // To avoid unnecessary ptr dereferences
    int i;
    for (i=0; i<=128;i++)
    {
        state_ptr[i] = DELIMITER_STATE;
    }
}

void print_transition_matrix(DFA dfa)
{
    int i; int j;
    printf("   |");
    for (i='!'; i<NUM_OF_CHARS-1; i++)
    { printf("%3c", i); }
    printf("\n");
    for (i='!'; i<NUM_OF_CHARS-1; i++)
    { printf("___"); }
    printf("\n");
    for(i=0; i<dfa->num_of_states; i++)
    {
        printf("%3d|", i);
        for (j='!'; j<NUM_OF_CHARS-1; j++)
        {
            printf("%3d",dfa->transition_matrix[i][j]);
        }
        printf("\n");
    }
}

