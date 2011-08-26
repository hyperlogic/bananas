#ifndef OBJ_H
#define OBJ_H

struct obj_struct;

typedef struct {
    struct obj_struct* car;
    struct obj_struct* cdr;
} pair_t;

typedef struct {
    struct obj_struct* args;
    struct obj_struct* body;
    struct obj_struct* env;
} closure_t;

typedef struct env_struct {
    struct obj_struct* plist;
    struct obj_struct* parent;
} env_t;

typedef struct obj_struct* (*prim_t)(struct obj_struct* args);

enum obj_type { SYMBOL_OBJ = 0, PAIR_OBJ, NUMBER_OBJ, PRIM_OBJ, CLOSURE_OBJ, ENV_OBJ };

typedef struct obj_struct {
    enum obj_type type;
    union {
        int symbol;
        double number;
        pair_t pair;
        prim_t prim;
        closure_t closure;
        env_t env;
    } data;
} obj_t;

extern obj_t* g_env;

// obj makers
obj_t* make_nil();
obj_t* make_true();  // true isnt really an obj type (YET)
obj_t* make_symbol(const char* str);
obj_t* make_symbol2(const char* start, const char* end);
obj_t* make_number(double num);
obj_t* make_number2(const char* start, const char* end);
obj_t* make_pair(obj_t* car, obj_t* cdr);
obj_t* make_prim(prim_t prim);
obj_t* make_closure(obj_t* args, obj_t* body, obj_t* env);
obj_t* make_env(obj_t* plist, obj_t* parent);

// obj type predicates
int is_nil(obj_t* obj);
int is_symbol(obj_t* obj);
int is_number(obj_t* obj);
int is_pair(obj_t* obj);
int is_prim(obj_t* obj);
int is_closure(obj_t* obj);
int is_env(obj_t* obj);

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
obj_t* member(obj_t* obj, obj_t* lst);
obj_t* assoc(obj_t* obj, obj_t* plist);  // dotted plist, and returns cdr of plist pair.

obj_t* quote(obj_t* obj);
obj_t* dump(obj_t* n, int to_stderr);

obj_t* eq(obj_t* a, obj_t* b);
obj_t* equal(obj_t* a, obj_t* b);

obj_t* def(obj_t* symbol, obj_t* value, obj_t* env);
obj_t* defined(obj_t* symbol, obj_t* env);

#endif
