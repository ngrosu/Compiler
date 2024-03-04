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
        report_error(ERR_INTERNAL, -1, "Memory allocation failed");
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
        report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION");
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
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY");
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
        report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION");
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
            set_rule = iterator_next(iter_set);
            // perform action on tree node

            if (set_rule->body[set_rule->dot] >= TOKEN_COUNT)
            { // if the item after the dot is a
                // non-terminal go into the inner loop
                iter_root = init_tree_iterator(grammar);
                while (!iterator_is_empty(iter_root))
                { // iterate over the grammar tree (main grammar rules)
                    root_rule = iterator_next(iter_root);

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
                        } else
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
    if (set==NULL) {return result;}

    ProdRule data = set->data;
    if (data->dot != data->bodySize) // make sure the dot is not at the end of the rule
    {
        if (data->body[data->dot] == symbol) // if the symbol after the dot matches
        {
            // advance the dot by 1 and add to the new set
            result = insert(result, init_LR1_item(data->head, data->body, data->bodySize,
                                                  (short)(data->dot+1), data->lookahead));
        }
    }
    // repeat for entire tree
    result = goto_helper(set->left, result, symbol);
    result = goto_helper(set->right, result, symbol);

    return result;
}

AVLNode* goto_func(AVLNode* root, AVLNode* set, intDynArrPtr* first_sets, int symbol)
{
    AVLNode* result = NULL;
    result = goto_helper(set, result, symbol); // generate the goto set
    return closure(root, result, first_sets);
}

setDynArrPtr generate_items(AVLNode* grammar)
{
    void* temp = malloc(sizeof(int));
    if(temp==NULL){
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY");
        return NULL;
    }

    // augment the grammar to have S` -> S, $
    *(int*)temp = SYMBOL_START;
    ProdRule augmented_start = init_LR1_item(SYMBOL_START_TAG, (int*)temp, 1, 0, TOKEN_EOF);
    free(temp);
    grammar = insert(grammar, augmented_start);

    // calculate first sets
    intDynArrPtr* first_sets = calculate_firsts(grammar, TOKEN_COUNT+SYMBOL_COUNT-1);

    // initialize a dynamic array to store the sets
    setDynArrPtr setArr = init_set_dynamic_array();
    add_to_set_dyn_array(setArr, closure(grammar, insert(NULL, augmented_start), first_sets));

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
                    for (int j = 0; j < i && !check; j++)
                    {
                        if (trees_is_equal(tempgoto, setArr->array[j]))
                        {
                            check = 1;
                        }
                    }
                    if (!check)
                    {
                        change=1;
                        add_to_set_dyn_array(setArr, tempgoto);
                    }
                }
            }
        }
    }
    return setArr;
}

