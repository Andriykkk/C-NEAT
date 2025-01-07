#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "utils.h"

// TODO: add mutation for activating and mutating edge and deactivating and mutating random node bias
// TODO: add node deleting mutation
// TODO: add creating new edges mutation
// TODO: add mutation for crossover of separately edges, nodes and weights
// TODO: add more different mutations
// TODO: make so that it look how much each mutation affects the fitness and randomly choose the best one

float sigmoidf(float x)
{
	return 1 / (1 + exp(-x));
}

float tanhf(float x)
{
	return (expf(x) - expf(-x)) / (expf(x) + expf(-x));
}

void add_edge_mutation(Genome *genome, int innovation)
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

		if (genome->nodes[from_node].type != OUTPUT || genome->nodes[to_node].type == INPUT)
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

float get_random_numberf(float min, float max)
{
	return min + (max - min) * ((float)rand() / RAND_MAX);
}

void add_node_mutation(Genome *genome, bool disable_node, bool only_active, bool only_inactive)
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

void feed_forward(Genome *genome, float *inputs)
{
}

int main()
{
	Genome genome = createGenome(4, 4);
	// save_genome(&genome, "test.txt");

	// Genome genome2 = load_genome("test.txt");
	printGenome(genome);

	printf("Hello World!, %d\n", genome.nodeCount);
	return 0;
}