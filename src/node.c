#include "defs.h"
#include <math.h>

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