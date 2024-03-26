//
// Created by Nimrod on 05/03/2024.
//

#include "Parser.h"
#include<unistd.h>



ASTNode *init_AST_node(int type, ASTNode **children, int num_of_children, Token token, unsigned long line)
{
    ASTNode* node = malloc(sizeof(ASTNode));
    if(node==NULL) { report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE AST node MEMORY", NULL); return NULL;}
    node->type = type;
    node->children = children;
    node->num_of_children = num_of_children;
    node->token=token;
    if(token)
        node->start_line=token->line;
    else
        node->start_line = line;

    return node;
}

void delete_AST(ASTNode* node)
{
    if(node == NULL)
    {return;}
    for(int i = 0; i < node->num_of_children; i++)
    {
        delete_AST(node->children[i]);
    }
    if(node->num_of_children == 0)
    { delete_token(node->token);}
    free(node);
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
            printf("%*s", 8, "│");
        else                    // else print out seven spaces and then another space in place of the line
            printf("%*s", 6, " ");
    }
    if(depth != 0)  // make sure the node isn't the base node
    {
        // check for depth-1 (which is unchecked by the for loop
        if(!finals[depth-1]) // if the last item in that depth has more brothers, print out a line with a
        { printf("%*s", 8, "├"); } // split on the right side
        else { printf("%*s", 8, "└"); } // else, print out a corner-y symbol that connects to the top and the right

        printf("───");  // afterward, print out sideways connection lines for the current node's symbol
    }
    if(node->num_of_children == 0)  // if terminal, print out the token name
    {
        printf("%s", get_symbol_name(node->type));
        switch (node->type)  // for specific token types, print out the lexeme
        {
            case TOKEN_IDENTIFIER:
            case TOKEN_INT_LITERAL:
            case TOKEN_STRING_LITERAL:
            case TOKEN_CHAR_LITERAL:
                printf(": %s", node->token->lexeme);
            default:
                break;
        }
        printf(" | LINE: %lu\n", node->start_line);
    }
    else
    {  // if non-terminal, print out the symbol name,
        printf(" %s | LINE: %lu\n", get_symbol_name(node->type), node->start_line);
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
    init_AST_funcs();
    Parser parser = malloc(sizeof(parserStruct));
    parser->tokens = tokens;
    parser->num_of_tokens = num_of_tokens;
    parser->grammar = grammar;
    generate_parse_tables(&(parser->action_table), &(parser->goto_table), &parser->num_of_states, load, save, action_fp, goto_fp);
    parser->stack = init_stack();
    return parser;
}

ASTNode* binary_infix(Parser parser)
{
    ASTNode** children = malloc(2 * (sizeof(ASTNode *)));
    ASTNode* arr[MAX_RULE_SIZE];
    for(int i = 2; i >=0; i--)
    {
        free(pop(parser->stack));  // for each index, pop and free the top of the stack (state)
        arr[i] = (ASTNode *) pop(parser->stack);
    }
    children[0] = arr[0]; children[1] = arr[2];
    return init_AST_node(arr[1]->type, children, 2, arr[1]->token, arr[0]->start_line);
}
ASTNode* parentheses(Parser parser)
{
    ASTNode* result;
    ast_skip_items(parser, 1);

    free(pop(parser->stack));
    result = pop(parser->stack); // save the middle node

    ast_skip_items(parser, 1);

    return result;
}
ASTNode* direct_pass(Parser parser)
{
    free(pop(parser->stack));
    return pop(parser->stack);
}
ASTNode* ast_default(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc(rule->bodySize * (sizeof(ASTNode *))); // malloc for the AST node children
    for (int i = rule->bodySize - 1; i >= 0; i--)
    {   // iterate in reverse over the indices of the AST node children
        free(pop(parser->stack));  // for each index, pop and free the top of the stack (current state)
        children[i] = (ASTNode *) pop(parser->stack);  // afterward, pop and add to children
        // this is done since the stack is pushed onto in pairs, with the state on top and the node under
    }
    unsigned long line;
    if (children[0] == NULL)
        line = -1;
    else
        line = children[0]->start_line;
    return init_AST_node(rule->head, children, rule->bodySize, NULL, line);
}
ASTNode* ast_chain(Parser parser)
{
    ASTNode** temp;
    ASTNode* node_tail, * node_head;
    free(pop(parser->stack));
    node_tail = pop(parser->stack);
    free(pop(parser->stack));
    node_head = pop(parser->stack);
    temp = realloc(node_head->children, (node_head->num_of_children + 1) * sizeof(ASTNode*));
    if (temp != NULL) { node_head->children = temp;}
    node_head->num_of_children++;

    node_head->children[node_head->num_of_children-1] = node_tail;
    return node_head;
}
ASTNode* ast_remove_end(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-1) * (sizeof(ASTNode *))); // malloc for the AST node children
    unsigned long line = peak_ast_symbol(parser)->start_line;
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-2, rule->bodySize-1);
    if(children != NULL) {line = children[0]->start_line;}
    return init_AST_node(rule->head, children, rule->bodySize - 1, NULL, line);

}
ASTNode* ast_keep_first(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ast_skip_items(parser, rule->bodySize-1);
    free(pop(parser->stack));
    return pop(parser->stack);

}
ASTNode* ast_keep_first_indirect(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode** child = malloc(sizeof(ASTNode*));
    ast_skip_items(parser, rule->bodySize-1);
    ast_keep_items(parser, child, 0, 1);
    return init_AST_node(rule->head, child, 1, NULL, child[0]->start_line);

}
ASTNode* ast_keep_second(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];

    ast_skip_items(parser, rule->bodySize-2);
    free(pop(parser->stack));
    ASTNode* result = pop(parser->stack);
    ast_skip_items(parser, 1);
    return result;

}
ASTNode* ast_remove_edges(Parser parser) {
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-2) * (sizeof(ASTNode *))); // malloc for the AST node children
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-3, rule->bodySize-2);
    unsigned long line = peak_ast_symbol(parser)->start_line;
    ast_skip_items(parser,1);
    return init_AST_node(rule->head, children, rule->bodySize - 2, NULL, line);
}

