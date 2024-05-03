//
// Created by Nimrod on 14/03/2024.
//

#include "SymbolTableManager.h"


ScopeNode *init_scope_node(ScopeType scope, TokenType return_type, unsigned long start_line)
{
    ScopeNode* result = malloc(sizeof(ScopeNode));
    if (result == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR SCOPE TREE NODE", NULL);
        exit(1);
    }
    result->num_of_children = 0;
    result->children = NULL;
    result->parent = NULL;
    result->table = init_hash_table((unsigned long (*)(void *)) djb2, (int (*)(void *, void *)) strcmp);
    result->return_type = return_type;
    result->scope = scope;
    result->returning = 0;
    result->curr_index = 0;
    result->bytes_used_since_last_frame = 0;
    result->bytes_cleared_since_last_frame = 0;
    result->start_line = start_line;
    return result;
}

void add_scope_child(ScopeNode* parent, ScopeNode *child)
{
    child->parent = parent;
    void* temp = realloc(parent->children, sizeof(ScopeNode*)*parent->num_of_children+1);
    if (temp == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR SCOPE TREE NODE CHILDREN", NULL);
        exit(1);
    }
    parent->children = temp;
    parent->children[parent->num_of_children] = child;
    parent->num_of_children++;
}

symbol_item *
init_symbol_item(char *name, int data_type, int symbol_type, Param *parameters, int num_of_params, int length,
                 unsigned int line, char assigned)
{
    symbol_item* result = malloc(sizeof(symbol_item));
    if (result == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR SYMBOL TABLE ITEM", NULL);
    }
    result->name = name;
    result->type[0] = data_type; result->type[1] = symbol_type;
    result->line_of_dec = line;
    result->parameters = parameters;
    result->num_of_params = num_of_params;
    result->assigned = assigned;
    result->size = length;
    return result;
}

Param* init_params(ASTNode* params)
{
    Param* result = malloc(sizeof(Param)*params->num_of_children);
    for(int i = 0; i < params->num_of_children; i++)
    {
        result[i].type = params->children[i]->type;
        result[i].name = params->children[i]->children[0]->token->lexeme;
    }
    return result;
}

int construct_symbol_table_rec(ASTNode *ast, ScopeNode *scope)
{
    static int offset = 0;
    if (ast == NULL)
    {return 1;}
    char* temp_str;
    Param* params;
    int num_of_items;
    unsigned int line;
    symbol_item* item;
    ScopeNode* new_node;
    char item_added_success = 0;
    char assigned;
    int error = 0;
    switch (ast->type)
    {
        case SYMBOL_FUNC_DEC + TOKEN_COUNT:
            line = ast->children[0]->token->line;
            if(ast->num_of_children==4)
            {
                params = init_params(ast->children[2]);
                num_of_items = ast->children[2]->num_of_children;
            }
            else { params = NULL; num_of_items=0; }
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type,
                                    SYMBOL_FUNC_DEC + TOKEN_COUNT, params, num_of_items, 1, line, 1);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            item->scope = scope;
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            new_node = init_scope_node(SCOPE_FUNCTION, ast->children[0]->type, ast->start_line);
            for(int i = 0; i<num_of_items; i++)
            {
                item = init_symbol_item(params[i].name, params[i].type, SYMBOL_VAR_DEC + TOKEN_COUNT,
                                        NULL, 0, 1, line, 1);
                item_added_success = add_item(new_node->table,params[i].name, item);
                item->scope = new_node;
                if (!item_added_success) {
                    error = 1;
                    report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
                }
            }
            add_scope_child(scope, new_node);
            if(num_of_items==0)
                error |= !construct_symbol_table_rec(ast->children[2], new_node);
            else
                error |= !construct_symbol_table_rec(ast->children[3], new_node);
            break;
        case SYMBOL_ARR_DEC + TOKEN_COUNT:
            line = ast->children[0]->token->line;
            assigned = 1;
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type,
                                    SYMBOL_ARR_DEC + TOKEN_COUNT, NULL, 0, (int) strtol(ast->children[2]->token->lexeme,
                                                                                        &temp_str, 10), line, assigned);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            item->scope = scope;
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            break;
        case SYMBOL_VAR_DEC + TOKEN_COUNT:
            line = ast->children[0]->token->line;
            assigned = (char)(ast->num_of_children != 2);
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type,
                                    SYMBOL_VAR_DEC + TOKEN_COUNT, NULL, 0, 1, line, assigned);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            item->scope = scope;
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            break;
        case SYMBOL_WHILE + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_LOOP, TOKEN_ERROR, ast->start_line);
            add_scope_child(scope, new_node);
            error |= !construct_symbol_table_rec(ast->children[1], new_node);
            break;
        case SYMBOL_FOR + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_LOOP, TOKEN_ERROR, ast->start_line);
            add_scope_child(scope, new_node);
            error |= !construct_symbol_table_rec(ast->children[0], new_node);
            error |= !construct_symbol_table_rec(ast->children[3], new_node);
            break;
        case SYMBOL_STATEMENTS + TOKEN_COUNT:
            for(int i = 0; i<ast->num_of_children; i++)
            {
                error |= !construct_symbol_table_rec(ast->children[i], scope);
            }
            break;
        case SYMBOL_IF + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_CONDITIONAL, TOKEN_ERROR, ast->start_line);
            add_scope_child(scope, new_node);
            error |= !construct_symbol_table_rec(ast->children[1], new_node);
            break;
        case SYMBOL_IF_ELSE + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_CONDITIONAL, TOKEN_ERROR, ast->start_line);
            add_scope_child(scope, new_node);
            error |= !construct_symbol_table_rec(ast->children[1], new_node);
            new_node = init_scope_node(SCOPE_CONDITIONAL, TOKEN_ERROR, ast->start_line);
            add_scope_child(scope, new_node);
            error |= !construct_symbol_table_rec(ast->children[2], new_node);
            break;
    }
    return !error;
}

