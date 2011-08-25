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
extern obj_t* g_true;

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        env_add(g_env, make_symbol_obj(p->name), make_prim_obj(p->prim));
        p++;
    }
}

obj_t* eval(obj_t* n)
{
    if (!n)
        return NULL;
    else {
        switch (n->type) {
        case NUMBER_OBJ:
        case PRIM_OBJ:
            return n;
        case SYMBOL_OBJ:
            return env_lookup(g_env, n);
        case CELL_OBJ:
            return apply(n);
        default:
            return NULL;
        }
    }
}

obj_t* apply(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    obj_t* f = eval(car(n));
    obj_t* args = cdr(n);
    assert(f);
    switch (f->type) {
    case PRIM_OBJ:
        return f->data.prim(args);
    case CLOSURE_OBJ:
    {
        env_t* arg_env = env_new(f->data.closure.env);
        obj_t* closure_args = f->data.closure.args;
        while(args && closure_args) {
            env_add(arg_env, car(closure_args), eval(car(args)));
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        obj_t* closure_body = f->data.closure.body;

        // make sure to eval closure body with the arg_env
        env_t* orig_env = g_env;
        g_env = arg_env;
        obj_t* result = eval(closure_body);
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

obj_t* DUMP(obj_t* n, int to_stderr)
{
    if (!n)
        PRINTF(" nil");
    else {
        switch (n->type) {
        case NUMBER_OBJ:
            PRINTF(" %f", n->data.number);
            break;
        case SYMBOL_OBJ:
            PRINTF(" %s", symbol_get(n->data.symbol));
            break;
        case CELL_OBJ:
            PRINTF(" (");
            while (n) {
                DUMP(car(n), to_stderr);
                n = cdr(n);
            }
            PRINTF(" )");
            break;
        case PRIM_OBJ:
            PRINTF(" <#prim 0x%p>", n->data.prim);
            break;
        case CLOSURE_OBJ:
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


obj_t* cons(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    return CONS(car(n), cadr(n));
}

obj_t* car(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    return n->data.cell.car;
}

obj_t* cdr(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    return n->data.cell.cdr;
}

obj_t* cadr(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    return car(cdr(n));
}

obj_t* def(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    obj_t* value = eval(cadr(n));
    env_add(g_env, car(n), value);
    return value;
}

obj_t* quote(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    return car(n);
}

obj_t* add(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    double total = 0;
    while (n) {
        obj_t* arg = eval(car(n));
        total += arg->type == NUMBER_OBJ ? arg->data.number : 0;
        n = cdr(n);
    }
    return make_number_obj(total);
}

obj_t* EQ(obj_t* a, obj_t* b)
{
    if (a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol ? g_true : NULL;
        case NUMBER_OBJ:
            return a->data.number == b->data.number ? g_true : NULL;
        case CELL_OBJ:
        case PRIM_OBJ:
        case CLOSURE_OBJ:
            return a == b ? g_true : NULL;
        }
    }
    return NULL;
}

obj_t* eq(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    obj_t* a = eval(car(n));
    obj_t* b = eval(cadr(n));
    return EQ(a, b);
}

obj_t* ASSOC(obj_t* key, obj_t* plist)
{
    while (plist) {
        obj_t* pair = car(plist);
        if (EQ(key, (car(pair))))
            return cadr(pair);
        plist = cdr(plist);
    }
    return NULL;
}

obj_t* assoc(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    obj_t* key = eval(car(n));
    obj_t* plist = eval(cadr(n));
    return ASSOC(key, plist);
}

obj_t* make_closure_obj(obj_t* args, obj_t* body, env_t* env)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = CLOSURE_OBJ;
    obj->data.closure.args = args;
    obj->data.closure.body = body;
    obj->data.closure.env = env;
    return obj;
}

obj_t* MEMBER(obj_t* obj, obj_t* lst)
{
    while (lst) {
        if (EQ(obj, car(lst)))
            return g_true;
        lst = cdr(lst);
    }
    return NULL;
}

void capture_closure(env_t* env, obj_t* args, obj_t* body)
{
    if (!body)
        return;

    if (body->type == SYMBOL_OBJ && !MEMBER(body, args)) {
        env_add(env, body, eval(body));
    } else if (body->type == CELL_OBJ) {
        capture_closure(env, args, car(body));
        capture_closure(env, args, cdr(body));
    }
}

obj_t* lambda(obj_t* n)
{
    assert(n && n->type == CELL_OBJ);
    obj_t* args = car(n);
    obj_t* body = cadr(n);
    env_t* env = env_new((env_t*)NULL);

    capture_closure(env, args, body);

    return make_closure_obj(args, body, env);
}
