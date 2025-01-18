#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "utils.h"
#include "hashmap.h"
#include "node.h"
#include "mutations.h"
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include <fcntl.h>

// TODO: add nodes biases to difference between species
// TODO: watct at saved websites to make better neat
// TODO: add more mutations
// 		edge: disable, enable, /delete/,
// 		node:  /delete/,
// 		crossover: nodes, edges
// TODO: remake finishing condition, while loop will work as many times as genome wants, this also will be mutation
// TODO: add adjactable input size, mutation ranges, several mutations per generation, how many times loop work in one generation
// TODO: add backpropagation for each generation like in rnn
// TODO: deal with growing genomes, with big genomes it harder and harder to divide on species, add mutations that mean something, etc.
// TODO: make so that nodes divided on layers, so i could perform batch normalisation on those layers
// TODO: add proper function to find inactive edges, not randomly look aroung for inactive or save inactive
// TODO: add function that divide genomes into species
// TODO: add backpropagation
// TODO: make adjastable mutations range, look what works best in history and use only them
// TODO: add several mutations throught one generation as genome growth, also make it adjastable as i dont know which factor of log i should use
// TODO: create proper populations, where it look at the history of mutations and fitness, create genome childrens, check them and take best ones
// TODO: add mutation for activating and mutating edge and deactivating and mutating random node bias
// TODO: optimise deleting, delete one, next move by one, then delete another so you should move by 2 and so on
// TODO: add mutation for crossover of separately edges, nodes and weights
// TODO: make so that there always be way from input to output, even during mutations these connections cannot be deleted
// TODO: make so that it look how much each mutation affects the fitness and randomly choose the best one
// TODO: optimise edges and remove types as i dont need them

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
		empty_outputs(genome);
	}
}

void test_population_fitness(Population *population, float *inputs, float *expected_outputs, int inputsCount)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		test_genome_fitness(&population->genomes[i], inputs, expected_outputs, inputsCount);
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
}

#define MAX_NAME_LENGTH 20
#define MAX_NAMES 33000

void tokenize_name(const char *name)
{
	int i = 0;
	while (name[i] != '\0')
	{
		printf("Token for '%c': %d\n", name[i], i);
		i++;
	}
}

int read_names(char names[MAX_NAMES][MAX_NAME_LENGTH])
{
	FILE *file = fopen("names.txt", "r");
	if (file == NULL)
	{
		printf("Error opening file!\n");
		return 1;
	}
	int name_count = 0;

	for (int i = 0; i < MAX_NAMES; i++)
	{
		for (int j = 0; j < MAX_NAME_LENGTH; j++)
		{
			names[i][j] = 0;
		}
	}

	while (fgets(names[name_count], MAX_NAME_LENGTH, file) != NULL && name_count < MAX_NAMES)
	{
		names[name_count][strcspn(names[name_count], "\n")] = '\0';
		name_count++;
	}

	fclose(file);
	return name_count;
}

void get_random_values(int *tokens, float *embedding, int names_count, float *context, float *result)
{
	unsigned int random_name = get_random_unsigned_int() % names_count;
	int max_length = 0;
	int random_position;

	for (int i = 0; i < MAX_NAME_LENGTH; i++)
	{
		if (tokens[random_name * MAX_NAME_LENGTH + i] == 0)
		{
			break;
		}
		max_length = i;
	}

	random_position = get_random_unsigned_int() % max_length;

	for (int i = 0; i < 3; i++)
	{
		context[i] = 0;
	}

	for (int i = 0; i < 3; i++)
	{
		if (random_position < 3 - i)
			continue;
		context[i] = embedding[tokens[(random_name * MAX_NAME_LENGTH) + random_position - 3 + i]];
	}

	for (int i = 0; i < 26; i++)
	{
		result[i] = 0;
	}
	result[tokens[random_name * MAX_NAME_LENGTH + random_position]] = 1;
}

int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);
	srand(time(NULL));
	init_mutation_range();

	HashMap *hashmap = create_map();
	char names[MAX_NAMES][MAX_NAME_LENGTH];
	int *tokens = calloc(MAX_NAMES * MAX_NAME_LENGTH, sizeof(int));
	float embedding[26];
	float context[3];
	float result[26];
	int name_count = read_names(names);

	int id = 0;
	map_insert(hashmap, 0, id++);
	for (int i = 0; i < MAX_NAMES; i++)
	{
		for (int j = 0; j < MAX_NAME_LENGTH; j++)
		{
			if (map_insert(hashmap, names[i][j], id))
			{
				id++;
			}
		}
	}
	for (int i = 0; i < MAX_NAMES; i++)
	{
		for (int j = 0; j < MAX_NAME_LENGTH; j++)
		{
			tokens[i * MAX_NAME_LENGTH + j] = map_find(hashmap, names[i][j]);
		}
	}
	for (int i = 0; i < 26; i++)
	{
		embedding[i] = 1.0 / (float)i;
	}

	Population population = createPopulation(1024, 3, 26, true, true);

	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			get_random_values(tokens, embedding, name_count, context, result);
			test_population_fitness(&population, context, result, 4);
			clean_population_nodes_outputs(&population);
		}
		for (int j = 0; j < population.genomesCount; j++)
		{
			population.genomes[j].fitness /= 100;
		}
		if (i % 1 == 0)
		{
			int best_genome = 0;
			for (int j = 0; j < population.genomesCount; j++)
			{
				if (population.genomes[j].fitness < population.genomes[best_genome].fitness)
				{
					best_genome = j;
				}
			}
			printf("Index %d  %f %d %d \n", i, population.genomes[best_genome].fitness, population.genomes[best_genome].edgeCount, population.genomes[best_genome].nodeCount);
		}
		delete_worst_in_species(&population, DELETING_GENOME_THRESHOLD);
		fill_deleted_genomes(&population);
		clean_genomes_fitness(&population);
		randomly_mutate_population(&population, MUTATION_THRESHOLD);
		for (int j = 0; j < population.genomesCount; j++)
		{
			fill_nodes_edges(&population.genomes[j]);
		}
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