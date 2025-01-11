#include "defs.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "neat.h"

Genome load_genome(char *filename)
{
    FILE *file = fopen(filename, "r");

    int *inputs, *outputs;
    int inputsCount, outputsCount;
    fscanf(file, "%d\n", &inputsCount);
    inputs = malloc(inputsCount * sizeof(int));
    for (int i = 0; i < inputsCount; i++)
    {
        fscanf(file, "%d ", &inputs[i]);
    }

    fscanf(file, "%d\n", &outputsCount);
    outputs = malloc(outputsCount * sizeof(int));
    for (int i = 0; i < outputsCount; i++)
    {
        fscanf(file, "%d ", &outputs[i]);
    }

    Genome genome = createGenome(inputsCount, outputsCount, false, false);

    fscanf(file, "%d\n", &genome.nodeCount);
    for (int i = 0; i < genome.nodeCount; i++)
    {
        fscanf(file, "%d %f %d %d ", &genome.nodes[i].id, &genome.nodes[i].bias, (int *)&genome.nodes[i].activationFunction, (int *)&genome.nodes[i].type);
    }

    fscanf(file, "%d\n", &genome.edgeCount);
    for (int i = 0; i < genome.edgeCount; i++)
    {
        fscanf(file, "%d %d %f %d ", &genome.edges[i].from, &genome.edges[i].to, &genome.edges[i].weight, (int *)&genome.edges[i].enabled);
    }

    fclose(file);

    return genome;
}

void save_genome(Genome *genome, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    fprintf(file, "%d\n", genome->inputsCount);
    for (int i = 0; i < genome->inputsCount; i++)
    {
        fprintf(file, "%d ", genome->inputs[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "%d\n", genome->outputsCount);
    for (int i = 0; i < genome->outputsCount; i++)
    {
        fprintf(file, "%d ", genome->outputs[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "%d\n", genome->nodeCount);
    for (int i = 0; i < genome->nodeCount; i++)
    {
        fprintf(file, "%d %f %d %d ", genome->nodes[i].id, genome->nodes[i].bias, genome->nodes[i].activationFunction, genome->nodes[i].type);
    }
    fprintf(file, "\n");
    fprintf(file, "%d\n", genome->edgeCount);
    for (int i = 0; i < genome->edgeCount; i++)
    {
        fprintf(file, "%d %d %f %d ", genome->edges[i].from, genome->edges[i].to, genome->edges[i].weight, genome->edges[i].enabled);
    }
    fclose(file);
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

void print_genome(Genome genome)
{
    printf("Edged:\n");

    for (int i = 0; i < genome.edgeCount; i++)
    {
        printf("Edge %d: %d -> %d status: %s weight: %f\n", i, genome.edges[i].from, genome.edges[i].to, (genome.edges[i].enabled) ? "enabled" : "disabled", genome.edges[i].weight);
    }

    printf("Nodes:\n");

    for (int i = 0; i < genome.nodeCount; i++)
    {
        printf("Node: %d bias: %f\n", i, genome.nodes[i].bias);
    }

    printf("Inputs:\n");

    for (int i = 0; i < genome.inputsCount; i++)
    {
        printf("Node: %d\n", i);
    }

    printf("Outputs:\n");

    for (int i = 0; i < genome.outputsCount; i++)
    {
        printf("Node: %d\n", i);
    }
}

float get_random_numberf(float min, float max)
{
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

unsigned int get_random_unsigned_int()
{
    unsigned int random_value = 0;

    random_value = (rand() & 0xFFFF) << 16;
    random_value |= (rand() & 0xFFFF);

    return random_value;
}