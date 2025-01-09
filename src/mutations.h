#include "defs.h"
#include "neat.h"

#ifndef MUTATIONS_H
#define MUTATIONS_H

void add_edge_mutation(Genome *genome);
void add_node_mutation(Genome *genome, short disable_node, short only_active, short only_inactive);

#endif