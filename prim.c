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

static obj_t* _eval(obj_t* obj, obj_t* env);

static prim_info_t s_prim_infos[] = {
    // forms
    {"define", form_define, TRUE},
    {"if", form_if, TRUE},
    {"quote", form_quote, TRUE},
    {"quasiquote", form_quasiquote, TRUE},
    {"set!", form_set, TRUE},
    {"begin", form_begin, TRUE},
    {"lambda", form_lambda, TRUE},

    // procs
    {"boolean?", proc_is_boolean, FALSE},
    {"null?", proc_is_null, FALSE},
    {"symbol?", proc_is_symbol, FALSE},
    {"number?", proc_is_number, FALSE},
    {"pair?", proc_is_pair, FALSE},
    {"environment?", proc_is_environment, FALSE},
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
    {"eval", proc_eval, FALSE},
    {"print", proc_print, FALSE},
    {"not", proc_not, FALSE},
    {"make-environment", proc_make_environment, FALSE},

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

#define ENTRY_ASSERT()                          \
    assert(obj);                                \
    assert(obj_is_environment(env));            \
    assert(!obj_is_garbage(obj))

obj_t* form_define(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);

    obj_t* symbol = PUSH(obj_car(obj));
    obj_t* value = PUSH(_eval(obj_cadr(obj), env));
    obj_env_define(env, symbol, value);
    POPF_RET(symbol);
}

obj_t* form_if(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);

    obj_t* pred = PUSH(_eval(obj_car(obj), env));
    if (obj_is_null(pred) || pred == KFALSE)
        if (obj_is_pair(obj_cdr(obj_cdr(obj))))
            POPF_RET(_eval(obj_car(obj_cdr(obj_cdr(obj))), env));
        else
            POPF_RET(KNULL);
    else
        POPF_RET(_eval(obj_cadr(obj), env));
}

obj_t* form_quote(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    return obj_car(obj);
}

static obj_t* _unquoted(obj_t* e, obj_t* env)
{
    if (obj_is_pair(e)) {
        obj_t* a = obj_car(e);
        obj_t* unquote = obj_make_symbol("unquote");
        if (obj_is_symbol(a) && obj_is_eq(a, unquote)) {
            return _eval(obj_cadr(e), env);
        }
    }
    return e;
}

static obj_t* _quasiquote(obj_t* obj, obj_t* env)
{
    if (!obj_is_pair(obj))
        return obj;
    else {
        obj_t* a = obj_car(obj);
        obj_t* e = PUSH(_unquoted(a, env));
        return PUSH(obj_cons(e, _quasiquote(obj_cdr(obj), env)));
    }
}

obj_t* form_quasiquote(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    obj_t* a = obj_car(obj);
    if (obj_is_pair(obj))
        POPF_RET(_quasiquote(a, env));
    else
        POPF_RET(a);
}

obj_t* form_set(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    obj_t* symbol = PUSH(obj_car(obj));
    obj_t* new_value = PUSH(_eval(obj_cadr(obj), env));
    obj_t* old_value = PUSH(obj_env_lookup(env, symbol));
    obj_env_define(env, symbol, new_value);
    POPF_RET(old_value);
}

obj_t* form_begin(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    PUSH(KNULL);
    while (obj_is_pair(obj)) {
        obj_stack_set(2, _eval(obj_car(obj), env));
        obj = obj_cdr(obj);
    }
    POPF_RET(obj_stack_get(2));
}

obj_t* form_lambda(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    obj_t* static_env = PUSH(obj_make_environment(KNULL, env));
    POPF_RET(obj_make_comp_proc(obj_car(obj), static_env, obj_cadr(obj)));
}

#define DEF_PROC(proc_func, obj_func)                              \
obj_t* proc_func(obj_t* obj, obj_t* env)                           \
{                                                                  \
    ENTRY_ASSERT();                                                \
    return obj_func(obj_car(obj));                                 \
}

#define DEF_PROC2(proc_func, obj_func)                             \
obj_t* proc_func(obj_t* obj, obj_t* env)                           \
{                                                                  \
    ENTRY_ASSERT();                                                \
    return obj_func(obj_car(obj), obj_cadr(obj));                  \
}

