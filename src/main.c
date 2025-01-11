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
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include <fcntl.h>

// TODO: after mutations need to remake edges for nodes
// TODO: add full saving and loading
// TODO: add neat for simple game
// TODO: make bigram
// TODO: after deleting remake innovation as right now it calculated from amount of nodes and edges
// TODO: remake finishing condition, while loop will work as many times as genome wants, this also will be mutation
// TODO: add function that divide genomes(find genomic distance) into species and save them into struct
// TODO: add function to crossover genomes
// TODO: add proper innovation number as it allow us to compare networks
// TODO: add more mutations
// edge: disable, enable, /delete/,
// node:  /delete/,
// crossover: nodes, edges
// TODO: add functions to save Population and load it
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
			free_genome(&population->genomes[i]);
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

void free_population(Population *population)
{
	for (int i = 0; i < population->genomesCount; i++)
	{
		free_genome(&population->genomes[i]);
	}

	free(population->genomes);
	// free(population);
}

#define WIDTH 20
#define HEIGHT 10

int kbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	newt.c_cc[VMIN] = 1;
	newt.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

void clear_screen()
{
	printf("\033[H\033[J");
}

// Function to draw the game board
void draw_board(int caret_pos, int ball_x, int ball_y)
{
	clear_screen();

	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			// Draw the wider caret, which is two characters wide ("##")
			if (y == HEIGHT - 1 && (x == caret_pos || x == caret_pos + 1))
			{
				printf("##");
				x++; // Skip the next cell because the caret is two characters wide
			}
			// Draw the ball
			else if (y == ball_y && x == ball_x)
			{
				printf("O");
			}
			else
			{
				printf(".");
			}
		}
		printf("\n");
	}
}

// Main game loop
int run_game()
{
	int caret_pos = WIDTH / 2;
	int ball_x = rand() % WIDTH;
	int ball_y = 0;
	int ball_speed = 1;		// Ball moves one step at a time
	int ball_direction = 1; // 1 means moving down, -1 means moving up
	int game_over = 0;
	int score = 0;
	int ball_movement_counter = 0; // Counter to slow down the ball

	while (!game_over)
	{
		// draw_board(caret_pos, ball_x, ball_y);

		// Ball movement (only move once every few frames)
		if (ball_movement_counter >= 5) // Control how often the ball moves
		{
			if (ball_direction == 1)
			{
				ball_y += ball_speed;
			}
			else
			{
				ball_y -= ball_speed;
			}

			// Ball bounce logic
			if (ball_y >= HEIGHT - 1)
			{
				// Ball hits the ground
				if (ball_x >= caret_pos && ball_x < caret_pos + 2)
				{
					ball_y = 0;
					ball_x = rand() % WIDTH;
					ball_direction = -1;
					score++;
				}
				else
				{
					game_over = 1; // Ball missed, game over
				}
			}
			else if (ball_y <= 0)
			{
				ball_direction = 1; // Ball hits the top and starts moving down
			}

			// Reset the counter for ball movement
			ball_movement_counter = 0;
		}
		else
		{
			ball_movement_counter++; // Increase the counter if the ball hasn't moved yet
		}

		// Check for user input to move caret
		// if (kbhit())
		// {
		// 	char ch = getchar();
		// 	if (ch == 'a' && caret_pos > 0)
		// 	{
		// 		caret_pos--;
		// 	}
		// 	else if (ch == 'd' && caret_pos < WIDTH - 2) // caret is two characters wide
		// 	{
		// 		caret_pos++;
		// 	}
		// }

		int move = rand() % 3;
		if (move == 0 && caret_pos > 0)
		{
			caret_pos--;
		}
		else if (move == 2 && caret_pos < WIDTH - 2) // caret is two characters wide
		{
			caret_pos++;
		}

		// usleep(100000); // Slow down the game loop, so the caret can be moved multiple times
	}

	// clear_screen();
	// printf("Game Over! You missed the ball. Score: %d\n", score);

	return score;
}

char *itoa(int num)
{
	static char buffer[20];
	snprintf(buffer, sizeof(buffer), "%d", num);
	return buffer;
}

int save_fully(Population population, char *name)
{
	struct stat st = {0};
	if (stat(name, &st) == -1)
	{

		if (mkdir(name, 0700))
		{
			perror("Error creating directory");
			return 1;
		}
	}

	char *filename = malloc(strlen(name) + strlen("/v0.0.1") + 1);
	if (!filename)
	{
		perror("Memory allocation failed");
		return 1;
	}

	snprintf(filename, strlen(name) + strlen("/v0.0.1") + 1, "%s/v0.0.1", name);

	if (stat(filename, &st) == -1)
	{
		if (mkdir(filename, 0700))
		{
			perror("Error creating directory");
			return 1;
		}
	}

	// save mutation range
	char *mutation_range_filename = malloc(strlen(filename) + strlen("/mutation_range") + 1);
	if (!mutation_range_filename)
	{
		perror("Memory allocation failed");
		return 1;
	}
	strcpy(mutation_range_filename, filename);
	strcat(mutation_range_filename, "/mutation_range.txt");

	FILE *mutation_range_file = fopen(mutation_range_filename, "w");
	fprintf(mutation_range_file, "%d\n", mutations_count);
	for (int i = 0; i < mutations_count + 1; i++)
	{
		fprintf(mutation_range_file, "%u ", mutations_range[i]);
	}
	fclose(mutation_range_file);
	free(mutation_range_filename);

	// save the population
	char *population_filename = malloc(strlen(filename) + strlen("/population") + 1);
	if (!population_filename)
	{
		perror("Memory allocation failed");
		return 1;
	}
	strcpy(population_filename, filename);
	strcat(population_filename, "/population.txt");

	FILE *population_file = fopen(population_filename, "w");
	fprintf(population_file, "%d\n", population.genomesCount);
	fclose(population_file);
	free(population_filename);
	for (int i = 0; i < population.genomesCount; i++)
	{
		char *genomes_filename = malloc(strlen(filename) + 100);
		if (!genomes_filename)
		{
			perror("Memory allocation failed");
			return 1;
		}
		strcpy(genomes_filename, filename);
		strcat(genomes_filename, "/genome-");
		strcat(genomes_filename, itoa(i));
		strcat(genomes_filename, ".txt");
		save_genome(&population.genomes[i], genomes_filename);
		free(genomes_filename);
	}

	return 0;
}

