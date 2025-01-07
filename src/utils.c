#include "defs.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

    Genome genome = createGenome(inputsCount, outputsCount);

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

void printGenome(Genome genome)
{
    printf("Edged:\n");

    for (int i = 0; i < genome.edgeCount; i++)
    {
        printf("Edge %d: %d -> %d status: %s\n", i, genome.edges[i].from, genome.edges[i].to, (genome.edges[i].enabled) ? "enabled" : "disabled");
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