ASTNode* remove_first2_and_second_last(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-3) * (sizeof(ASTNode *))); // malloc for the AST node children


    ast_keep_items(parser, children, rule->bodySize-4, 1);
    ast_skip_items(parser, 1);

    ast_keep_items(parser, children, rule->bodySize-5, rule->bodySize - 4);

    ast_skip_items(parser, 1);
    unsigned long line = peak_ast_symbol(parser)->start_line;
    ast_skip_items(parser, 1);
    return init_AST_node(rule->head, children, rule->bodySize - 3, NULL, line);


}

ASTNode* ast_for(Parser parser)
{
    //return ast_default((parser));
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-4) * (sizeof(ASTNode *))); // malloc for the AST node children

    ast_keep_items(parser, children, rule->bodySize - 5, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize - 6, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize - 7, 2);
    ast_skip_items(parser, 1);
    unsigned long line = peak_ast_symbol(parser)->start_line;
    ast_skip_items(parser, 1);
    return init_AST_node(rule->head, children, rule->bodySize - 4, NULL, line);

}

ASTNode* ast_char(Parser parser)
{
    free(pop(parser->stack));
    ASTNode* temp = pop(parser->stack);
    char num = temp->token->lexeme[1];
    sprintf(temp->token->lexeme, "%d", num);
    temp->type = TOKEN_INT_LITERAL;
    temp->token->type = TOKEN_INT_LITERAL;
    return temp;
}

void ast_skip_items(Parser parser, int num)
{
    for(int i =0; i<num; i++)
    {
        free(pop(parser->stack));
        ASTNode* node = pop(parser->stack);
        delete_token(node->token); free(node);
    }
}

void ast_keep_items(Parser parser, ASTNode** children, int curr_index, int num)
{
    for (int i = curr_index; i > curr_index-num; i--)
    {   // iterate in reverse over the indices of the AST node children
        free(pop(parser->stack));  // for each index, pop and free the top of the stack (current state)
        children[i] = (ASTNode *) pop(parser->stack);  // afterward, pop and add to children
        // this is done since the stack is pushed onto in pairs, with the state on top and the node under
    }

}

ASTNode* peak_ast_symbol(Parser parser)
{
    return parser->stack->content->next->data;
}

ASTNode* remove_last_and_third_to_last(Parser parser)
{

    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-2) * (sizeof(ASTNode *))); // malloc for the AST node children

    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-3, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-4, rule->bodySize-3);
    return init_AST_node(rule->head, children, rule->bodySize - 2, NULL, children[0]->start_line);
}

