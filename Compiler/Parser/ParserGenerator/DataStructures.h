//
// Created by Nimrod on 29/02/2024.
//

#ifndef COMPILER_DATASTRUCTURES_H
#define COMPILER_DATASTRUCTURES_H
#include "../../Shared/token.h"
#include "../../Shared/error.h"
#include "symbol.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int head;
    int body[MAX_RULE_SIZE];
    short bodySize;
    short dot;
    int lookahead;
    ASTFuncs ASTFunc;
} *ProdRule, ProdRuleStruct;

//create new production rule
ProdRule init_prod_rule(int head, const int *body, short bodySize, short dot, ASTFuncs ASTFunc);

// init a prod rule with a single symbol body
ProdRule init_short_prod_rule(int head, int body, short dot);

// init a prod rule with a lookahead symbol (LR1 item)
ProdRule init_LR1_item(int head, const int* body, short bodySize, short dot, int lookahead); // init a prod rule

// compare two production rules (for AVL), return -1 if a < b, 0 if equal, 1 if a > b
short compare_prod_rules(ProdRule a, ProdRule b);

// AVL tree
typedef struct AVLNode {
    ProdRule data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;


int height(AVLNode* N); //get height of a node

int max(int a, int b); // max of 2 ints

int get_balance(AVLNode* N); // get balance of node

AVLNode* newNode(ProdRule data); // create a new node

AVLNode* right_rotate(AVLNode* y); // perform a right rotation

AVLNode* left_rotate(AVLNode* x); // perform a left rotation

AVLNode* insert(AVLNode* root, ProdRule data); // insert into AVL

AVLNode* find(AVLNode* root, ProdRule data); // find in AVL tree

AVLNode* find_head(AVLNode* root, int symbol); //  find node whose head is a certain symbol

// find the number of the rule in the grammar tree
unsigned int find_pos(AVLNode* grammar, ProdRule data);

void delete_tree(AVLNode* root, int deleteData);  // delete the tree, if deleteData is 1: delete the rules as well

char trees_is_equal(AVLNode* a, AVLNode* b); // check if two trees are equal

void in_order(AVLNode* root); // print AVL tree in order


typedef struct TreeIterator
{
    struct Stack* stack;
    AVLNode* current_node;
} TreeIterator;

TreeIterator* init_tree_iterator(AVLNode* root);

int iterator_is_empty(TreeIterator* iter);

void* iterator_next(TreeIterator* iter);

// Dynamic Array

typedef struct intDynamicArray
{
    int array_size;
    int array_capacity;
    int* array;
} *intDynArrPtr,intDynArr;

// initialize an int dynamic array
intDynArrPtr init_int_dynamic_array();

// delete an int dynamic array
void delete_int_dynamic_array(intDynArrPtr arr);

// add an integer to the dynamic array
void add_to_int_dyn_array(intDynArrPtr arr, int num);

typedef struct genDynamicArray
{
    int array_size;
    int array_capacity;
    void** array;
} *genDynArrPtr,genDynArr;

// initialize an set dynamic array
genDynArrPtr init_gen_dynamic_array();

// delete an set dynamic array
void delete_set_dynamic_array(genDynArrPtr arr);

// add an integer to the dynamic array
void add_to_gen_dyn_array(genDynArrPtr arr, void *data);

// create a dynamic array from the elements of the tree in order for iterative traversal
genDynArrPtr convert_AVL_to_array(AVLNode* tree);

// Linked List

typedef struct LinkedList
{
    void* data;
    struct LinkedList* next;
} LinkedList;

LinkedList* create_linked_list_node(void* data, LinkedList* next);

void delete_linked_list(void* del_func(void*), LinkedList* curr);

// Stack

typedef struct Stack
{
    LinkedList* content;
} Stack;

// initialize a stack
Stack* init_stack();

// push an item onto the stack
void push(Stack* stack, void* data);

// push an int onto the stack
void push_int(Stack* stack, unsigned int data);

// pop from the stack
void* pop(Stack* stack);




#endif //COMPILER_DATASTRUCTURES_H
