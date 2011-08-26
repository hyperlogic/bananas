#include "obj.h"
#include "symbol.h"
#include "prim.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

obj_t* g_env = NULL;

void init()
{
    g_env = make_env(make_nil(), make_nil());
    prim_init();
}

obj_t* make_nil()
{
    return NULL;
}

obj_t* make_true()
{
    return make_number(1);
}

obj_t* make_symbol(const char* str)
{
    int len = strlen(str);
    int id = symbol_find(str, len);
    if (id < 0)
        id = symbol_add(str, len);

    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;
    return obj;
}

obj_t* make_symbol2(const char* start, const char* end)
{
    int len = end - start;
    int id = symbol_find(start, len);
    if (id < 0)
        id = symbol_add(start, len);

    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;
    return obj;
}

obj_t* make_number(double num)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = NUMBER_OBJ;
    obj->data.number = num;
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
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = PAIR_OBJ;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
    return obj;
}

obj_t* make_prim(prim_t prim)
{
   obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
   obj->type = PRIM_OBJ;
   obj->data.prim = prim;
   return obj;
}

obj_t* make_closure(obj_t* args, obj_t* body, obj_t* env)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = CLOSURE_OBJ;
    obj->data.closure.args = args;
    obj->data.closure.body = body;
    obj->data.closure.env = env;
    return obj;
}

obj_t* make_env(obj_t* plist, obj_t* parent)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = ENV_OBJ;
    obj->data.env.plist = plist;
    obj->data.env.parent = parent;
    return obj;
}

int is_nil(obj_t* obj)
{
    return obj == NULL;
}

int is_symbol(obj_t* obj)
{
    return obj && obj->type == SYMBOL_OBJ;
}

int is_number(obj_t* obj)
{
    return obj && obj->type == NUMBER_OBJ;
}

int is_pair(obj_t* obj)
{
    return obj && obj->type == PAIR_OBJ;
}

int is_prim(obj_t* obj)
{
    return obj && obj->type == PRIM_OBJ;
}

int is_closure(obj_t* obj)
{
    return obj && obj->type == CLOSURE_OBJ;
}

int is_env(obj_t* obj)
{
    return obj && obj->type == ENV_OBJ;
}

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
    obj->data.pair.car = value;
}

void set_cdr(obj_t* obj, obj_t* value)
{
    assert(is_pair(obj));
    obj->data.pair.cdr = value;
}

obj_t* member(obj_t* obj, obj_t* lst)
{
    while (lst) {
        if (eq(obj, car(lst)))
            return make_true();
        lst = cdr(lst);
    }
    return NULL;
}

// dotted plist, and returns cdr of plist pair.
obj_t* assoc(obj_t* key, obj_t* plist)
{
    while (plist) {
        obj_t* pair = car(plist);
        if (equal(key, (car(pair))))
            return cdr(pair);
        plist = cdr(plist);
    }
    return NULL;
}

obj_t* quote(obj_t* obj)
{
    return list2(make_symbol("quote"), obj);
}

#define PRINTF(args...)                          \
    do {                                         \
        if (to_stderr)                           \
            fprintf(stderr, args);               \
        else                                     \
            printf(args);                        \
    } while(0)

obj_t* dump(obj_t* obj, int to_stderr)
{
    if (!obj)
        PRINTF(" nil");
    else {
        switch (obj->type) {
        case NUMBER_OBJ:
            PRINTF(" %f", obj->data.number);
            break;
        case SYMBOL_OBJ:
            PRINTF(" %s", symbol_get(obj->data.symbol));
            break;
        case PAIR_OBJ:
            PRINTF(" (");
            while (obj) {
                dump(car(obj), to_stderr);
                obj = cdr(obj);
                if (obj && !is_pair(obj)) {
                    PRINTF(" .");
                    dump(obj, to_stderr);
                    break;
                }
            }
            PRINTF(" )");
            break;
        case PRIM_OBJ:
            PRINTF(" <#prim 0x%p>", obj->data.prim);
            break;
        case CLOSURE_OBJ:
            PRINTF(" <#closure ");
            dump(obj->data.closure.args, to_stderr);
            dump(obj->data.closure.body, to_stderr);
            PRINTF(" >");
            break;
        default:
            PRINTF(" ???");
            break;
        }
    }
    
    return NULL;
}

obj_t* eq(obj_t* a, obj_t* b)
{
    if (is_nil(a) && is_nil(b))
        return make_true();
    else if (a && b && a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol ? make_true() : make_nil();
        case NUMBER_OBJ:
            return a->data.number == b->data.number ? make_true() : make_nil();
        default:    
            return a == b ? make_true() : make_nil();
        }
    }
    return NULL;
}

obj_t* equal(obj_t* a, obj_t* b)
{
    if (is_pair(a) && is_pair(b)) {
        obj_t* car_eq = equal(car(a), car(b));
        obj_t* cdr_eq = equal(cdr(a), cdr(b));
        return !is_nil(car_eq) && !is_nil(cdr_eq) ? make_true() : make_nil();
    } else {
        return eq(a, b);
    }
}

obj_t* def(obj_t* symbol, obj_t* value, obj_t* env)
{
    assert(is_symbol(symbol));
    if (is_nil(env))
        env = g_env;
    assert(env);

    obj_t* plist = env->data.env.plist;
    if (plist) {
        // iterate over plist.
        obj_t* prev = plist;
        while (plist) {
            obj_t* pair = car(plist);
            if (eq(symbol, car(pair)))
                break;
            prev = plist;
            plist = cdr(plist);
        }

        if (plist)
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

    obj_t* result = assoc(symbol, env->data.env.plist);
    if (result)
        return result;
    else {
        if (is_env(env->data.env.parent))
            return defined(symbol, env->data.env.parent);
        else
            return make_nil();
    }
}
