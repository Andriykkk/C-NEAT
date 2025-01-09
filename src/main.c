#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "utils.h"
#include "mutations.h"

// TODO: add function that divide genomes(find genomic distance) into species and save them into struct
// TODO: add function to crossover genomes
// TODO: add proper function to find inactive edges, not randomly look aroung for inactive or save inactive
// TODO: add function that divide genomes into species
// TODO: add backpropagation
// TODO: create proper populations, where it look at the history of mutations and fitness, create genome childrens, check them and take best ones
// TODO: add mutation for activating and mutating edge and deactivating and mutating random node bias
// TODO: add node deleting mutation
// TODO: add creating new edges mutation
// TODO: add mutation for crossover of separately edges, nodes and weights
// TODO: make so that there always be way from input to output, even during mutations these connections cannot be deleted
// TODO: add more different mutations
// TODO: make so that it look how much each mutation affects the fitness and randomly choose the best one
// TODO: optimise edges and remove types as i dont need them

float sigmoidf(float x)
{
	return 1 / (1 + exp(-x));
}

float tanhf(float x)
{
	return (expf(x) - expf(-x)) / (expf(x) + expf(-x));
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

	free(queue);
}

void empty_outputs(Genome *genome)
{
	for (int i = 0; i < genome->outputsCount; i++)
	{
		genome->nodes[genome->outputs[i]].output = 0;
	}
}

void empty_inputs(Genome *genome)
{
	for (int i = 0; i < genome->inputsCount; i++)
	{
		genome->nodes[genome->inputs[i]].output = 0;
	}
}

void test_genome_fitness(Genome *genome, float *inputs, float *expected_outputs, int inputsCount, int batchSize)
{
	int count = genome->outputsCount * inputsCount;

	for (int i = 0; i < inputsCount; i++)
	{
		feed_forward(genome, inputs);
		for (int j = 0; j < genome->outputsCount; j++)
		{
			genome->fitness += pow(genome->nodes[genome->outputs[j]].output - expected_outputs[j], 2);
		}
		empty_outputs(genome);
	}

	genome->fitness /= (count * batchSize);
}

void test_population_fitness(Population *population, float *inputs, float *expected_outputs, int inputsCount, int batchSize)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		test_genome_fitness(&population->genomes[i], inputs, expected_outputs, inputsCount, batchSize);
	}
}

void copy_best_and_mutate_genome(Population *population)
{
	int min_fitness_id = 0;
	float min_fitness = population->genomes[0].fitness;
	for (int i = 0; i < population->genomesCount; i++)
	{
		if (population->genomes[i].fitness < min_fitness)
		{
			min_fitness = population->genomes[i].fitness;
			min_fitness_id = i;
		}
	}

	for (int i = 0; i < population->genomesCount; i++)
	{
		if (i != min_fitness_id)
		{
			freeGenome(&population->genomes[i]);
			population->genomes[i] = copyGenome(&population->genomes[min_fitness_id]);
			call_random_mutation(&population->genomes[i]);
		}
	}
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

void freePopulation(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		freeGenome(&population->genomes[i]);
	}

	free(population->genomes);
	// free(population);
}

int main()
{
	srand(time(NULL));
	init_mutation_range();

	Population population = createPopulation(2, 2, 1, true, true);
	float xor_inputs[4][2] = {
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1}};
	float xor_outputs[4][1] = {{0}, {1}, {1}, {0}};

	for (int i = 0; i < 100000; i++)
	{
		test_population_fitness(&population, xor_inputs[0], xor_outputs[0], 4, 4);
		test_population_fitness(&population, xor_inputs[1], xor_outputs[1], 4, 4);
		test_population_fitness(&population, xor_inputs[2], xor_outputs[2], 4, 4);
		test_population_fitness(&population, xor_inputs[3], xor_outputs[3], 4, 4);
		if (i % 10000 == 0)
		{
			printf("%f %d %d \n", population.genomes[0].fitness, population.genomes[0].edgeCount, population.genomes[0].nodeCount);
		}
		copy_best_and_mutate_genome(&population);
	}

	freePopulation(&population);
	free_mutation_range();
	// Genome genome = createGenome(2, 2, true, true);
	// fill_nodes_edges(&genome);

	// float inputs[4] = {1.7f, 1.0f};
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