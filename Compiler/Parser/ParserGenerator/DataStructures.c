//
// Created by Nimrod on 29/02/2024.
//

#include "DataStructures.h"

// Prod Rules

ProdRule init_prod_rule(int head, const int* body, short bodySize, short dot) // init a prod rule
{
    ProdRule rule = malloc(sizeof(ProdRuleStruct)); // allocate space for rule
    rule->head = head;
    for(int i=0; i<bodySize; i++)
    {
        rule->body[i] = body[i];
    }
    rule->bodySize = bodySize;
    rule->dot = dot;
    rule->lookahead = 0;
    return rule;
}

ProdRule init_LR1_item(int head, const int* body, short bodySize, short dot, int lookahead) // init a prod rule
{
    ProdRule rule = malloc(sizeof(ProdRuleStruct)); // allocate space for rule
    rule->head = head;
    for(int i=0; i<bodySize; i++)
    {
        rule->body[i] = body[i];
    }
    rule->bodySize = bodySize;
    rule->dot = dot;
    rule->lookahead = lookahead;
    return rule;
}

ProdRule init_short_prod_rule(int head, int body, short dot) // init a prod rule with a single symbol body
{
    ProdRule rule = malloc(sizeof(ProdRuleStruct)); // allocate space for rule
    rule->head = head;
    rule->body[0] = body;
    rule->bodySize = 1;
    rule->dot = dot;
    return rule;
}



short compare_prod_rules(ProdRule a, ProdRule b) // compare two production rules (for AVL)
{
    // compare heads
    if (a->head < b->head) return -1;
    else if (a->head > b->head) return 1;

        // If heads are equal, compare the body sizes
        if (a->bodySize < b->bodySize) return -1;
        else if (a->bodySize > b->bodySize) return 1;

            // If body sizes are equal, compare the body contents element-wise
            for (int i = 0; i < a->bodySize; ++i) {
                if (a->body[i] < b->body[i]) return -1;
                else if (a->body[i] > b->body[i]) return 1;
            }

                // If contents are equal, compare the dot locations
                if (a->dot < b->dot) return -1;
                else if (a->dot > b->dot) return 1;

                    // If dots are equal compare lookahead (only relevant for items)
                    if(a->lookahead < b->lookahead) return -1;
                    else if (a->lookahead > b->lookahead) return 1;

    return 0;
}
// AVL Tree

int height(AVLNode *N)
{
    if (N == NULL)
        return 0;
    return N->height;
}


int get_balance(AVLNode *N)
{
    if (N == NULL)
        return 0;
    return height(N->left) - height(N->right);
}


int max(int a, int b) {
    return (a > b)? a : b;
}


AVLNode* newNode(ProdRule data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    node->height = 1; // new node is initially added at leaf
    return(node);
}


AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left; //perform a right rotation
    AVLNode* z = x->right;
    x->right = y;
    y->left = z;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x; // new root
}


AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right; //perform a left rotation
    AVLNode* z = y->left;
    y->left = x;
    x->right = z;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return y; // new root
}


