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

// static object pool
#define MAX_OBJS 131072
obj_t g_obj_pool[MAX_OBJS];  // 8meg

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

#ifdef REF_COUNT_DEBUG
    fprintf(stderr, "ALLOC obj %p\n", obj);
#endif

    return obj;
}

static void _pool_free(obj_t* obj)
{
#ifdef REF_COUNT_DEBUG
    fprintf(stderr, "FREE  obj %p\n", obj);
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

static void _destroy(obj_t* obj)
{
    assert(!obj_is_immediate(obj));
    switch (obj->type) {
    case PAIR_OBJ:
        obj_unref(obj->data.pair.car);
        obj_unref(obj->data.pair.cdr);
        break;
    case ENV_OBJ:
        obj_unref(obj->data.env.plist);
        obj_unref(obj->data.env.parent);
        break;
    case COMPOUND_OPERATIVE_OBJ:
        obj_unref(obj->data.compound_operative.formals);
        obj_unref(obj->data.compound_operative.eformal);
        obj_unref(obj->data.compound_operative.body);
        obj_unref(obj->data.compound_operative.static_env);
        break;
    case APPLICATIVE_OBJ:
        obj_unref(obj->data.applicative.operative);
    default:
        break;
    }
}

//
// ref counting
//

void obj_ref(obj_t* obj)
{
    if (!obj_is_immediate(obj))
        obj->ref_count++;
}

void obj_unref(obj_t* obj)
{
    if (!obj_is_immediate(obj)) {
        obj->ref_count--;
        assert(obj->ref_count >= 0);
        if (obj->ref_count == 0) {
            _destroy(obj);
            _pool_free(obj);
        }
    }
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
    obj->ref_count = 1;
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
    obj->ref_count = 1;
    return obj;
}

obj_t* obj_make_number(double num)
{
    obj_t* obj = _pool_alloc();
    obj->type = NUMBER_OBJ;
    obj->data.number = num;
    obj->ref_count = 1;
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
    obj_ref(car);
    obj_ref(cdr);
    obj_t* obj = _pool_alloc();
    obj->type = PAIR_OBJ;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    obj->ref_count = 1;
    return obj;
}

obj_t* obj_make_environment(obj_t* plist, obj_t* parent)
{
    obj_ref(plist);
    obj_ref(parent);
    obj_t* obj = _pool_alloc();
    obj->type = ENV_OBJ;
    obj->data.env.plist = plist;
    obj->data.env.parent = parent;
    obj->ref_count = 1;
    return obj;
}

obj_t* obj_make_prim_operative(prim_operative_t prim)
{
    obj_t* obj = _pool_alloc();
    obj->type = PRIM_OPERATIVE_OBJ;
    obj->data.prim_operative = prim;  // prim is a c-function, no need to ref it.
    obj->ref_count = 1;
    return obj;
}

obj_t* obj_make_compound_operative(obj_t* formals, obj_t* eformal, obj_t* body, obj_t* static_env)
{
    obj_ref(formals);
    obj_ref(eformal);
    obj_ref(body);
    obj_ref(static_env);
    obj_t* obj = _pool_alloc();
    obj->type = COMPOUND_OPERATIVE_OBJ;
    obj->data.compound_operative.formals = formals;
    obj->data.compound_operative.eformal = eformal;
    obj->data.compound_operative.body = body;
    obj->data.compound_operative.static_env = static_env;
    obj->ref_count = 1;
    return obj;
}

obj_t* obj_make_applicative(obj_t* operative)
{
    obj_ref(operative);
    obj_t* obj = _pool_alloc();
    obj->type = APPLICATIVE_OBJ;
    obj->data.applicative.operative = operative;
    obj->ref_count = 1;
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

obj_t* obj_cons_own(obj_t* a, obj_t* b)
{
    return obj_make_pair(a, b);
}

obj_t* obj_car_own(obj_t* obj)
{
    assert(obj_is_pair(obj));
    obj_t* result = obj->data.pair.car;
    obj_ref(result);
    return result;
}

obj_t* obj_cdr_own(obj_t* obj)
{
    assert(obj_is_pair(obj));
    obj_t* result = obj->data.pair.cdr;
    obj_ref(result);
    return result;
}

obj_t* obj_cons_deny(obj_t* a, obj_t* b)
{
    obj_t* result = obj_make_pair(a, b);
    result->ref_count = 0;
    return result;
}

obj_t* obj_car_deny(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.car;
}

obj_t* obj_cdr_deny(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.cdr;
}

void obj_set_car(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj_ref(value);
    obj_unref(obj->data.pair.car);
    obj->data.pair.car = value;
}

void obj_set_cdr(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj_ref(value);
    obj_unref(obj->data.pair.cdr);
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
        obj_t* a_head = obj_car_deny(a);
        obj_t* a_tail = obj_cdr_deny(a);
        obj_t* b_head = obj_car_deny(b);
        obj_t* b_tail = obj_cdr_deny(b);
        return obj_is_equal(a_head, b_head) && obj_is_equal(a_tail, b_tail);
    }
    else
        return obj_is_eq(a, b);
}

obj_t* obj_env_lookup_deny(obj_t* env, obj_t* symbol)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    obj_t* pair = _assq_deny(symbol, env->data.env.plist);
    if (!obj_is_null(pair)) {
        return obj_cdr_deny(pair);
    }
    else {
        if (obj_is_environment(env->data.env.parent))
            return obj_env_lookup_deny(env->data.env.parent, symbol);
        else {

            // AJT: REMOVE
            fprintf(stderr, "Warning: could not find symbol \"%s\" in env\n", symbol_get(symbol->data.symbol));

            return KNULL;
        }
    }
}

