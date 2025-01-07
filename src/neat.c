#include "neat.h"
#include "defs.h"
#include "utils.h"
#include "stdbool.h"
#include <stdlib.h>

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type)
{
    Node node;
    node.id = id;
    node.bias = bias;
    node.activationFunction = activationFunction;
    node.type = type;
    node.output = 0;
    node.edges = malloc(sizeof(Edge));
    return node;
}

Edge createEdge(int from, int to, float weight, bool enabled, int innovation)
{
    Edge edge;
    edge.from = from;
    edge.to = to;
    edge.weight = weight;
    edge.enabled = enabled;
    edge.innovation = innovation;
    return edge;
}

Genome createGenome(int inputs, int outputs, bool randomBias, bool randomWeights)
{
    Genome genome;
    genome.inputs = malloc(sizeof(int) * inputs);
    genome.outputs = malloc(sizeof(int) * outputs);
    genome.inputsCount = inputs;
    genome.outputsCount = outputs;
    genome.nodeCount = 0;
    genome.edgeCount = 0;
    genome.fitness = 0;

    genome.nodes = malloc(sizeof(Node) * NODES_LIMIT);
    genome.edges = malloc(sizeof(Edge) * EDGES_LIMIT);
    genome.sortedEdges = malloc(sizeof(Edge) * EDGES_LIMIT);

    for (int i = 0; i < inputs; i++)
    {
        float bias = randomBias ? get_random_numberf(-1, 1) : 1e-5;
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, bias, LINEAR, INPUT);
        genome.inputs[i] = genome.nodeCount - 1;
    }

    for (int i = 0; i < outputs; i++)
    {
        float bias = randomBias ? get_random_numberf(-1, 1) : 1e-5;
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, bias, LINEAR, OUTPUT);
        genome.outputs[i] = genome.nodeCount - 1;
    }

    for (int i = 0; i < inputs; i++)
    {
        for (int j = 0; j < outputs; j++)
        {
            float weight = randomWeights ? get_random_numberf(-1, 1) : 1e-5;
            genome.edges[genome.edgeCount++] = createEdge(genome.inputs[i], genome.outputs[j], weight, true, genome.edgeCount - 1);
        }
    }

    return genome;
}

void fill_nodes_edges(Genome *genome)
{
    for (int i = 0; i < genome->nodeCount; i++)
    {
        Node *node = &genome->nodes[i];
        node->edgeCount = 0;
        node->edges = realloc(node->edges, sizeof(Edge));
    }
    for (int i = 0; i < genome->edgeCount; i++)
    {
        if (genome->edges[i].enabled)
        {
            int from_id = genome->edges[i].from;
            Node *node = &genome->nodes[from_id];
            node->edges = realloc(node->edges, sizeof(Edge) * (node->edgeCount + 1));
            node->edges[node->edgeCount++] = genome->edges[i];
        }
    }
}
