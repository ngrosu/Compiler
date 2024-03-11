//
// Created by Nimrod on 05/03/2024.
//

#include "Parser.h"
#include<unistd.h>


ASTNode *init_AST_node(int type, ASTNode **children, int num_of_children, Token token)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    if(node==NULL) { report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE AST node MEMORY", NULL); return NULL;}
    node->type = type;
    node->children = children;
    node->num_of_children = num_of_children;
    node->token=token;
    return node;
}

//int get_AST_height()

void printAST(ASTNode *node, int depth, char *finals)
{
    if(node==NULL) // null guard
    {return;}
    int i;

    for(i = 0; i < depth-1; i++) // iterate 0->depth-1, if the last item placed in that depth has more brothers,
    {                            // checked by !finals[i], print out 7 spaces and then a line
        if(!finals[i])
            printf("%*s", 7, "│");
        else                    // else print out seven spaces and then another space in place of the line
            printf("%*s", 7, " ");
    }
    if(depth != 0)  // make sure the node isn't the base node
    {
        // check for depth-1 (which is unchecked by the for loop
        if(!finals[depth-1]) // if the last item in that depth has more brothers, print out a line with a
        { printf("%*s", 7, "├"); } // split on the right side
        else { printf("%*s", 7, "└"); } // else, print out a corner-y symbol that connects to the top and the right

        printf("───");  // afterward, print out sideways connection lines for the current node's symbol
    }
    if(node->type < TOKEN_COUNT)  // if terminal, print out the token name
    {
        printf("%s", get_symbol_name(node->type));
        switch (node->type)  // for specific token types, print out the lexeme
        {
            case TOKEN_IDENTIFIER:
            case TOKEN_INT_LITERAL:
            case TOKEN_STRING_LITERAL:
                printf(": %s", node->token->lexeme);
            default:
                break;
        }
        printf("\n");
    }
    else
    {  // if non-terminal, print out the symbol name,
        printf(" %s\n", get_symbol_name(node->type));
        // mark the current depth as unfinished
        finals[depth] = 0;
        for(int j = 0; j < node->num_of_children-1; j++)  // call the function for all except for last of the children
        {
            printAST(node->children[j], depth + 1, finals);
        }
        finals[depth] = 1;  // set the current depth to 1 then call the function with the last child
        printAST(node->children[node->num_of_children - 1], depth + 1, finals);
    }
    //├─│
}

Parser
init_parser(Token *tokens, unsigned int num_of_tokens, genDynArrPtr grammar, char load, char save, char *action_fp,
            char *goto_fp)
{
    Parser parser = malloc(sizeof(parserStruct));
    parser->tokens = tokens;
    parser->num_of_tokens = num_of_tokens;
    parser->grammar = grammar;
    generate_parse_tables(&(parser->action_table), &(parser->goto_table), &parser->num_of_states, load, save, action_fp, goto_fp);
    parser->stack = init_stack();
    return parser;
}

void parse(Parser parser)
{
    char run = 1;
    char error = 0;

    Actions action;
    unsigned int num;
    ProdRule rule;
    unsigned int current_state;
    ASTNode** children = NULL;

    push_int(parser->stack, 0);
    while(run) // loop until condition set otherwise
    {
        unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data); // get the top value of the stack
        // which is the current state
        action = parser->action_table[stack_peek][(*parser->tokens)->type*2]; // check the action
        num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the action value
        switch (action)
        {
            case ACTION_ERROR: // if the action is error, report it and stop the loop
                report_error(ERR_SYNTAX, (*parser->tokens)->line,"Unexpected token | " ,
                             get_symbol_name((*parser->tokens)->type));
                error = 1;
                run=0;
                break;
            case ACTION_SHIFT: // if the action is shift, push the input token onto the stack as a node of the AST
                push(parser->stack, init_AST_node((*parser->tokens)->type, NULL, 0, *parser->tokens));
                parser->tokens++; // advance to the next input token
                push_int(parser->stack, num); // push the action value (state) onto the top of the stack
                break;
            case ACTION_REDUCE:  // if the action is reduce get the rule from index action value,
                rule = ((ProdRule*)parser->grammar->array)[num];  // and save it for easy access
                children = malloc(rule->bodySize * (sizeof(ASTNode*))); // malloc for the AST node children
                for(int i = rule->bodySize-1; i >= 0; i--)
                {   // iterate in reverse over the indices of the AST node children
                    free(pop(parser->stack));  // for each index, pop and free the top of the stack (current state)
                    children[i] = (ASTNode*)pop(parser->stack);  // afterward, pop and add to children
                    // this is done since the stack is pushed onto in pairs, with the state on top and the node under
                }
                current_state = *(unsigned int*)(parser->stack->content->data); // save the current state
                push(parser->stack, init_AST_node(rule->head,// push the reduced node onto the stack
                                                  children,rule->bodySize, NULL));
                current_state=parser->goto_table[current_state][rule->head-TOKEN_COUNT]; // update state with goto_table
                push_int(parser->stack, current_state); // push the new state onto the stack
                break;
            case ACTION_ACCEPT: // if action is accept, the parser is done
                run=0;
                break;
        }

    }
}

