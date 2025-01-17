#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>

HashMap *create_map()
{
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    *map->table = (HashNode *)malloc(sizeof(HashNode *) * TABLE_SIZE);
    map->values = (int *)calloc(TABLE_SIZE, sizeof(int));
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        map->table[i] = NULL;
    }
    return map;
}

HashNode *create_node(char key, int value)
{
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

unsigned int create_hash(char key)
{
    return (unsigned int)key % TABLE_SIZE;
}

int map_insert(HashMap *map, char key, int value)
{
    if (map_find(map, key) != -1)
        return 0;
    unsigned int index = create_hash(key);
    HashNode *new_node = create_node(key, value);

    if (map->size >= TABLE_SIZE)
    {
        map->values = (int *)realloc(map->values, sizeof(int) * (map->size * 2));
    }
    map->values[map->size++] = index;

    if (map->table[index] == NULL)
    {
        map->table[index] = new_node;
    }
    else
    {
        new_node->next = map->table[index];
        map->table[index] = new_node;
    }
    return 1;
}

int map_find(HashMap *map, char key)
{
    unsigned int index = create_hash(key);
    HashNode *current = map->table[index];

    while (current != NULL)
    {
        if (current->key == key)
        {
            return current->value;
        }
        current = current->next;
    }

    return -1;
}