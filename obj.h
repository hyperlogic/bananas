#ifndef OBJ_H
#define OBJ_H

// TODO: rename to obj
enum node_type { SYMBOL_NODE = 0, CELL_NODE, NUMBER_NODE, PRIM_NODE, CLOSURE_NODE };

struct node_struct;
struct env_struct;

typedef struct {
    struct node_struct* car;
    struct node_struct* cdr;
} cell_t;

typedef struct {
    struct node_struct* args;
    struct node_struct* body;
    struct env_struct* env;
} closure_t;

typedef struct node_struct* (*prim_t)(struct node_struct* args);

typedef struct node_struct {
    enum node_type type;
    union {
        int symbol;
        cell_t cell;
        float number;
        prim_t prim;
        closure_t closure;
    } data;
} node_t;

typedef struct env_struct {
    node_t** data;
    int max_nodes;
    int num_nodes;
    struct env_struct* parent;
} env_t;

// TODO: make these first class
// environments
env_t* env_new(env_t* parent);
void env_add(env_t* env, node_t* symbol, node_t* value);
node_t* env_lookup(env_t* env, node_t* symbol);

// node makers
node_t* make_symbol_node(const char* str);
node_t* make_symbol_node_from_string(const char* start, const char* end);
node_t* make_number_node(double num);
node_t* make_prim_node(prim_t prim);
node_t* make_cell_node(node_t* car, node_t* cdr);

#endif
