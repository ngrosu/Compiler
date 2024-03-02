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
} *ProdRule, ProdRuleStruct;

//create new production rule
ProdRule init_prod_rule(int head, const int* body, short bodySize, short dot);

ProdRule init_short_prod_rule(int head, int body, short dot);

// return -1 if a < b, 0 if equal, 1 if a > b
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

AVLNode* find_head(AVLNode* root, int symbol); // find first_helper node to have head

void pre_order(AVLNode* root); // print AVL tree in order

// Symbol


typedef struct DynamicArray
{
    int array_size;
    int array_capacity;
    int* array;
} *intDynArrPtr,intDynArr;

intDynArrPtr init_array();

void add_to_array(intDynArrPtr arr, int num);

#endif //COMPILER_DATASTRUCTURES_H
