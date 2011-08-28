#include "prim.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef struct { 
    const char* name;
    prim_operative_t prim;
    int wrap;
} prim_info_t;

static prim_info_t s_prim_infos[] = {

    {"inert?", $is_inert, 1}, 
    {"ignore?", $is_ignore, 1}, 
    {"boolean?", $is_boolean, 1}, 
    {"null?", $is_null, 1}, 
    {"symbol?", $is_symbol, 1}, 
    {"number?", $is_number, 1}, 
    {"pair?", $is_pair, 1}, 
    {"environment?", $is_environment, 1}, 
    {"operative?", $is_operative, 1}, 
    {"applicative?", $is_applicative, 1}, 
    {"$quote", $quote, 0}, 
    {"eq?", $eq, 1},
    {"equal?", $equal, 1},
    {"eval", $eval, 1},
    {"$define!", $define, 0}, 
    {"$if", $if, 0},
    {"cons", $cons, 1},
    {"set-car!", $set_car, 1},
    {"set-cdr!", $set_cdr, 1},
    {"make-environment", $make_environment, 1},
    {"$vau", $vau, 0},

    {"", NULL}
};

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        if (p->wrap) {
            obj_t* symbol = make_symbol(p->name);
            obj_t* applicative = make_applicative(make_prim_operative(p->prim));
            $define(cons(symbol, cons(applicative, KNULL)), g_env);
        } else {
            obj_t* symbol = make_symbol(p->name);
            obj_t* prim = make_prim_operative(p->prim);
            $define(cons(symbol, cons(prim, KNULL)), g_env);
        }
        p++;
    }
}

#define DEF_TYPE_PREDICATE(name)                \
    obj_t* $##name(obj_t* obj, obj_t* env)      \
    {                                           \
        while (is_pair(obj)) {                  \
            if (! name(car(obj)))               \
                return KFALSE;                  \
            obj = cdr(obj);                     \
        }                                       \
        return KTRUE;                           \
    }

DEF_TYPE_PREDICATE(is_inert)
DEF_TYPE_PREDICATE(is_ignore)
DEF_TYPE_PREDICATE(is_boolean)
DEF_TYPE_PREDICATE(is_null)
DEF_TYPE_PREDICATE(is_symbol)
DEF_TYPE_PREDICATE(is_number)
DEF_TYPE_PREDICATE(is_pair)
DEF_TYPE_PREDICATE(is_environment)
DEF_TYPE_PREDICATE(is_operative)
DEF_TYPE_PREDICATE(is_applicative)

obj_t* $quote(obj_t* obj, obj_t* env) 
{ 
    return car(obj);
}

obj_t* $eq(obj_t* obj, obj_t* env)
{
    return is_eq(car(obj), car(cdr(obj))) ? KTRUE : KFALSE;
}

obj_t* $equal(obj_t* obj, obj_t* env)
{
    return is_equal(car(obj), car(cdr(obj))) ? KTRUE : KFALSE;
}

static obj_t* eval(obj_t* obj, obj_t* env);

static obj_t* mapcar_eval(obj_t* obj, obj_t* env)
{
    obj_t* result = KNULL;
    obj_t* a = KNULL;
    while (is_pair(obj)) {
        obj_t* pair = cons(KNULL, KNULL);
        if (is_null(a)) {
            result = pair;
            a = pair;
        } else {
            set_cdr(a, pair);
        }
        set_car(pair, eval(car(obj), env));
        obj = cdr(obj);
        a = pair;
    }
    return result;
}

static void match(obj_t* param_tree, obj_t* value_tree, obj_t* env)
{
    if (is_ignore(param_tree) || is_null(param_tree)) {
        return;
    } else if (is_symbol(param_tree)) {
        env_define(env, param_tree, value_tree);
    } else if (is_pair(param_tree) && is_pair(value_tree)) {
        match(car(param_tree), car(value_tree), env);
        match(cdr(param_tree), cdr(value_tree), env);
    }
}

static obj_t* eval(obj_t* obj, obj_t* env)
{
    assert(obj);
    assert(is_environment(env));
    if (is_symbol(obj))
        return env_lookup(env, obj);
    else if (is_pair(obj)) {
        obj_t* f = eval(car(obj), env);
        obj_t* d = cdr(obj);
        if (is_operative(f)) {
            if (is_prim_operative(f)) {
                return f->data.prim_operative(d, env);
            } else {
                assert(is_compound_operative(f));
                obj_t* formals = f->data.compound_operative.formals;
                obj_t* eformal = f->data.compound_operative.eformal;
                obj_t* body = f->data.compound_operative.body;
                obj_t* static_env = f->data.compound_operative.static_env;

                obj_t* local_env = make_environment(KNULL, static_env);
                match(formals, d, local_env);
                if (is_symbol(eformal))
                    env_define(local_env, eformal, eval(eformal, env));
                return eval(body, local_env);
            }
        } else if (is_applicative(f)) {
            // let dd be the evaluated arguments d in env
            obj_t* dd = mapcar_eval(d, env);
            ref(dd);

            // let ff be the underlying operative of f
            obj_t* ff = f->data.applicative.operative;

            // return eval cons(f' d') in env
            obj_t* result = eval(cons(ff, dd), env);
            unref(dd);
            return result;
        } else {
            fprintf(stderr, "ERROR: f is not a applicative or operative\n");
            assert(0);  // bad f
        }
    } else 
        return obj;
}