AVLNode* insert(AVLNode* root, ProdRule data)
{
    if (root == NULL)
        return(newNode(data));

    short cmp = compare_prod_rules(data, root->data); // get the comparison between the two rules

    // normal BST traversal to find the location for the new node
    if (cmp == -1)
        root->left = insert(root->left, data);
    else if (cmp == 1)
        root->right = insert(root->right, data);
    else
    {
        // if the node already exists
        report_error(ERR_INTERNAL, -1, "Tried to add existing item to AVL", NULL);
        return root;
    }

    // update height
    root->height = 1 + max(height(root->left), height(root->right));


    // AVL tree balancing
    int balance = get_balance(root);

    if (balance > 1 && compare_prod_rules(data, root->left->data)==-1)
        return right_rotate(root);

    // Right, Right Case
    if (balance < -1 && compare_prod_rules(data, root->right->data)==1)
        return left_rotate(root);

    // Left, Right Case
    if (balance > 1 && compare_prod_rules(data, root->left->data)==1)
    {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    // Right Left Case
    if (balance < -1 && compare_prod_rules(data, root->right->data)==-1)
    {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }
    return root;
}


AVLNode* find(AVLNode* root, ProdRule data)
{
    if (root == NULL)
        return NULL;

    short cmp = compare_prod_rules(data, root->data);
    // normal BST navigation
    if (cmp == -1)
        return find(root->left, data);
    else if (cmp == 1)
        return find(root->right, data);
    else
        return root;
}


AVLNode* find_head(AVLNode* root, int symbol) // find node whose head is a certain symbol
{
    if (root == NULL)
        return root;

    // normal BST navigation
    if (symbol<root->data->head)
        return find_head(root->left, symbol);
    else if (symbol>root->data->head)
        return find_head(root->right, symbol);
    else
        return root;
}

unsigned int find_pos(AVLNode* grammar, ProdRule data)
{
    TreeIterator* iter = init_tree_iterator(grammar);
    ProdRule temp;
    unsigned int count = 0;
    while(!iterator_is_empty(iter))  // iterate over the tree in order
    {
        temp = ((AVLNode*)iterator_next(iter))->data;
        int check = 1;
        // check if the current node has the data
        if(data->head == temp->head && data->bodySize == temp->bodySize)
        {
            for(int i = 0; i<data->bodySize && check; i++)
            {   // if it doesn't match, set check to 0
                if (data->body[i] != temp->body[i]) { check=0; }
            }  // if check is still 1, return count
            if (check) {return count;}

        }
        // if node isn't matching increment count and keep looking
        count++;
    }
    free(iter->stack);
    free(iter);
    return count;
}

void delete_tree(AVLNode* root, int deleteData)
{
    if (root == NULL)
        {return;}
    delete_tree(root->left, deleteData);
    delete_tree(root->right, deleteData);
    if(deleteData)
        { free(root->data); }
    free(root);
}

void in_order(AVLNode *root)
{
    if(root != NULL)
    {
        in_order(root->right);
        int sum =0;
        sum += printf("   %-9s=> ", get_symbol_name(root->data->head));
        for(int i=0; i<root->data->bodySize; i++)
        {
            if(i==root->data->dot)
                {sum += printf("• ");}
            sum +=printf("%s ", get_symbol_name(root->data->body[i]));
        }
        if(root->data->bodySize==root->data->dot)
        {sum += printf("• ");}
        printf("%*s| %s \n", 34-sum, "", get_symbol_name(root->data->lookahead));
        //printf("%d, ", root->data->head);
        in_order(root->left);
    }
}

TreeIterator* init_tree_iterator(AVLNode* root)
{
    TreeIterator* iter = malloc(sizeof(TreeIterator));
    if (iter==NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION", NULL);
        return NULL;
    }
    iter->stack = init_stack();
    iter->current_node = root;
    return iter;
}

int iterator_is_empty(TreeIterator* iter) {
    return iter->stack->content == NULL && iter->current_node == NULL;
}

void* iterator_next(TreeIterator* iter) {
    while (iter->current_node != NULL)
    {
        push(iter->stack, iter->current_node);
        iter->current_node = iter->current_node->left;
    }

    iter->current_node = pop(iter->stack);
    AVLNode* node = iter->current_node;
    iter->current_node = iter->current_node->right;

    return node;
}

char trees_is_equal(AVLNode* a, AVLNode* b)
{
    TreeIterator* iterA = init_tree_iterator(a);
    TreeIterator* iterB = init_tree_iterator(b);
    int cmp=0;
    while(!iterator_is_empty(iterA) && !iterator_is_empty(iterB) && cmp==0)
    {
        cmp = compare_prod_rules(((AVLNode*)iterator_next(iterA))->data, ((AVLNode*)iterator_next(iterB))->data);
    }
    if(!iterator_is_empty(iterA) || !iterator_is_empty(iterB) || cmp!=0)
    {
        free(iterA->stack);
        free(iterB->stack);
        free(iterA);
        free(iterB);
        return 0;}
    free(iterA->stack);
    free(iterB->stack);
    free(iterA);
    free(iterB);
    return 1;
}

// Symbol

// Dynamic Array

intDynArrPtr init_int_dynamic_array()
{ // initialize an int dynamic array
    intDynArrPtr arr = malloc(sizeof(intDynArr));
    arr->array_capacity=1;
    arr->array = malloc(sizeof(int));
    arr->array_size=0;
    return arr;
}

void delete_int_dynamic_array(intDynArrPtr arr)
{
    free(arr->array);
    free(arr);
}

void add_to_int_dyn_array(intDynArrPtr arr, int num)
{ // add an integer to the dynamic array
    if (arr->array_size +1 > arr->array_capacity ) { // check if the array is too small
        void *temp = realloc(arr->array, sizeof(int) * arr->array_capacity * 2); //increase arr size
        if (temp == NULL) {
            report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION", NULL);
            return;
        }
        arr->array_capacity *= 2; // update array size
        arr->array = temp;
    }
        // add int to array
        arr->array[arr->array_size] = num;
        arr->array_size++;

}

genDynArrPtr init_gen_dynamic_array()
{ // initialize a set dynamic array
    genDynArrPtr arr = malloc(sizeof(genDynArr));
    arr->array_capacity=1;
    arr->array = malloc(sizeof(void**));
    arr->array_size=0;
    return arr;
}


// delete an set dynamic array
void delete_set_dynamic_array(genDynArrPtr arr)
{ // DOES NOT FREE THE STRUCTS IN THE ARRAY
    free(arr->array);
    free(arr);
}


void add_to_gen_dyn_array(genDynArrPtr arr, void *data)
{ // add a data to the dynamic array
    if(data == NULL)
    {
        report_error(ERR_INTERNAL, -1, "TRIED ADDING NULL SET TO DYN ARR", NULL);
    return;}
    if (arr->array_size +1 > arr->array_capacity ) { // check if the array is too small
        void *temp = realloc(arr->array, sizeof(void**) * arr->array_capacity * 2); //increase arr size
        if (temp == NULL) {
            report_error(ERR_INTERNAL, -1, "FAILED MEMORY ALLOCATION", NULL);
            return;
        }
        arr->array_capacity *= 2; // update array size
        arr->array = temp;
    }
    // add int to array
    arr->array[arr->array_size] = data;
    arr->array_size++;
}


genDynArrPtr convert_AVL_to_array(AVLNode* tree)
{
    genDynArrPtr arr = init_gen_dynamic_array();
    TreeIterator* iter = init_tree_iterator(tree);
    while(!iterator_is_empty(iter)) // iterate over the tree in order
    {
        // add contents to dynamic array
        ProdRule temp = ((AVLNode*)iterator_next(iter))->data;
        add_to_gen_dyn_array(arr, temp);
    }
    return arr;
}


// Linked List
LinkedList* create_linked_list_node(void* data, LinkedList* next)
{
    LinkedList* result = malloc(sizeof(LinkedList));
    if(result==NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY", NULL);
        return NULL;
    }

    result->data = data;
    result->next = next;
    return result;
}

void delete_linked_list(void* del_func(void*), LinkedList* curr)
{
    LinkedList* next;
    while(curr!=NULL)
    {
        next = curr->next;
        del_func(curr->data);
        free(curr);
        curr=next;
    }
}

// Stack

Stack* init_stack()
{
    Stack* stack = malloc(sizeof(Stack));
    stack->content = NULL;
    return stack;
}

// push an item onto the stack
void push(Stack* stack, void* data)
{
    stack->content = create_linked_list_node(data, stack->content);
}

void push_int(Stack* stack, unsigned int data)
{
    unsigned int* temp = malloc(sizeof(unsigned int)); // allocate memory since stack receives void*
    *temp = data; // add the data
    stack->content = create_linked_list_node(temp, stack->content);
}

// pop from the stack
void* pop(Stack* stack)
{
    void* data = stack->content->data;
    LinkedList* next = stack->content->next;
    free(stack->content);
    stack->content=next;

    return data;
}

