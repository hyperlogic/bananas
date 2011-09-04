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
    {"wrap", $wrap, 1},
    {"unwrap", $unwrap, 1},
    {"+", $add, 1},
    {"-", $sub, 1},
    {"*", $mul, 1},
    {"/", $div, 1},
    {">?", $gt, 1},
    {">=?", $gteq, 1},
    {"<?", $lt, 1},
    {"<=?", $lteq, 1},

    {"", NULL}
};

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->prim) {
        if (p->wrap) {
            obj_t* symbol = obj_make_symbol(p->name);
            obj_t* operative = obj_make_prim_operative(p->prim);
            obj_t* applicative = obj_make_applicative(operative);
            $define(obj_cons_deny(symbol, obj_cons_deny(applicative, KNULL)), g_env);
            obj_unref(applicative);
            obj_unref(operative);
            obj_unref(symbol);

        } else {
            obj_t* symbol = obj_make_symbol(p->name);
            obj_t* operative = obj_make_prim_operative(p->prim);
            $define(obj_cons_deny(symbol, obj_cons_deny(operative, KNULL)), g_env);
            obj_unref(operative);
            obj_unref(symbol);
        }
        p++;
    }
}

#define DEF_TYPE_PREDICATE(name)                \
    obj_t* $##name(obj_t* obj, obj_t* env)      \
    {                                           \
        while (obj_is_pair(obj)) {              \
            if (!obj_##name(obj_car_deny(obj)))      \
                return KFALSE;                  \
            obj = obj_cdr_deny(obj);            \
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
    return obj_car_own(obj);
}

obj_t* $eq(obj_t* obj, obj_t* env)
{
    return obj_is_eq(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj))) ? KTRUE : KFALSE;
}

obj_t* $equal(obj_t* obj, obj_t* env)
{
    return obj_is_equal(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj))) ? KTRUE : KFALSE;
}

static obj_t* _eval(obj_t* obj, obj_t* env);

static void _match(obj_t* param_tree, obj_t* value_tree, obj_t* env)
{
    if (obj_is_ignore(param_tree) || obj_is_null(param_tree)) {
        return;
    } else if (obj_is_symbol(param_tree)) {
        obj_env_define(env, param_tree, value_tree);
    } else if (obj_is_pair(param_tree) && obj_is_pair(value_tree)) {
        _match(obj_car_deny(param_tree), obj_car_deny(value_tree), env);
        _match(obj_cdr_deny(param_tree), obj_cdr_deny(value_tree), env);
    }
}

static obj_t* _mapcar_eval(obj_t* obj, obj_t* env)
{
    if (obj_is_null(obj))
        return KNULL;
    else if (obj_is_pair(obj)) {
        obj_t* a = _eval(obj_car_deny(obj), env);
        obj_t* d = _mapcar_eval(obj_cdr_deny(obj), env);
        obj_t* result = obj_cons_own(a, d);
        obj_unref(d);
        obj_unref(a);
        return result;
    }
    else
        return _eval(obj, env);
}

static obj_t* _eval(obj_t* obj, obj_t* env)
{
    assert(obj);
    assert(obj_is_environment(env));

    if (obj_is_symbol(obj)) {
        return obj_env_lookup_own(env, obj);
    } else if (obj_is_pair(obj)) {
        obj_t* f = _eval(obj_car_deny(obj), env);
        obj_t* d = obj_cdr_deny(obj);
        obj_t* result = KNULL;
        if (obj_is_operative(f)) {
            if (obj_is_prim_operative(f)) {
                return f->data.prim_operative(d, env);
            } else {
                assert(obj_is_compound_operative(f));
                obj_t* formals = f->data.compound_operative.formals;
                obj_t* eformal = f->data.compound_operative.eformal;
                obj_t* body = f->data.compound_operative.body;
                obj_t* static_env = f->data.compound_operative.static_env;
                obj_t* local_env = obj_make_environment(KNULL, static_env);
                _match(formals, d, local_env);
                if (obj_is_symbol(eformal))
                    obj_env_define(local_env, eformal, env);
                result = _eval(body, local_env);
                obj_unref(local_env);
            }
        } else if (obj_is_applicative(f)) {
            // let dd be the evaluated arguments d in env
            obj_t* dd = _mapcar_eval(d, env);
            // let ff be the underlying operative of f
            obj_t* ff = f->data.applicative.operative;
            // return eval cons(f' d') in env
            obj_t* ops = obj_cons_own(ff, dd);
            result = _eval(ops, env);
            obj_unref(ops);
            obj_unref(dd);
        } else {
            fprintf(stderr, "ERROR: f is not a applicative or operative\n");
            assert(0);  // bad f
        }
        obj_unref(f);
        return result;
    } else {
        obj_ref(obj);
        return obj;
    }
}

