//
// Created by Nimrod on 14/03/2024.
//

#include "SymbolTableManager.h"


ScopeNode* init_scope_node(ScopeType scope)
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
    result->scope = scope;
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
                 unsigned int line)
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

char construct_symbol_table_rec(ASTNode *ast, ScopeNode *scope, ScopeNode *global)
{
    if (ast == NULL)
    {return 1;}
    char* temp_str;
    Param* params;
    int num_of_items;
    unsigned int line;
    symbol_item* item;
    ScopeNode* new_node;
    char item_added_success = 0;
    char error = 0;
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
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type ,
                                     SYMBOL_FUNC_DEC + TOKEN_COUNT, params, num_of_items, 1, line);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            printf("%s: %lu\n", ast->children[1]->token->lexeme, scope->table->hash(ast->children[1]->token->lexeme));
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            new_node = init_scope_node(SCOPE_FUNCTION);
            for(int i = 0; i<num_of_items; i++)
            {
                item = init_symbol_item(params[i].name, params[i].type, SYMBOL_VAR_DEC + TOKEN_COUNT,
                                        NULL,0,1, line);
                item_added_success = add_item(new_node->table,params[i].name, item);
                printf("%s: %lu\n", params[i].name, scope->table->hash(params[i].name));
                if (!item_added_success) {
                    error = 1;
                    report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
                }
            }
            add_scope_child(scope, new_node);
            if(num_of_items==0)
                construct_symbol_table_rec(ast->children[2], new_node, global);
            else
                construct_symbol_table_rec(ast->children[3], new_node, global);
            break;
        case SYMBOL_ARR_DEC + TOKEN_COUNT:
            line = ast->children[0]->token->line;
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type ,
                                    SYMBOL_ARR_DEC + TOKEN_COUNT, NULL, 0, (int)strtol(ast->children[2]->token->lexeme,
                                                                         &temp_str, 10), line);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            break;
        case SYMBOL_VAR_DEC + TOKEN_COUNT:
            line = ast->children[0]->token->line;
            item = init_symbol_item(ast->children[1]->token->lexeme, ast->children[0]->type ,
                                    SYMBOL_VAR_DEC + TOKEN_COUNT, NULL, 0, 1, line);
            item_added_success = add_item(scope->table, ast->children[1]->token->lexeme, item);
            printf("%s: %lu\n",ast->children[1]->token->lexeme, scope->table->hash(ast->children[1]->token->lexeme));
            if (!item_added_success) {
                error = 1;
                report_error(ERR_SEMANTIC, line, "Symbol Already Declared | ", item->name);
            }
            break;
        case SYMBOL_WHILE + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_LOOP);
            add_scope_child(scope, new_node);
            construct_symbol_table_rec(ast->children[1], new_node, global);
            break;
        case SYMBOL_FOR + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_LOOP);
            add_scope_child(scope, new_node);
            construct_symbol_table_rec(ast->children[0], new_node, global);
            construct_symbol_table_rec(ast->children[3], new_node, global);
            break;
        case SYMBOL_STATEMENTS + TOKEN_COUNT:
            for(int i = 0; i<ast->num_of_children; i++)
            {
                construct_symbol_table_rec(ast->children[i], scope, global);
            }
            break;
        case SYMBOL_IF + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_GENERIC);
            add_scope_child(scope, new_node);
            construct_symbol_table_rec(ast->children[1], new_node, global);
            break;
        case SYMBOL_IF_ELSE + TOKEN_COUNT:
            new_node = init_scope_node(SCOPE_GENERIC);
            add_scope_child(scope, new_node);
            construct_symbol_table_rec(ast->children[1], new_node, global);
            new_node = init_scope_node(SCOPE_GENERIC);
            add_scope_child(scope, new_node);
            construct_symbol_table_rec(ast->children[2], new_node, global);
            break;
    }
    return (char)!error;
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
        printf(" %d\n", node->scope);
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
                printf("%*s:%s -> %s %s l: %d\n", 6, "", (char*)item->key, get_symbol_name(sItem->type[0]), get_symbol_name(sItem->type[1]), sItem->size);
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