enum node_type { SYMBOL_NODE = 0, CELL_NODE, NUMBER_NODE, NIL_NODE, NUM_NODES };

struct node_struct;

typedef struct {
    struct node_struct* car;
    struct node_struct* cdr;
} cell_t;

typedef struct node_struct {
    enum node_type type;
    union {
        int symbol;
        cell_t cell;
        float number;
    } data;
} node_t;

node_t* read(const char* line);
node_t* eval(node_t* expr);
void print(node_t* expr);