Population load_fully(char *name)
{

	struct stat st = {0};
	if (stat(name, &st) == -1)
	{
		perror("Directory does not exist");
		exit(1);
	}

	char *filename = malloc(strlen(name) + strlen("/v0.0.1") + 1);
	if (!filename)
	{
		perror("Memory allocation failed");
		exit(1);
	}
	snprintf(filename, strlen(name) + strlen("/v0.0.1") + 1, "%s/v0.0.1", name);
	if (stat(filename, &st) == -1)
	{
		perror("Version directory does not exist");
		free(filename);
		exit(1);
	}

	// load mutation range
	char *mutation_range_filename = malloc(strlen(filename) + strlen("/mutation_range.txt") + 1);
	if (!mutation_range_filename)
	{
		perror("Memory allocation failed");
		free(filename);
		exit(1);
	}
	strcpy(mutation_range_filename, filename);
	strcat(mutation_range_filename, "/mutation_range.txt");

	FILE *mutation_range_file = fopen(mutation_range_filename, "r");
	if (!mutation_range_file)
	{
		perror("Error opening mutation range file");
		free(mutation_range_filename);
		free(filename);
		exit(1);
	}

	int mutations_count;
	fscanf(mutation_range_file, "%d\n", &mutations_count);

	unsigned int *mutations_range = malloc((mutations_count + 1) * sizeof(unsigned int));
	for (int i = 0; i < mutations_count + 1; i++)
	{
		fscanf(mutation_range_file, "%u", &mutations_range[i]);
	}
	fclose(mutation_range_file);
	free(mutation_range_filename);

	// Load population data
	Population population;

	char *population_filename = malloc(strlen(filename) + strlen("/population.txt") + 1);
	if (!population_filename)
	{
		perror("Memory allocation failed");
		free(filename);
		free(mutations_range);
		exit(1);
	}
	strcpy(population_filename, filename);
	strcat(population_filename, "/population.txt");

	FILE *population_file = fopen(population_filename, "r");
	if (!population_file)
	{
		perror("Error opening population file");
		free(population_filename);
		free(filename);
		free(mutations_range);
		exit(1);
	}

	fscanf(population_file, "%d", &population.genomesCount);
	fclose(population_file);
	free(population_filename);

	// Load genomes
	population.genomes = malloc(population.genomesCount * sizeof(Genome));

	for (int i = 0; i < population.genomesCount; i++)
	{
		char *genomes_filename = malloc(strlen(filename) + 100);
		if (!genomes_filename)
		{
			perror("Memory allocation failed");
			free(filename);
			free(mutations_range);
			exit(1);
		}
		strcpy(genomes_filename, filename);
		strcat(genomes_filename, "/genome-");

		char str[20];
		sprintf(str, "%d", i);
		strcat(genomes_filename, str);
		strcat(genomes_filename, ".txt");

		// Load the genome from the file
		population.genomes[i] = load_genome(genomes_filename);
		free(genomes_filename);
	}

	free(filename);
	free(mutations_range);

	return population;
}

int main()
{
	srand(time(NULL));
	init_mutation_range();

	Population population = load_fully("test");

	run_game();

	// Population population = createPopulation(3, 1, 3, true, true);
	printf("Created population %f\n", population.genomes[0].edges[0].weight);
	// if (save_fully(population, "test"))
	// {
	// 	printf("Error saving population\n");
	// 	return 1;
	// }

	// for (int i = 0; i < 10000; i++)
	// {
	// 	test_population_fitness(&population, xor_inputs[0], xor_outputs[0], 4, 4);
	// 	test_population_fitness(&population, xor_inputs[1], xor_outputs[1], 4, 4);
	// 	test_population_fitness(&population, xor_inputs[2], xor_outputs[2], 4, 4);
	// 	test_population_fitness(&population, xor_inputs[3], xor_outputs[3], 4, 4);
	// 	if (i % 1000 == 0)
	// 	{
	// 		printf("%f %d %d \n", population.genomes[0].fitness, population.genomes[0].edgeCount, population.genomes[0].nodeCount);
	// 	}
	// 	copy_best_and_mutate_genome(&population);
	// }

	// free_population(&population);
	// free_mutation_range();

	return 0;
}