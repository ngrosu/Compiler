//
// Created by Nimrod on 29/02/2024.
//

#include "Functions.h"

void first_helper(int symbol, AVLNode *node, AVLNode *root, intDynArrPtr arr, char* membership)
{
    if (node == NULL)
    {return;}
    if (symbol < TOKEN_COUNT) // check whether the symbol is a terminal by checking if it's a token
    {
        if(membership[symbol]==0) // check whether the symbol already exists in the array
        {
            add_to_int_dyn_array(arr, symbol);
            membership[symbol]=1;
        }
    }
    else // if non-terminal
    {
        AVLNode* temp = find_head(node, symbol); // get the first node which has a production rule's head be the
        // symbol
        if (temp==NULL)
        {return;}
        ProdRule prod = temp->data;

        if (membership[prod->body[0]] == 0) // check if the next symbol has been explored
        {
            if (prod->body[0] >= TOKEN_COUNT)
                membership[prod->body[0]] = 1; // if it's not a terminal symbol, set it as explored
            first_helper(prod->body[0], root, root, arr, membership); // find the FIRST of the production rule
        }
        // look if other production rules have the same head
        first_helper(symbol, temp->right, root, arr, membership);
        first_helper(symbol, temp->left, root, arr, membership);
    }
}

void calculate_first(int symbol, AVLNode *node, intDynArrPtr arr, short numOfSymbols)
{
    char *membership;
    membership = calloc(numOfSymbols, sizeof(char)); // create an empty membership array
    if (membership==NULL)
    {
        report_error(ERR_INTERNAL, -1, "Memory allocation failed", NULL);
        return;
    }

    first_helper(symbol, node, node, arr, membership); // call the recursive function

    free(membership);
}


intDynArrPtr* calculate_firsts(AVLNode *grammar, short numOfSymbols)
{
    intDynArrPtr* result = calloc(numOfSymbols, sizeof(intDynArrPtr));
    if (result==NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION", NULL);
    }
    for (int i =0; i < numOfSymbols; i++)
    {
        result[i] = init_int_dynamic_array();
        calculate_first(i, grammar, result[i], numOfSymbols);
    }
    return result;
}

// add all relevant FIRST sets to the appropriate symbols' FOLLOW set in the membership array and 'track' the
// symbols at the end of rules' bodies
void follows_helper_firsts(AVLNode *curr, char* membership_arr, intDynArrPtr* first_sets, short numOfSymbols,
                           intDynArrPtr tracking)
{
    if (curr==NULL) {return;} // null guard

    int size = curr->data->bodySize;
    int* body = curr->data->body;


    for (int i=0; i<size-1; i++) // iterate over the rule's body except for the final symbol
    {
        intDynArrPtr first_set = first_sets[body[i+1]]; // get the FIRST set of the following symbol

        // set all the symbols from the set in the membership array
        for(int j=0; j<first_set->array_size; j++)
        {
            membership_arr[body[i]*numOfSymbols+ first_set->array[j]] = 1;
        }
    }

    // save the final symbol in the body, and the head into the tracking array to properly update associations later
    add_to_int_dyn_array(tracking, body[size-1]);  // first add the final symbol in the body
    add_to_int_dyn_array(tracking, curr->data->head); // and then add the head

    // recurse all the way down the production rule AVL tree
    follows_helper_firsts(curr->right, membership_arr, first_sets, numOfSymbols, tracking);
    follows_helper_firsts(curr->left, membership_arr, first_sets, numOfSymbols, tracking);
}

// finish updating the membership array for any tracking symbols that were set in the first helper function
void follows_helper_tracking(char* membership_arr, short numOfSymbols, intDynArrPtr tracking)
{
    char changed = 1; // check if a change has occurred
    while (changed)
    {
        changed = 0;
        // iterate over the tracking array 2 items at a time (since it is built like FOLLOW[i] contains FOLLOW[i+1]
        for(int i = 0; i<tracking->array_size; i+=2)
        {
            for(int j = 0; j<numOfSymbols; j++) // iterate over all symbols
            {
                // if the symbol is set in the membership array for the 'tracked' symbol
                if(membership_arr[tracking->array[i+1]*numOfSymbols+j])
                    // if the symbol is NOT set in the membership array for the 'tracking' symbol
                    if(!membership_arr[tracking->array[i]*numOfSymbols+j])
                    {
                        // set the symbol in the membership array for the 'tracking' symbol and update the changed
                        // condition
                        membership_arr[tracking->array[i]*numOfSymbols+j] =1;
                        changed = 1;
                    }
            }
        }
    }
}

