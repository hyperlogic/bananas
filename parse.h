enum cell_type { SYMBOL_TYPE = 0, CONS_CELL_TYPE, NUMBER_TYPE };

typedef struct {
    int i;
} symbol_t;

typedef struct {
    struct node_t* car;
    struct node_t* cdr;
} cell_t;

typedef struct {
    double number;
} number_t;

typedef struct {
    enum cell_type type;
    union {
        symbol_t symbol;
        cell_t cell;
        number_t number;
    } data;
} node_t;

node_t* read(const char* line);
node_t* eval(node_t* expr);
void print(node_t* expr);
