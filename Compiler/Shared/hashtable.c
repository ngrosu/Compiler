//
// Created by Nimrod on 14/03/2024.
//

#include "hashtable.h"

// Hash Table
hash_table *init_hash_table(unsigned long (*hash_func)(void *), int (*compare)(void *, void *))
{
    hash_table* result = malloc(sizeof(hash_table));
    if (result == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR HASH TABLE", NULL);
        return NULL;
    }
    result->array = calloc(HASH_START_SIZE, sizeof(LinkedList*));
    if (result->array == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR HASH TABLE", NULL);
        return NULL;
    }
    result->array_size = HASH_START_SIZE;
    result->hash = hash_func;
    result->compare = compare;
    result->num_of_items = 0;
    return result;
}

hash_table_item* init_hash_table_item(void* key, void* data)
{
    hash_table_item* result = malloc(sizeof(hash_table_item));
    if (result == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY FOR HASH TABLE ITEM", NULL);
        return NULL;
    }
    result->key = key;
    result->data = data;
    return result;
}

void resize_hash_table(hash_table* table)
{
    LinkedList** new = calloc(table->array_size<<1, sizeof(LinkedList*));
    unsigned int newSize = table->array_size<<1;
    if(new == NULL)
    {
        report_error(ERR_INTERNAL, -1, "FAILED TO ALLOCATE MEMORY WHEN RESIZING HASH TABLE", NULL);
    }

    for(int i = 0; i < table->array_size; i++)
    {
        LinkedList* bucket = table->array[i];
        LinkedList* next;
        while(bucket != NULL)
        {
            unsigned int index = table->hash(((hash_table_item*)bucket->data)->key)%newSize;
            next = bucket->next;
            bucket->next = new[index];
            new[index] = bucket;
            bucket = next;
        }
    }
    table->array = new;
    table->array_size = newSize;
}

void* get_item(hash_table* table, void* key)
{
    unsigned int index = table->hash(key) % table->array_size;
    LinkedList* bucket = table->array[index];
    if (bucket==NULL) {return NULL;}

    char check = 0;
    while(bucket != NULL && check == 0)
    {
        int cmp = table->compare(key, ((hash_table_item*)(bucket->data))->key);
        if(cmp == 0)
        {check = 1;}
        else
        {bucket = bucket->next;}
    }
    if(check == 0)
    {return NULL;}
    else
    {
        return ((hash_table_item*)bucket->data)->data;
    }
}

unsigned long djb2(char* str) // djb2 hashing algorithm by bernstein
{
    unsigned long hash = 5381;
    int c;
    while ((c = (int)*str++))
    {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

char add_item(hash_table *table, void *key, void *item)
{
    unsigned int index = table->hash(key) % table->array_size;
    LinkedList* bucket = table->array[index];
    if (bucket==NULL)
    {
        hash_table_item* new_item = init_hash_table_item(key, item);
        table->array[index] = create_linked_list_node(new_item, NULL);
        return 1;
    }
    char check = 0;
    while(bucket->next!=NULL && check == 0)
    {
        int cmp = table->compare(key, ((hash_table_item*)bucket->data)->key);
        if (cmp==0)
        {check = 1;}
        else
        {bucket = bucket->next;}
    }
    if(check)
    {
        report_error(ERR_INTERNAL, -1, "TRIED TO ADD EXISTING ITEM TO HASH TABLE", NULL);
        return 0;}
    else
    {
        hash_table_item* new_item = init_hash_table_item(key, item);
        bucket->next = create_linked_list_node(new_item, NULL);
        table->num_of_items++;
        if(table->num_of_items > table->array_size << 2) // if num of elements is 4 times as much as the capacity
        {
            resize_hash_table(table);
        }
        return 1;
    }
}