// return an array of dynamic arrays where the index corresponds to a symbol ID
intDynArrPtr* calculate_follows(AVLNode *node, short numOfSymbols, intDynArrPtr* first_sets)
{
    // potential improvement: refactor to have the 2d array have each bit represent a symbol instead of each byte
    // unnecessary due to the fact this extra memory overhead will only be present for the generation of the tables,
    // so I chose code readability and development time over memory efficiency
    char* membership_arr = calloc(numOfSymbols*numOfSymbols, sizeof(char));
    if (membership_arr==NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY", NULL);
        return NULL;
    }

    membership_arr[(TOKEN_COUNT+SYMBOL_START)*numOfSymbols+ TOKEN_EOF] = 1; // the start symbol always has an
    // endmarker in the follow

    intDynArrPtr tracking = init_int_dynamic_array(); // init the tracking array

    // add all the firsts to follow sets, except for symbols at the end of production rules' body
    follows_helper_firsts(node, membership_arr, first_sets, numOfSymbols, tracking);
    // fix the follow sets for the symbols at the end of production rules' body
    follows_helper_tracking(membership_arr, numOfSymbols, tracking);

    // set up the result based on the membership table
    intDynArrPtr* result = calloc(numOfSymbols, sizeof(intDynArrPtr));
    if (result==NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION", NULL);
    }
    for (int i = 0; i < numOfSymbols; i++)
    {
        // set up a dynamic array
        result[i] = init_int_dynamic_array();
        for(int j = 0; j<numOfSymbols; j++) // add all symbols present in the membership table to the array
        {
            if(membership_arr[i*numOfSymbols+j])
                add_to_int_dyn_array(result[i], j);
        }
    }
    // free resources
    free(membership_arr);
    delete_int_dynamic_array(tracking);
    return result;
}

// modify the set to be CLOSURE(set) and return a pointer to the grammar of the set
AVLNode* closure(AVLNode* grammar, AVLNode* set, intDynArrPtr* first_sets)
{
    TreeIterator* iter_set;
    TreeIterator* iter_root;

    intDynArrPtr first;
    ProdRule new_rule;

    char change = 1;
    ProdRule set_rule; // set production rule node
    ProdRule root_rule; // grammar production rule node
    while(change)
    {
        iter_set = init_tree_iterator(set);
        change = 0;
        while (!iterator_is_empty(iter_set))
        {
            set_rule = ((AVLNode*)iterator_next(iter_set))->data;
            // perform action on tree node

            if (set_rule->body[set_rule->dot] >= TOKEN_COUNT)
            { // if the item after the dot is a
                // non-terminal go into the inner loop
                iter_root = init_tree_iterator(grammar);
                while (!iterator_is_empty(iter_root))
                { // iterate over the grammar tree (main grammar rules)
                    root_rule = ((AVLNode*)iterator_next(iter_root))->data;

                    // check if the found prod rule's head is the non-terminal after the dot
                    if (root_rule->head == set_rule->body[set_rule->dot])
                    {
                        if (set_rule->dot == set_rule->bodySize - 1)
                        {// if dot is just before the end of the body
                            // insert the rule into the set
                            ProdRule item = init_LR1_item(root_rule->head, root_rule->body, root_rule->bodySize,
                                                          0, set_rule->lookahead);
                            if (!find(set, item))
                            {
                                change = 1;
                                set = insert(set, init_LR1_item(root_rule->head, root_rule->body, root_rule->bodySize,
                                                                0, set_rule->lookahead));
                            }
                            else {free(item);}
                        } else if (set_rule->dot < set_rule->bodySize)
                        {
                            // add the symbol after the symbol that's after the dot's first set as lookahead
                            first = first_sets[set_rule->body[set_rule->dot + 1]];
                            for (int i = 0; i < first->array_size; i++)
                            {
                                new_rule = init_LR1_item(root_rule->head, root_rule->body,
                                                         root_rule->bodySize, 0, first->array[i]);
                                if (!find(set, new_rule)) // if the rule doesn't already exist
                                {
                                    change = 1;
                                    set = insert(set, new_rule); // add the new rule to the set
                                }
                            }
                        }
                    }
                }
                free(iter_root->stack);
                free(iter_root);
            }
        } // end of inner loop
        free(iter_set->stack);
        free(iter_set);
    } // end of outer loop
        return set;
    } // end of wrapping loop (main while)



