#include "defs.h"
#include "neat.h"

#ifndef MUTATIONS_H
#define MUTATIONS_H

void add_edge_mutation(Genome *genome, int innovation);
void add_node_mutation(Genome *genome, int edge_innovation, int node_innovation, short disable_node, short only_active, short only_inactive);

void delete_node_mutation(Genome *genome, short delete_connected);
void delete_edge_mutation(Genome *genome);
void delete_edge(Genome *genome, int edge_index);
int find_disabled_edges(Genome *genome, int *disabled_edges);

void shift_edge_mutation(Genome *genome);
void shift_node_mutation(Genome *genome);

#endif