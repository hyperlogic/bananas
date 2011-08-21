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

// TODO: ref counting

static env_t* g_env = NULL;

typedef struct { 
    const char* name;
    cfunc_t* func;
} cfunc_info_t;

static cfunc_info_t s_cfunc_infos[] = {
    {"eval", eval}, 
    {"apply", apply}, 
    {"print", print},
    {"cons", cons},
    {"car", car},
    {"cdr", cdr},
    {"cadr", cadr},
    {"def", def},
    {"quote", quote},
    {"+", add},
    {"", NULL}
};

void init()
{
    g_env = env_new();

    // register cfuncs
    cfunc_info_t* p = s_cfunc_infos;
    while (p->func) {
        env_add(g_env, make_symbol_node(p->name), make_cfunc_node(p->func));
        p++;
    }
}

//
// symbols
//
const char* symbol_get(int id)
{
    assert(id >= 0 && id < MAX_SYMBOLS);
    return g_symbol_array[id];
}

int symbol_find(const char* str, int len)
{
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

//
// environements
//

env_t* env_new()
{
    env_t* env = (env_t*)malloc(sizeof(env_t));

    // start off with 16 nodes
    const int initial_max_nodes = 16;
    env->data = (node_t**)malloc(sizeof(node_t*) * initial_max_nodes);
    env->max_nodes = initial_max_nodes;
    env->num_nodes = 0;

    return env;
}

void env_add(env_t* env, node_t* symbol, node_t* value)
{
    assert(env);
    assert(symbol->type == SYMBOL_NODE);

    if (env->num_nodes == env->max_nodes) {
        // realloc more nodes!
        int new_max_nodes = env->max_nodes * env->max_nodes;
        env->data = (node_t**)realloc(env->data, sizeof(node_t*) * new_max_nodes);
        env->max_nodes = new_max_nodes;
    }
    env->data[env->num_nodes] = make_cell_node(symbol, value);
    env->num_nodes++;
}

node_t* env_lookup(env_t* env, node_t* symbol)
{
    assert(env);
    assert(symbol->type == SYMBOL_NODE);
    int i;
    for (i = 0; i < env->num_nodes; i++) {
        if (symbol->data.symbol == car(env->data[i])->data.symbol)
            return env->data[i]->data.cell.cdr;
    }
    return NULL;
}

#define ADVANCE() *pp = *pp + 1
#define PEEK(i) *(*pp + i)

node_t* make_symbol_node(const char* str)
{
    int len = strlen(str);
    int id = symbol_find(str, len);
    if (id < 0)
        id = symbol_add(str, len);

    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = SYMBOL_NODE;
    node->data.symbol = id;
    return node;
}

node_t* make_symbol_node_from_string(const char* start, const char* end)
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

node_t* make_cfunc_node(cfunc_t* cfunc)
{
   node_t* node = (node_t*)malloc(sizeof(node_t));
   node->type = CFUNC_NODE;
   node->data.cfunc = cfunc;
   return node;
}

node_t* parse_symbol(const char** pp)
{
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

    return make_symbol_node_from_string(start, *pp);
}

node_t* make_number_node(double num)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = NUMBER_NODE;
    node->data.number = num;
    return node;
}

node_t* make_number_node_from_string(const char* start, const char* end)
{
    // sneaky, lets null terminate the string.
    char orig = *(end + 1);
    char* non_const_end = (char*)end;
    *(non_const_end + 1) = 0;

    // lets lean on libc to do the conversion.
    double num = atof(start);

    // restore original charater.
    *(non_const_end + 1) = orig;
    
    return make_number_node(num);
}

node_t* parse_number(const char** pp)
{
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
        return make_number_node_from_string(start, *pp);
    
    // [0-9]*
    while(1) {
        if (isdigit(PEEK(0)))
            ADVANCE();
        else 
            break;
    }

    return make_number_node_from_string(start, *pp);
}

node_t* parse_atom(const char** pp)
{
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

node_t* parse_quoted_expr(const char** pp)
{
    if (PEEK(0) == '\'')
        ADVANCE();
    else
        PARSE_ERROR("Expected a quote");

    node_t* quote = make_symbol_node("quote");
    node_t* e = parse_expr(pp);
    return make_cell_node(quote, make_cell_node(e, 0));
}

node_t* parse_expr(const char** pp)
{
    while (isspace(PEEK(0)))
        ADVANCE();

    if (PEEK(0) == 0)
        PARSE_ERROR("Unexpected NULL character");
    else if (PEEK(0) == '(')
        return parse_list(pp);
    else if (PEEK(0) == '\'')
        return parse_quoted_expr(pp);
    else
        return parse_atom(pp);
}

node_t* read(const char* str)
{
    return parse_expr(&str);
}

node_t* eval(node_t* n)
{
    if (!n)
        return NULL;
    else {
        switch (n->type) {
        case NUMBER_NODE:
        case CFUNC_NODE:
            return n;
        case SYMBOL_NODE:
            return env_lookup(g_env, n);
        case CELL_NODE:
            return apply(make_cell_node(eval(car(n)), cdr(n)));
        default:
            return NULL;
        }
    }
}

node_t* apply(node_t* n)
{
    assert(n);
    assert(car(n)->type == CFUNC_NODE);
    return car(n)->data.cfunc(cdr(n));
}

node_t* print(node_t* n)
{
    if (!n)
        printf(" nil");
    else {
        switch (n->type) {
        case NUMBER_NODE:
            printf(" %f", n->data.number);
            break;
        case SYMBOL_NODE:
            printf(" %s", symbol_get(n->data.symbol));
            break;
        case CELL_NODE:
            printf(" (");
            while (n) {
                print(car(n));
                n = cdr(n);
            }
            printf(" )");
            break;
        case CFUNC_NODE:
            printf(" <#cfunc 0x%p>", n->data.cfunc);
            break;
        default:
            printf(" ???");
            break;
        }
    }
    
    return NULL;
}

node_t* cons(node_t* n)
{
    assert(n->type == CELL_NODE);
    return make_cell_node(car(n), car(cdr(n)));
}

node_t* car(node_t* n)
{
    assert(n->type == CELL_NODE);
    return n->data.cell.car;
}

node_t* cdr(node_t* n)
{
    assert(n->type == CELL_NODE);
    return n->data.cell.cdr;
}

node_t* cadr(node_t* n)
{
    return car(cdr(n));
}

node_t* def(node_t* n)
{
    env_add(g_env, car(n), cadr(n));
    return cadr(n);
}

node_t* quote(node_t* n)
{
    return car(n);
}

node_t* add(node_t* n)
{
    return make_number_node(car(n)->data.number + cadr(n)->data.number);
}
