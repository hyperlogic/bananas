#include "obj.h"
#include "parse.h"
#include "symbol.h"
#include "prim.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// static object pool
#define MAX_OBJS 8192
obj_t g_obj_pool[MAX_OBJS];  // 512 k

// free list
obj_t* g_free_objs = NULL;
int g_num_free_objs = 0;

// used list
obj_t* g_used_objs = NULL;
int g_num_used_objs = 0;

// root environment
obj_t* g_env = NULL;

static obj_t* assq(obj_t* key, obj_t* plist);

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
    case ENV_OBJ:
        unref(obj->data.env.plist);
        unref(obj->data.env.parent);
        break;
    case COMPOUND_OPERATIVE_OBJ:
        unref(obj->data.compound_operative.formals);
        unref(obj->data.compound_operative.eformal);
        unref(obj->data.compound_operative.body);
        unref(obj->data.compound_operative.static_env);
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

obj_t* make_environment(obj_t* plist, obj_t* parent)
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

obj_t* make_prim_operative(prim_operative_t prim)
{
    obj_t* obj = pool_alloc();
    obj->type = PRIM_OPERATIVE_OBJ;
    obj->data.prim_operative = prim;
    return obj;
}

obj_t* make_compound_operative(obj_t* formals, obj_t* eformal, obj_t* body, obj_t* static_env)
{
    ref(formals);
    ref(eformal);
    ref(body);
    ref(static_env);
    obj_t* obj = pool_alloc();
    obj->type = COMPOUND_OPERATIVE_OBJ;
    obj->data.compound_operative.formals = formals;
    obj->data.compound_operative.eformal = eformal;
    obj->data.compound_operative.body = body;
    obj->data.compound_operative.static_env = static_env;
    obj->ref_count = 0;
    return obj;
}

