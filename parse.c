#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_SYMBOLS 1000
static const char* g_symbol_array[MAX_SYMBOLS];
static int g_num_symbols = 0;

#define PARSE_ERROR(err)                                        \
    do {                                                        \
        fprintf(stderr, "%s\n", err);                           \
        exit(1);                                                \
    } while(0)

const char* symbol_get(int id)
{
    assert(id >= 0 && id < MAX_SYMBOLS);
    return g_symbol_array[id];
}

int symbol_find(const char* str, int len)
{
    // TODO: FIXME: O(n)
    int i;
    for (i = 0; i < g_num_symbols; i++) {
        if (strncmp(g_symbol_array[i], str, len) == 0)
            return i;
    }
    return -1;
}

int symbol_add(const char* str, int len)
{
    assert(g_num_symbols < MAX_SYMBOLS);
    char* symbol_string = (char*)malloc(len + 1);
    memcpy(symbol_string, str, len);
    symbol_string[len] = 0;
    g_symbol_array[g_num_symbols] = symbol_string;
    g_num_symbols++;
    return g_num_symbols - 1;
}

#define ADVANCE() *pp = *pp + 1
#define PEEK(i) *(*pp + i)

node_t* make_symbol_node(const char* start, const char* end)
{
    int len = end - start;
    int id = symbol_find(start, len);
    if (id < 0)
        id = symbol_add(start, len);

    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = SYMBOL_NODE;
    node->data.symbol = id;
    return node;
}

node_t* parse_symbol(const char** pp)
{
    printf("parse_symbol, pp = \"%s\"\n", *pp);

    const char* start = *pp;
    
    // ^[()\s\d]
    if (PEEK(0) == '(' || PEEK(0) == ')' || isspace(PEEK(0)) || isdigit(PEEK(0)))
        PARSE_ERROR("Symbol cannot begin with parenthesis, digits or white-space");
    else
        ADVANCE();

    // ^[()\s]*
    while (1) {
        if (PEEK(0) == '(' || PEEK(0) == ')' || isspace(PEEK(0)) || PEEK(0) == '\0')
            break;
        else
            ADVANCE();
    }

    return make_symbol_node(start, *pp);
}

node_t* make_number_node(const char* start, const char* end)
{
    // sneaky, lets null terminate the string.
    char orig = *(end + 1);
    char* non_const_end = (char*)end;
    *(non_const_end + 1) = 0;

    // lets lean on libc to do the conversion.
    double num = atof(start);

    // restore original charater.
    *(non_const_end + 1) = orig;

    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = NUMBER_NODE;
    node->data.number = num;
    return node;
}

node_t* parse_number(const char** pp)
{
    printf("parse_number, pp = \"%s\"\n", *pp);

    const char* start = *pp;

    // [+-]?
    if (PEEK(0) == '+')
        ADVANCE();
    else if (PEEK(0) == '-')
        ADVANCE();

    // [0-9]+
    do {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    } while(1);

    // \.?
    if (PEEK(0) == '.')
        ADVANCE();
    else
        return make_number_node(start, *pp);
    
    // [0-9]*
    while(1) {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    }

    return make_number_node(start, *pp);
}

node_t* parse_atom(const char** pp)
{
    printf("parse_atom, pp = \"%s\"\n", *pp);

    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (((PEEK(0) == '+' || PEEK(0) == '-') && isdigit(PEEK(1))) || isdigit(PEEK(0)))
        return parse_number(pp);
    else
        return parse_symbol(pp);
}

node_t* make_cell_node(node_t* car, node_t* cdr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = CELL_NODE;
    node->data.cell.car = car;
    node->data.cell.cdr = cdr;
    return node;
}

node_t* parse_expr(const char** pp);

node_t* parse_list(const char** pp)
{
    printf("parse_list, pp = \"%s\"\n", *pp);

    // ( EXPR* )
    if (PEEK(0) != '(')
        PARSE_ERROR("List must start with left-parenthesis");
    else
        ADVANCE();

    node_t* root = make_cell_node(0, 0);
    node_t* p = root;

    while (1) {
        while (isspace(PEEK(0)))
            ADVANCE();

        if (PEEK(0) == 0)
            PARSE_ERROR("Unexpected NULL character");
        else if (PEEK(0) == ')')
            break;

        node_t* car = parse_expr(pp);
        if (p->data.cell.car == 0)
            p->data.cell.car = car;
        else {
            p->data.cell.cdr = make_cell_node(car, 0);
            p = p->data.cell.cdr;
        }
    }

    if (PEEK(0) == ')')
        ADVANCE();

    return root;
}

node_t* parse_expr(const char** pp)
{
    printf("parse_expr, pp = \"%s\"\n", *pp);

    while (isspace(PEEK(0)))
        ADVANCE();

    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (PEEK(0) == '(')
        return parse_list(pp);
    else
        return parse_atom(pp);
}

node_t* read(const char* str)
{
    return parse_expr(&str);
}

node_t* eval(node_t* expr)
{
    return NULL;
}

void print(node_t* node)
{
    if (node) {
        switch (node->type) {
        case NUMBER_NODE:
            printf(" %f", node->data.number);
            break;
        case SYMBOL_NODE:
            printf(" %s", symbol_get(node->data.symbol));
            break;
        case CELL_NODE:
            printf(" (");
            while (node) {
                print(node->data.cell.car);
                node = node->data.cell.cdr;
            }
            printf(" )");
            break;
        case NIL_NODE:
            printf(" nil");
            break;
        default:
            printf(" ???");
            break;
        }
    }
}
