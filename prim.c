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
    {"eval", prim_eval}, 
    {"apply", prim_apply}, 
    {"cons", prim_cons},
    {"car", prim_car},
    {"cdr", prim_cdr},
    {"cadr", prim_cadr},
    {"def", prim_def},
    {"quote", prim_quote},
    {"+", prim_add},
    {"eq?", prim_eq},
    {"assoc", prim_assoc},
    {"lambda", prim_lambda},
    {"", NULL}
};

extern env_t* g_env;
extern obj_t* g_true;

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        env_add(g_env, make_symbol(p->name), make_prim(p->prim));
        p++;
    }
}

obj_t* prim_eval(obj_t* obj)
{
    if (is_symbol(obj))
        return env_lookup(g_env, obj);
    else if (is_pair(obj))
        return prim_apply(obj);
    else
        return obj;
}

obj_t* prim_apply(obj_t* obj)
{
    assert(is_pair(obj));
    obj_t* f = prim_eval(car(obj));
    obj_t* args = cdr(obj);
    assert(f);
    switch (f->type) {
    case PRIM_OBJ:
        return f->data.prim(args);
    case CLOSURE_OBJ:
    {
        env_t* arg_env = env_new(f->data.closure.env);
        obj_t* closure_args = f->data.closure.args;
        while(args && closure_args) {
            env_add(arg_env, car(closure_args), prim_eval(car(args)));
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        obj_t* closure_body = f->data.closure.body;

        // make sure to eval closure body with the arg_env
        env_t* orig_env = g_env;
        g_env = arg_env;
        obj_t* result = prim_eval(closure_body);
        g_env = orig_env;

        return result;
    }
    default:
        assert(0);  // illegal function type
    }
}

obj_t* prim_cons(obj_t* obj)
{
    obj_t* a = prim_eval(car(obj));
    obj_t* b = prim_eval(cadr(obj));
    return cons(a, b);
}

obj_t* prim_car(obj_t* obj)
{
    return prim_eval(car(obj));
}

obj_t* prim_cdr(obj_t* obj)
{
    return prim_eval(cdr(obj));
}

obj_t* prim_cadr(obj_t* obj)
{
    return prim_eval(cadr(obj));
}

obj_t* prim_def(obj_t* obj)
{
    obj_t* value = prim_eval(cadr(obj));

    // AJT: TODO: BUG: FIXME: env_add does not remove original value from env
    // just adds it on the end.

    env_add(g_env, car(obj), value);
    return value;
}

obj_t* prim_quote(obj_t* obj)
{
    return car(obj);
}

obj_t* prim_add(obj_t* obj)
{
    double total = 0;
    while (obj) {
        obj_t* arg = prim_eval(car(obj));
        total += is_number(arg) ? arg->data.number : 0;
        obj = cdr(obj);
    }
    return make_number(total);
}

obj_t* prim_eq(obj_t* obj)
{
    obj_t* a = prim_eval(car(obj));
    obj_t* b = prim_eval(cadr(obj));
    return eq(a, b);
}

obj_t* prim_assoc(obj_t* obj)
{
    obj_t* key = prim_eval(car(obj));
    obj_t* plist = prim_eval(cadr(obj));
    return assoc(key, plist);
}

void capture_closure(env_t* env, obj_t* args, obj_t* body)
{
    if (!body)
        return;

    if (body->type == SYMBOL_OBJ && !member(body, args)) {
        env_add(env, body, prim_eval(body));
    } else if (body->type == PAIR_OBJ) {
        capture_closure(env, args, car(body));
        capture_closure(env, args, cdr(body));
    }
}

obj_t* prim_lambda(obj_t* n)
{
    assert(n && n->type == PAIR_OBJ);
    obj_t* args = car(n);
    obj_t* body = cadr(n);
    env_t* env = env_new((env_t*)NULL);

    capture_closure(env, args, body);

    return make_closure(args, body, env);
}
