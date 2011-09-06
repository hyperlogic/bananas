// (setq show-trailing-whitespace t)
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

enum obj_type { SYMBOL_OBJ = 0, NUMBER_OBJ, PAIR_OBJ, ENV_OBJ,
                PRIM_OPERATIVE_OBJ, COMPOUND_OPERATIVE_OBJ, APPLICATIVE_OBJ, GARBAGE_OBJ };

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
    int mark;                        // + 52
    char padding[8];                 // + 56
} obj_t;                      // sizeof = 64

extern int g_num_free_objs;
extern int g_num_used_objs;
extern obj_t* g_env;

void obj_gc();

// stack which prevents gc from collecting intermediate results.
void obj_stack_frame_push();
void obj_stack_frame_pop();
void obj_stack_push(obj_t* obj);
obj_t* obj_stack_get(int i);
void obj_stack_set(int i, obj_t* obj);

// all of these may trigger a gc.
obj_t* obj_make_symbol(const char* str);
obj_t* obj_make_symbol2(const char* start, const char* end);
obj_t* obj_make_number(double num);
obj_t* obj_make_number2(const char* start, const char* end);
obj_t* obj_make_pair(obj_t* car, obj_t* cdr);
obj_t* obj_make_environment(obj_t* plist, obj_t* parent);
obj_t* obj_make_prim_operative(prim_operative_t prim);
obj_t* obj_make_compound_operative(obj_t* formals, obj_t* eformal, obj_t* body, obj_t* static_env);
obj_t* obj_make_applicative(obj_t* operative);

// obj type predicates
// none of these will trigger a gc.
int obj_is_immediate(obj_t* obj);
int obj_is_inert(obj_t* obj);
int obj_is_ignore(obj_t* obj);
int obj_is_boolean(obj_t* obj);
int obj_is_null(obj_t* obj);
int obj_is_symbol(obj_t* obj);
int obj_is_number(obj_t* obj);
int obj_is_inexact(obj_t* obj);
int obj_is_pair(obj_t* obj);
int obj_is_environment(obj_t* obj);
int obj_is_prim_operative(obj_t* obj);
int obj_is_compound_operative(obj_t* obj);
int obj_is_operative(obj_t* obj);
int obj_is_applicative(obj_t* obj);

obj_t* obj_cons(obj_t* a, obj_t* b);  // gc
obj_t* obj_car(obj_t* obj);  // no gc
obj_t* obj_cdr(obj_t* obj);  // no gc
obj_t* obj_cadr(obj_t* obj); // no gc

void obj_set_car(obj_t* obj, obj_t* value); // no gc
void obj_set_cdr(obj_t* obj, obj_t* value); // no gc

int obj_is_eq(obj_t* a, obj_t* b);    // no gc
int obj_is_equal(obj_t* a, obj_t* b); // no gc

obj_t* obj_env_lookup(obj_t* env, obj_t* symbol);  // no gc

void obj_env_define(obj_t* env, obj_t* symbol, obj_t* value);  // gc

// debug output
void obj_dump(obj_t* n, int to_stderr);  // no gc

obj_t* obj_eval_expr(obj_t* obj, obj_t* env); // gc
obj_t* obj_eval_str(const char* str, obj_t* env); // gc

// interpreter init, this needs happen before any thing else.
void obj_init();

#endif