AVLNode* goto_helper(AVLNode* set, AVLNode* result, int symbol)
{
    ProdRule data;

    TreeIterator* iter = init_tree_iterator(set);
    while(!iterator_is_empty(iter))
    {
        data = ((AVLNode* )iterator_next(iter))->data;
        if (data->dot != data->bodySize) // make sure the dot is not at the end of the rule
        {
            if (data->body[data->dot] == symbol) // if the symbol after the dot matches
            {
                // advance the dot by 1 and add to the new set
                result = insert(result, init_LR1_item(data->head, data->body, data->bodySize,
                                                      (short)(data->dot+1), data->lookahead));
            }
        }
    }

//    if (set==NULL) {return result;}
//    data = set->data;
//    if (data->dot != data->bodySize) // make sure the dot is not at the end of the rule
//    {
//        if (data->body[data->dot] == symbol) // if the symbol after the dot matches
//        {
//            // advance the dot by 1 and add to the new set
//            result = insert(result, init_LR1_item(data->head, data->body, data->bodySize,
//                                                  (short)(data->dot+1), data->lookahead));
//        }
//    }
//
//    result = goto_helper(set->left, result, symbol); //repeat for entire tree
//    result = goto_helper(set->right, result, symbol);

    return result;
}

AVLNode* goto_func(AVLNode* root, AVLNode* set, intDynArrPtr* first_sets, int symbol)
{
    AVLNode* result = NULL;
    result = goto_helper(set, result, symbol); // generate the goto set
    return closure(root, result, first_sets);
}

genDynArrPtr generate_items(AVLNode* grammar)
{
    int temp;


    // starting set
    temp = TOKEN_COUNT+SYMBOL_START;
    ProdRule augmented_start = init_LR1_item(TOKEN_COUNT + SYMBOL_START_TAG, &temp, 1, 0, TOKEN_EOF);

    // calculate first sets
    intDynArrPtr* first_sets = calculate_firsts(grammar, TOKEN_COUNT+SYMBOL_COUNT-1);

    // initialize a dynamic array to store the sets
    genDynArrPtr setArr = init_gen_dynamic_array();
    AVLNode* start_set = insert(NULL, augmented_start);
    AVLNode* start_closure = closure(grammar, start_set, first_sets);
    add_to_gen_dyn_array(setArr, start_closure);

    // create items
    char change = 1;
    while(change)
    {
        change = 0;
        for (int i = 0; i < setArr->array_size; i++)
        {
            for (int symbol = 0; symbol < TOKEN_COUNT + SYMBOL_COUNT - 1; symbol++)
            {
                AVLNode *tempgoto = goto_func(grammar, setArr->array[i], first_sets, symbol);
                if (tempgoto != NULL)
                {
                    char check = 0;
                    for (int j = 0; j < setArr->array_size && !check; j++)
                    {

                        if (trees_is_equal(tempgoto, setArr->array[j]))
                        {
                            delete_tree(tempgoto, 1);
                            check = 1;
                        }
                    }
                    if (!check)
                    {
                        change=1;
                        add_to_gen_dyn_array(setArr, tempgoto);
                    }
                }
            }
        }
    }
    return setArr;
}

