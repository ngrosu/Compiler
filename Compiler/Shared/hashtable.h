//
// Created by Nimrod on 14/03/2024.
//

#ifndef COMPILER_HASHTABLE_H
#define COMPILER_HASHTABLE_H
#include "./error.h"
#include "../Parser/ParserGenerator/DataStructures.h"
#include <stdlib.h>
#include <string.h>


typedef struct hash_table
{
    unsigned long (*hash)(void*);
    int (*compare)(void*, void*);
    struct LinkedList **array;
    unsigned int array_size;
    unsigned int num_of_items;
} hash_table;

typedef struct hash_table_item
{
    void* key;
    void* data;
} hash_table_item;

hash_table *init_hash_table(unsigned long (*hash_func)(void *), int (*compare)(void *, void *));

hash_table_item* init_hash_table_item(void* key, void* data);

void* get_item(hash_table* table, void* key);

char add_item(hash_table *table, void *key, void *item);

unsigned long djb2(char*);

void resize_hash_table(hash_table* table);


#endif //COMPILER_HASHTABLE_H
