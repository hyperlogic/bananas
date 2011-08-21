enum node_type { SYMBOL_NODE = 0, CELL_NODE, NUMBER_NODE, PRIM_NODE };

struct node_struct;

typedef struct {
    struct node_struct* car;
    struct node_struct* cdr;
} cell_t;

typedef struct node_struct* (*prim_t)(struct node_struct* args);

typedef struct node_struct {
    enum node_type type;
    union {
        int symbol;
        cell_t cell;
        float number;
        prim_t prim;
    } data;
} node_t;

// interned symbols
int symbol_add(const char* str, int len);
const char* symbol_get(int id);
int symbol_find(const char* str, int len);

// node makers
node_t* make_cell_node(node_t* car, node_t* cdr);
node_t* make_number_node(double num);
node_t* make_symbol_node(const char* str);
node_t* make_prim_node(prim_t prim);

void init();

node_t* read_string(const char* line);

// these don't expect an argument list, nor do they eval their arguments.
#define CONS(_car, _cdr) make_cell_node(_car, _cdr)
#define LIST2(a, b) CONS(a, CONS(b, NULL))
#define LIST3(a, b, c) CONS(a, CONS(b, CONS(c, NULL)))
#define QUOTE(n) LIST2(make_symbol_node("quote"), n)
node_t* DUMP(node_t* n);
node_t* EQ(node_t* a, node_t* b);
node_t* ASSOC(node_t* key, node_t* plist);

// environments
node_t* env_add(node_t* env, node_t* symbol, node_t* value);
node_t* env_lookup(node_t* env, node_t* symbol);

// prims - all of these expect an argument list.  Many of them eval their arguments.
node_t* eval(node_t* n);
node_t* apply(node_t* n);
node_t* cons(node_t* n);
node_t* car(node_t* n);
node_t* cdr(node_t* n);
node_t* cadr(node_t* n);
node_t* def(node_t* n);
node_t* quote(node_t* n);
node_t* add(node_t* n);
node_t* map(node_t* n);
node_t* assoc(node_t* n);
node_t* eq(node_t* n);
