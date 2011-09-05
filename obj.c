// (setq show-trailing-whitespace t)
#include "obj.h"
#include "parse.h"
#include "symbol.h"
#include "prim.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

// static object pool
#define MAX_OBJS 131072 * 2
obj_t g_obj_pool[MAX_OBJS];  // 16 meg

// free list
obj_t* g_free_objs = NULL;
int g_num_free_objs = 0;

// used list
obj_t* g_used_objs = NULL;
int g_num_used_objs = 0;

// root environment
obj_t* g_env = NULL;

static obj_t* _assq_deny(obj_t* key, obj_t* plist);

//
// obj pool
//

static void _pool_init()
{
    // init obj pool
    int i;
    for (i = 0; i < MAX_OBJS - 1; i++)
        g_obj_pool[i].next = g_obj_pool + i + 1;
    g_free_objs = g_obj_pool;
    g_num_free_objs = MAX_OBJS;
    g_num_used_objs = 0;
}

static obj_t* _pool_alloc()
{
    // take from front of free list
    assert(g_free_objs);
    obj_t* obj = g_free_objs;
    g_free_objs = g_free_objs->next;
    g_free_objs->prev = NULL;
    g_num_free_objs--;

    // add to front of used list
    obj->next = g_used_objs;
    if (g_used_objs)
        g_used_objs->prev = obj;
    g_used_objs = obj;
    g_num_used_objs++;

    assert(g_num_used_objs + g_num_free_objs == MAX_OBJS);

    obj->mark = 0;
    return obj;
}

static void _pool_free(obj_t* obj)
{
#ifdef GC_DEBUG
    fprintf(stderr, "FREE obj %p, \n", obj);
    obj_dump(obj, 1);
    fprintf(stderr, "\n");
#endif

    assert(obj);

    // remove from used list
    if (obj->prev)
        obj->prev->next = obj->next;
    if (obj->next)
        obj->next->prev = obj->prev;
    g_num_used_objs--;

    // add to start of free list
    obj->next = g_used_objs;
    g_used_objs->prev = obj;
    g_used_objs = obj;
    g_num_free_objs++;

    assert(g_num_used_objs + g_num_free_objs == MAX_OBJS);
}

//
// obj makers
//

obj_t* obj_make_symbol(const char* str)
{
    int len = strlen(str);
    int id = symbol_find(str, len);
    if (id < 0)
        id = symbol_add(str, len);

    obj_t* obj = _pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, symbol = %s\n", obj, symbol_get(id));
#endif

    return obj;
}

obj_t* obj_make_symbol2(const char* start, const char* end)
{
    int len = end - start;
    int id = symbol_find(start, len);
    if (id < 0)
        id = symbol_add(start, len);

    obj_t* obj = _pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, symbol = %s\n", obj, symbol_get(id));
#endif

    return obj;
}

obj_t* obj_make_number(double num)
{
    obj_t* obj = _pool_alloc();
    obj->type = NUMBER_OBJ;
    obj->data.number = num;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, number = %f\n", obj, num);
#endif

    return obj;
}

obj_t* obj_make_number2(const char* start, const char* end)
{
    double num = 0;
    int len = end - start;
    if (len > 0) {
        char* temp = (char*)malloc(len + 1);
        memcpy(temp, start, len);
        temp[len] = 0;
        num = atof(temp);
        free(temp);
    }

    return obj_make_number(num);
}

obj_t* obj_make_pair(obj_t* car, obj_t* cdr)
{
    obj_t* obj = _pool_alloc();
    obj->type = PAIR_OBJ;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, pair\n", obj);
#endif

    return obj;
}

obj_t* obj_make_environment(obj_t* plist, obj_t* parent)
{
    obj_t* obj = _pool_alloc();
    obj->type = ENV_OBJ;
    obj->data.env.plist = plist;
    obj->data.env.parent = parent;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, environment\n", obj);
#endif

    return obj;
}

obj_t* obj_make_prim_operative(prim_operative_t prim)
{
    obj_t* obj = _pool_alloc();
    obj->type = PRIM_OPERATIVE_OBJ;
    obj->data.prim_operative = prim;  // prim is a c-function, no need to ref it.

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, prim-operative\n", obj);
#endif

    return obj;
}

