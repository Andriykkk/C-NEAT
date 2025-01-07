#include <stdbool.h>

#ifndef DEFS_H
#define DEFS_H

#define NODES_LIMIT 1000
#define EDGES_LIMIT 10000

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
    HIDDEN
} NodeType;

typedef struct
{
    int id;
    float output;
    float bias;
    ActivationFunction activationFunction;
    NodeType type;
} Node;

typedef struct
{
    int from;
    int to;
    float weight;
    bool enabled;
    int innovation;
} Edge;

typedef struct
{
    Node *nodes;
    Edge *edges;
    Edge *sortedEdges;
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
    int genomeCount;
} Specie;

#endif