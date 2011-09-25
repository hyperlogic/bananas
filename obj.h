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

typedef struct obj_struct* (*prim_func_t)(struct obj_struct* args, struct obj_struct* env);

typedef struct {
    struct obj_struct* formals;
    struct obj_struct* env;
    struct obj_struct* body;
} comp_proc_t;

enum obj_type { SYMBOL_OBJ = 0, NUMBER_OBJ, PAIR_OBJ, ENV_OBJ,
                PRIM_FORM_OBJ, PRIM_PROC_OBJ, COMP_PROC_OBJ, GARBAGE_OBJ };

// the last 5 bits are used to indicate immediate values.
enum obj_immedate_tags { IMM_TAG = 1, TRUE_TAG = 2,
                         FALSE_TAG = 4, NULL_TAG = 8,
                         UNUSED1_TAG = 16, UNUSED2_TAG = 32, TAG_MASK = 63 };

// immediate constants
#define KTRUE ((obj_t*)(TRUE_TAG | IMM_TAG))
#define KFALSE ((obj_t*)(FALSE_TAG | IMM_TAG))
#define KNULL ((obj_t*)(NULL_TAG | IMM_TAG))

typedef struct obj_struct {
    union {
        int symbol;
        double number;
        pair_t pair;
        env_t env;
        prim_func_t prim_func;
        comp_proc_t comp_proc;
    } data;                          // +  0
    struct obj_struct* next;         // + 24
    struct obj_struct* prev;         // + 32
    enum obj_type type;              // + 40
    int mark;                        // + 44
    char padding[16];                // + 48
} obj_t;                      // sizeof = 64

extern int g_num_free_objs;
extern int g_num_used_objs;
extern obj_t* g_env;

void obj_gc();

// stack which prevents gc from collecting intermediate results.
void obj_stack_frame_push();
void obj_stack_frame_pop();
obj_t* obj_stack_push(obj_t* obj);
obj_t* obj_stack_get(int i);
void obj_stack_set(int i, obj_t* obj);
void obj_stack_dump(const char* desc);  // TODO: REMOVE for debugging

#define PUSHF obj_stack_frame_push
#define PUSH(OBJ) obj_stack_push(OBJ)
#define PUSH2(OBJ1, OBJ2)                       \
    do {                                        \
        obj_stack_push(OBJ1);                   \
        obj_stack_push(OBJ2);                   \
    } while (0)

#define PUSH3(OBJ1, OBJ2, OBJ3)                 \
    do {                                        \
        obj_stack_push(OBJ1);                   \
        obj_stack_push(OBJ2);                   \
        obj_stack_push(OBJ3);                   \
    } while (0)

#define PUSH4(OBJ1, OBJ2, OBJ3, OBJ4)           \
    do {                                        \
        obj_stack_push(OBJ1);                   \
        obj_stack_push(OBJ2);                   \
        obj_stack_push(OBJ3);                   \
        obj_stack_push(OBJ4);                   \
    } while (0)

#define POPF_RET(OBJ)                           \
    do {                                        \
        obj_t* __ret = OBJ;                     \
        assert(!obj_is_garbage(__ret));         \
        obj_stack_frame_pop();                  \
        assert(!obj_is_garbage(__ret));         \
        return __ret;                           \
    } while (0)

#define POPF obj_stack_frame_pop

// all of these may trigger a gc.
obj_t* obj_make_symbol(const char* str);
obj_t* obj_make_symbol2(const char* start, const char* end);
obj_t* obj_make_number(double num);
obj_t* obj_make_number2(const char* start, const char* end);
obj_t* obj_make_pair(obj_t* car, obj_t* cdr);
obj_t* obj_make_environment(obj_t* plist, obj_t* parent);
obj_t* obj_make_prim_form(prim_func_t prim_func);
obj_t* obj_make_prim_proc(prim_func_t prim_func);
obj_t* obj_make_comp_proc(obj_t* formals, obj_t* env, obj_t* body);

// obj type predicates
int obj_is_garbage(obj_t* obj);  // for debugging gc
int obj_is_immediate(obj_t* obj);
int obj_is_boolean(obj_t* obj);
int obj_is_null(obj_t* obj);
int obj_is_symbol(obj_t* obj);
int obj_is_number(obj_t* obj);
int obj_is_pair(obj_t* obj);
int obj_is_environment(obj_t* obj);
int obj_is_prim_form(obj_t* obj);
int obj_is_prim_proc(obj_t* obj);
int obj_is_comp_proc(obj_t* obj);
int obj_is_proc(obj_t* obj);

obj_t* obj_cons(obj_t* a, obj_t* b);
obj_t* obj_car(obj_t* obj);
obj_t* obj_cdr(obj_t* obj);
obj_t* obj_cadr(obj_t* obj);

void obj_set_car(obj_t* obj, obj_t* value);
void obj_set_cdr(obj_t* obj, obj_t* value);

int obj_is_eq(obj_t* a, obj_t* b);
int obj_is_equal(obj_t* a, obj_t* b);

obj_t* obj_env_lookup(obj_t* env, obj_t* symbol);

void obj_env_define(obj_t* env, obj_t* symbol, obj_t* value);

// debug output
void obj_dump(obj_t* n, int to_stderr);

obj_t* obj_eval_expr(obj_t* obj, obj_t* env); // gc
obj_t* obj_eval_str(const char* str, obj_t* env); // gc

// interpreter init, this needs happen before any thing else.
void obj_init();

#endif
