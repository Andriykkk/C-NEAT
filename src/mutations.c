#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "mutations.h"
#include "utils.h"

int *find_disabled_edges(Genome *genome)
{
    int *disabled_edges = malloc(genome->edgeCount / 100);
    int index = 0;

    for (int i = 0; i < genome->edgeCount; i++)
    {
        if (!genome->edges[i].enabled)
        {
            disabled_edges[index++] = i;
        }
        if (i >= genome->edgeCount / 100)
        {
            break;
        }
    }

    return disabled_edges;
}

void add_edge_mutation(Genome *genome, int innovation)
{
    if (genome->edgeCount >= EDGES_LIMIT)
        return;

    unsigned int to_node, from_node;
    bool valid = false;
    int attempts = 0;

    while (attempts < 100 && !valid)
    {
        to_node = get_random_unsigned_int() % genome->nodeCount;
        from_node = get_random_unsigned_int() % genome->nodeCount;

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
        float weight = (float)(rand() / RAND_MAX) * 2 - 1;
        genome->edges[genome->edgeCount++] = createEdge(from_node, to_node, weight, true, innovation);
    }
}

void add_node_mutation(Genome *genome, int edge_innovation, int node_innovation, short disable_node, short only_active, short only_inactive)
{
    if (genome->nodeCount >= NODES_LIMIT)
        return;

    unsigned int edge_index = 0;

    if (only_active == only_inactive)
    {
        edge_index = get_random_unsigned_int() % genome->edgeCount;
        goto add_node;
    }

    int attempts = 100;
    if (only_active)
    {
        while (!genome->edges[edge_index].enabled && attempts > 0)
        {
            edge_index = get_random_unsigned_int() % genome->edgeCount;
            attempts--;
        }
    }
    if (only_inactive)
    {
        int *disabled_edges = find_disabled_edges(genome);
        int disabled_edges_count = genome->edgeCount / 100;
        if (disabled_edges_count == 0)
            return;
        edge_index = disabled_edges[get_random_unsigned_int() % disabled_edges_count];
    }

    if (attempts == 0)
    {
        return;
    }

add_node:

    if (disable_node)
    {
        genome->edges[edge_index].enabled = false;
    }

    Node new_node = createNode(node_innovation, get_random_numberf(-1, 1), LINEAR, HIDDEN);
    genome->nodes[genome->nodeCount++] = new_node;

    genome->edges[genome->edgeCount++] = createEdge(genome->edges[edge_index].from, genome->nodeCount - 1, get_random_numberf(-1, 1), true, edge_innovation++);
    genome->edges[genome->edgeCount++] = createEdge(genome->nodeCount - 1, genome->edges[edge_index].to, get_random_numberf(-1, 1), true, edge_innovation);
}

void delete_edge_mutation(Genome *genome)
{
    int edge_index = get_random_unsigned_int() % genome->edgeCount;

    delete_edge(genome, edge_index);
}

void delete_edge(Genome *genome, int edge_index)
{
    for (int i = edge_index; i < genome->edgeCount; ++i)
    {
        genome->edges[i] = genome->edges[i + 1];
    }

    genome->edgeCount--;
}

void delete_node_mutation(Genome *genome, short delete_connected)
{
    unsigned int node_id_to_delete = get_random_unsigned_int() % genome->nodeCount;

    if (delete_connected)
    {
        for (int i = 0; i < genome->edgeCount; ++i)
        {
            if (genome->edges[i].from == node_id_to_delete || genome->edges[i].to == node_id_to_delete)
            {
                delete_edge(genome, i);
                genome->edgeCount--;
            }
        }
    }
    else
    {
        int is_connected = 0;
        for (int i = 0; i < genome->edgeCount; ++i)
        {
            if (genome->edges[i].from == node_id_to_delete || genome->edges[i].to == node_id_to_delete)
            {
                is_connected = 1;
                break;
            }
        }

        if (!is_connected)
        {
            return;
        }
    }

    genome->nodeCount--;
    for (int i = node_id_to_delete; i < genome->nodeCount; ++i)
    {
        genome->nodes[i] = genome->nodes[i + 1];
    }
}

void shift_node_mutation(Genome *genome)
{
    int node_id = get_random_unsigned_int() % genome->nodeCount;
    float bias = get_random_numberf(-0.1, 0.1);

    genome->nodes[node_id].bias += bias;
}

void shift_edge_mutation(Genome *genome)
{
    int edge_index = get_random_unsigned_int() % genome->edgeCount;
    float weight = get_random_numberf(-0.1, 0.1);

    genome->edges[edge_index].weight += weight;
}