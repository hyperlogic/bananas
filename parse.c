#include "parse.h"
#include <stdio.h>
#include <string.h>

#define MAX_SYMBOLS 1000
static const char* symbol_table[MAX_SYMBOLS];
static int num_symbols = 0;

node_t* read(const char* line)
{
    printf("line = \"%s\"", line);
    return NULL;
}

node_t* eval(node_t* expr)
{
    return NULL;
}

void print(node_t* expr)
{
    return;
}
