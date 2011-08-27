#include "obj.h"
#include "parse.h"
#include "symbol.h"
#include "prim.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// static object pool
#define MAX_OBJS 10000
obj_t g_obj_pool[MAX_OBJS];

// free list
obj_t* g_free_objs = NULL;
int g_num_free_objs = 0;

// used list
obj_t* g_used_objs = NULL;
int g_num_used_objs = 0;

// root environment
obj_t* g_env = NULL;

//
// obj pool
//

static void pool_init()
{
    // init obj pool
    int i;
    for (i = 0; i < MAX_OBJS - 1; i++)
        g_obj_pool[i].next = g_obj_pool + i + 1;
    g_free_objs = g_obj_pool;
    g_num_free_objs = MAX_OBJS;
    g_num_used_objs = 0;
}

static obj_t* pool_alloc()
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

    //fprintf(stderr, "ALLOC obj %p\n", obj);

    return obj;
}

static void pool_free(obj_t* obj)
{
    //fprintf(stderr, "FREE  obj %p\n", obj);

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

static void destroy_obj(obj_t* obj)
{
    assert(!is_immediate(obj));
    switch (obj->type) {
    case PAIR_OBJ:
        unref(obj->data.pair.car);
        unref(obj->data.pair.cdr);
        break;
    case CLOSURE_OBJ:
        unref(obj->data.closure.args);
        unref(obj->data.closure.body);
        unref(obj->data.closure.env);
        break;
    case ENV_OBJ:
        unref(obj->data.env.plist);
        unref(obj->data.env.parent);
        break;
    default:
        break;
    }
}

//
// ref counting
//

void ref(obj_t* obj)
{
    if (!is_immediate(obj))
        obj->ref_count++;
}

void unref(obj_t* obj)
{
    if (!is_immediate(obj)) {
        obj->ref_count--;
        assert(obj->ref_count >= 0);
        if (obj->ref_count == 0) {
            destroy_obj(obj);
            pool_free(obj);
        }
    }
}

//
// obj makers
//

obj_t* make_nil()
{
    return (obj_t*)(NIL_TAG | IMM_TAG);
}

obj_t* make_true()
{
    return (obj_t*)(TRUE_TAG | IMM_TAG);
}

obj_t* make_symbol(const char* str)
{
    int len = strlen(str);
    int id = symbol_find(str, len);
    if (id < 0)
        id = symbol_add(str, len);

    obj_t* obj = pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_symbol2(const char* start, const char* end)
{
    int len = end - start;
    int id = symbol_find(start, len);
    if (id < 0)
        id = symbol_add(start, len);

    obj_t* obj = pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_number(double num)
{
    obj_t* obj = pool_alloc();
    obj->type = NUMBER_OBJ;
    obj->data.number = num;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_number2(const char* start, const char* end)
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
    
    return make_number(num);
}

obj_t* make_pair(obj_t* car, obj_t* cdr)
{
    ref(car);
    ref(cdr);
    obj_t* obj = pool_alloc();
    obj->type = PAIR_OBJ;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_prim(prim_t prim)
{
    obj_t* obj = pool_alloc();
    obj->type = PRIM_OBJ;
    obj->data.prim = prim;
    return obj;
}

obj_t* make_closure(obj_t* args, obj_t* body, obj_t* env)
{
    ref(args);
    ref(body);
    ref(env);
    obj_t* obj = pool_alloc();
    obj->type = CLOSURE_OBJ;
    obj->data.closure.args = args;
    obj->data.closure.body = body;
    obj->data.closure.env = env;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_env(obj_t* plist, obj_t* parent)
{
    ref(plist);
    ref(parent);
    obj_t* obj = pool_alloc();
    obj->type = ENV_OBJ;
    obj->data.env.plist = plist;
    obj->data.env.parent = parent;
    obj->ref_count = 0;
    return obj;
}

//
// obj type predicates
//

int is_immediate(obj_t* obj)
{
    assert(obj);
    return (long)obj & IMM_TAG;
}

int is_nil(obj_t* obj)
{
    assert(obj);
    return obj == (obj_t*)(NIL_TAG | IMM_TAG);
}

int is_true(obj_t* obj)
{
    assert(obj);
    return obj == (obj_t*)(TRUE_TAG | IMM_TAG);
}

int is_symbol(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == SYMBOL_OBJ;
}

int is_number(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == NUMBER_OBJ;
}

int is_pair(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == PAIR_OBJ;
}

int is_prim(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == PRIM_OBJ;
}

int is_closure(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == CLOSURE_OBJ;
}

int is_env(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == ENV_OBJ;
}

//
// pair functions
//

obj_t* cons(obj_t* a, obj_t* b)
{
    return make_pair(a, b);
}

obj_t* car(obj_t* obj)
{
    assert(is_pair(obj));
    return obj->data.pair.car;
}

obj_t* cdr(obj_t* obj)
{
    assert(is_pair(obj));
    return obj->data.pair.cdr;
}

obj_t* cadr(obj_t* obj)
{
    return car(cdr(obj));
}

obj_t* list1(obj_t* a)
{
    return cons(a, make_nil());
}

obj_t* list2(obj_t* a, obj_t* b)
{
    return cons(a, cons(b, make_nil()));
}

obj_t* list3(obj_t* a, obj_t* b, obj_t* c)
{
    return cons(a, cons(b, cons(c, make_nil())));
}

void set_car(obj_t* obj, obj_t* value)
{
    assert(is_pair(obj));
    ref(value);
    unref(obj->data.pair.car);
    obj->data.pair.car = value;
}

void set_cdr(obj_t* obj, obj_t* value)
{
    assert(is_pair(obj));
    ref(value);
    unref(obj->data.pair.cdr);
    obj->data.pair.cdr = value;
}

obj_t* member(obj_t* obj, obj_t* lst)
{
    while (!is_nil(lst)) {
        if (!is_nil(is_eq(obj, car(lst))))
            return make_true();
        lst = cdr(lst);
    }
    return make_nil();
}

obj_t* assoc(obj_t* key, obj_t* plist)
{
    while (!is_nil(plist)) {
        obj_t* pair = car(plist);
        if (!is_nil(is_equal(key, (car(pair))))) {
            return pair;
        }
        plist = cdr(plist);
    }
    return make_nil();
}

//
// equality
//

obj_t* is_eq(obj_t* a, obj_t* b)
{
    if (is_nil(a) && is_nil(b))
        return make_true();
    else if (is_true(a) && is_true(b)) {
        return make_true();
    } else if (!is_immediate(a) && !is_immediate(b) && a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol ? make_true() : make_nil();
        case NUMBER_OBJ:
            return a->data.number == b->data.number ? make_true() : make_nil();
        default:    
            return a == b ? make_true() : make_nil();
        }
    }
    return make_nil();
}

obj_t* is_equal(obj_t* a, obj_t* b)
{
    if (is_pair(a) && is_pair(b)) {
        obj_t* car_eq = is_equal(car(a), car(b));
        obj_t* cdr_eq = is_equal(cdr(a), cdr(b));
        return !is_nil(car_eq) && !is_nil(cdr_eq) ? make_true() : make_nil();
    } else {
        return is_eq(a, b);
    }
}

//
// env functions
//

obj_t* def(obj_t* symbol, obj_t* value, obj_t* env)
{
    assert(is_symbol(symbol));
    if (is_nil(env))
        env = g_env;
    assert(env);

    obj_t* plist = env->data.env.plist;
    if (!is_nil(plist)) {
        // iterate over plist.
        obj_t* prev = plist;
        while (!is_nil(plist)) {
            obj_t* pair = car(plist);
            if (!is_nil(is_eq(symbol, car(pair))))
                break;
            prev = plist;
            plist = cdr(plist);
        }

        if (!is_nil(plist))
            set_cdr(car(plist), value);
        else 
            set_cdr(prev, cons(cons(symbol, value), make_nil()));
    } else
        env->data.env.plist = cons(cons(symbol, value), make_nil());

    return value;
}

obj_t* defined(obj_t* symbol, obj_t* env)
{
    assert(is_symbol(symbol));
    if (is_nil(env))
        env = g_env;
    assert(env);

    obj_t* pair = assoc(symbol, env->data.env.plist);
    if (!is_nil(pair))
        return cdr(pair);
    else {
        if (is_env(env->data.env.parent))
            return defined(symbol, env->data.env.parent);
        else
            return make_nil();
    }
}

//
// special forms
//

obj_t* quote(obj_t* obj)
{
    return list2(make_symbol("quote"), obj);
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

void dump(obj_t* obj, int to_stderr)
{
    if (is_nil(obj))
        PRINTF("nil");
    else if (is_true(obj))
        PRINTF("#t");
    else {
        assert(!is_immediate(obj));
        switch (obj->type) {
        case NUMBER_OBJ:
            PRINTF("%f", obj->data.number);
            break;
        case SYMBOL_OBJ:
            PRINTF("%s", symbol_get(obj->data.symbol));
            break;
        case PAIR_OBJ:
            PRINTF("(");
            while (!is_nil(obj)) {
                dump(car(obj), to_stderr);
                obj = cdr(obj);
                if (!is_nil(obj) && !is_pair(obj)) {
                    PRINTF(" . ");
                    dump(obj, to_stderr);
                    break;
                }
                if (!is_nil(obj))
                    PRINTF(" ");
            }
            PRINTF(")");
            break;
        case PRIM_OBJ:
            PRINTF("#<prim 0x%p>", obj->data.prim);
            break;
        case CLOSURE_OBJ:
            PRINTF("#<closure 0x%p>", obj);
            break;
        default:
            PRINTF("???");
            break;
        }
    }
}

//
// interpreter init, this needs happen before any thing else.
//

void init()
{
    pool_init();
    g_env = make_env(make_nil(), make_nil());
    prim_init();
    
    // boot strap
    obj_t* seq = read_file("bootstrap.ooo");
    dump(seq, 1);
    fprintf(stderr, "\n");
}