AVLNode* init_grammar()
{
    AVLNode* root = NULL;
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_START_TAG, TOKEN_COUNT+SYMBOL_START, 0));
    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_STATEMENTS};
    // start
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_START, arr1, 1, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_START, TOKEN_EOF, 0));

    // statement
    int arr2[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_COUNT + SYMBOL_END_STATEMENT};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_STATEMENT, arr2, 2, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_DECLARATION, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_ASSIGNMENT, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_IF, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_IF_ELSE, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_FOR, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_WHILE, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_RETURN, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_SCOPE, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT + SYMBOL_END_STATEMENT, 0));
    // statements
    arr2[0] = SYMBOL_STATEMENTS + TOKEN_COUNT; arr2[1] = SYMBOL_STATEMENT + TOKEN_COUNT;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_STATEMENTS, arr2, 2, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_STATEMENTS, TOKEN_COUNT+SYMBOL_STATEMENT, 0));



    // if statement
    int if_arr[MAX_RULE_SIZE] = {TOKEN_IF, TOKEN_L_PAREN, TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_R_PAREN, TOKEN_COUNT + SYMBOL_SCOPE};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_IF, if_arr, 5, 0));
    if_arr[0] = SYMBOL_IF + TOKEN_COUNT; if_arr[1] = TOKEN_ELSE; if_arr[2] = TOKEN_COUNT + SYMBOL_SCOPE;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_IF_ELSE, if_arr, 3, 0));

    // while statement
    int while_arr[MAX_RULE_SIZE] = {TOKEN_WHILE, TOKEN_L_PAREN, TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_R_PAREN, TOKEN_COUNT + SYMBOL_SCOPE};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_WHILE, while_arr, 5, 0));
    // for statement
    int for_arr[MAX_RULE_SIZE] = {TOKEN_FOR, TOKEN_L_PAREN, TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_COUNT + SYMBOL_END_STATEMENT, TOKEN_COUNT+SYMBOL_STATEMENT, TOKEN_R_PAREN, TOKEN_COUNT + SYMBOL_SCOPE};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FOR, for_arr, 8, 0));



    // declaration
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_DECLARATION, TOKEN_COUNT + SYMBOL_ARR_DEC, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_DECLARATION, TOKEN_COUNT + SYMBOL_VAR_DEC, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_DECLARATION, TOKEN_COUNT + SYMBOL_FUNC_DEC, 0));

    int dec_arr[MAX_RULE_SIZE] = {TOKEN_COUNT + SYMBOL_TYPE, TOKEN_IDENTIFIER, TOKEN_EQUAL, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_COUNT+SYMBOL_END_STATEMENT};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_VAR_DEC, dec_arr, 5, 0));
    dec_arr[2] = TOKEN_COUNT + SYMBOL_END_STATEMENT;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_VAR_DEC, dec_arr, 3, 0));
    dec_arr[2] = TOKEN_R_BRACKET; dec_arr[3] = TOKEN_COUNT + SYMBOL_NUMBER; dec_arr[4] = TOKEN_L_BRACKET; dec_arr[5] = TOKEN_COUNT + SYMBOL_END_STATEMENT;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_ARR_DEC, dec_arr, 6, 0));
    // assignment
    dec_arr[0] = TOKEN_IDENTIFIER; dec_arr[1] = TOKEN_EQUAL; dec_arr[2] = TOKEN_COUNT + SYMBOL_EXPRESSION; dec_arr[3] = TOKEN_COUNT + SYMBOL_END_STATEMENT;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_ASSIGNMENT, dec_arr, 4, 0));


    // function declaration
    dec_arr[0] = TOKEN_COUNT + SYMBOL_TYPE; dec_arr[1] = TOKEN_IDENTIFIER; dec_arr[2] = TOKEN_COLON; dec_arr[3] = TOKEN_COUNT + SYMBOL_PARAMETERS; dec_arr[4] = TOKEN_COUNT + SYMBOL_RET_SCOPE;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_DEC, dec_arr, 5, 0));
    dec_arr[0] = TOKEN_VOID; dec_arr[4] = TOKEN_COUNT + SYMBOL_SCOPE;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_DEC, dec_arr, 5, 0));
    dec_arr[2] = TOKEN_COUNT + SYMBOL_SCOPE;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_DEC, dec_arr, 3, 0));
    dec_arr[2] = TOKEN_COUNT + SYMBOL_RET_SCOPE; dec_arr[0] = TOKEN_COUNT + SYMBOL_TYPE;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_DEC, dec_arr, 3, 0));

