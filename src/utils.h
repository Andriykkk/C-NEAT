#include "defs.h"

Genome load_genome(char *filename);
void save_genome(Genome *g, char *filename);
Population load_fully(char *name);
int save_fully(Population population, char *name);
void print_genome(Genome genome);
float get_random_numberf(float min, float max);
unsigned int get_random_unsigned_int();