#include "neat.h"
#include "defs.h"
#include "utils.h"
#include "mutations.h"
#include "stdbool.h"
#include "node.h"
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type)
{
    Node node;
    node.id = id;
    node.bias = bias;
    node.activationFunction = activationFunction;
    node.type = type;
    node.output = 0;
    node.edges = malloc(sizeof(Edge));
    return node;
}

Edge createEdge(int from, int to, float weight, bool enabled, int id)
{
    Edge edge;
    edge.from = from;
    edge.to = to;
    edge.weight = weight;
    edge.enabled = enabled;
    edge.id = id;
    return edge;
}

Genome createGenome(int inputs, int outputs, bool randomBias, bool randomWeights)
{
    Genome genome;
    genome.inputs = malloc(sizeof(int) * inputs);
    genome.outputs = malloc(sizeof(int) * outputs);
    genome.inputsCount = inputs;
    genome.outputsCount = outputs;
    genome.nodeCount = 0;
    genome.edgeCount = 0;
    genome.fitness = 0;

    genome.nodes = malloc(sizeof(Node) * NODES_LIMIT);
    genome.edges = malloc(sizeof(Edge) * EDGES_LIMIT);
    if (genome.nodes == NULL || genome.edges == NULL || genome.inputs == NULL || genome.outputs == NULL)
    {
        printf("Error allocating memory\n");
        exit(1);
    }

    for (int i = 0; i < inputs; i++)
    {
        float bias = randomBias ? get_random_numberf(-1, 1) : 1e-5;
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, bias, LINEAR, INPUT);
        genome.inputs[i] = genome.nodeCount - 1;
    }

    for (int i = 0; i < outputs; i++)
    {
        float bias = randomBias ? get_random_numberf(-1, 1) : 1e-5;
        genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, bias, LINEAR, OUTPUT);
        genome.outputs[i] = genome.nodeCount - 1;
    }

    for (int i = 0; i < inputs; i++)
    {
        for (int j = 0; j < outputs; j++)
        {
            float weight = randomWeights ? get_random_numberf(-1, 1) : 1e-5;
            genome.edges[genome.edgeCount++] = createEdge(genome.inputs[i], genome.outputs[j], weight, true, genome.edgeCount);
        }
    }

    fill_nodes_edges(&genome);

    return genome;
}

Genome copy_genome(Genome *original)
{
    Genome copy;
    copy.inputsCount = original->inputsCount;
    copy.outputsCount = original->outputsCount;
    copy.nodeCount = original->nodeCount;
    copy.edgeCount = original->edgeCount;
    copy.fitness = original->fitness;

    copy.nodes = malloc(sizeof(Node) * NODES_LIMIT);
    copy.edges = malloc(sizeof(Edge) * EDGES_LIMIT);
    copy.inputs = malloc(sizeof(int) * copy.inputsCount);
    copy.outputs = malloc(sizeof(int) * copy.outputsCount);

    for (int i = 0; i < copy.nodeCount; i++)
    {
        copy.nodes[i].id = original->nodes[i].id;
        copy.nodes[i].output = original->nodes[i].output;
        copy.nodes[i].bias = original->nodes[i].bias;
        copy.nodes[i].activationFunction = original->nodes[i].activationFunction;
        copy.nodes[i].type = original->nodes[i].type;
        copy.nodes[i].edgeCount = original->nodes[i].edgeCount;

        copy.nodes[i].edges = malloc(sizeof(Edge) * original->nodes[i].edgeCount);
        for (int j = 0; j < original->nodes[i].edgeCount; j++)
        {
            copy.nodes[i].edges[j] = original->nodes[i].edges[j];
        }
    }

    for (int i = 0; i < copy.edgeCount; i++)
    {
        copy.edges[i] = original->edges[i];
    }

    for (int i = 0; i < copy.inputsCount; i++)
    {
        copy.inputs[i] = original->inputs[i];
    }

    for (int i = 0; i < copy.outputsCount; i++)
    {
        copy.outputs[i] = original->outputs[i];
    }

    return copy;
}

void free_genome(Genome *genome)
{
    free(genome->inputs);
    free(genome->outputs);
    for (int i = 0; i < genome->nodeCount; i++)
    {
        free(genome->nodes[i].edges);
    }
    free(genome->nodes);
    free(genome->edges);
}

Population createPopulation(int genomesCount, int inputs, int outputs, bool randomBias, bool randomWeights)
{
    Population population;
    population.genomes = malloc(sizeof(Genome) * genomesCount);
    population.species = malloc(sizeof(int) * genomesCount);
    population.genomesCount = genomesCount;
    population.edge_innovation = inputs * outputs;
    population.node_innovation = inputs + outputs;
    for (int i = 0; i < genomesCount; i++)
    {
        population.genomes[i] = createGenome(inputs, outputs, randomBias, randomWeights);
    }
    return population;
}

