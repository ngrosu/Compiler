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
        report_error(ERR_INTERNAL, -1, "Failed to allocate lexer", NULL);
        exit(EXIT_FAILURE);
    }
    lexer->dfa = create_DFA();

    init_DFA(lexer->dfa);  // initialize the DFA lookup table
    lexer->file = fopen(file_path, "r");
    if (lexer->file == NULL) {
        report_error(ERR_INTERNAL, -1, "Failed to open file", NULL);
        free(lexer); // Free the previously allocated memory
        exit(EXIT_FAILURE);
    }

    lexer->tokens = NULL;

    lexer->num_of_tokens=0;
    lexer->tokens_allocated_size=0;
    lexer->curr_line=1;

    return lexer;
}

void skip_whitespaces(Lexer lexer)
{
    int c;
    c = fgetc(lexer->file); // check c against all whitespace characters
    while(c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\v' || c == '\f')
    {
        if (c=='\n') // check if we moved down a line
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
        {
            report_error(ERR_INTERNAL, lexer->curr_line, "failed to allocate more *token space", NULL);
            exit(EXIT_FAILURE);}
        lexer->tokens_allocated_size*=2;
        lexer->tokens = temp_ptr;
    }
    lexer->tokens[lexer->num_of_tokens] = token;
    lexer->num_of_tokens++;
}

Token get_next_token(Lexer lexer)
{
    memset(lexer->token_buffer, 0, sizeof(lexer->token_buffer)); // reset the lexer token buffer
    int lexeme_length=0;
    int chr;
    StatesInt temp_state;
    StatesInt state = 1;
    chr = fgetc(lexer->file); // get the next character
    while (chr != EOF && lexeme_length < TOKEN_MAXSIZE) //
    {
        temp_state = lexer->dfa->transition_matrix[state][chr];
        switch (temp_state) {
            case 0:
            {
                lexer->token_buffer[lexeme_length++] = (char)chr;
                while(!(chr == ' ' || chr == '\n' || chr == '\t' || chr == '\r' || chr == '\v' || chr == '\f'))
                {
                    chr = fgetc(lexer->file);
                    lexer->token_buffer[lexeme_length] = (char)chr;
                    lexeme_length++;
                }
                ungetc(chr, lexer->file);
                lexer->token_buffer[--lexeme_length] = '\0';
                report_error(ERR_LEXICAL, lexer->curr_line, lexer->token_buffer, NULL);
                return init_token(TOKEN_ERROR, lexer->token_buffer, lexer->curr_line);
            }
            case DELIMITER_STATE: {
                ungetc(chr, lexer->file);
                return init_token(lexer->dfa->states[state], lexer->token_buffer, lexer->curr_line);

            }

        }
        state = temp_state;
        lexer->token_buffer[lexeme_length] = (char)chr;
        lexeme_length++;
        chr = fgetc(lexer->file);
    }
    if(state==START_STATE)
    {return init_token(TOKEN_EOF, "", lexer->curr_line);}
    return init_token(lexer->dfa->states[state], lexer->token_buffer, lexer->curr_line);
}

void print_tokens(Lexer lexer)
{
    int i;
    Token token;
    for(i=0; i<lexer->num_of_tokens; i++)
    {
        token = lexer->tokens[i];  // if a token is ID, INT_LIT, or comment, print its contents as well
        if (token->type==TOKEN_IDENTIFIER ||
        token->type == TOKEN_INT_LITERAL ||
        token->type == TOKEN_COMMENT ||
        token->type == TOKEN_STRING_LITERAL )
        {
            printf("%s(%s), ", get_token_name(lexer->tokens[i]->type), lexer->tokens[i]->lexeme);
        }
        else {
            printf("%s, ", get_token_name(lexer->tokens[i]->type));
        }
    }
}

char tokenize(Lexer lexer)
{
    char error = 0;
    char run=1;
    while (run)
    {
        Token next_token;

        skip_whitespaces(lexer);  // get to the start of the next token
        next_token = get_next_token(lexer); // get the token

        if(next_token->type == 0) // if it's a token error, report an error
        {
            report_error(ERR_LEXICAL, lexer->curr_line, "Invalid Token/Unexpected character", NULL);
            error = 1; // exit_lexer()
        }
        add_token(lexer, next_token); // add the token to the lexer
        if(next_token->type == TOKEN_EOF)
        { run = 0; } // in the case of an EOF token, stop reading for more tokens

    }
    return (!error);
}