//    // function calls
    int func_arr[MAX_RULE_SIZE] = {TOKEN_IDENTIFIER, TOKEN_L_PAREN, TOKEN_R_PAREN, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_CALL, func_arr, 3, 0));
    func_arr[2] = TOKEN_COUNT+SYMBOL_ARGUMENTS;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FUNC_CALL, func_arr, 4, 0));
    func_arr[0] = TOKEN_COUNT + SYMBOL_ARGUMENTS; func_arr[1] = TOKEN_COMMA;

    // scopes
    int scope_arr[MAX_RULE_SIZE] = {TOKEN_L_CURLY_B, TOKEN_COUNT + SYMBOL_STATEMENTS, TOKEN_R_CURLY_B};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_SCOPE, scope_arr, 3, 0));
    scope_arr[1] = TOKEN_R_CURLY_B;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_SCOPE, scope_arr, 2, 0));
    scope_arr[1] = TOKEN_COUNT + SYMBOL_STATEMENTS; scope_arr[2] = TOKEN_COUNT + SYMBOL_RETURN; scope_arr[3] = TOKEN_R_CURLY_B;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_RET_SCOPE, scope_arr, 4, 0));
    scope_arr[1] = TOKEN_L_CURLY_B;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_RET_SCOPE, scope_arr+1, 3, 0));
    scope_arr[0] = TOKEN_RETURN; scope_arr[1] = TOKEN_COUNT + SYMBOL_EXPRESSION; scope_arr[2] = TOKEN_COUNT + SYMBOL_END_STATEMENT;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_RETURN, scope_arr, 3, 0));





    // parameter declaration
    int par_arr[NUM_OF_CHARS] = {TOKEN_COUNT + SYMBOL_PARAMETERS, TOKEN_COMMA, TOKEN_COUNT + SYMBOL_PARAMETER};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_PARAMETERS, par_arr, 3, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_PARAMETERS, TOKEN_COUNT + SYMBOL_PARAMETER, 0));
    par_arr[0] = TOKEN_COUNT + SYMBOL_TYPE; par_arr[1] = TOKEN_IDENTIFIER;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_PARAMETER, par_arr, 2, 0));


    // expressions
    int expr_arr[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_PLUS_OP, TOKEN_COUNT+SYMBOL_TERM};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, expr_arr, 3, 0));
    expr_arr[1] = TOKEN_MINUS_OP;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, expr_arr, 3, 0));
    expr_arr[1] = TOKEN_BITWISE_OR;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, expr_arr, 3, 0));

    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION,  TOKEN_COUNT + SYMBOL_TERM, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION,  TOKEN_COUNT + SYMBOL_BOOL_EXPR, 0));

    int term_arr[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_ASTERISK, TOKEN_COUNT+SYMBOL_FACTOR};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, term_arr, 3, 0));
    term_arr[1] = TOKEN_F_SLASH;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, term_arr, 3, 0));
    term_arr[1] = TOKEN_BITWISE_AND;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, term_arr, 3, 0));
    term_arr[1] = TOKEN_BITWISE_XOR;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, term_arr, 3, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TERM, TOKEN_COUNT+SYMBOL_FACTOR, 0));

    int fact_arr[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, fact_arr, 3, 0));
    fact_arr[0] = TOKEN_BITWISE_NOT; fact_arr[1] = TOKEN_COUNT + SYMBOL_FACTOR;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, fact_arr, 2, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_COUNT + SYMBOL_NUMBER, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_IDENTIFIER, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_COUNT + SYMBOL_FUNC_CALL, 0));


    // bool expression

    int bool_arr[MAX_RULE_SIZE] = {TOKEN_COUNT + SYMBOL_BOOL_EXPR, TOKEN_OR, TOKEN_COUNT + SYMBOL_BOOL_TERM};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_EXPR, bool_arr, 3, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_EXPR, TOKEN_COUNT + SYMBOL_BOOL_TERM, 0));

    int bool_t_arr[MAX_RULE_SIZE] = {TOKEN_COUNT + SYMBOL_BOOL_TERM, TOKEN_AND, TOKEN_COUNT + SYMBOL_BOOL_FACTOR};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_TERM, bool_t_arr, 3, 0));

    int bool_f_arr[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_BOOL_EXPR, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_FACTOR, bool_f_arr, 3, 0));
    bool_f_arr[0] = TOKEN_NOT; bool_f_arr[1] = TOKEN_COUNT + SYMBOL_BOOL_FACTOR;
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_FACTOR, bool_f_arr, 2, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_BOOL_FACTOR, TOKEN_COUNT + SYMBOL_RELAT_EXPR, 0));

    int relat_arr[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_EXPR, relat_arr, 5, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_GREATER_THAN_EQUAL_OP, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_GREATER_THAN_OP, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_LESS_THAN_EQUAL_OP, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_LESS_THAN_OP, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_EQUAL_EQUAL_OP, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_RELAT_OP, TOKEN_NOT_EQUAL_OP, 0));


    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TYPE, TOKEN_INT, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TYPE, TOKEN_UINT, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TYPE, TOKEN_LONG, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TYPE, TOKEN_ULONG, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TYPE, TOKEN_CHAR, 0));

    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_NUMBER, TOKEN_CHAR_LITERAL, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_NUMBER, TOKEN_INT_LITERAL, 0));
    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_END_STATEMENT, TOKEN_SEMICOLON, 0));

