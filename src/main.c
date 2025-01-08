#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "utils.h"

// TODO: add function to mutate genome
// TODO: add function that divide genomes(find genomic distance) into species and save them into struct
// TODO: add function to crossover genomes
// TODO: add function that divide genomes into species
// TODO: add mutation for activating and mutating edge and deactivating and mutating random node bias
// TODO: add node deleting mutation
// TODO: add creating new edges mutation
// TODO: add mutation for crossover of separately edges, nodes and weights
// TODO: make so that there always be way from input to output, even during mutations these connections cannot be deleted
// TODO: add more different mutations
// TODO: make so that it look how much each mutation affects the fitness and randomly choose the best one
// TODO: optimise edges and remove types as i dont need them

typedef struct
{
	Genome *genomes;
	int genomesCount;
} Population;

typedef struct
{
	int start;
	int end;
	char *parameters;
	int parametersCount;
	void (*func_ptr)();
} MutationFunction;

MutationFunction init_mutation_function(int start, int end, short *parameters, int parametersCount, void (*func_ptr)())
{
	MutationFunction mutation_function;
	mutation_function.start = start;
	mutation_function.end = end;
	mutation_function.parametersCount = parametersCount;
	mutation_function.parameters = malloc(sizeof(short) * parametersCount);
	for (int i = 0; i < parametersCount; i++)
	{
		mutation_function.parameters[i] = parameters[i];
	}

	mutation_function.func_ptr = func_ptr;
	return mutation_function;
}

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

void node_activation(Node node)
{
	switch (node.activationFunction)
	{
	case SIGMOID:
		node.output = sigmoidf(node.output);
		break;

	case TANH:
		node.output = tanhf(node.output);
		break;
	case LINEAR:
	}
}

void feed_forward(Genome *genome, float *inputs)
{
	int *queue = malloc(sizeof(int) * QUEUE_SIZE);
	int start_q = 0;
	int end_q = 0;

	int emptyOutputCount = genome->outputsCount;

	for (int i = 0; i < genome->inputsCount; i++)
	{
		int input_id = genome->inputs[i];
		genome->nodes[input_id].output = inputs[i];
		queue[end_q++] = input_id;
	}

	// work until all outputs are full
	while (emptyOutputCount > 0)
	{
		emptyOutputCount = genome->outputsCount;

		// forward pass
		for (int i = 0; i < genome->nodeCount; i++)
		{
			Node *current_node = &genome->nodes[queue[start_q++]];
			current_node->output += current_node->bias;

			for (int i = 0; i < current_node->edgeCount; i++)
			{
				Edge edge = current_node->edges[i];

				node_activation(*current_node);
				genome->nodes[edge.to].output += current_node->output * edge.weight;

				queue[end_q++] = edge.to;
			}
		}

		// check output
		for (int i = 0; i < genome->outputsCount; i++)
		{
			int output_id = genome->outputs[i];
			if (genome->nodes[output_id].output != 0)
			{
				emptyOutputCount--;
			}
		}
	}
}

void test_genome_fitness(Genome *genome, float *inputs, float *expected_outputs, int inputsCount)
{
	int count = genome->outputsCount * inputsCount;

	for (int i = 0; i < inputsCount; i++)
	{
		feed_forward(genome, inputs);
		for (int j = 0; j < genome->outputsCount; j++)
		{
			genome->fitness += pow(genome->nodes[genome->outputs[j]].output - expected_outputs[j], 2);
		}
	}

	genome->fitness /= count;
}

void test_feed_forward(Genome *genome, double seconds, float *inputs)
{
	clock_t start_time, end_time;
	double elapsed_time;
	int count = 0;
	int secondsCount = 0;

	start_time = clock();
	while (1)
	{
		feed_forward(genome, inputs);
		count++;

		end_time = clock();
		elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

		if (elapsed_time >= seconds)
		{
			break;
		}
	}

	printf("count: %d \n", count);
}

void mutate_genome(Genome *genome)
{
}

void call_mutation_function(MutationFunction *mutation_function, Genome *genome)
{
	switch (mutation_function->parametersCount)
	{
	case 0:
		mutation_function->func_ptr(genome);
	case 1:
		mutation_function->func_ptr(genome, mutation_function->parameters[0]);
	case 2:
		mutation_function->func_ptr(genome, mutation_function->parameters[0], mutation_function->parameters[1]);
	case 3:
		mutation_function->func_ptr(genome, mutation_function->parameters[0], mutation_function->parameters[1], mutation_function->parameters[2]);
	case 4:
		mutation_function->func_ptr(genome, mutation_function->parameters[0], mutation_function->parameters[1], mutation_function->parameters[2], mutation_function->parameters[3]);
	}
}

int main()
{
	// srand(time(NULL));
	MutationFunction *mutation_functions = malloc(sizeof(MutationFunction) * 5);
	mutation_functions[0] = init_mutation_function(0, 0, NULL, 0, add_edge_mutation);

	Genome genome = createGenome(2, 2, true, true);
	fill_nodes_edges(&genome);
	float inputs[4] = {1.7f, 1.0f};
	// feed_forward(&genome, inputs);

	// for (int i = 0; i < genome.outputsCount; i++)
	// {
	// 	printf("output %d: %f \n", i, genome.nodes[genome.outputs[i]].output);
	// }

	// save_genome(&genome, "test.txt");

	// Genome genome2 = load_genome("test.txt");
	// printGenome(genome);

	return 0;
}