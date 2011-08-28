#ifndef OBJ_H
#define OBJ_H

struct obj_struct;

typedef struct {
    struct obj_struct* car;
    struct obj_struct* cdr;
} pair_t;

typedef struct env_struct {
    struct obj_struct* plist;
    struct obj_struct* parent;
} env_t;

typedef struct obj_struct* (*prim_operative_t)(struct obj_struct* args, struct obj_struct* env);

typedef struct {
    struct obj_struct* formals;
    struct obj_struct* eformal;
    struct obj_struct* static_env;
    struct obj_struct* body;
} compound_operative_t;

typedef struct {
    struct obj_struct* operative;
} applicative_t;

enum obj_type { SYMBOL_OBJ = 0, PAIR_OBJ, NUMBER_OBJ, ENV_OBJ, 
                PRIM_OPERATIVE_OBJ, COMPOUND_OPERATIVE_OBJ, APPLICATIVE_OBJ };

// the last 5 bits are used to indicate immediate values.
enum obj_immedate_tags { IMM_TAG = 1, INERT_TAG = 2, 
                         IGNORE_TAG = 4, TRUE_TAG = 8, 
                         FALSE_TAG = 16, NULL_TAG = 32, TAG_MASK = 63 };

// immediate constants
#define KINERT ((obj_t*)(INERT_TAG | IMM_TAG))
#define KIGNORE ((obj_t*)(IGNORE_TAG | IMM_TAG))
#define KTRUE ((obj_t*)(TRUE_TAG | IMM_TAG))
#define KFALSE ((obj_t*)(FALSE_TAG | IMM_TAG))
#define KNULL ((obj_t*)(NULL_TAG | IMM_TAG))

typedef struct obj_struct {
    union {
        int symbol;
        double number;
        pair_t pair;
        env_t env;
        prim_operative_t prim_operative;
        compound_operative_t compound_operative;
        applicative_t applicative;
    } data;                          // +  0
    struct obj_struct* next;         // + 32
    struct obj_struct* prev;         // + 40
    enum obj_type type;              // + 48
    int ref_count;                   // + 52
    char padding[8];                 // + 56
} obj_t;                      // sizeof = 64

extern int g_num_free_objs;
extern int g_num_used_objs;
extern obj_t* g_env;

// ref counting
void ref(obj_t* obj);
void unref(obj_t* obj);

// obj makers
obj_t* make_symbol(const char* str);
obj_t* make_symbol2(const char* start, const char* end);
obj_t* make_number(double num);
obj_t* make_number2(const char* start, const char* end);
obj_t* make_pair(obj_t* car, obj_t* cdr);
obj_t* make_environment(obj_t* plist, obj_t* parent);
obj_t* make_prim_operative(prim_operative_t prim);
obj_t* make_compound_operative(obj_t* formals, obj_t* eformal, obj_t* body, obj_t* static_env);
obj_t* make_applicative(obj_t* operative);

// obj type predicates
int is_immediate(obj_t* obj);
int is_inert(obj_t* obj);
int is_ignore(obj_t* obj);
int is_boolean(obj_t* obj);
int is_null(obj_t* obj);
int is_symbol(obj_t* obj);
int is_number(obj_t* obj);
int is_pair(obj_t* obj);
int is_environment(obj_t* obj);
int is_prim_operative(obj_t* obj);
int is_compound_operative(obj_t* obj);
int is_operative(obj_t* obj);
int is_applicative(obj_t* obj);

// helpers
obj_t* cons(obj_t* a, obj_t* b);
obj_t* car(obj_t* obj);
obj_t* cdr(obj_t* obj);
obj_t* set_car(obj_t* obj, obj_t* value);
obj_t* set_cdr(obj_t* obj, obj_t* value);

int is_eq(obj_t* a, obj_t* b);
int is_equal(obj_t* a, obj_t* b);

obj_t* env_define(obj_t* env, obj_t* symbol, obj_t* value);
obj_t* env_lookup(obj_t* env, obj_t* symbol);

/*
// pair functions
obj_t* cadr(obj_t* obj);
obj_t* list1(obj_t* a);
obj_t* list2(obj_t* a, obj_t* b);
obj_t* list3(obj_t* a, obj_t* b, obj_t* c);
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
*/

// debug output
void dump(obj_t* n, int to_stderr);

// interpreter init, this needs happen before any thing else.
void init();

#endif