obj_t* $define(obj_t* obj, obj_t* env)
{
    obj_t* param_tree = car(obj);
    obj_t* value_tree = eval(car(cdr(obj)), env);
    match(param_tree, value_tree, env);
    return KINERT;
}

obj_t* $eval(obj_t* obj, obj_t* env)
{
    obj_t* a = car(obj);
    obj_t* d = cdr(obj);
    if (is_pair(d))
        return eval(a, car(cdr(obj)));
    else
        return eval(a, env);
}

obj_t* $if(obj_t* obj, obj_t* env)
{
    obj_t* expr = eval(car(obj), env);
    ref(expr);
    if (!is_boolean(expr)) {
        unref(expr);
        fprintf(stderr, "Error: result of $if expression is not a boolean\n");
        assert(0);
    }
    obj_t* true_body = car(cdr(obj));
    obj_t* else_body = car(cdr(cdr(obj)));
    obj_t* result;
    if (expr == KTRUE)
        result = eval(true_body, env);
    else
        result = eval(else_body, env);
    unref(expr);
    return result;
}

obj_t* $cons(obj_t* obj, obj_t* env)
{
    return cons(car(obj), car(cdr(obj)));
}

obj_t* $set_car(obj_t* obj, obj_t* env)
{
    set_car(car(obj), car(cdr(obj)));
    return KINERT;
}

obj_t* $set_cdr(obj_t* obj, obj_t* env)
{
    set_cdr(car(obj), car(cdr(obj)));
    return KINERT;
}

obj_t* $make_environment(obj_t* obj, obj_t* env)
{
    obj_t* parent;
    if (is_null(obj))
        parent = KNULL;
    else
        parent = car(obj);
    ref(parent);
    obj_t* result = make_environment(KNULL, parent);
    unref(parent);
    return result;
}

obj_t* $vau(obj_t* obj, obj_t* env)
{
    return make_compound_operative(car(obj), car(cdr(obj)), car(cdr(cdr(obj))), env);
}

/*
//
// pair stuff
//

WRAP_FEXPR2(cons)
WRAP_FEXPR1(car)
WRAP_FEXPR1(cdr)
WRAP_FEXPR1(cadr)
WRAP_FEXPR2(assoc)
WRAP_FEXPR2(set_car)
WRAP_FEXPR2(set_cdr)

//
// env stuff
//
obj_t* prim_curr_env(obj_t* obj, obj_t* env)
{
    return env;
}

obj_t* prim_make_env(obj_t* obj, obj_t* env)
{
    obj_t* parent;
    if (is_nil(obj))
        parent = make_nil();
    else
        parent = eval(car(obj), env);
    ref(parent);
    obj_t* result = make_env(make_nil(), parent);
    unref(parent);
    return result;
}

obj_t* prim_def(obj_t* obj, obj_t* env)
{
    obj_t* symbol = car(obj);
    obj_t* value = eval(cadr(obj), env);
    ref(value);
    obj_t* result = def(symbol, value, env);
    unref(value);
    return result;
}

obj_t* prim_defined(obj_t* obj, obj_t* env)
{
    obj_t* symbol = eval(car(obj), env);
    ref(symbol);
    obj_t* result = defined(symbol, env);
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
obj_t* prim_##name(obj_t* obj, obj_t* env)      \
{                                               \
    obj_t* root = obj;                          \
    double accum;                               \
    do {                                        \
        obj_t* arg = eval(car(obj), env);       \
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

obj_t* prim_eval(obj_t* obj, obj_t* env)
{
    obj_t* expr = eval(car(obj), env);
    ref(expr);
    if (is_pair(cdr(obj))) {
        obj_t* env_arg = eval(cadr(obj), env);
        ref(env_arg);
        obj_t* result = eval(expr, env_arg);
        unref(env_arg);
        unref(expr);
        return result;
    }
    else {
        obj_t* result = eval(expr, env);
        unref(expr);
        return result;
    }
}

obj_t* prim_apply(obj_t* obj, obj_t* env)
{
    obj_t* args = car(obj);
    if (!is_nil(cdr(obj)))
        return apply(args, cadr(obj));
    else
        return apply(args, env);
}

static void capture_closure(obj_t* local_env, obj_t* static_env, obj_t* args, obj_t* body)
{
    if (is_nil(body))
        return;

    if (body->type == SYMBOL_OBJ && is_nil(member(body, args))) {
        def(body, eval(body, static_env), local_env);
    } else if (body->type == PAIR_OBJ) {
        capture_closure(local_env, static_env, args, car(body));
        capture_closure(local_env, static_env, args, cdr(body));
    }
}

obj_t* prim_lambda(obj_t* n, obj_t* env)
{
    obj_t* args = car(n);
    obj_t* body = cadr(n);
    obj_t* local_env = make_env(make_nil(), make_nil());

    // evaluates symbols in the body using the static_env.
    // these values are then captured in the local_env.
    capture_closure(local_env, env, args, body);

    return make_closure(args, body, local_env);
}
*/