//    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_START_TAG, TOKEN_COUNT+SYMBOL_START, 0));
//    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_COUNT+SYMBOL_EXPRESSION};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_START, arr1, 2, 0));
//    int arr2[MAX_RULE_SIZE] = {TOKEN_IDENTIFIER, TOKEN_COUNT+SYMBOL_EXPRESSION};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr2, 2, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_INT_LITERAL, 0));
    // Expression => Expression + Term
    // Expression => Term
    // Term => Term * Factor
    // Term => Term / Factor
    // Term => Factor
    // Factor => (Expression)
    // Factor => INTEGER


//    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_START_TAG, TOKEN_COUNT+SYMBOL_START, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT+SYMBOL_START, TOKEN_COUNT+SYMBOL_EXPRESSION, 0));
//    int arr1[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_EXPRESSION, TOKEN_PLUS_OP, TOKEN_COUNT+SYMBOL_TERM};
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION,  TOKEN_COUNT + SYMBOL_TERM, 0));
//
//    int arr2[MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_ASTERISK, TOKEN_COUNT+SYMBOL_FACTOR};
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_TERM, TOKEN_COUNT+SYMBOL_FACTOR, 0));
//    int arr4 [MAX_RULE_SIZE] = {TOKEN_COUNT+SYMBOL_TERM, TOKEN_F_SLASH, TOKEN_COUNT+SYMBOL_FACTOR};
//
//    int arr3[MAX_RULE_SIZE] = {TOKEN_L_PAREN, TOKEN_COUNT + SYMBOL_EXPRESSION, TOKEN_R_PAREN};
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_EXPRESSION, arr1, 3, 0));
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, arr2, 3, 0));
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_TERM, arr4, 3, 0));
//    root = insert(root, init_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, arr3, 3, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_INT_LITERAL, 0));
//    root = insert(root, init_short_prod_rule(TOKEN_COUNT + SYMBOL_FACTOR, TOKEN_IDENTIFIER, 0));

    return root;
}

