#include "obj.h"
#include "symbol.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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
    env->data[env->num_objs] = make_cell_obj(symbol, value);
    env->num_objs++;
}

obj_t* env_lookup(env_t* env, obj_t* symbol)
{
    int i;
    
    assert(env);
    assert(symbol->type == SYMBOL_OBJ);
    for (i = 0; i < env->num_objs; i++) {
        if (symbol->data.symbol == env->data[i]->data.cell.car->data.symbol)
            return env->data[i]->data.cell.cdr;
    }

    if (env->parent)
        return env_lookup(env->parent, symbol);
    else
        return NULL;
}

obj_t* make_symbol_obj(const char* str)
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

obj_t* make_symbol_obj_from_string(const char* start, const char* end)
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

obj_t* make_number_obj(double num)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = NUMBER_OBJ;
    obj->data.number = num;
    return obj;
}

obj_t* make_prim_obj(prim_t prim)
{
   obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
   obj->type = PRIM_OBJ;
   obj->data.prim = prim;
   return obj;
}

obj_t* make_cell_obj(obj_t* car, obj_t* cdr)
{
    obj_t* obj = (obj_t*)malloc(sizeof(obj_t));
    obj->type = CELL_OBJ;
    obj->data.cell.car = car;
    obj->data.cell.cdr = cdr;
    return obj;
}
