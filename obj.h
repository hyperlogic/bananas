#ifndef OBJ_H
#define OBJ_H

// TODO: rename to obj
enum obj_type { SYMBOL_OBJ = 0, PAIR_OBJ, NUMBER_OBJ, PRIM_OBJ, CLOSURE_OBJ };

struct obj_struct;
struct env_struct;

typedef struct {
    struct obj_struct* car;
    struct obj_struct* cdr;
} pair_t;

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
        pair_t pair;
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
obj_t* make_nil();
obj_t* make_symbol(const char* str);
obj_t* make_symbol2(const char* start, const char* end);
obj_t* make_number(double num);
obj_t* make_number2(const char* start, const char* end);
obj_t* make_pair(obj_t* car, obj_t* cdr);
obj_t* make_prim(prim_t prim);
obj_t* make_closure(obj_t* args, obj_t* body, env_t* env);

// obj type predicates
int is_nil(obj_t* obj);
int is_symbol(obj_t* obj);
int is_number(obj_t* obj);
int is_prim(obj_t* obj);
int is_pair(obj_t* obj);
int is_closure(obj_t* obj);

// pair functions
obj_t* cons(obj_t* a, obj_t* b);
obj_t* car(obj_t* obj);
obj_t* cdr(obj_t* obj);
obj_t* cadr(obj_t* obj);
obj_t* list1(obj_t* a);
obj_t* list2(obj_t* a, obj_t* b);
obj_t* list3(obj_t* a, obj_t* b, obj_t* c);
void set_car(obj_t* obj, obj_t* value);
void set_cdr(obj_t* obj, obj_t* value);
obj_t* assoc(obj_t* key, obj_t* plist);
obj_t* member(obj_t* obj, obj_t* lst);

obj_t* quote(obj_t* obj);
obj_t* dump(obj_t* n, int to_stderr);

obj_t* eq(obj_t* a, obj_t* b);
obj_t* equal(obj_t* a, obj_t* b);

#endif
