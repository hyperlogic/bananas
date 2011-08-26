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

    // type predicates
    {"nil?", prim_is_nil}, 
    {"symbol?", prim_is_symbol}, 
    {"number?", prim_is_number}, 
    {"pair?", prim_is_pair}, 
    {"prim?", prim_is_prim}, 
    {"closure?", prim_is_closure}, 
    {"env?", prim_is_env}, 

    // pair stuff
    {"cons", prim_cons},
    {"car", prim_car},
    {"cdr", prim_cdr},
    {"cadr", prim_cadr},
    {"assoc", prim_assoc},

    // env stuff
    {"def", prim_def},
    {"defined?", prim_defined},

    // equality
    {"eq?", prim_is_eq},
    {"equal?", prim_is_equal},

    // math
    {"+", prim_add},
    {"-", prim_sub},
    {"*", prim_mul},
    {"/", prim_div},

    // special forms
    {"quote", prim_quote},
    {"eval", prim_eval}, 
    {"apply", prim_apply}, 
    {"lambda", prim_lambda},
    {"if", prim_if},
    {"", NULL}
};

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        def(make_symbol(p->name), make_prim(p->prim), g_env);
        p++;
    }
}

//
// type predicates
//

obj_t* prim_is_nil(obj_t* obj)
{
    return is_nil(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_symbol(obj_t* obj)
{
    return is_symbol(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_number(obj_t* obj)
{
    return is_number(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_pair(obj_t* obj)
{
    return is_pair(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_prim(obj_t* obj)
{
    return is_prim(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_closure(obj_t* obj)
{
    return is_closure(prim_eval(obj)) ? make_true() : make_nil();
}

obj_t* prim_is_env(obj_t* obj)
{
    return is_env(prim_eval(obj)) ? make_true() : make_nil();
}

//
// pair stuff
//

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

obj_t* prim_assoc(obj_t* obj)
{
    return assoc(prim_eval(car(obj)), prim_eval(cadr(obj)));
}

//
// env stuff
//

obj_t* prim_def(obj_t* obj)
{
    obj_t* symbol = car(obj);
    obj_t* value = prim_eval(cadr(obj));
    return def(symbol, value, g_env);
}

obj_t* prim_defined(obj_t* obj)
{
    obj_t* symbol = prim_eval(car(obj));
    return defined(symbol, g_env);
}

//
// equality
//

obj_t* prim_is_eq(obj_t* obj)
{
    obj_t* a = prim_eval(car(obj));
    obj_t* b = prim_eval(cadr(obj));
    return eq(a, b);
}

obj_t* prim_is_equal(obj_t* obj)
{
    obj_t* a = prim_eval(car(obj));
    obj_t* b = prim_eval(cadr(obj));
    return equal(a, b);
}

//
// math
//

#define MATH_PRIM(name, op)                     \
obj_t* prim_##name(obj_t* obj)                  \
{                                               \
    obj_t* root = obj;                          \
    double accum;                               \
    do {                                        \
        obj_t* arg = prim_eval(car(obj));       \
        assert(is_number(arg));                 \
        if (obj == root)                        \
            accum = arg->data.number;           \
        else                                    \
            accum op arg->data.number;          \
        obj = cdr(obj);                         \
    } while (obj);                              \
    return make_number(accum);                  \
}

MATH_PRIM(add, +=)
MATH_PRIM(sub, -=)
MATH_PRIM(mul, *=)
MATH_PRIM(div, /=)

//
// special forms
//

obj_t* prim_quote(obj_t* obj)
{
    return car(obj);
}

obj_t* prim_eval(obj_t* obj)
{
    if (is_symbol(obj))
        return defined(obj, g_env);
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
        obj_t* local_env = make_env(make_nil(), f->data.closure.env);
        obj_t* closure_args = f->data.closure.args;
        while(args && closure_args) {
            def(car(closure_args), prim_eval(car(args)), local_env);
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        obj_t* closure_body = f->data.closure.body;

        // TODO: eval should take an env argument.
        // make sure to eval closure body with the local_env
        obj_t* orig_env = g_env;
        g_env = local_env;
        obj_t* result = prim_eval(closure_body);
        g_env = orig_env;

        return result;
    }
    default:
        assert(0);  // illegal function type
    }
}

static void capture_closure(obj_t* env, obj_t* args, obj_t* body)
{
    if (!body)
        return;

    if (body->type == SYMBOL_OBJ && !member(body, args)) {
        def(body, prim_eval(body), env);
    } else if (body->type == PAIR_OBJ) {
        capture_closure(env, args, car(body));
        capture_closure(env, args, cdr(body));
    }
}

obj_t* prim_lambda(obj_t* n)
{
    obj_t* args = car(n);
    obj_t* body = cadr(n);
    obj_t* env = make_env(make_nil(), make_nil());

    capture_closure(env, args, body);

    return make_closure(args, body, env);
}

obj_t* prim_if(obj_t* obj)
{
    obj_t* expr = prim_eval(car(obj));
    obj_t* true_body = car(cdr(obj));
    obj_t* else_body = car(cdr(cdr(obj)));
    if (!is_nil(expr))
        return prim_eval(true_body);
    else if (!is_nil(else_body))
        return prim_eval(else_body);
    else
        return make_nil();
}
