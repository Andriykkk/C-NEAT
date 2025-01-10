#include "defs.h"
#include "neat.h"

#ifndef MUTATIONS_H
#define MUTATIONS_H

void add_edge_mutation(Genome *genome);
void add_node_mutation(Genome *genome, short disable_node, short only_active, short only_inactive);

void delete_node_mutation(Genome *genome, short delete_connected);
void delete_edge_mutation(Genome *genome);
void delete_edge(Genome *genome, int edge_index);

void shift_edge_mutation(Genome *genome);
void shift_node_mutation(Genome *genome);

#endif