ASTNode* remove_second_last_chain(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode** temp;
    ASTNode* node_tail, * node_head;

    free(pop(parser->stack));
    node_tail = pop(parser->stack);

    ast_skip_items(parser, 1);

    free(pop(parser->stack));
    node_head = pop(parser->stack);

    temp = realloc(node_head->children, (node_head->num_of_children + 1) * sizeof(ASTNode*));
    if (temp != NULL) { node_head->children = temp;}
    node_head->num_of_children++;

    node_head->children[node_head->num_of_children-1] = node_tail;
    node_head->type = rule->head;
    return node_head;
}

ASTNode* remove_second_last(Parser parser)
{

    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-1) * (sizeof(ASTNode *))); // malloc for the AST node children

    ast_keep_items(parser, children, rule->bodySize-2, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-3, rule->bodySize-2);

    return init_AST_node(rule->head, children, rule->bodySize - 1, NULL, children[0]->start_line);
}

ASTNode* remove_l_3_l_4_l_6_l(Parser parser)
{

    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-4) * (sizeof(ASTNode *))); // malloc for the AST node children

    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-5, 1);
    ast_skip_items(parser, 2);
    ast_keep_items(parser, children, rule->bodySize-6, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-7,rule->bodySize-6);

    return init_AST_node(rule->head, children, rule->bodySize - 4, NULL, children[0]->start_line);
}

ASTNode* remove_l_two_and_fourth_l(Parser parser)
{

    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-3) * (sizeof(ASTNode *))); // malloc for the AST node children

    ast_skip_items(parser, 2);
    ast_keep_items(parser, children, rule->bodySize-4, 1);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, rule->bodySize-5, rule->bodySize-4);
    return init_AST_node(rule->head, children, rule->bodySize - 3, NULL, children[0]->start_line);
}

ASTNode* ast_remove_third(Parser parser)
{
    unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data);
    unsigned int num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the rule num
    ProdRule rule = ((ProdRule*)parser->grammar->array)[num];
    ASTNode **children = malloc((rule->bodySize-1) * (sizeof(ASTNode *))); // malloc for the AST node children
    ast_keep_items(parser, children, rule->bodySize-2, rule->bodySize-3);
    ast_skip_items(parser, 1);
    ast_keep_items(parser, children, 1, 2);

    return init_AST_node(rule->head, children, rule->bodySize - 1, NULL, children[0]->start_line);

}

ASTNode* (*reductionFuncs[FUNCS_COUNT])(Parser);

void init_AST_funcs()
{
    reductionFuncs[FUNC_DEFAULT] = ast_default;
    reductionFuncs[FUNC_BINARY_INFIX] = binary_infix;
    reductionFuncs[FUNC_PARENTHESES] = parentheses;
    reductionFuncs[FUNC_DIRECT_PASS] = direct_pass;
    reductionFuncs[FUNC_CHAIN] = ast_chain;
    reductionFuncs[FUNC_REMOVE_END] = ast_remove_end;
    reductionFuncs[FUNC_KEEP_FIRST] = ast_keep_first;
    reductionFuncs[FUNC_KEEP_FIRST_INDIRECT] = ast_keep_first_indirect;
    reductionFuncs[FUNC_KEEP_SECOND] = ast_keep_second;
    reductionFuncs[FUNC_REMOVE_EDGES] = ast_remove_edges;
    reductionFuncs[FUNC_REM_F_TWO_AND_SECOND_L] = remove_first2_and_second_last;
    reductionFuncs[FUNC_REM_L_AND_THIRD_L] = remove_last_and_third_to_last;
    reductionFuncs[FUNC_REM_L_THIRD_L_FOURTH_L_SIXTH_L] = remove_l_3_l_4_l_6_l;
    reductionFuncs[FUNC_FOR] = ast_for;
    reductionFuncs[FUNC_CHAR] = ast_char;
    reductionFuncs[FUNC_REM_SECOND_L] = remove_second_last;
    reductionFuncs[FUNC_REM_SECOND_L_CHAIN] = remove_second_last_chain;
    reductionFuncs[FUNC_REM_LAST_TWO_AND_FOURTH_L] = remove_l_two_and_fourth_l;
    reductionFuncs[FUNC_REMOVE_THIRD] = ast_remove_third;
}

