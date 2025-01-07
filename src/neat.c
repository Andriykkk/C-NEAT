#include "neat.h"
#include "defs.h"
#include "stdbool.h"
#include <stdlib.h>

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type)
{
    Node node;
    node.id = id;
    node.bias = bias;
    node.activationFunction = activationFunction;
    node.type = type;
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

Genome createGenome(int inputs, int outputs)
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
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, INPUT);
        genome.inputs[i] = genome.nodeCount - 1;
    }

    for (int i = 0; i < outputs; i++)
    {
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, OUTPUT);
        genome.outputs[i] = genome.nodeCount - 1;
    }
    genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, HIDDEN);

    for (int i = 0; i < inputs; i++)
    {
        genome.edges[genome.edgeCount++] = createEdge(i, genome.nodeCount - 1, 0.5, true, genome.edgeCount - 1);
    }

    for (int i = genome.edgeCount; i < inputs + outputs; i++)
    {
        genome.edges[genome.edgeCount++] = createEdge(genome.nodeCount - 1, i, 0.5, true, genome.edgeCount - 1);
    }

    return genome;
}