void
generate_parse_tables(unsigned int ***action_table, unsigned int ***goto_table, unsigned int *num_of_states, char load,
                      char save, const char *action_fp, const char *goto_fp)
{

    AVLNode* grammar = init_grammar();
    if(load)
    {
        *action_table = load_table(action_fp);
        *goto_table = load_table(goto_fp);
        return;
    }

    intDynArrPtr* first_sets = calculate_firsts(grammar, TOKEN_COUNT+ SYMBOL_COUNT-1);

    // generate LR1 item sets
    genDynArrPtr item_sets = generate_items(grammar);

    *num_of_states = item_sets->array_size;

    // Initializing the action table
    *action_table = calloc(item_sets->array_size,sizeof(unsigned int*));
    if(*action_table == NULL){
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE ACTION TABLE MEMORY", NULL);
        return;}
    for(int i = 0; i < item_sets->array_size; i++)
    {
        (*action_table)[i] = calloc((TOKEN_COUNT)*2,sizeof(unsigned int));
        if((*action_table)[i] == NULL){
            report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE ACTION TABLE MEMORY", NULL);
            return;}
    }
    // initialize the goto table
    *goto_table = calloc(item_sets->array_size,sizeof(unsigned int*));
    if(*goto_table == NULL){
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE GOTO TABLE MEMORY", NULL);
        return;}
    for(int i = 0; i < item_sets->array_size; i++)
    {
        (*goto_table)[i] = calloc(SYMBOL_COUNT,sizeof(unsigned int));
        if((*goto_table)[i] == NULL){
            report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE GOTO TABLE MEMORY", NULL);
            return;}
    }

    // setup
    AVLNode* current_set;
    TreeIterator* iter;
    ProdRule item;
    // iterate over all item sets
    for(int i = 0; i< item_sets->array_size; i++)
    {
        current_set = item_sets->array[i];
        iter = init_tree_iterator(current_set);
        while(!iterator_is_empty(iter))  // iterate over every item in the set
        {
            item = ((AVLNode*)iterator_next(iter))->data; // get the next item

            // condition 1, for items where the dot is not at the end of the body
            if (item->dot < item->bodySize)
            {
                // if the dot is a terminal symbol
                if(item->body[item->dot] < TOKEN_COUNT)
                {
                    // generate a temporary goto set GOTO(current_set, body[dot])
                    AVLNode* temp_goto = goto_func(grammar, current_set, first_sets, item->body[item->dot]);
                    for(int j = 0; j< item_sets->array_size; j++) // iterate over every item set
                    {   // check if the trees are equal
                        if(trees_is_equal(temp_goto, item_sets->array[j]))
                        {
                            // if they are, update the action table accordingly
                            (*action_table)[i][item->body[item->dot]*2] = ACTION_SHIFT;
                            (*action_table)[i][item->body[item->dot]*2+1] = j;
                        }
                    }
                    delete_tree(temp_goto, 1);
                }
            }
            // condition 2, for where the dot is at the end of the body and the head isn't the augmented start
            else if (item->head != SYMBOL_START_TAG+TOKEN_COUNT)
            {
                if ((*action_table)[i][item->lookahead*2]) // check if the state is already set
                { report_error(ERR_INTERNAL, -1, "INVALID GRAMMAR, ACTION TABLE CONFLICT", get_symbol_name(item->head)); return;}
                (*action_table)[i][item->lookahead*2] = ACTION_REDUCE;
                (*action_table)[i][item->lookahead*2+1] = find_pos(grammar, item);
            }
            // condition 3, for where the dot is at the end and the item is the augmented start
            else if (item->head == SYMBOL_START_TAG+TOKEN_COUNT)
            {
                if ((*action_table)[i][item->lookahead*2]) // check if the state is already set
                { report_error(ERR_INTERNAL, -1, "INVALID GRAMMAR, ACTION TABLE CONFLICT", get_symbol_name(item->head)); return;}
                (*action_table)[i][item->lookahead*2] = ACTION_ACCEPT;
            }
        }
        free(iter->stack);  // free the iterator
        free(iter);

        for(int A = 0; A<SYMBOL_COUNT; A++)  // iterate over every non-terminal symbol
        {
            // create a temporary goto set, GOTO(current_set, A)
            AVLNode* temp_goto = goto_func(grammar, current_set, first_sets, A+TOKEN_COUNT);
            for(int j = 0; j< item_sets->array_size; j++) // iterate over every item set
            {
                // if the temp goto set and the item set are equal,
                // then goto_table at current state symbol A equals matching set number
                if(trees_is_equal(temp_goto, item_sets->array[j]))
                {
                    (*goto_table)[i][A] = j;
                }
            }
            delete_tree(temp_goto, 1);
        }
    }
    if (save)
    {
        save_table(*action_table, (TOKEN_COUNT)*2, *num_of_states, action_fp);
        save_table(*goto_table, SYMBOL_COUNT, *num_of_states, goto_fp);
    }
}

void save_table(unsigned int **table, unsigned int row_size, unsigned int num_of_rows, const char* filename)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("File failed to open");
        return;
    }
    fwrite(&num_of_rows, sizeof(unsigned int), 1, file);
    fwrite(&row_size, sizeof(unsigned int), 1, file);
    for (int i = 0; i < num_of_rows; i++)
    {
        fwrite(table[i], sizeof(unsigned int), row_size, file);
    }
    fclose(file);
}

unsigned int ** load_table(const char* filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("File failed to open");
        return NULL;
    }
    unsigned int** table;
    unsigned int num_of_rows;
    unsigned int row_size;
    fread(&num_of_rows, sizeof(unsigned int), 1, file);
    fread(&row_size, sizeof(unsigned int), 1, file);
    table = malloc(num_of_rows * sizeof(unsigned int*));
    for(int i = 0; i < num_of_rows; i++)
    {
        table[i] = malloc(sizeof(unsigned int) * row_size);
        fread(table[i], sizeof(unsigned int), row_size, file);
    }
    return table;
}