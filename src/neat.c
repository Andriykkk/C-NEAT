#include "neat.h"
#include "defs.h"
#include "utils.h"
#include "mutations.h"
#include "stdbool.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

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

    fill_nodes_edges(&genome);

    return genome;
}

Genome copyGenome(Genome *original)
{
    Genome copy;
    copy.inputsCount = original->inputsCount;
    copy.outputsCount = original->outputsCount;
    copy.nodeCount = original->nodeCount;
    copy.edgeCount = original->edgeCount;
    copy.fitness = original->fitness;

    copy.nodes = malloc(sizeof(Node) * NODES_LIMIT);
    copy.edges = malloc(sizeof(Edge) * EDGES_LIMIT);
    copy.inputs = malloc(sizeof(int) * copy.inputsCount);
    copy.outputs = malloc(sizeof(int) * copy.outputsCount);

    for (int i = 0; i < copy.nodeCount; i++)
    {
        copy.nodes[i].id = original->nodes[i].id;
        copy.nodes[i].output = original->nodes[i].output;
        copy.nodes[i].bias = original->nodes[i].bias;
        copy.nodes[i].activationFunction = original->nodes[i].activationFunction;
        copy.nodes[i].type = original->nodes[i].type;
        copy.nodes[i].edgeCount = original->nodes[i].edgeCount;

        copy.nodes[i].edges = malloc(sizeof(Edge) * original->nodes[i].edgeCount);
        for (int j = 0; j < original->nodes[i].edgeCount; j++)
        {
            copy.nodes[i].edges[j] = original->nodes[i].edges[j];
        }
    }

    for (int i = 0; i < copy.edgeCount; i++)
    {
        copy.edges[i] = original->edges[i];
    }

    for (int i = 0; i < copy.inputsCount; i++)
    {
        copy.inputs[i] = original->inputs[i];
    }

    for (int i = 0; i < copy.outputsCount; i++)
    {
        copy.outputs[i] = original->outputs[i];
    }

    return copy;
}

void free_genome(Genome *genome)
{
    free(genome->inputs);
    free(genome->outputs);
    for (int i = 0; i < genome->nodeCount; i++)
    {
        free(genome->nodes[i].edges);
    }
    free(genome->nodes);
    free(genome->edges);
}

Population createPopulation(int genomesCount, int inputs, int outputs, bool randomBias, bool randomWeights)
{
    Population population;
    population.genomes = malloc(sizeof(Genome) * genomesCount);
    population.genomesCount = genomesCount;
    for (int i = 0; i < genomesCount; i++)
    {
        population.genomes[i] = createGenome(inputs, outputs, randomBias, randomWeights);
    }
    return population;
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

unsigned int *mutations_range;
unsigned int mutations_count = 9;

void init_mutation_range()
{
    mutations_range = malloc(sizeof(unsigned int) * (mutations_count + 1));
    unsigned int range = UINT_MAX / mutations_count + 1;

    mutations_range[0] = 0;
    mutations_range[mutations_count] = UINT_MAX;
    for (int i = 1; i < mutations_count; i++)
    {
        mutations_range[i] = range * i;
    }
}

void free_mutation_range()
{
    free(mutations_range);
}

void call_random_mutation(Genome *genome)
{
    unsigned int random_index = get_random_unsigned_int();

    if (random_index >= mutations_range[0] && random_index < mutations_range[1])
    {
        add_edge_mutation(genome);
    }

    // add node mutations and disable original edge
    else if (random_index >= mutations_range[1] && random_index < mutations_range[2])
    {
        add_node_mutation(genome, 0, 1, 0);
    }
    else if (random_index >= mutations_range[2] && random_index < mutations_range[3])
    {
        add_node_mutation(genome, 0, 0, 1);
    }
    else if (random_index >= mutations_range[3] && random_index < mutations_range[4])
    {
        add_node_mutation(genome, 0, 1, 1);
    }

    // // add node mutations and dont disable original edge
    else if (random_index >= mutations_range[4] && random_index <= mutations_range[5])
    {
        add_node_mutation(genome, 1, 1, 0);
    }
    else if (random_index >= mutations_range[5] && random_index <= mutations_range[6])
    {
        add_node_mutation(genome, 1, 0, 1);
    }
    else if (random_index >= mutations_range[6] && random_index <= mutations_range[7])
    {
        add_node_mutation(genome, 1, 1, 1);
    }

    // update
    else if (random_index >= mutations_range[7] && random_index <= mutations_range[8])
    {
        shift_edge_mutation(genome);
    }
    else if (random_index >= mutations_range[8] && random_index <= mutations_range[9])
    {
        shift_node_mutation(genome);
    }
}