obj_t* $define(obj_t* obj, obj_t* env)
{
    obj_t* param_tree = obj_car_deny(obj);
    obj_t* value_tree = _eval(obj_car_deny(obj_cdr_deny(obj)), env);
    _match(param_tree, value_tree, env);
    obj_unref(value_tree);
    return KINERT;
}

obj_t* $eval(obj_t* obj, obj_t* env)
{
    obj_t* a = obj_car_deny(obj);
    obj_t* d = obj_cdr_deny(obj);
    if (obj_is_pair(d))
        return _eval(a, obj_car_deny(obj_cdr_deny(obj)));
    else
        return _eval(a, env);
}

obj_t* $if(obj_t* obj, obj_t* env)
{
    obj_t* expr = _eval(obj_car_deny(obj), env);
    if (!obj_is_boolean(expr)) {
        obj_unref(expr);
        fprintf(stderr, "Error: result of $if expression is not a boolean\n");
        assert(0);
    }
    obj_t* true_body = obj_car_deny(obj_cdr_deny(obj));
    obj_t* else_body = obj_car_deny(obj_cdr_deny(obj_cdr_deny(obj)));
    if (expr == KTRUE)
        return _eval(true_body, env);
    else
        return _eval(else_body, env);
}

obj_t* $cons(obj_t* obj, obj_t* env)
{
    return obj_cons_own(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj)));
}

obj_t* $set_car(obj_t* obj, obj_t* env)
{
    obj_set_car(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj)));
    return KINERT;
}

obj_t* $set_cdr(obj_t* obj, obj_t* env)
{
    obj_set_cdr(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj)));
    return KINERT;
}

obj_t* $make_environment(obj_t* obj, obj_t* env)
{
    obj_t* parent;
    if (obj_is_null(obj))
        parent = KNULL;
    else
        parent = obj_car_deny(obj);
    return obj_make_environment(KNULL, parent);
}

obj_t* $vau(obj_t* obj, obj_t* env)
{
    return obj_make_compound_operative(obj_car_deny(obj), obj_car_deny(obj_cdr_deny(obj)), 
                                       obj_car_deny(obj_cdr_deny(obj_cdr_deny(obj))), env);
}

obj_t* $wrap(obj_t* obj, obj_t* env)
{
    if (!obj_is_operative(obj_car_deny(obj))) {
        fprintf(stderr, "ERROR: only operatives can be wrapped\n");
        assert(0);
    }
    return obj_make_applicative(obj_car_deny(obj));
}

obj_t* $unwrap(obj_t* obj, obj_t* env)
{
    if (!obj_is_applicative(obj_car_deny(obj))) {
        fprintf(stderr, "ERROR: only applicatives can be unwrapped\n");
        assert(0);
    }
    obj_t* result = obj_car_deny(obj)->data.applicative.operative;
    obj_ref(result);
    return result;
}

#define MATH_PRIM(name, op)                     \
obj_t* $##name(obj_t* obj, obj_t* env)          \
{                                               \
    obj_t* root = obj;                          \
    double accum;                               \
    do {                                        \
        obj_t* arg = obj_car_deny(obj);         \
        assert(obj_is_number(arg));             \
        if (obj == root)                        \
            accum = arg->data.number;           \
        else                                    \
            accum op arg->data.number;          \
        obj = obj_cdr_deny(obj);                \
    } while (obj_is_pair(obj));                 \
    return obj_make_number(accum);              \
}

MATH_PRIM(add, +=)
MATH_PRIM(sub, -=)
MATH_PRIM(mul, *=)
MATH_PRIM(div, /=)

#define MATH_CMP_PRIM(name, op)                                 \
obj_t* $##name(obj_t* obj, obj_t* env)                          \
{                                                               \
    obj_t* a = obj_car_deny(obj);                               \
    assert(obj_is_number(a));                                   \
    obj_t* b = obj_car_deny(obj_cdr_deny(obj));                 \
    assert(obj_is_number(b));                                   \
    return a->data.number op b->data.number ? KTRUE : KFALSE;   \
}

MATH_CMP_PRIM(gt, >)
MATH_CMP_PRIM(gteq, >=)
MATH_CMP_PRIM(lt, <)
MATH_CMP_PRIM(lteq, <=)
