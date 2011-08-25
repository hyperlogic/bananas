#ifndef OBJ_H
#define OBJ_H

// TODO: rename to obj
enum obj_type { SYMBOL_OBJ = 0, CELL_OBJ, NUMBER_OBJ, PRIM_OBJ, CLOSURE_OBJ };

struct obj_struct;
struct env_struct;

typedef struct {
    struct obj_struct* car;
    struct obj_struct* cdr;
} cell_t;

typedef struct {
    struct obj_struct* args;
    struct obj_struct* body;
    struct env_struct* env;
} closure_t;

typedef struct obj_struct* (*prim_t)(struct obj_struct* args);

typedef struct obj_struct {
    enum obj_type type;
    union {
        int symbol;
        cell_t cell;
        float number;
        prim_t prim;
        closure_t closure;
    } data;
} obj_t;

typedef struct env_struct {
    obj_t** data;
    int max_objs;
    int num_objs;
    struct env_struct* parent;
} env_t;

// TODO: make these first class
// environments
env_t* env_new(env_t* parent);
void env_add(env_t* env, obj_t* symbol, obj_t* value);
obj_t* env_lookup(env_t* env, obj_t* symbol);

// obj makers
obj_t* make_symbol_obj(const char* str);
obj_t* make_symbol_obj_from_string(const char* start, const char* end);
obj_t* make_number_obj(double num);
obj_t* make_prim_obj(prim_t prim);
obj_t* make_cell_obj(obj_t* car, obj_t* cdr);

#endif
