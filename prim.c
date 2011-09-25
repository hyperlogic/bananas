#include "prim.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

typedef struct { 
    const char* name;
    prim_func_t func;
    int form;
} prim_info_t;

#define TRUE 1
#define FALSE 0

static prim_info_t s_prim_infos[] = {
    // forms
    {"define", form_define, TRUE},
    {"if", form_if, TRUE},
    {"quote", form_quote, TRUE},
    {"set!", form_set, TRUE},
    {"lambda", form_lambda, FALSE},

    // procs
    {"boolean?", proc_is_boolean, FALSE},
    {"null?", proc_is_null, FALSE},
    {"symbol?", proc_is_symbol, FALSE},
    {"number?", proc_is_number, FALSE},
    {"pair?", proc_is_pair, FALSE},
    {"procedure?", proc_is_procedure, FALSE},
    {"eq?", proc_is_eq, FALSE},
    {"equal?", proc_is_equal, FALSE},
    {"cons", proc_cons, FALSE},
    {"car", proc_car, FALSE},
    {"cdr", proc_cdr, FALSE},
    {"set-car!", proc_set_car, FALSE},
    {"set-cdr!", proc_set_cdr, FALSE},
    {"+", proc_add, FALSE},
    {"-", proc_sub, FALSE},
    {"*", proc_mul, FALSE},
    {"/", proc_div, FALSE},
    {">", proc_num_gt, FALSE},
    {">=", proc_num_gteq, FALSE},
    {"=", proc_num_eq, FALSE},
    {"<", proc_num_lt, FALSE},
    {"<=", proc_num_lteq, FALSE},
    {"abs", proc_num_abs, FALSE},
    {"print", proc_print, FALSE},
    {"eval", proc_eval, FALSE},

    {"", NULL}
};

void prim_init()
{
    // register prims
    prim_info_t* p = s_prim_infos;
    while (p->func) {
        if (p->form) {
            PUSHF();
            obj_t* symbol = PUSH(obj_make_symbol(p->name));
            obj_t* obj = PUSH(obj_make_prim_form(p->func));
            obj_env_define(g_env, symbol, obj);
            POPF();
        } else {
            PUSHF();
            obj_t* symbol = PUSH(obj_make_symbol(p->name));
            obj_t* obj = PUSH(obj_make_prim_proc(p->func));
            obj_env_define(g_env, symbol, obj);
            POPF();
        }
        p++;
    }
}