obj_t* obj_env_lookup_own(obj_t* env, obj_t* symbol)
{
    obj_t* result = obj_env_lookup_deny(env, symbol);
    obj_ref(result);
    return result;
}

void obj_env_define(obj_t* env, obj_t* symbol, obj_t* value)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    obj_t* pair = _assq_deny(symbol, env->data.env.plist);
    if (obj_is_null(pair)) {
        // did not find it. so add a new property to the beginning of the plist.
        obj_t* plist = env->data.env.plist;
        env->data.env.plist = obj_cons_own(obj_cons_deny(symbol, value), plist);
        obj_unref(plist);
    } else {
        // found it, change the value
        obj_set_cdr(pair, value);
    }
}

static obj_t* _assq_deny(obj_t* key, obj_t* plist)
{
    while (obj_is_pair(plist)) {
        if (obj_is_eq(key, obj_car_deny(obj_car_deny(plist))))
            return obj_car_deny(plist);
        plist = obj_cdr_deny(plist);
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
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}%f", obj->ref_count, obj->data.number);
#else
            PRINTF("%f", obj->data.number);
#endif
            break;
        case SYMBOL_OBJ:
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}%s", obj->ref_count, symbol_get(obj->data.symbol));
#else
            PRINTF("%s", symbol_get(obj->data.symbol));
#endif
            break;
        case PAIR_OBJ:
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}(", obj->ref_count);
#else
            PRINTF("(");
#endif
            while (obj_is_pair(obj)) {
                obj_dump(obj_car_deny(obj), to_stderr);
                obj = obj_cdr_deny(obj);
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
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}#<env 0x%p>", obj->ref_count, obj);
#else
            PRINTF("#<env 0x%p>", obj);
#endif
            break;
        case PRIM_OPERATIVE_OBJ:
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}#<prim-operative 0x%p>", obj->ref_count, obj);
#else
            PRINTF("#<prim-operative 0x%p>", obj);
#endif
            break;
        case COMPOUND_OPERATIVE_OBJ:
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}#<compound-operative 0x%p>", obj->ref_count, obj);
#else
            PRINTF("#<compound-operative 0x%p>", obj);
#endif
            break;
        case APPLICATIVE_OBJ:
#ifdef REF_COUNT_DEBUG
            PRINTF("{%d}#<applicative 0x%p>", obj->ref_count, obj);
#else
            PRINTF("#<applicative 0x%p>", obj);
#endif
            break;
        default:
            PRINTF("???");
            break;
        }
    }
}

obj_t* obj_eval_expr(obj_t* obj, obj_t* env)
{
   obj_t* temp_expr_list = obj_cons_own(obj, KNULL);
   obj_t* result = $eval(temp_expr_list, env);
   obj_unref(temp_expr_list);
   return result;
}

obj_t* obj_eval_str(const char* str, obj_t* env)
{
   obj_t* temp_expr = read(str);
   obj_t* result = obj_eval_expr(temp_expr, env);
   obj_unref(temp_expr);
   return result;
}

//
// interpreter init, this needs happen before any thing else.
//

void obj_init()
{
    assert(sizeof(obj_t) == 64);

    _pool_init();
    g_env = obj_make_environment(KNULL, KNULL);
    obj_ref(g_env);
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
    obj_unref(result);

    // bootstrap
    obj_t* bootstrap = read_file("bootstrap.ooo");
    result = obj_eval_expr(bootstrap, g_env);
    obj_unref(bootstrap);
    obj_unref(result);
}
