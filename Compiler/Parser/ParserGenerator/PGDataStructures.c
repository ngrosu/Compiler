//
// Created by Nimrod on 29/02/2024.
//

#include "PGDataStructures.h"

// Prod Rules

ProdRule init_prod_rule(TokenType head, const TokenType* body, short bodySize, short dot)
{
    ProdRule rule = malloc(sizeof(ProdRuleStruct)); // allocate space for rule
    rule->head = head;
    for(int i=0; i<bodySize; i++)
    {
        rule->body[i] = body[i];
    }
    rule->bodySize = bodySize;
    rule->dot = dot;
    return rule;
}

short compare_prod_rules(ProdRule a, ProdRule b)
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

    short cmp = compare_prod_rules(data, root->data);
    // normal BST insertion
    if (cmp == -1)
        root->left = insert(root->left, data);
    else if (cmp == 1)
        root->right = insert(root->right, data);
    else
    {
        report_error(ERR_INTERNAL, -1, "Tried to add existing item to AVL");
        return root;
    }

    // update height
    root->height = 1 + max(height(root->left), height(root->right));

    int balance = get_balance(root);

    if (balance > 1 && compare_prod_rules(data, root->left->data)==-1)
        return right_rotate(root);

    // Right Right Case
    if (balance < -1 && compare_prod_rules(data, root->right->data)==1)
        return left_rotate(root);

    // Left Right Case
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
        return root;

    short cmp = compare_prod_rules(data, root->data);
    // normal BST navigation
    if (cmp == -1)
        return find(root->left, data);
    else if (cmp == 1)
        return find(root->right, data);
    else
        return root;
}

void pre_order(AVLNode *root)
{
    if(root != NULL)
    {
        printf("%d, ", root->data->head);
        pre_order(root->left);
        pre_order(root->right);
    }
}
