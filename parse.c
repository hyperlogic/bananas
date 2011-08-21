#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_SYMBOLS 1000
static const char* g_symbol_array[MAX_SYMBOLS];
static int g_symbol_len_array[MAX_SYMBOLS];
static int g_num_symbols = 0;

#define PARSE_ERROR(err)                                        \
    do {                                                        \
        fprintf(stderr, "%s\n", err);                           \
        exit(1);                                                \
    } while(0)

// TODO: ref counting

static env_t* g_env = NULL;
static node_t* g_true = NULL;

typedef struct { 
    const char* name;
    prim_t prim;
} prim_info_t;

static prim_info_t s_prim_infos[] = {
    {"eval", eval}, 
    {"apply", apply}, 
    {"cons", cons},
    {"car", car},
    {"cdr", cdr},
    {"cadr", cadr},
    {"def", def},
    {"quote", quote},
    {"+", add},
    {"eq?", eq},
    {"assoc", assoc},
    {"lambda", lambda},
    {"", NULL}
};

void init()
{
    g_env = env_new((env_t*)NULL);
    g_true = make_symbol_node("t");

    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        env_add(g_env, make_symbol_node(p->name), make_prim_node(p->prim));
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
        if (len == g_symbol_len_array[i] && strncmp(g_symbol_array[i], str, len) == 0)
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
    g_symbol_len_array[g_num_symbols] = len;
    g_num_symbols++;
    return g_num_symbols - 1;
}

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
        if (symbol->data.symbol == car(env->data[i])->data.symbol)
            return env->data[i]->data.cell.cdr;
    }

    if (env->parent)
        return env_lookup(env->parent, symbol);
    else
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

node_t* make_prim_node(prim_t prim)
{
   node_t* node = (node_t*)malloc(sizeof(node_t));
   node->type = PRIM_NODE;
   node->data.prim = prim;
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

    node_t* root = CONS(0, 0);
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
            p->data.cell.cdr = CONS(car, 0);
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

    node_t* e = parse_expr(pp);
    return QUOTE(e);
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

node_t* read_string(const char* str)
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
        case PRIM_NODE:
            return n;
        case SYMBOL_NODE:
            return env_lookup(g_env, n);
        case CELL_NODE:
            return apply(n);
        default:
            return NULL;
        }
    }
}

node_t* apply(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* f = eval(car(n));
    node_t* args = cdr(n);
    assert(f);
    switch (f->type) {
    case PRIM_NODE:
        return f->data.prim(args);
    case CLOSURE_NODE:
    {
        env_t* arg_env = env_new(f->data.closure.env);
        node_t* closure_args = f->data.closure.args;
        while(args && closure_args) {
            env_add(arg_env, car(closure_args), eval(car(args)));
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        node_t* closure_body = f->data.closure.body;

        // make sure to eval closure body with the arg_env
        env_t* orig_env = g_env;
        g_env = arg_env;
        node_t* result = eval(closure_body);
        g_env = orig_env;

        return result;
    }
    default:
        assert(0);  // illegal function type
    }
}

#define PRINTF(args...)                          \
    do {                                         \
        if (to_stderr)                           \
            fprintf(stderr, args);               \
        else                                     \
            printf(args);                        \
    } while(0)

node_t* DUMP(node_t* n, int to_stderr)
{
    if (!n)
        PRINTF(" nil");
    else {
        switch (n->type) {
        case NUMBER_NODE:
            PRINTF(" %f", n->data.number);
            break;
        case SYMBOL_NODE:
            PRINTF(" %s", symbol_get(n->data.symbol));
            break;
        case CELL_NODE:
            PRINTF(" (");
            while (n) {
                DUMP(car(n), to_stderr);
                n = cdr(n);
            }
            PRINTF(" )");
            break;
        case PRIM_NODE:
            PRINTF(" <#prim 0x%p>", n->data.prim);
            break;
        case CLOSURE_NODE:
            PRINTF(" <#closure ");
            DUMP(n->data.closure.args, to_stderr);
            DUMP(n->data.closure.body, to_stderr);
            PRINTF(" >");
            break;
        default:
            PRINTF(" ???");
            break;
        }
    }
    
    return NULL;
}

node_t* cons(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    return CONS(car(n), cadr(n));
}

node_t* car(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    return n->data.cell.car;
}

node_t* cdr(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    return n->data.cell.cdr;
}

node_t* cadr(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    return car(cdr(n));
}

node_t* def(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* value = eval(cadr(n));
    env_add(g_env, car(n), value);
    return value;
}

node_t* quote(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    return car(n);
}

node_t* add(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    double total = 0;
    while (n) {
        node_t* arg = eval(car(n));
        total += arg->type == NUMBER_NODE ? arg->data.number : 0;
        n = cdr(n);
    }
    return make_number_node(total);
}

// TODO: NO WORKY
node_t* map(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* f = car(n);
    n = cdr(n);
    node_t* nn = CONS(0, 0);
    node_t* p = nn;
    while (n) {
        p->data.cell.car = apply(CONS(f, car(n)));
        n = cdr(n);
        if (n) {
            p->data.cell.cdr = CONS(0, 0);
            p = cdr(p);
        }
    }
    return nn;
}

node_t* EQ(node_t* a, node_t* b)
{
    if (a->type == b->type) {
        switch (a->type) {
        case SYMBOL_NODE:
            return a->data.symbol == b->data.symbol ? g_true : NULL;
        case NUMBER_NODE:
            return a->data.number == b->data.number ? g_true : NULL;
        case CELL_NODE:
        case PRIM_NODE:
        case CLOSURE_NODE:
            return a == b ? g_true : NULL;
        }
    }
    return NULL;
}

node_t* eq(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* a = eval(car(n));
    node_t* b = eval(cadr(n));
    return EQ(a, b);
}

node_t* ASSOC(node_t* key, node_t* plist)
{
    while (plist) {
        node_t* pair = car(plist);
        if (EQ(key, (car(pair))))
            return cadr(pair);
        plist = cdr(plist);
    }
    return NULL;
}

node_t* assoc(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* key = eval(car(n));
    node_t* plist = eval(cadr(n));
    return ASSOC(key, plist);
}

node_t* make_closure_node(node_t* args, node_t* body, env_t* env)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = CLOSURE_NODE;
    node->data.closure.args = args;
    node->data.closure.body = body;
    node->data.closure.env = env;
    return node;
}

node_t* MEMBER(node_t* obj, node_t* lst)
{
    while (lst) {
        if (EQ(obj, car(lst)))
            return g_true;
        lst = cdr(lst);
    }
    return NULL;
}

void capture_closure(env_t* env, node_t* args, node_t* body)
{
    if (!body)
        return;

    if (body->type == SYMBOL_NODE && !MEMBER(body, args)) {
        env_add(env, body, eval(body));
    } else if (body->type == CELL_NODE) {
        capture_closure(env, args, car(body));
        capture_closure(env, args, cdr(body));
    }
}

node_t* lambda(node_t* n)
{
    assert(n && n->type == CELL_NODE);
    node_t* args = car(n);
    node_t* body = cadr(n);
    env_t* env = env_new((env_t*)NULL);

    capture_closure(env, args, body);

    return make_closure_node(args, body, env);
}