obj_t* make_applicative(obj_t* operative)
{
    ref(operative);
    obj_t* obj = pool_alloc();
    obj->type = APPLICATIVE_OBJ;
    obj->data.applicative.operative = operative;
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

int is_inert(obj_t* obj)
{
    assert(obj);
    return obj == KINERT;
}

int is_ignore(obj_t* obj)
{
    assert(obj);
    return obj == KIGNORE;
}

int is_boolean(obj_t* obj)
{
    assert(obj);
    return obj == KTRUE || obj == KFALSE;
}

int is_null(obj_t* obj)
{
    assert(obj);
    return obj == KNULL;
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
    return !is_immediate(obj) && obj->type == PAIR_OBJ && !is_null(obj);
}

int is_environment(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == ENV_OBJ;
}

int is_prim_operative(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == PRIM_OPERATIVE_OBJ;
}

int is_compound_operative(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == COMPOUND_OPERATIVE_OBJ;
}

int is_operative(obj_t* obj)
{
    assert(obj);
    return is_prim_operative(obj) || is_compound_operative(obj);
}

int is_applicative(obj_t* obj)
{
    assert(obj);
    return !is_immediate(obj) && obj->type == APPLICATIVE_OBJ;
}

obj_t* cons(obj_t* a, obj_t* b)
{
    return make_pair(a, b);
}

obj_t* set_car(obj_t* obj, obj_t* value)
{
    assert(is_pair(obj));
    ref(value);
    unref(obj->data.pair.car);
    obj->data.pair.car = value;
    return KINERT;
}

obj_t* set_cdr(obj_t* obj, obj_t* value)
{
    assert(is_pair(obj));
    ref(value);
    unref(obj->data.pair.cdr);
    obj->data.pair.cdr = value;
    return KINERT;
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

int is_eq(obj_t* a, obj_t* b)
{
    if (is_immediate(a) && is_immediate(b)) {
        return a == b;
    } else if (!is_immediate(a) && !is_immediate(b) && a->type == b->type) {
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

int is_equal(obj_t* a, obj_t* b)
{
    if (is_pair(a) && is_pair(b))
        return is_equal(car(a), car(b)) && is_equal(cdr(a), cdr(b));
    else
        return is_eq(a, b);
}

obj_t* env_lookup(obj_t* env, obj_t* symbol)
{
    assert(is_symbol(symbol));
    assert(is_environment(env));

    obj_t* pair = assq(symbol, env->data.env.plist);
    if (!is_null(pair))
        return cdr(pair);
    else {
        if (is_environment(env->data.env.parent))
            return env_lookup(env->data.env.parent, symbol);
        else
            return KNULL;
    }
}

obj_t* env_define(obj_t* env, obj_t* symbol, obj_t* value)
{
    assert(is_symbol(symbol));
    assert(is_environment(env));

    obj_t* plist = env->data.env.plist;
    if (is_pair(plist)) {

        // find symbol in plist.
        obj_t* prev = plist;
        while (is_pair(plist)) {
            obj_t* pair = car(plist);
            if (is_eq(symbol, car(pair)))
                break;
            prev = plist;
            plist = cdr(plist);
        }

        if (is_pair(plist)) {
            // found it. replace old cdr with new value.
            set_cdr(car(plist), value);
        } else {
            // did not find it. so add a new property to the end.
            set_cdr(prev, cons(cons(symbol, value), KNULL));
        }
    } else {
        // brand new plist.
        obj_t* new_plist = cons(cons(symbol, value), KNULL);
        ref(new_plist);
        env->data.env.plist = new_plist;
    }

    return value;
}


static obj_t* assq(obj_t* key, obj_t* plist)
{
    while (is_pair(plist)) {
        obj_t* pair = car(plist);
        if (is_eq(key, car(pair))) {
            return pair;
        }
        plist = cdr(plist);
    }
    return KNULL;
}

/*
//
// pair functions
//

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

obj_t* member(obj_t* obj, obj_t* lst)
{
    while (!is_nil(lst)) {
        if (!is_nil(is_eq(obj, car(lst))))
            return make_true();
        lst = cdr(lst);
    }
    return make_nil();
}

//
// equality
//


//
// env functions
//


//
// special forms
//

obj_t* eval(obj_t* obj, obj_t* env)
{
    assert(obj);
    assert(is_env(env));
    if (is_symbol(obj))
        return defined(obj, env);
    else if (is_pair(obj))
        return apply(obj, env);
    else
        return obj;
}

obj_t* apply(obj_t* obj, obj_t* env)
{
    assert(is_pair(obj));
    obj_t* f = eval(car(obj), env);
    ref(f);
    obj_t* args = cdr(obj);
    assert(!is_immediate(f));
    obj_t* result;
    switch (f->type) {
    case PRIM_OBJ:
        result = f->data.prim(args, env);
        break;
    case CLOSURE_OBJ:
    {
        obj_t* local_env = make_env(make_nil(), f->data.closure.env);
        obj_t* closure_args = f->data.closure.args;
        while(!is_nil(args) && !is_nil(closure_args)) {
            def(car(closure_args), eval(car(args), env), local_env);
            args = cdr(args);
            closure_args = cdr(closure_args);
        }
        obj_t* closure_body = f->data.closure.body;

        // make sure to eval closure body with the local_env
        result = eval(closure_body, local_env);
        break;
    }
    default:
        assert(0);  // illegal function type
    }

    unref(f);
    return result;
}
*/

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
    if (is_immediate(obj)) {
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
            while (is_pair(obj)) {
                dump(car(obj), to_stderr);
                obj = cdr(obj);
                if (!is_null(obj) && !is_pair(obj)) {
                    PRINTF(" . ");
                    dump(obj, to_stderr);
                    break;
                }
                if (!is_null(obj))
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

//
// interpreter init, this needs happen before any thing else.
//

void init()
{
    assert(sizeof(obj_t) == 64);

    pool_init();
    g_env = make_environment(KNULL, KNULL);
    ref(g_env);
    prim_init();
    
    // boot strap
    obj_t* seq = read_file("bootstrap.ooo");
    dump(seq, 1);
    fprintf(stderr, "\n");
}
