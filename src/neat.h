#include "defs.h"

#ifndef NEAT_H
#define NEAT_H

extern unsigned int *mutations_range;
extern unsigned int mutations_count;

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type);
Edge createEdge(int from, int to, float weight, bool enabled, int innovation);
Genome createGenome(int inputs, int outputs, bool randomBias, bool randomWeights);
Genome copyGenome(Genome *original);
Population createPopulation(int genomesCount, int inputs, int outputs, bool randomBias, bool randomWeights);
void free_genome(Genome *g);
void fill_nodes_edges(Genome *genome);
void init_mutation_range();
void free_mutation_range();
void call_random_mutation(Genome *genome);

#endif