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
void free_population(Population *population);
void feed_forward(Genome *genome, float *inputs);
void clean_nodes_outputs(Genome *genome);
float calculate_genetic_distance(Genome *genome_a, Genome *genome_b);
void free_genome(Genome *g);
void fill_nodes_edges(Genome *genome);
void init_mutation_range();
void free_mutation_range();
void call_random_mutation(Population *population, int genome_index);
void empty_outputs(Genome *genome);
void empty_nodes(Genome *genome);
void empty_inputs(Genome *genome);

#endif