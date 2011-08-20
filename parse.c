#include "parse.h"
#include "lex.h"
#include <stdio.h>
#include <string.h>

#define MAX_SYMBOLS 1000
static const char* symbol_table[MAX_SYMBOLS];
static int num_symbols = 0;

node_t* read(const char* line)
{
    token_vec_t* vec = lex(line);

    int i, len;
    char temp[1024];
    for (i = 0; i < vec->num_tokens; i++) {
        switch (vec->data[i].type) {
        case SYMBOL_TOKEN:
            len = vec->data[i].end - vec->data[i].start;
            memcpy(temp, vec->data[i].start, len);
            temp[len] = 0;
            printf("SYMBOL = \"%s\"\n", temp);
            break;
        case QUOTE_TOKEN:
            printf("QUOTE\n");
            break;
        case LPAREN_TOKEN:
            printf("LPAREN\n");
            break;
        case RPAREN_TOKEN:
            printf("RPAREN\n");
            break;
        case NUMBER_TOKEN:
            len = vec->data[i].end - vec->data[i].start;
            memcpy(temp, vec->data[i].start, len);
            temp[len] = 0;
            printf("NUMBER = \"%s\"\n", temp);
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
