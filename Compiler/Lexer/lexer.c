//
// Created by Nimrod on 29/01/2024.
//

#include "lexer.h"
#include <string.h>

Lexer init_lexer(char* file_path)
{  // create a new lexer instance
    Lexer lexer = malloc(sizeof(LexerStruct));
    if (lexer == NULL)
    {
        report_error(ERR_INTERNAL, -1, "Failed to allocate lexer");
        exit(EXIT_FAILURE);
    }
    lexer->dfa = create_DFA();

    init_DFA(lexer->dfa);  // initialize the DFA lookup table
    lexer->file = fopen(file_path, "r");
    if (lexer->file == NULL) {
        report_error(ERR_INTERNAL, -1, "Failed to open file");
        free(lexer); // Free the previously allocated memory
        exit(EXIT_FAILURE);
    }

    lexer->tokens = NULL;

    lexer->num_of_tokens=0;
    lexer->tokens_allocated_size=0;
    lexer->curr_line=0;

    return lexer;
}

void skip_whitespaces(Lexer lexer)
{
    int c;
    c = fgetc(lexer->file);
    while(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f')
    {
        if (c=='\n')
        {lexer->curr_line++;}
        c = fgetc(lexer->file);
    }
    ungetc(c, lexer->file);
}

void add_token(Lexer lexer, Token token)
{
    if (lexer->num_of_tokens+1 > lexer->tokens_allocated_size)
    {
        if(lexer->tokens_allocated_size==0)
        {lexer->tokens_allocated_size=1;}
        void* temp_ptr = realloc(lexer->tokens, (lexer->tokens_allocated_size*2)*sizeof(Token));
        if (!temp_ptr)
        { report_error(ERR_INTERNAL, lexer->curr_line, "failed to allocate more *token space");
            exit(EXIT_FAILURE);}
        lexer->tokens_allocated_size*=2;
        lexer->tokens = temp_ptr;
    }
    lexer->tokens[lexer->num_of_tokens] = token;
    lexer->num_of_tokens++;
}

Token get_next_token(Lexer lexer)
{
    memset(lexer->token_buffer, 0, sizeof(lexer->token_buffer));
    int lexeme_length=0;
    int c;
    StatesInt temp_state;
    StatesInt state = 1;
    c = fgetc(lexer->file);
    while (c != EOF && lexeme_length<TOKEN_MAXSIZE)
    {
        temp_state = lexer->dfa->transition_matrix[state][c];
        switch (temp_state) {
            case 0:
            {
                lexer->token_buffer[lexeme_length++] = (char)c;
                report_error(ERR_LEXICAL, lexer->curr_line, lexer->token_buffer);
                return init_token(TOKEN_ERROR, lexer->token_buffer);
            }
            case DELIMITER_STATE: {
                ungetc(c, lexer->file);
                return init_token(lexer->dfa->states[state], lexer->token_buffer);

            }
        }
        state = temp_state;
        lexer->token_buffer[lexeme_length] = (char)c;
        lexeme_length++;
        c = fgetc(lexer->file);
    }
    if(state==START_STATE)
    {return init_token(TOKEN_EOF, "");}
    return init_token(lexer->dfa->states[state], lexer->token_buffer);
}

void print_tokens(Lexer lexer)
{
    int i;
    for(i=0; i<lexer->num_of_tokens; i++)
    {
        printf("%s(%s), ", get_token_name(lexer->tokens[i]->type), lexer->tokens[i]->lexeme);
    }
}

void tokenize(Lexer lexer)
{
    while (1)
    {
        Token next_token;

        skip_whitespaces(lexer);
        next_token = get_next_token(lexer);

        if(next_token->type == 0)
        {
            report_error(ERR_LEXICAL, lexer->curr_line, "Invalid Token/Unexpected character");
            exit(1);
        }
        add_token(lexer, next_token);
        if(next_token->type == TOKEN_EOF)
        { return; }

    }
}
