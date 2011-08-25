#include "prim.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

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

extern env_t* g_env;
extern node_t* g_true;

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        env_add(g_env, make_symbol_node(p->name), make_prim_node(p->prim));
        p++;
    }
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