obj_t* form_define(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* form_if(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* form_quote(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* form_set(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* form_lambda(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_boolean(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_null(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_symbol(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_number(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_pair(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_procedure(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_eq(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_is_equal(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_cons(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_car(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_cdr(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_set_car(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_set_cdr(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_add(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_sub(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_mul(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_div(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_gt(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_gteq(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_eq(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_lt(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_lteq(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_num_abs(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_print(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}

obj_t* proc_eval(obj_t* obj, obj_t* env)
{
    assert(0);
    return KNULL;
}


/*
#define DEF_TYPE_PREDICATE(name)                \
    obj_t* $##name(obj_t* obj, obj_t* env)      \
    {                                           \
        while (obj_is_pair(obj)) {              \
            if (!obj_##name(obj_car(obj)))      \
                return KFALSE;                  \
            obj = obj_cdr(obj);                 \
        }                                       \
        return KTRUE;                           \
    }

DEF_TYPE_PREDICATE(is_inert)
DEF_TYPE_PREDICATE(is_ignore)
DEF_TYPE_PREDICATE(is_boolean)
DEF_TYPE_PREDICATE(is_null)
DEF_TYPE_PREDICATE(is_symbol)
DEF_TYPE_PREDICATE(is_number)
DEF_TYPE_PREDICATE(is_inexact)
DEF_TYPE_PREDICATE(is_pair)
DEF_TYPE_PREDICATE(is_environment)
DEF_TYPE_PREDICATE(is_operative)
DEF_TYPE_PREDICATE(is_applicative)

obj_t* $quote(obj_t* obj, obj_t* env)
{
    return obj_car(obj);
}

obj_t* $eq(obj_t* obj, obj_t* env)
{
    return obj_is_eq(obj_car(obj), obj_car(obj_cdr(obj))) ? KTRUE : KFALSE;
}

obj_t* $equal(obj_t* obj, obj_t* env)
{
    return obj_is_equal(obj_car(obj), obj_car(obj_cdr(obj))) ? KTRUE : KFALSE;
}

static obj_t* _eval(obj_t* obj, obj_t* env);

static void _match(obj_t* param_tree, obj_t* value_tree, obj_t* env)
{
    if (obj_is_ignore(param_tree) || obj_is_null(param_tree)) {
        return;
    } else if (obj_is_symbol(param_tree)) {
        obj_env_define(env, param_tree, value_tree);
    } else if (obj_is_pair(param_tree) && obj_is_pair(value_tree)) {
        _match(obj_car(param_tree), obj_car(value_tree), env);
        _match(obj_cdr(param_tree), obj_cdr(value_tree), env);
    }
}

static obj_t* _mapcar_eval(obj_t* obj, obj_t* env)
{
    if (obj_is_null(obj))
        return KNULL;
    else if (obj_is_pair(obj)) {
        PUSHF();
        obj_t* a = PUSH(_eval(obj_car(obj), env));  // 0
        obj_t* d = PUSH(_mapcar_eval(obj_cdr(obj), env));  // 1
        POPF_RET(obj_cons(a, d));
    }
    else
        return _eval(obj, env);
}

#define EVAL_TAIL_CALL(EXPR, ENV)               \
    do {                                        \
        obj_stack_frame_pop();                  \
        obj = EXPR;                             \
        env = ENV;                              \
        goto tail_call;                         \
    } while (0)

static obj_t* _eval(obj_t* obj, obj_t* env)
{
tail_call:
    assert(obj);
    assert(obj_is_environment(env));
    assert(!obj_is_garbage(obj));

    PUSHF();
    PUSH2(obj, env);

    if (obj_is_symbol(obj)) {
        POPF_RET(obj_env_lookup(env, obj));
    } else if (obj_is_pair(obj)) {
        obj_t* f = PUSH(_eval(obj_car(obj), env));
        obj_t* d = PUSH(obj_cdr(obj));
        if (obj_is_operative(f)) {
            if (obj_is_prim_operative(f)) {
                POPF_RET(f->data.prim_operative(d, env));
            } else {
                assert(obj_is_compound_operative(f));
                obj_t* formals = f->data.compound_operative.formals;
                obj_t* eformal = f->data.compound_operative.eformal;
                obj_t* body = f->data.compound_operative.body;
                obj_t* static_env = f->data.compound_operative.static_env;
                obj_t* local_env = PUSH(obj_make_environment(KNULL, static_env));
                _match(formals, d, local_env);
                if (obj_is_symbol(eformal))
                    obj_env_define(local_env, eformal, env);
                EVAL_TAIL_CALL(body, local_env);
            }
        } else if (obj_is_applicative(f)) {
            // let dd be the evaluated arguments d in env
            obj_t* dd = PUSH(_mapcar_eval(d, env));
            // let ff be the underlying operative of f
            obj_t* ff = f->data.applicative.operative;
            // return eval cons(f' d') in env
            obj_t* pair = PUSH(obj_cons(ff, dd));
            EVAL_TAIL_CALL(pair, env);
        } else {
            fprintf(stderr, "ERROR: f is not a applicative or operative\n");
            assert(0);  // bad f
            POPF_RET(KNULL);
        }
    } else {
        POPF_RET(obj);
    }
}

obj_t* $define(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    obj_t* param_tree = obj_car(obj);
    obj_t* ad = PUSH(_eval(obj_car(obj_cdr(obj)), env));
    _match(param_tree, ad, env);
    POPF_RET(KINERT);
}

obj_t* $eval(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    obj_t* a = obj_car(obj);
    obj_t* d = obj_cdr(obj);
    if (obj_is_pair(d))
        POPF_RET(_eval(a, obj_car(obj_cdr(obj))));
    else
        POPF_RET(_eval(a, env));
}

obj_t* $if(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    obj_t* expr = PUSH(_eval(obj_car(obj), env));
    if (!obj_is_boolean(expr)) {
        fprintf(stderr, "Error: result of $if expression is not a boolean\n");
        assert(0);
    }
    obj_t* true_body = obj_car(obj_cdr(obj));
    obj_t* else_body = obj_car(obj_cdr(obj_cdr(obj)));
    if (expr == KTRUE)
        POPF_RET(_eval(true_body, env));
    else
        POPF_RET(_eval(else_body, env));
}

obj_t* $cons(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    POPF_RET(obj_cons(obj_car(obj), obj_car(obj_cdr(obj))));
}

obj_t* $set_car(obj_t* obj, obj_t* env)
{
    obj_set_car(obj_car(obj), obj_car(obj_cdr(obj)));
    return KINERT;
}

obj_t* $set_cdr(obj_t* obj, obj_t* env)
{
    obj_set_cdr(obj_car(obj), obj_car(obj_cdr(obj)));
    return KINERT;
}

obj_t* $make_environment(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);

    obj_t* parent;
    if (obj_is_null(obj))
        parent = KNULL;
    else
        parent = obj_car(obj);

    POPF_RET(obj_make_environment(KNULL, parent));
}

obj_t* $vau(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    POPF_RET(obj_make_compound_operative(obj_car(obj), obj_car(obj_cdr(obj)),
                                         obj_car(obj_cdr(obj_cdr(obj))), env));
}

obj_t* $wrap(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env);
    if (!obj_is_operative(obj_car(obj))) {
        fprintf(stderr, "ERROR: only operatives can be wrapped\n");
        assert(0);
    }

    POPF_RET(obj_make_applicative(obj_car(obj)));
}

obj_t* $unwrap(obj_t* obj, obj_t* env)
{
    if (!obj_is_applicative(obj_car(obj))) {
        fprintf(stderr, "ERROR: only applicatives can be unwrapped\n");
        assert(0);
    }
    obj_t* result = obj_car(obj)->data.applicative.operative;
    return result;
}

#define MATH_PRIM(name, op)                     \
obj_t* $##name(obj_t* obj, obj_t* env)          \
{                                               \
    PUSHF();                                    \
    PUSH2(obj, env);                            \
    obj_t* root = obj;                          \
    double accum;                               \
    do {                                        \
        obj_t* arg = obj_car(obj);              \
        assert(obj_is_number(arg));             \
        if (obj == root)                        \
            accum = arg->data.number;           \
        else                                    \
            accum op arg->data.number;          \
        obj = obj_cdr(obj);                     \
    } while (obj_is_pair(obj));                 \
                                                \
    POPF_RET(obj_make_number(accum));           \
}

MATH_PRIM(add, +=)
MATH_PRIM(sub, -=)
MATH_PRIM(mul, *=)
MATH_PRIM(div, /=)

#define MATH_CMP_PRIM(name, op)                                 \
obj_t* $##name(obj_t* obj, obj_t* env)                          \
{                                                               \
    obj_t* a = obj_car(obj);                                    \
    assert(obj_is_number(a));                                   \
    obj_t* b = obj_cadr(obj);                                   \
    assert(obj_is_number(b));                                   \
    return a->data.number op b->data.number ? KTRUE : KFALSE;   \
}

MATH_CMP_PRIM(num_gt, >)
MATH_CMP_PRIM(num_gteq, >=)
MATH_CMP_PRIM(num_eq, ==)
MATH_CMP_PRIM(num_lt, <)
MATH_CMP_PRIM(num_lteq, <=)

#define MATH_FUNC(name, func)                                   \
obj_t* $##name(obj_t* obj, obj_t* env)                          \
{                                                               \
    PUSHF();                                                    \
    PUSH2(obj, env);                                            \
    obj_t* a = obj_car(obj);                                    \
    assert(obj_is_number(a));                                   \
    POPF_RET(obj_make_number(func(a->data.number)));            \
}

MATH_FUNC(num_abs, fabs)
// TODO: sin, cos etc..

obj_t* $print(obj_t* obj, obj_t* env)
{
    while (obj_is_pair(obj)) {
        obj_dump(obj_car(obj), 0);
        printf(" ");
        obj = obj_cdr(obj);
    }
    printf("\n");
    return KINERT;
}
*/
