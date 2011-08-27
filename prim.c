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
    {"true?", prim_is_true}, 
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

// TODO: this is suspisiously like wraping a $vau, let's do that instead.

#define WRAP_BOOL_FEXPR1(name)                              \
obj_t* prim_##name(obj_t* obj)                              \
{                                                           \
    obj_t* a = prim_eval(car(obj));                         \
    ref(a);                                                 \
    obj_t* result = name(a) ? make_true() : make_nil();     \
    unref(a);                                               \
    return result;                                          \
}

#define WRAP_FEXPR1(name)                                   \
obj_t* prim_##name(obj_t* obj)                              \
{                                                           \
    obj_t* a = prim_eval(car(obj));                         \
    ref(a);                                                 \
    obj_t* result = name(a);                                \
    unref(a);                                               \
    return result;                                          \
}

#define WRAP_FEXPR2(name)                                   \
obj_t* prim_##name(obj_t* obj)                              \
{                                                           \
    obj_t* a = prim_eval(car(obj));                         \
    ref(a);                                                 \
    obj_t* b = prim_eval(cadr(obj));                        \
    ref(b);                                                 \
    obj_t* result = name(a, b);                             \
    unref(a);                                               \
    unref(b);                                               \
    return result;                                          \
}

WRAP_BOOL_FEXPR1(is_nil)
WRAP_BOOL_FEXPR1(is_true)
WRAP_BOOL_FEXPR1(is_symbol)
WRAP_BOOL_FEXPR1(is_number)
WRAP_BOOL_FEXPR1(is_pair)
WRAP_BOOL_FEXPR1(is_prim)
WRAP_BOOL_FEXPR1(is_closure)
WRAP_BOOL_FEXPR1(is_env)

//
// pair stuff
//

WRAP_FEXPR2(cons)
WRAP_FEXPR1(car)
WRAP_FEXPR1(cdr)
WRAP_FEXPR1(cadr)
WRAP_FEXPR2(assoc)

//
// env stuff
//

obj_t* prim_def(obj_t* obj)
{
    obj_t* symbol = car(obj);
    obj_t* value = prim_eval(cadr(obj));
    ref(value);
    obj_t* result = def(symbol, value, g_env);
    unref(value);
    return result;
}

obj_t* prim_defined(obj_t* obj)
{
    obj_t* symbol = prim_eval(car(obj));
    ref(symbol);
    obj_t* result = defined(symbol, g_env);
    unref(symbol);
    return result;
}

//
// equality
//

WRAP_FEXPR2(is_eq)
WRAP_FEXPR2(is_equal)

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
        ref(arg);                               \
        assert(is_number(arg));                 \
        if (obj == root)                        \
            accum = arg->data.number;           \
        else                                    \
            accum op arg->data.number;          \
        unref(arg);                             \
        obj = cdr(obj);                         \
    } while (!is_nil(obj));                     \
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
    ref(f);
    obj_t* args = cdr(obj);
    assert(!is_immediate(f));
    obj_t* result;
    switch (f->type) {
    case PRIM_OBJ:
        result = f->data.prim(args);
        break;
    case CLOSURE_OBJ:
    {
        obj_t* local_env = make_env(make_nil(), f->data.closure.env);
        obj_t* closure_args = f->data.closure.args;
        while(!is_nil(args) && !is_nil(closure_args)) {
            def(car(closure_args), prim_eval(car(args)), local_env);
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        obj_t* closure_body = f->data.closure.body;

        // TODO: eval should take an env argument.
        // make sure to eval closure body with the local_env
        obj_t* orig_env = g_env;
        g_env = local_env;
        result = prim_eval(closure_body);
        g_env = orig_env;
        break;
    }
    default:
        assert(0);  // illegal function type
    }

    unref(f);
    return result;
}

static void capture_closure(obj_t* env, obj_t* args, obj_t* body)
{
    if (is_nil(body))
        return;

    if (body->type == SYMBOL_OBJ && is_nil(member(body, args))) {
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
    ref(expr);
    obj_t* true_body = car(cdr(obj));
    obj_t* else_body = car(cdr(cdr(obj)));
    obj_t* result;
    if (!is_nil(expr))
        result = prim_eval(true_body);
    else if (!is_nil(else_body))
        result = prim_eval(else_body);
    else
        result = make_nil();
    unref(expr);
    return result;
}