#define DEF_BOOL_PROC(proc_func, obj_func)                         \
obj_t* proc_func(obj_t* obj, obj_t* env)                           \
{                                                                  \
    ENTRY_ASSERT();                                                \
    return obj_func(obj_car(obj)) ? KTRUE : KFALSE;                \
}

#define DEF_BOOL_PROC2(proc_func, obj_func)                        \
obj_t* proc_func(obj_t* obj, obj_t* env)                           \
{                                                                  \
    ENTRY_ASSERT();                                                \
    return obj_func(obj_car(obj), obj_cadr(obj)) ? KTRUE : KFALSE; \
}

#define DEF_NULL_PROC2(proc_func, obj_func)                        \
obj_t* proc_func(obj_t* obj, obj_t* env)                           \
{                                                                  \
    ENTRY_ASSERT();                                                \
    obj_func(obj_car(obj), obj_cadr(obj));                         \
    return KNULL;                                                  \
}

DEF_BOOL_PROC(proc_is_boolean, obj_is_boolean)
DEF_BOOL_PROC(proc_is_null, obj_is_null)
DEF_BOOL_PROC(proc_is_symbol, obj_is_symbol)
DEF_BOOL_PROC(proc_is_number, obj_is_number)
DEF_BOOL_PROC(proc_is_pair, obj_is_pair)
DEF_BOOL_PROC(proc_is_environment, obj_is_environment)
DEF_BOOL_PROC(proc_is_procedure, obj_is_proc)
DEF_BOOL_PROC2(proc_is_eq, obj_is_eq)
DEF_BOOL_PROC2(proc_is_equal, obj_is_equal)
DEF_PROC2(proc_cons, obj_cons)
DEF_PROC(proc_car, obj_car)
DEF_PROC(proc_cdr, obj_cdr)
DEF_NULL_PROC2(proc_set_car, obj_set_car)
DEF_NULL_PROC2(proc_set_cdr, obj_set_cdr)

#define DEF_MATH_PROC(proc_func, op, ident)         \
obj_t* proc_func(obj_t* obj, obj_t* env)            \
{                                                   \
    ENTRY_ASSERT();                                 \
    PUSHF();                                        \
    PUSH2(obj, env);                                \
    PUSH(KNULL);                                    \
    obj_t* root = obj;                              \
    double accum = ident;                           \
    while (obj_is_pair(obj)) {                      \
        obj_t* arg = obj_car(obj);                  \
        assert(obj_is_number(arg));                 \
        if (obj == root)                            \
            accum = arg->data.number;               \
        else                                        \
            accum op arg->data.number;              \
        obj = obj_cdr(obj);                         \
    }                                               \
                                                    \
    POPF_RET(obj_make_number(accum));               \
}

DEF_MATH_PROC(proc_add, +=, 0.0)
DEF_MATH_PROC(proc_mul, *=, 1.0)
DEF_MATH_PROC(proc_div, /=, 1.0)

obj_t* proc_sub(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    if (obj_is_null(obj)) {
        // no args
        POPF_RET(obj_make_number(0.0));
    } else if (obj_is_null(obj_cdr(obj))) {
        // one arg
        obj_t* arg = obj_car(obj);
        POPF_RET(obj_make_number(-arg->data.number));
    } else if (obj_is_pair(obj_cdr(obj))) {
        // two or more args
        obj_t* root = obj;
        double accum = 0.0f;
        PUSH(KNULL);
        while (obj_is_pair(obj)){
            obj_t* arg = obj_car(obj);
            assert(obj_is_number(arg));
            if (obj == root)
                accum = arg->data.number;
            else
                accum -= arg->data.number;
            obj = obj_cdr(obj);
        }
        POPF_RET(obj_make_number(accum));
    } else {
        assert(0);
        POPF_RET(obj_make_number(0.0));
    }
    return KNULL;
}

