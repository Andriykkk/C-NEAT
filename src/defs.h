#include <stdbool.h>

#ifndef DEFS_H
#define DEFS_H

#define NODES_LIMIT 100
#define EDGES_LIMIT 10000
#define QUEUE_SIZE 10000

#define DELETING_GENOME_THRESHOLD 0.8
#define SIMILAR_SPECIES_THRESHOLD 0.5
#define MUTATION_THRESHOLD 0.8

typedef enum
{
    LINEAR,
    SIGMOID,
    TANH
} ActivationFunction;

typedef enum
{
    INPUT,
    OUTPUT,
    HIDDEN,
    REMEMBER
} NodeType;

typedef struct
{
    int id;
    int from;
    int to;
    float weight;
    bool enabled;
} Edge;

typedef struct
{
    int id;
    float output;
    float bias;
    ActivationFunction activationFunction;
    NodeType type;

    Edge *edges;
    int edgeCount;
} Node;

typedef struct
{
    Node *nodes;
    Edge *edges;
    int *inputs;
    int *outputs;
    int inputsCount;
    int outputsCount;
    int nodeCount;
    int edgeCount;
    float fitness;
} Genome;

typedef struct
{
    Genome *genomes;
    int *species;
    int genomesCount;
    int edge_innovation;
    int node_innovation;
} Population;

typedef struct
{
    Genome *genomes;
    int genomeCount;
} Specie;

#endif