symbol_item* find_var(ScopeNode* curr_scope, char* name)
{
    symbol_item* item = NULL;
    if(curr_scope == NULL)
        return NULL;
    char run = 1;
    while(run)
    {
        item = get_item(curr_scope->table, name);
        if(item==NULL)
        {
            if(curr_scope->scope == SCOPE_GLOBAL)
            { run = 0; }
            else
            { curr_scope = curr_scope->parent; }
        }
        else
        { run = 0; }
    }
    return item;
}

ScopeNode * is_in_scope(ScopeNode* curr_scope, ScopeType search)
{
    ScopeNode* result = NULL;
    int run = 1;
    while(curr_scope != NULL && run)
    {
        if (curr_scope->scope == search)
        { result = curr_scope; run = 0;}
        else if(curr_scope->scope == SCOPE_FUNCTION) run = 0;
        curr_scope = curr_scope->parent;
    }
    return result;
}

void print_scope_tree(ScopeNode *node, int depth, char *finals)
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
    else
    printf("%*s", 4, "");
    {  // if non-terminal, print out the symbol name,
        printf(" %d %s\n", node->scope, node->return_type ? get_symbol_name(node->return_type) : "");
        for(int x = 0; x < node->table->array_size; x++)
        {
            LinkedList* bucket = node->table->array[x];
            while(bucket != NULL)
            {
                for(i = 0; i < depth; i++) // iterate 0->depth-1, if the last item placed in that depth has more brothers,
                {                            // checked by !finals[i], print out 7 spaces and then a line
                    if(!finals[i])
                        printf("%*s", 8, "│");
                    else                    // else print out seven spaces and then another space in place of the line
                        printf("%*s", 6, " ");
                }
                hash_table_item* item = bucket->data;
                symbol_item* sItem = item->data;
                printf("%*s:%s -> %s %s len: %d %s\n", 6, "", (char*)item->key, get_symbol_name(sItem->type[0]), get_symbol_name(sItem->type[1]), sItem->size,  sItem->assigned ? "Assigned" : "");
                bucket = bucket->next;
            }
        }
        // mark the current depth as unfinished
        finals[depth] = 0;
        for(int j = 0; j < node->num_of_children-1; j++)  // call the function for all except for last of the children
        {
            print_scope_tree(node->children[j], depth + 1, finals);
        }
        finals[depth] = 1;  // set the current depth to 1 then call the function with the last child
        if(node->num_of_children != 0)
        print_scope_tree(node->children[node->num_of_children - 1], depth + 1, finals);
    }
    //├─│
}