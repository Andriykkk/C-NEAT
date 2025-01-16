#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <math.h>
#include "defs.h"
#include "neat.h"
#include "utils.h"
#include "node.h"
#include "mutations.h"
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include <fcntl.h>

// TODO: make bigram
// TODO: add combining genomes
// TODO: remake finishing condition, while loop will work as many times as genome wants, this also will be mutation
// TODO: add nodes biases to difference between species
// TODO: add function to crossover genomes https://www.youtube.com/watch?v=yVtdp1kF0I4&list=PL9WkKCom5t9_2VBiwERbNECPNJNPn2wkk&index=53
// TODO: add proper innovation number as it allow us to compare networks
// TODO: add more mutations
// 		edge: disable, enable, /delete/,
// 		node:  /delete/,
// 		crossover: nodes, edges
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
// TODO: add node deleting mutation
// TODO: optimise deleting, delete one, next move by one, then delete another so you should move by 2 and so on
// TODO: add creating new edges mutation
// TODO: add mutation for crossover of separately edges, nodes and weights
// TODO: make so that there always be way from input to output, even during mutations these connections cannot be deleted
// TODO: add more different mutations
// TODO: make so that it look how much each mutation affects the fitness and randomly choose the best one
// TODO: optimise edges and remove types as i dont need them

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
			call_random_mutation(population, i);
			fill_nodes_edges(&population->genomes[i]);
		}
	}
}

void randomly_mutate_population(Population *population, float mutate_threshold)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		if (get_random_numberf(0, 1) < mutate_threshold)
		{
			call_random_mutation(population, i);
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
}

int assign_to_species(Population *population, float threshold)
{
	int *species = population->species;
	for (int i = 0; i < population->genomesCount; i++)
	{
		species[i] = -1;
	}

	int species_count = 0;

	for (int i = 0; i < population->genomesCount; i++)
	{
		char assigned = false;
		for (int j = 0; j < species_count; j++)
		{
			if (species[j] != -1 && calculate_genetic_distance(&population->genomes[i], &population->genomes[species[j]]) < threshold)
			{
				species[i] = species[j];
				assigned = true;
				break;
			}
		}

		if (!assigned)
		{
			species[i] = species_count++;
		}
	}

	return species_count;
}

void delete_worst_in_species(Population *population, float threshold)
{
	int species_count = assign_to_species(population, SIMILAR_SPECIES_THRESHOLD);
	float population_fitness = 0;
	float *species_fitness = (float *)calloc(species_count, sizeof(float));
	int *species_size = (int *)calloc(species_count, sizeof(int));

	// average fitness for each species
	for (int i = 0; i < population->genomesCount; i++)
	{
		species_fitness[population->species[i]] += population->genomes[i].fitness;
		species_size[population->species[i]]++;
	}
	for (int i = 0; i < species_count; i++)
	{
		species_fitness[i] /= species_size[i];
	}

	// average fitness between species
	for (int i = 0; i < species_count; i++)
	{
		population_fitness += species_fitness[i];
	}
	population_fitness /= species_count;

	// randomly delete genomes
	for (int i = 0; i < population->genomesCount; i++)
	{
		float t = threshold;
		float s_fitness = species_fitness[population->species[i]];
		float g_fitness = population->genomes[i].fitness;

		if (s_fitness < population_fitness)
		{
			t /= fabsf(s_fitness - population_fitness);
		}
		else
		{
			t *= fabsf(s_fitness - population_fitness);
		}

		if (g_fitness < s_fitness)
		{
			t /= fabsf(s_fitness - g_fitness);
		}
		else
		{
			t *= fabsf(s_fitness - g_fitness);
		}

		t = fmax(0.0f, fmin(1.0f, t));
		// printf("fitness: %f threshold: %f\n", g_fitness, t);

		if (get_random_numberf(0.0, 1.0) < t)
		{
			// printf("copy fitness: %f threshold: %f\n", g_fitness, t);
			population->genomes[i].fitness = -1;
		}
	}

	free(species_fitness);
	free(species_size);
}

void fill_deleted_genomes(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		if (population->genomes[i].fitness == -1)
		{
			unsigned int attempt = 0;
			unsigned int copy_id = i;
			free_genome(&population->genomes[i]);

			while (attempt < 100)
			{
				copy_id = get_random_unsigned_int() % population->genomesCount;
				if (population->genomes[copy_id].fitness != -1)
					break;

				attempt++;
			}

			population->genomes[i] = copyGenome(&population->genomes[copy_id]);
		}
	}
}

void clean_genomes_fitness(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		population->genomes[i].fitness = 0;
	}
}

void clean_population_nodes_outputs(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		clean_nodes_outputs(&population->genomes[i]);
	}
}

int main()
{
	srand(time(NULL));
	init_mutation_range();

	Population population = createPopulation(2000, 2, 1, true, true);
	float xor_inputs[4][2] = {
		{0, 0},
		{0, 1},
		{1, 0},
		{1, 1}};
	float xor_outputs[4][1] = {{0}, {1}, {1}, {0}};
	for (int i = 0; i < 1000; i++)
	{

		test_population_fitness(&population, xor_inputs[0], xor_outputs[0], 4, 4);
		clean_population_nodes_outputs(&population);
		test_population_fitness(&population, xor_inputs[1], xor_outputs[1], 4, 4);
		clean_population_nodes_outputs(&population);
		test_population_fitness(&population, xor_inputs[2], xor_outputs[2], 4, 4);
		clean_population_nodes_outputs(&population);
		test_population_fitness(&population, xor_inputs[3], xor_outputs[3], 4, 4);
		clean_population_nodes_outputs(&population);
		if (i % 10 == 0)
		{
			int best_genome = 0;
			for (int j = 0; j < population.genomesCount; j++)
			{
				if (population.genomes[j].fitness < population.genomes[best_genome].fitness)
				{
					best_genome = j;
				}
			}
			printf("%f %d %d \n", population.genomes[best_genome].fitness, population.genomes[best_genome].edgeCount, population.genomes[best_genome].nodeCount);
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

	free_population(&population);
	free_mutation_range();

	return 0;
}