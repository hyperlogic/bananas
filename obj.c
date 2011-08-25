#include "obj.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//
// environements
//
env_t* env_new(env_t* parent)
{
    env_t* env = (env_t*)malloc(sizeof(env_t));

    // start off with 16 nodes
    const int initial_max_nodes = 16;
    env->data = (node_t**)malloc(sizeof(node_t*) * initial_max_nodes);
    env->max_nodes = initial_max_nodes;
    env->num_nodes = 0;
    env->parent = parent;

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
    int i;
    
    assert(env);
    assert(symbol->type == SYMBOL_NODE);
    for (i = 0; i < env->num_nodes; i++) {
        if (symbol->data.symbol == env->data[i]->data.cell.car->data.symbol)
            return env->data[i]->data.cell.cdr;
    }

    if (env->parent)
        return env_lookup(env->parent, symbol);
    else
        return NULL;
}

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

node_t* make_number_node(double num)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = NUMBER_NODE;
    node->data.number = num;
    return node;
}

node_t* make_prim_node(prim_t prim)
{
   node_t* node = (node_t*)malloc(sizeof(node_t));
   node->type = PRIM_NODE;
   node->data.prim = prim;
   return node;
}

node_t* make_cell_node(node_t* car, node_t* cdr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = CELL_NODE;
    node->data.cell.car = car;
    node->data.cell.cdr = cdr;
    return node;
}