obj_t* obj_make_compound_operative(obj_t* formals, obj_t* eformal, obj_t* body, obj_t* static_env)
{
    obj_t* obj = _pool_alloc();
    obj->type = COMPOUND_OPERATIVE_OBJ;
    obj->data.compound_operative.formals = formals;
    obj->data.compound_operative.eformal = eformal;
    obj->data.compound_operative.body = body;
    obj->data.compound_operative.static_env = static_env;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, compound-operative\n", obj);
#endif

    return obj;
}

obj_t* obj_make_applicative(obj_t* operative)
{
    obj_t* obj = _pool_alloc();
    obj->type = APPLICATIVE_OBJ;
    obj->data.applicative.operative = operative;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, applicative\n", obj);
#endif

    return obj;
}

//
// obj type predicates
//

int obj_is_immediate(obj_t* obj)
{
    assert(obj);
    return (long)obj & IMM_TAG;
}

int obj_is_inert(obj_t* obj)
{
    assert(obj);
    return obj == KINERT;
}

int obj_is_ignore(obj_t* obj)
{
    assert(obj);
    return obj == KIGNORE;
}

int obj_is_boolean(obj_t* obj)
{
    assert(obj);
    return obj == KTRUE || obj == KFALSE;
}

int obj_is_null(obj_t* obj)
{
    assert(obj);
    return obj == KNULL;
}

int obj_is_symbol(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == SYMBOL_OBJ;
}

int obj_is_number(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == NUMBER_OBJ;
}

int obj_is_inexact(obj_t* obj)
{
    assert(obj);
    return 1;
}

int obj_is_pair(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == PAIR_OBJ && !obj_is_null(obj);
}

int obj_is_environment(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == ENV_OBJ;
}

int obj_is_prim_operative(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == PRIM_OPERATIVE_OBJ;
}

int obj_is_compound_operative(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == COMPOUND_OPERATIVE_OBJ;
}

int obj_is_operative(obj_t* obj)
{
    assert(obj);
    return obj_is_prim_operative(obj) || obj_is_compound_operative(obj);
}

int obj_is_applicative(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == APPLICATIVE_OBJ;
}

obj_t* obj_cons(obj_t* a, obj_t* b)
{
    return obj_make_pair(a, b);
}

obj_t* obj_car(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.car;
}

obj_t* obj_cdr(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.cdr;
}

obj_t* obj_cadr(obj_t* obj)
{
    return obj_car(obj_cdr(obj));
}

void obj_set_car(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj->data.pair.car = value;
}

void obj_set_cdr(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj->data.pair.cdr = value;
}

int obj_is_eq(obj_t* a, obj_t* b)
{
    if (obj_is_immediate(a) && obj_is_immediate(b)) {
        return a == b;
    } else if (!obj_is_immediate(a) && !obj_is_immediate(b) && a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol;
        case NUMBER_OBJ:
            return a->data.number == b->data.number;
        default:
            return a == b;
        }
    }
    return 0;
}

int obj_is_equal(obj_t* a, obj_t* b)
{
    if (obj_is_pair(a) && obj_is_pair(b)) {
        obj_t* a_head = obj_car(a);
        obj_t* a_tail = obj_cdr(a);
        obj_t* b_head = obj_car(b);
        obj_t* b_tail = obj_cdr(b);
        return obj_is_equal(a_head, b_head) && obj_is_equal(a_tail, b_tail);
    }
    else
        return obj_is_eq(a, b);
}

obj_t* obj_env_lookup(obj_t* env, obj_t* symbol)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    obj_t* pair = _assq_deny(symbol, env->data.env.plist);
    if (!obj_is_null(pair)) {
        return obj_cdr(pair);
    }
    else {
        if (obj_is_environment(env->data.env.parent))
            return obj_env_lookup(env->data.env.parent, symbol);
        else {

            // AJT: REMOVE
            fprintf(stderr, "Warning: could not find symbol \"%s\" in env\n", symbol_get(symbol->data.symbol));

            return KNULL;
        }
    }
}

void obj_env_define(obj_t* env, obj_t* symbol, obj_t* value)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    obj_t* pair = _assq_deny(symbol, env->data.env.plist);
    if (obj_is_null(pair)) {
        // did not find it. so add a new property to the beginning of the plist.
        obj_t* plist = env->data.env.plist;
        env->data.env.plist = obj_cons(obj_cons(symbol, value), plist);
    } else {
        // found it, change the value
        obj_set_cdr(pair, value);
    }
}

