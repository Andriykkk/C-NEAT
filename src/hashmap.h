#define TABLE_SIZE 256

typedef struct HashNode
{
    char key;
    int value;
    struct HashNode *next;
} HashNode;

typedef struct
{
    HashNode *table[TABLE_SIZE];
    int *values;
    int size;
} HashMap;

HashMap *create_map();
HashNode *create_node(char key, int value);
unsigned int create_hash(char key);
int map_insert(HashMap *map, char key, int value);
int map_find(HashMap *map, char key);