void parse_error_recovery(Parser parser, unsigned int* s, unsigned int symbol)
{
    push(parser->stack, NULL); // push a NULL symbol onto the stack
    push_int(parser->stack, *s); // push back onto the stack the state that was last popped
    *s = parser->goto_table[*s][symbol];
    push(parser->stack, NULL); // push a NULL symbol onto the stack
    push_int(parser->stack, *s); // push the new state onto the stack from the goto table
}

char parse(Parser parser)
{
    char run = 1;
    char error = 0;
    Actions action;
    unsigned int num;
    ProdRule rule;
    unsigned int current_state;
    ASTNode* node;

    push_int(parser->stack, 0);
    while(run) // loop until condition set otherwise
    {
        unsigned int stack_peek = *(unsigned int*)(parser->stack->content->data); // get the top value of the stack
        // which is the current state
        while(parser->tokens[0]->type == TOKEN_COMMENT)
        {
            parser->tokens++;
        }
        if(parser->tokens[0]->type == TOKEN_STRING_LITERAL)
        {
            char* l = parser->tokens[0]->lexeme;
            while(l[1]!='"')
            {
                l[0] = l[1];
                l++;
            }
            *l=0;
            if(l == parser->tokens[0]->lexeme)
                *l = 0;
        }
        action = parser->action_table[stack_peek][(*parser->tokens)->type*2]; // check the action
        num = parser->action_table[stack_peek][((*parser->tokens)->type*2)+1];  // check the action value
        switch (action)
        {
            case ACTION_ERROR: // if the action is error, report it and stop the loop
                report_error(ERR_SYNTAX, (*parser->tokens)->line,"Unexpected token | " ,
                             get_symbol_name((*parser->tokens)->type));
                error = 1; // mark that an error has occurred
                unsigned int* s;
                int discard = 1;
                while(discard)  // loop for discarding stack states
                {
                    s = pop(parser->stack); // get the current state
                    delete_AST(pop(parser->stack)); // discard the symbol on the stack
                    if(parser->goto_table[*s][SYMBOL_STATEMENT])
                    {
                        discard=0;
                        parse_error_recovery(parser, s, SYMBOL_STATEMENT);
                    }
                    else if (parser->goto_table[*s][SYMBOL_SCOPE])
                    {
                        discard=0;
                        parse_error_recovery(parser, s, SYMBOL_SCOPE);
                    }
                    free(s);
                }
                discard = 1;
                while(discard) // loop for discarding input tokens
                {
                    if(*parser->tokens == NULL) // make sure the end of the input isn't reached
                    {
                        run=0;
                        discard = 0;
                    }
                    else
                    {
                        stack_peek = *(unsigned int *) (parser->stack->content->data); // get the top value of the stack
                        if (parser->action_table[stack_peek][(*parser->tokens)->type * 2] != 0)
                            // check if the next token in the input can be a valid continuation from the current state
                            discard = 0; // if it is, stop discarding
                        else
                        {
                            delete_token(*parser->tokens);
                            parser->tokens++;
                        }
                    }
                }
                break;
            case ACTION_SHIFT: // if the action is shift, push the input token onto the stack as a node of the AST
                push(parser->stack, init_AST_node((*parser->tokens)->type, NULL, 0, *parser->tokens, 0));
                parser->tokens++; // advance to the next input token
                push_int(parser->stack, num); // push the action value (state) onto the top of the stack
                break;
            case ACTION_REDUCE:  // if the action is reduce get the rule from index action value,
                rule = ((ProdRule*)parser->grammar->array)[num];  // and save it for easy access
                if(error)
                {node = reductionFuncs[FUNC_DEFAULT](parser);}
                else
                { node = reductionFuncs[rule->ASTFunc](parser); }
                current_state = *(unsigned int*)(parser->stack->content->data);
                current_state=parser->goto_table[current_state][rule->head-TOKEN_COUNT]; // update state with goto_table
                push(parser->stack, node);// push the reduced node onto the stack
                push_int(parser->stack, current_state); // push the new state onto the stack
                break;
            case ACTION_ACCEPT: // if action is accept, the parser is done
                run=0;
                break;
        }
    }
    return !error;
}