static obj_t* _assq_deny(obj_t* key, obj_t* plist)
{
    while (obj_is_pair(plist)) {
        if (obj_is_eq(key, obj_car(obj_car(plist))))
            return obj_car(plist);
        plist = obj_cdr(plist);
    }
    return KNULL;
}

//
// debug output
//

#define PRINTF(args...)                          \
    do {                                         \
        if (to_stderr)                           \
            fprintf(stderr, args);               \
        else                                     \
            printf(args);                        \
    } while(0)

void obj_dump(obj_t* obj, int to_stderr)
{
    if (obj_is_immediate(obj)) {
        if (obj == KIGNORE)
            PRINTF("#ignore");
        else if (obj == KINERT)
            PRINTF("#inert");
        else if (obj == KTRUE)
            PRINTF("#t");
        else if (obj == KFALSE)
            PRINTF("#f");
        else if (obj == KNULL)
            PRINTF("()");
        else
            PRINTF("#???");
    } else {
        switch (obj->type) {
        case NUMBER_OBJ:
            PRINTF("%f", obj->data.number);
            break;
        case SYMBOL_OBJ:
            PRINTF("%s", symbol_get(obj->data.symbol));
            break;
        case PAIR_OBJ:
            PRINTF("(");
            while (obj_is_pair(obj)) {
                obj_dump(obj_car(obj), to_stderr);
                obj = obj_cdr(obj);
                if (!obj_is_null(obj) && !obj_is_pair(obj)) {
                    PRINTF(" . ");
                    obj_dump(obj, to_stderr);
                    break;
                }
                if (!obj_is_null(obj))
                    PRINTF(" ");
            }
            PRINTF(")");
            break;
        case ENV_OBJ:
            PRINTF("#<env 0x%p>", obj);
            break;
        case PRIM_OPERATIVE_OBJ:
            PRINTF("#<prim-operative 0x%p>", obj);
            break;
        case COMPOUND_OPERATIVE_OBJ:
            PRINTF("#<compound-operative 0x%p>", obj);
            break;
        case APPLICATIVE_OBJ:
            PRINTF("#<applicative 0x%p>", obj);
            break;
        default:
            PRINTF("???");
            break;
        }
    }
}

obj_t* obj_eval_expr(obj_t* obj, obj_t* env)
{
    obj_t* temp_expr = obj_cons(obj, KNULL);
    obj_t* result = $eval(temp_expr, env);
    return result;
}

obj_t* obj_eval_str(const char* str, obj_t* env)
{
   obj_t* temp_expr = read(str);
   obj_t* result = obj_eval_expr(temp_expr, env);
   return result;
}

//
// interpreter init, this needs happen before any thing else.
//

// TODO: GC FIX
void obj_init()
{
    assert(sizeof(obj_t) == 64);

    _pool_init();
    g_env = obj_make_environment(KNULL, KNULL);

    obj_t* symbol = obj_make_symbol("#e-infinity");
    obj_t* value = obj_make_number(-INFINITY);
    $define(obj_cons(symbol, obj_cons(value, KNULL)), g_env);

    symbol = obj_make_symbol("#e+infinity");
    value = obj_make_number(INFINITY);
    $define(obj_cons(symbol, obj_cons(value, KNULL)), g_env);

    prim_init();

    // we need $sequence before we can load files.
    // because files contain sequences of expressions.
    const char* seq_str = "($define! $sequence ((wrap ($vau ($seq2) #ignore \
                             ($seq2 ($define! $aux \
                               ($vau (head . tail) env ($if (null? tail) \
                                 (eval head env) ($seq2 \
                                   (eval head env) \
                                     (eval (cons $aux tail) env))))) \
                                        ($vau body env ($if (null? body) \
                                                            #inert (eval (cons $aux body) env)))))) \
                     ($vau (first second) env ((wrap ($vau #ignore #ignore (eval second env))) \
                                               (eval first env)))))";

    // define $sequence first.
    obj_t* result = obj_eval_str(seq_str, g_env);

    // bootstrap
    obj_t* bootstrap = read_file("bootstrap.ooo");
    result = obj_eval_expr(bootstrap, g_env);

    // unit-test
    //obj_t* unit_test_env = obj_make_environment(KNULL, g_env);
    obj_t* unit_test = read_file("unit-test.ooo");
    result = obj_eval_expr(unit_test, g_env);
}