#define DEF_MATH_CMP_PROC(proc_func, op)                        \
obj_t* proc_func(obj_t* obj, obj_t* env)                        \
{                                                               \
    ENTRY_ASSERT();                                             \
    obj_t* a = obj_car(obj);                                    \
    assert(obj_is_number(a));                                   \
    obj_t* b = obj_cadr(obj);                                   \
    assert(obj_is_number(b));                                   \
    return a->data.number op b->data.number ? KTRUE : KFALSE;   \
}

DEF_MATH_CMP_PROC(proc_num_gt, >)
DEF_MATH_CMP_PROC(proc_num_gteq, >=)
DEF_MATH_CMP_PROC(proc_num_eq, ==)
DEF_MATH_CMP_PROC(proc_num_lt, <)
DEF_MATH_CMP_PROC(proc_num_lteq, <=)

#define MATH_FUNC(proc_func, obj_func)                          \
obj_t* proc_func(obj_t* obj, obj_t* env)                        \
{                                                               \
    ENTRY_ASSERT();                                             \
    PUSHF();                                                    \
    PUSH2(obj, env);                                            \
    obj_t* a = obj_car(obj);                                    \
    assert(obj_is_number(a));                                   \
    POPF_RET(obj_make_number(obj_func(a->data.number)));        \
}

// TODO: make this iterative.
static obj_t* _map_eval(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env); // 0, 1
    if (obj_is_null(obj)) {
        POPF_RET(KNULL);
    }
    if (obj_is_pair(obj)) {
        obj_t* a = PUSH(_eval(obj_car(obj), env));
        POPF_RET(obj_cons(a, _map_eval(obj_cdr(obj), env)));
    } else {
        assert(0);
        POPF_RET(KNULL);
    }
}

MATH_FUNC(proc_num_abs, fabs)
// TODO: sin, cos etc..

// TODO: tail call optimization
static obj_t* _eval(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);

    if (obj_is_symbol(obj)) {
        POPF_RET(obj_env_lookup(env, obj));
    } else if (obj_is_pair(obj)) {
        obj_t* args = PUSH(obj_cons(obj_car(obj), KNULL));
        obj_t* f = PUSH(proc_eval(args, env));
        obj_t* d = PUSH(obj_cdr(obj));
        switch (f->type) {
        case PRIM_FORM_OBJ:
            POPF_RET(f->data.prim_func(d, env));
        case PRIM_PROC_OBJ:
        {
            obj_t* dd = PUSH(_map_eval(d, env));
            POPF_RET(f->data.prim_func(dd, env));
            break;
        }
        case COMP_PROC_OBJ:
        {
            obj_t* dynamic_env = PUSH(obj_make_environment(KNULL, env));
            obj_t* formals = f->data.comp_proc.formals;
            obj_t* body = f->data.comp_proc.body;
            while (obj_is_pair(formals)) {
                obj_env_define(dynamic_env, obj_car(formals), _eval(obj_car(d), env));
                formals = obj_cdr(formals);
                d = obj_cdr(d);
            }
            POPF_RET(_eval(body, dynamic_env));
            break;
        }
        default:
            fprintf(stderr, "ERROR: f is not a procedure or form\n");
            assert(0);  // bad f
            POPF_RET(KNULL);
        }
    } else {
        POPF_RET(obj);
    }
}

obj_t* proc_eval(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);

    obj_t* a = obj_car(obj);
    obj_t* d = obj_cdr(obj);
    if (obj_is_pair(d))
        POPF_RET(_eval(a, obj_car(d)));
    else
        POPF_RET(_eval(a, env));
}

obj_t* proc_print(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH(KNULL);
    while (obj_is_pair(obj)) {
        obj_stack_set(0, obj_car(obj));
        obj_dump(obj_stack_get(0), 0);
        printf(" ");
        obj = obj_cdr(obj);
    }
    printf("\n");
    POPF_RET(KNULL);
}

obj_t* proc_not(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    obj_t* a = PUSH(obj_car(obj));
    POPF_RET((a == KNULL || a == KFALSE) ? KTRUE : KFALSE);
}

obj_t* proc_make_environment(obj_t* obj, obj_t* env)
{
    ENTRY_ASSERT();
    PUSHF();
    PUSH2(obj, env);
    POPF_RET(obj_make_environment(KNULL, env));
}
