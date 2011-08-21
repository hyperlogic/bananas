enum node_type { SYMBOL_NODE = 0, CELL_NODE, NUMBER_NODE, CFUNC_NODE, NUM_NODES };

struct node_struct;

typedef struct {
    struct node_struct* car;
    struct node_struct* cdr;
} cell_t;

typedef struct node_struct* cfunc_t(struct node_struct* args);

typedef struct node_struct {
    enum node_type type;
    union {
        int symbol;
        cell_t cell;
        float number;
        cfunc_t* cfunc;
    } data;
} node_t;

typedef struct {
    node_t** data;
    int max_nodes;
    int num_nodes;
} env_t;

// interned symbols
int symbol_add(const char* str, int len);
const char* symbol_get(int id);
int symbol_find(const char* str, int len);

// environments
env_t* env_new();
void env_add(env_t* env, node_t* symbol, node_t* value);
node_t* env_lookup(env_t* env, node_t* symbol);

// node makers
node_t* make_cell_node(node_t* car, node_t* cdr);
node_t* make_number_node(double num);
node_t* make_symbol_node(const char* str);
node_t* make_cfunc_node(cfunc_t* cfunc);

void init();

node_t* read(const char* line);

// cfuncs
node_t* eval(node_t* n);
node_t* apply(node_t* n);
node_t* print(node_t* n);
node_t* cons(node_t* n);
node_t* car(node_t* n);
node_t* cdr(node_t* n);
node_t* cadr(node_t* n);
node_t* def(node_t* n);
node_t* quote(node_t* n);
node_t* add(node_t* n);