void free_population(Population *population)
{
    for (int i = 0; i < population->genomesCount; i++)
    {
        free_genome(&population->genomes[i]);
    }
    free(population->species);
    free(population->genomes);
    // free(population);
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

    free(queue);
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

Genome combine_genomes(Genome *genome1, Genome *genome2)
{
    Genome *best_genome;
    Genome *worst_genome;

    if (genome1->fitness > genome2->fitness)
    {
        best_genome = genome2;
        worst_genome = genome1;
    }
    else
    {
        best_genome = genome1;
        worst_genome = genome2;
    }

    Genome new_genome = copy_genome(best_genome);

    // Randomly combine nodes
    for (int i = 0; i < new_genome.nodeCount; i++)
    {
        if (i < genome1->nodeCount && i < genome2->nodeCount)
        {
            new_genome.nodes[i] = (get_random_unsigned_int() % 3 == 0) ? worst_genome->nodes[i] : best_genome->nodes[i];
        }
        else if (i < genome1->nodeCount)
        {
            new_genome.nodes[i] = genome1->nodes[i];
        }
        else
        {
            new_genome.nodes[i] = genome2->nodes[i];
        }
    }

    // Randomly combine edges
    for (int i = 0; i < new_genome.edgeCount; i++)
    {
        if (i < genome1->edgeCount && i < genome2->edgeCount)
        {
            new_genome.edges[i] = (get_random_unsigned_int() % 3 == 0) ? worst_genome->edges[i] : best_genome->edges[i];
        }
        else if (i < genome1->edgeCount)
        {
            new_genome.edges[i] = genome1->edges[i];
        }
        else
        {
            new_genome.edges[i] = genome2->edges[i];
        }
    }

    return new_genome;
}

void fill_deleted_genomes(Population *population)
{
    for (int i = 0; i < population->genomesCount; i++)
    {
        if (population->genomes[i].fitness == -1)
        {
            unsigned int randomIndex1 = get_random_unsigned_int() % population->genomesCount;
            unsigned int randomIndex2 = get_random_unsigned_int() % population->genomesCount;
            unsigned int attempt = 0;

            while (attempt < 100 && randomIndex1 == randomIndex2 && (population->genomes[randomIndex1].fitness == -1 || population->genomes[randomIndex2].fitness == -1))
            {
                randomIndex1 = get_random_unsigned_int() % population->genomesCount;
                randomIndex2 = get_random_unsigned_int() % population->genomesCount;

                attempt++;
            }

            Genome *genome1 = &population->genomes[randomIndex1];
            Genome *genome2 = &population->genomes[randomIndex2];

            unsigned int copy_id = i;
            free_genome(&population->genomes[i]);

            population->genomes[i] = combine_genomes(genome1, genome2);
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

void clean_nodes_outputs(Genome *genome)
{
    for (int i = 0; i < genome->nodeCount; i++)
    {
        if (genome->nodes[i].type != REMEMBER)
        {
            genome->nodes[i].output = 0;
        }
    }
}

float calculate_genetic_distance(Genome *genome_a, Genome *genome_b)
{
    int disjoint = 0;
    int excess = 0;
    float weight_diff = 0;
    float bias_diff = 0;
    int matching_nodes = 0;
    int matching_edges = 0;

    // compare edges
    int max_edge_innovation = fmax(genome_a->edges[genome_a->edgeCount - 1].id, genome_b->edges[genome_b->edgeCount - 1].id);
    int i = 0, j = 0;
    while (i < genome_a->edgeCount && j < genome_b->edgeCount)
    {
        if (genome_a->edges[i].id < genome_b->edges[j].id)
        {
            disjoint++;
            i++;
            continue;
        }
        else if (genome_b->edges[j].id < genome_a->edges[i].id)
        {
            disjoint++;
            j++;
            continue;
        }
        else if (genome_a->edges[i].id == genome_b->edges[j].id && genome_a->edges[i].enabled && genome_b->edges[j].enabled)
        {
            matching_edges++;
            weight_diff += fabs(genome_a->edges[i].weight - genome_b->edges[j].weight);
            i++;
            j++;
            continue;
        }
        i++;
        j++;
    }

    while (i < genome_a->edgeCount)
    {
        excess++;
        i++;
    }

    while (j < genome_b->edgeCount)
    {
        excess++;
        j++;
    }

    // compare nodes
    int max_node_innovation = fmax(genome_a->nodes[genome_a->nodeCount - 1].id, genome_b->nodes[genome_b->nodeCount - 1].id);
    int m = 0, n = 0;
    while (m < genome_a->nodeCount && n < genome_b->nodeCount)
    {
        if (genome_a->nodes[m].id < genome_b->nodes[n].id)
        {
            disjoint++;
            m++;
            continue;
        }
        else if (genome_b->nodes[n].id < genome_a->nodes[m].id)
        {
            disjoint++;
            n++;
            continue;
        }
        else if (genome_a->nodes[m].id == genome_b->nodes[n].id)
        {
            matching_nodes++;
            bias_diff += fabs(genome_a->nodes[m].bias - genome_b->nodes[n].bias);
            if (genome_a->nodes[m].activationFunction != genome_b->nodes[n].activationFunction)
            {
                bias_diff += 1.0f;
            }
            m++;
            n++;
            continue;
        }
        m++;
        n++;
    }

    while (m < genome_a->nodeCount)
    {
        excess++;
        m++;
    }

    while (n < genome_b->nodeCount)
    {
        excess++;
        n++;
    }

    float normalized_distance = 0.0f;
    float node_normalizer = fmax(max_node_innovation, 1);
    float edge_normalizer = fmax(max_edge_innovation, 1);
    if (matching_nodes > 0 && matching_edges > 0)
    {
        normalized_distance = (disjoint + 2 * excess + weight_diff / matching_edges + bias_diff / matching_nodes) / (node_normalizer + edge_normalizer);
    }
    else
    {
        normalized_distance = (disjoint + 2 * excess + weight_diff + bias_diff) / (node_normalizer + edge_normalizer);
    }

    return normalized_distance;
}

void fill_nodes_edges(Genome *genome)
{
    for (int i = 0; i < genome->nodeCount; i++)
    {
        Node *node = &genome->nodes[i];
        node->edgeCount = 0;
        node->edges = realloc(node->edges, sizeof(Edge));
    }
    for (int i = 0; i < genome->edgeCount; i++)
    {
        if (genome->edges[i].enabled)
        {
            int from_id = genome->edges[i].from;
            Node *node = &genome->nodes[from_id];
            node->edges = realloc(node->edges, sizeof(Edge) * (node->edgeCount + 1));
            node->edges[node->edgeCount++] = genome->edges[i];
        }
    }
}

void empty_outputs(Genome *genome)
{
    for (int i = 0; i < genome->outputsCount; i++)
    {
        genome->nodes[genome->outputs[i]].output = 0;
    }
}

void empty_nodes(Genome *genome)
{
    for (int i = 0; i < genome->nodeCount; i++)
    {
        genome->nodes[i].output = 0;
    }
}

void empty_inputs(Genome *genome)
{
    for (int i = 0; i < genome->inputsCount; i++)
    {
        genome->nodes[genome->inputs[i]].output = 0;
    }
}

unsigned int *mutations_range;
unsigned int mutations_count = 9;

void init_mutation_range()
{
    mutations_range = malloc(sizeof(unsigned int) * (mutations_count + 1));
    unsigned int range = UINT_MAX / mutations_count + 1;

    mutations_range[0] = 0;
    mutations_range[mutations_count] = UINT_MAX;
    for (int i = 1; i < mutations_count; i++)
    {
        mutations_range[i] = range * i;
    }
}

void free_mutation_range()
{
    free(mutations_range);
}

void call_random_mutation(Population *population, int genome_index)
{
    unsigned int random_index = get_random_unsigned_int();

    if (random_index >= mutations_range[0] && random_index < mutations_range[1])
    {
        add_edge_mutation(&population->genomes[genome_index], population->edge_innovation++);
    }

    // add node mutations and disable original edge
    else if (random_index >= mutations_range[1] && random_index < mutations_range[2])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 0, 1, 0);
        population->edge_innovation += 2;
    }
    else if (random_index >= mutations_range[2] && random_index < mutations_range[3])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 0, 0, 1);
        population->edge_innovation += 2;
    }
    else if (random_index >= mutations_range[3] && random_index < mutations_range[4])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 0, 1, 1);
        population->edge_innovation += 2;
    }

    // add node mutations and dont disable original edge
    else if (random_index >= mutations_range[4] && random_index <= mutations_range[5])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 1, 1, 0);
        population->edge_innovation += 2;
    }
    else if (random_index >= mutations_range[5] && random_index <= mutations_range[6])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 1, 0, 1);
        population->edge_innovation += 2;
    }
    else if (random_index >= mutations_range[6] && random_index <= mutations_range[7])
    {
        add_node_mutation(&population->genomes[genome_index], population->edge_innovation, population->node_innovation++, 1, 1, 1);
        population->edge_innovation += 2;
    }

    // update
    else if (random_index >= mutations_range[7] && random_index <= mutations_range[8])
    {
        shift_edge_mutation(&population->genomes[genome_index]);
    }
    else if (random_index >= mutations_range[8] && random_index <= mutations_range[9])
    {
        shift_node_mutation(&population->genomes[genome_index]);
    }
}
