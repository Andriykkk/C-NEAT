#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define NODES_LIMIT 1000
#define EDGES_LIMIT 10000

typedef enum
{
	LINEAR,
	SIGMOID,
	TANH
} ActivationFunction;

typedef enum
{
	INPUT,
	OUTPUT,
	HIDDEN
} NodeType;

typedef struct
{
	int id;
	float output;
	float bias;
	ActivationFunction activationFunction;
	NodeType type;
} Node;

typedef struct
{
	int from;
	int to;
	float weight;
	bool enabled;
	int innovation;
} Edge;

typedef struct
{
	Node *nodes;
	Edge *edges;
	Edge *sortedEdges;
	int *inputs;
	int *outputs;
	int inputsCount;
	int outputsCount;
	int nodeCount;
	int edgeCount;
	float fitness;
} Genome;

typedef struct
{
	Genome *genomes;
	int genomeCount;
} Specie;

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type)
{
	Node node;
	node.id = id;
	node.bias = bias;
	node.activationFunction = activationFunction;
	node.type = type;
	return node;
}

Edge createEdge(int from, int to, float weight, bool enabled, int innovation)
{
	Edge edge;
	edge.from = from;
	edge.to = to;
	edge.weight = weight;
	edge.enabled = enabled;
	edge.innovation = innovation;
	return edge;
}

Genome createGenome(int inputs, int outputs)
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
	genome.sortedEdges = malloc(sizeof(Edge) * EDGES_LIMIT);

	for (int i = 0; i < inputs; i++)
	{
		genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, INPUT);
		genome.inputs[i] = genome.nodeCount - 1;
	}

	for (int i = 0; i < outputs; i++)
	{
		genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, OUTPUT);
		genome.outputs[i] = genome.nodeCount - 1;
	}
	genome.nodes[genome.nodeCount++] = createNode(genome.nodeCount, 0, LINEAR, HIDDEN);

	for (int i = 0; i < inputs; i++)
	{
		genome.edges[genome.edgeCount++] = createEdge(i, genome.nodeCount - 1, 0.5, true, genome.edgeCount - 1);
	}

	for (int i = genome.edgeCount; i < inputs + outputs; i++)
	{
		genome.edges[genome.edgeCount++] = createEdge(genome.nodeCount - 1, i, 0.5, true, genome.edgeCount - 1);
	}

	return genome;
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
	}
}

void printGenome(Genome genome)
{
	printf("Edged:\n");

	for (int i = 0; i < genome.nodeCount - 1; i++)
	{
		printf("Edge %d: %d -> %d\n", i, genome.edges[i].from, genome.edges[i].to);
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

int main()
{
	Genome genome = createGenome(4, 4);
	printGenome(genome);

	printf("Hello World!, %d\n", genome.nodeCount);
	return 0;
}