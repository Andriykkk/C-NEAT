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
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include <fcntl.h>

// TODO: make bigram
// TODO: clean nodes, but do this accordint to type, if it memory dont clean esle clean
// TODO: after deleting remake innovation as right now it calculated from amount of nodes and edges
// TODO: remake finishing condition, while loop will work as many times as genome wants, this also will be mutation
// TODO: add function that divide genomes(find genomic distance) into species and save them into struct
// TODO: add function to crossover genomes https://www.youtube.com/watch?v=yVtdp1kF0I4&list=PL9WkKCom5t9_2VBiwERbNECPNJNPn2wkk&index=53
// TODO: add proper innovation number as it allow us to compare networks
// TODO: add more mutations
// 		edge: disable, enable, /delete/,
// 		node:  /delete/,
// 		crossover: nodes, edges
// TODO: add adjactable input size, mutation ranges, several mutations per generation, how many times loop work in one generation
// TODO: add backpropagation for each generation like in rnn
// TODO: make so that nodes divided on layers, so i could perform batch normalisation on those layers
// TODO: add proper function to find inactive edges, not randomly look aroung for inactive or save inactive
// TODO: add function that divide genomes into species
// TODO: add backpropagation
// TODO: make adjastable mutations range, look what works best in history and use only them
// TODO: add several mutations throught one generation as genome growth, also make it adjastable as i dont know which factor of log i should use
// TODO: create proper populations, where it look at the history of mutations and fitness, create genome childrens, check them and take best ones
// TODO: add mutation for activating and mutating edge and deactivating and mutating random node bias
// TODO: add node deleting mutation
// TODO: optimise deleting, delete one, next move by one, then delete another so you should move by 2 and so on
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
	// while (emptyOutputCount > 0)
	// {
	// 	emptyOutputCount = genome->outputsCount;

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
	// 	for (int i = 0; i < genome->outputsCount; i++)
	// 	{
	// 		int output_id = genome->outputs[i];
	// 		if (genome->nodes[output_id].output != 0)
	// 		{
	// 			emptyOutputCount--;
	// 		}
	// 	}
	// }

	free(queue);
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
			free_genome(&population->genomes[i]);
			population->genomes[i] = copyGenome(&population->genomes[min_fitness_id]);
			call_random_mutation(&population->genomes[i]);
			fill_nodes_edges(&population->genomes[i]);
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

void free_population(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		free_genome(&population->genomes[i]);
	}

	free(population->genomes);
	// free(population);
}

int main()
{
	srand(time(NULL));
	init_mutation_range();

	Population population = createPopulation(1000, 1, 3, true, true);
	int max_score = 0;

	for (int i = 0; i < 500; i++)
	{

		copy_best_and_mutate_genome(&population);
	}

	// Population population = load_fully("test");
	// if (save_fully(population, "test"))
	// {
	// 	printf("Error saving population\n");
	// 	return 1;
	// }

	// free_population(&population);
	// free_mutation_range();

	return 0;
}