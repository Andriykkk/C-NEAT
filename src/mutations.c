#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "mutations.h"
#include "utils.h"

void add_edge_mutation(Genome *genome)
{
    if (genome->edgeCount >= EDGES_LIMIT)
        return;

    int to_node, from_node;
    bool valid = false;
    int attempts = 0;

    while (attempts < 100 && !valid)
    {
        to_node = rand() % genome->nodeCount;
        from_node = rand() % genome->nodeCount;

        if ((genome->nodes[from_node].type != OUTPUT || genome->nodes[to_node].type == INPUT) && to_node != from_node)
        {
            valid = true;

            for (int i = 0; i < genome->edgeCount; i++)
            {
                if (genome->edges[i].from == from_node && genome->edges[i].to == to_node)
                {
                    valid = false;
                    break;
                }
            }
        }

        attempts++;
    }

    if (valid)
    {
        float weight = (float)rand() / RAND_MAX * 2 - 1;
        genome->edges[genome->edgeCount++] = createEdge(from_node, to_node, weight, true, genome->edgeCount - 1);
    }
}

void add_node_mutation(Genome *genome, short disable_node, short only_active, short only_inactive)
{
    if (genome->nodeCount >= NODES_LIMIT)
        return;

    int edge_index = rand() % genome->edgeCount;

    int attempts = 100;
    if (only_active)
    {
        while (!genome->edges[edge_index].enabled && attempts > 0)
        {
            edge_index = rand() % genome->edgeCount;
            attempts--;
        }
    }
    else if (only_inactive)
    {
        while (genome->edges[edge_index].enabled && attempts > 0)
        {
            edge_index = rand() % genome->edgeCount;
            attempts--;
        }
    }

    if (attempts == 0)
    {
        return;
    }

    if (disable_node)
    {
        genome->edges[edge_index].enabled = false;
    }

    Node new_node = createNode(genome->nodeCount, get_random_numberf(-1, 1), LINEAR, HIDDEN);
    genome->nodes[genome->nodeCount++] = new_node;

    genome->edges[genome->edgeCount++] = createEdge(genome->edges[edge_index].from, new_node.id, get_random_numberf(-1, 1), true, genome->edgeCount - 1);
    genome->edges[genome->edgeCount++] = createEdge(new_node.id, genome->edges[edge_index].to, get_random_numberf(-1, 1), true, genome->edgeCount - 1);
}