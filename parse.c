#include "parse.h"
#include "lex.h"
#include <stdio.h>
#include <string.h>

#define MAX_SYMBOLS 1000
static const char* symbol_table[MAX_SYMBOLS];
static int num_symbols = 0;

node_t* read(const char* line)
{
    printf("line = \"%s\"\n", line);
    token_vec_t* vec = lex(line);

    int i;
    for (i = 0; i < vec->num_tokens; i++) {
        switch (vec->data[i].type) {
        case SYMBOL_TOKEN:
            printf("SYMBOL = \"%s\"\n", vec->data[i].string);
            break;
        case QUOTE_TOKEN:
            printf("QUOTE = \"%s\"\n", vec->data[i].string);
            break;
        case LPAREN_TOKEN:
            printf("LPAREN = \"%s\"\n", vec->data[i].string);
            break;
        case RPAREN_TOKEN:
            printf("RPAREN = \"%s\"\n", vec->data[i].string);
            break;
        case NUMBER_TOKEN:
            printf("NUMBER = \"%s\"\n", vec->data[i].string);
            break;
        default:
            printf("UNKNOWN!\n");
            break;
        }
    }

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
