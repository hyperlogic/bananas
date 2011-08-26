#include "obj.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

//
// environements
//
env_t* env_new(env_t* parent)
{
    env_t* env = (env_t*)malloc(sizeof(env_t));

    // start off with 16 objs
    const int initial_max_objs = 16;
    env->data = (obj_t**)malloc(sizeof(obj_t*) * initial_max_objs);
    env->max_objs = initial_max_objs;
    env->num_objs = 0;
    env->parent = parent;

    return env;
}

void env_add(env_t* env, obj_t* symbol, obj_t* value)
{
    assert(env);
    assert(symbol->type == SYMBOL_OBJ);

    if (env->num_objs == env->max_objs) {
        // realloc more objs!
        int new_max_objs = env->max_objs * env->max_objs;
        env->data = (obj_t**)realloc(env->data, sizeof(obj_t*) * new_max_objs);
        env->max_objs = new_max_objs;
    }
    env->data[env->num_objs] = make_pair(symbol, value);
    env->num_objs++;
}

obj_t* env_lookup(env_t* env, obj_t* symbol)
{
    int i;
    
    assert(env);
    assert(symbol->type == SYMBOL_OBJ);
    for (i = 0; i < env->num_objs; i++) {
        if (symbol->data.symbol == env->data[i]->data.pair.car->data.symbol)
            return env->data[i]->data.pair.cdr;
    }

    if (env->parent)
        return env_lookup(env->parent, symbol);
    else
        return NULL;
}

obj_t* make_nil()
{
    return NULL;
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

obj_t* make_closure(obj_t* args, obj_t* body, env_t* env)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = CLOSURE_OBJ;
    obj->data.closure.args = args;
    obj->data.closure.body = body;
    obj->data.closure.env = env;
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

obj_t* assoc(obj_t* key, obj_t* plist)
{
    while (plist) {
        obj_t* pair = car(plist);
        if (eq(key, (car(pair))))
            return cadr(pair);
        plist = cdr(plist);
    }
    return NULL;
}

obj_t* member(obj_t* obj, obj_t* lst)
{
    while (lst) {
        if (eq(obj, car(lst)))
            return make_number(1);
        lst = cdr(lst);
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
        return make_number(1);
    else if (a && b && a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol ? make_number(1) : make_nil();
        case NUMBER_OBJ:
            return a->data.number == b->data.number ? make_number(1) : make_nil();
        default:    
            return a == b ? make_number(1) : make_nil();
        }
    }
    return NULL;
}

obj_t* equal(obj_t* a, obj_t* b)
{
    if (is_pair(a) && is_pair(b)) {
        obj_t* car_eq = equal(car(a), car(b));
        obj_t* cdr_eq = equal(cdr(a), cdr(b));
        return !is_nil(car_eq) && !is_nil(cdr_eq) ? make_number(1) : make_nil();
    } else {
        return eq(a, b);
    }
}

