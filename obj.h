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

typedef struct obj_struct* (*prim_t)(struct obj_struct* args, struct obj_struct* env);

enum obj_type { SYMBOL_OBJ = 0, PAIR_OBJ, NUMBER_OBJ, PRIM_OBJ, CLOSURE_OBJ, ENV_OBJ };

// the last 3 bits are used to indicate immediate values.
enum obj_immedate_tags { IMM_TAG = 1, NIL_TAG = 2, TRUE_TAG = 4, TAG_MASK = 7 };

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
    int ref_count;
    struct obj_struct* next;
    struct obj_struct* prev;
} obj_t;

extern int g_num_free_objs;
extern int g_num_used_objs;
extern obj_t* g_env;

// ref counting
void ref(obj_t* obj);
void unref(obj_t* obj);

// obj makers
obj_t* make_nil();
obj_t* make_true();
obj_t* make_symbol(const char* str);
obj_t* make_symbol2(const char* start, const char* end);
obj_t* make_number(double num);
obj_t* make_number2(const char* start, const char* end);
obj_t* make_pair(obj_t* car, obj_t* cdr);
obj_t* make_prim(prim_t prim);
obj_t* make_closure(obj_t* args, obj_t* body, obj_t* env);
obj_t* make_env(obj_t* plist, obj_t* parent);

// obj type predicates
int is_immediate(obj_t* obj);
int is_nil(obj_t* obj);
int is_true(obj_t* obj);
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
obj_t* set_car(obj_t* obj, obj_t* value);
obj_t* set_cdr(obj_t* obj, obj_t* value);
obj_t* member(obj_t* obj, obj_t* lst);
obj_t* assoc(obj_t* obj, obj_t* plist);

// equality
obj_t* is_eq(obj_t* a, obj_t* b);
obj_t* is_equal(obj_t* a, obj_t* b);

// env functions
obj_t* def(obj_t* symbol, obj_t* value, obj_t* env);
obj_t* defined(obj_t* symbol, obj_t* env);  // NOTE: this returns value not #t

// special forms
obj_t* quote(obj_t* obj);
obj_t* eval(obj_t* obj, obj_t* env);
obj_t* apply(obj_t* obj, obj_t* env);

// debug output
void dump(obj_t* n, int to_stderr);

// interpreter init, this needs happen before any thing else.
void init();

#endif
