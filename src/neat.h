#include "defs.h"

Node createNode(int id, float bias, ActivationFunction activationFunction, NodeType type);
Edge createEdge(int from, int to, float weight, bool enabled, int innovation);
Genome createGenome(int inputs, int outputs, bool randomBias, bool randomWeights);
void fill_nodes_edges(Genome *genome);