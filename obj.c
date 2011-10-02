#include "obj.h"
#include "parse.h"
#include "symbol.h"
#include "prim.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// static object pool
#define MAX_OBJS 131072 * 4  // 16 meg
obj_t g_obj_pool[MAX_OBJS];

// free list
obj_t* g_free_objs = NULL;
int g_num_free_objs = 0;

// used list
obj_t* g_used_objs = NULL;
int g_num_used_objs = 0;

// root environment
obj_t* g_env = KNULL;

// root stack
#define MAX_STACK_OBJS 100000
#define MAX_STACK_FRAMES 10000
obj_t* g_stack[MAX_STACK_OBJS];
int g_num_stack_objs;
int g_stack_frames[MAX_STACK_FRAMES];
int g_num_stack_frames;

int g_mark = 0;

static obj_t* _assq(obj_t* key, obj_t* plist);

//
// obj pool
//

static void _pool_init()
{
    // init obj pool
    int i;
    for (i = 0; i < MAX_OBJS - 1; i++)
        g_obj_pool[i].next = g_obj_pool + i + 1;
    g_free_objs = g_obj_pool;
    g_num_free_objs = MAX_OBJS;
    g_num_used_objs = 0;
}

static obj_t* _pool_alloc()
{
    // TODO: REMOVE
    // Really hammer on gc...
    /*
    static int count = 0;
    if (count >= 1)
        obj_gc();
    count++;
    */

    if (!g_free_objs)
        obj_gc();

    // take from front of free list
    assert(g_free_objs);
    obj_t* obj = g_free_objs;
    g_free_objs = g_free_objs->next;
    if (g_free_objs)
        g_free_objs->prev = NULL;
    g_num_free_objs--;

    // add to front of used list
    obj->prev = NULL;
    obj->next = g_used_objs;
    if (g_used_objs)
        g_used_objs->prev = obj;
    g_used_objs = obj;
    g_num_used_objs++;

    assert(g_num_used_objs + g_num_free_objs == MAX_OBJS);

    obj->mark = 0;
    return obj;
}

static void _pool_free(obj_t* obj)
{
#ifdef GC_DEBUG
    fprintf(stderr, "FREE obj %p, ", obj);
    obj_dump(obj, 1);
    fprintf(stderr, "\n");
#endif

    assert(obj);

    // remove from used list
    if (obj->prev)
        obj->prev->next = obj->next;
    else
        g_used_objs = obj->next;
    if (obj->next)
        obj->next->prev = obj->prev;
    g_num_used_objs--;

    // add to start of free list
    obj->next = g_free_objs;
    if (g_free_objs)
        g_free_objs->prev = obj;
    g_free_objs = obj;
    g_num_free_objs++;

    // helps track down gc problems.
    obj->type = GARBAGE_OBJ;

    assert(g_num_used_objs + g_num_free_objs == MAX_OBJS);
}

#ifdef GC_DEBUG
static void _dump_list(const char* desc, obj_t* obj)
{
    fprintf(stderr, "%s = [\n", desc);
    obj_t* p = obj;
    while (p) {
        fprintf(stderr, "    <%p prev = %p, next = %p>\n", p, p->prev, p->next);
        p = p->next;
    }
    fprintf(stderr, "]\n");
}
#endif

//
// gc
//

#ifndef NDEBUG
static int _pool_num_used()
{
    int num_used = 0;
    obj_t* used_objs = g_used_objs;
    while (used_objs) {
        num_used++;
        used_objs = used_objs->next;
    }
    assert(g_num_used_objs == num_used);
    return num_used;
}
#endif

static void _gc_mark(obj_t* obj)
{
    if (!obj_is_immediate(obj)) {

        if (obj->mark == g_mark)
            return;
        else
            obj->mark = g_mark;

        switch (obj->type) {
        case SYMBOL_OBJ:
        case NUMBER_OBJ:
        case PRIM_FORM_OBJ:
        case PRIM_PROC_OBJ:
            // These objs don't reference anything.
            break;
        case PAIR_OBJ:
            _gc_mark(obj->data.pair.car);
            _gc_mark(obj->data.pair.cdr);
            break;
        case ENV_OBJ:
            _gc_mark(obj->data.env.plist);
            _gc_mark(obj->data.env.parent);
            break;
        case COMP_PROC_OBJ:
            _gc_mark(obj->data.comp_proc.formals);
            _gc_mark(obj->data.comp_proc.env);
            _gc_mark(obj->data.comp_proc.body);
            break;
        default:
            assert(0);  // bad obj type!
            break;
        }
    }
}

void obj_gc()
{
#if GC_DEBUG
    // _dump_list("g_used_objs", g_used_objs);
#endif

    // mark phase
    g_mark++;
    assert(obj_is_environment(g_env));
    _gc_mark(g_env);

    int i;
    for (i = 0; i < g_num_stack_objs; ++i)
        _gc_mark(g_stack[i]);

    // sweep phase
    obj_t* p = g_used_objs;
#ifndef NDEBUG
    int initial_count = _pool_num_used();
    int count = initial_count;
#endif
    while (p) {
        obj_t* obj = p;
        p = p->next;
        if (obj->mark != g_mark) {
            _pool_free(obj);
#ifndef NDEBUG
            count--;
#endif
        }
    }
    assert(_pool_num_used() == count);
}

//
// stack which prevents gc from collecting intermediate results.
//

static void _stack_init()
{
    g_num_stack_objs = 0;
    g_num_stack_frames = 0;
}

void obj_stack_frame_push()
{
    assert(g_num_stack_frames < MAX_STACK_FRAMES);  // stack frame overflow.
    g_stack_frames[g_num_stack_frames++] = g_num_stack_objs;
}

void obj_stack_frame_pop()
{
    assert(g_num_stack_frames > 0);  // stack frame underflow.
    g_num_stack_frames--;
    g_num_stack_objs = g_stack_frames[g_num_stack_frames];
}

obj_t* obj_stack_push(obj_t* obj)
{
    assert(g_num_stack_objs < MAX_STACK_OBJS);  // stack overflow
    g_stack[g_num_stack_objs++] = obj;
    return obj;
}

static int _stack_index(int i)
{
    assert(g_num_stack_frames > 0);  // no stack frames!
    int bottom = g_stack_frames[g_num_stack_frames - 1];
    if (i >= 0) {
        assert(bottom + i < g_num_stack_objs);  // past the top.
        return bottom + i;
    } else {
        assert(g_num_stack_objs + i >= bottom);  // past the bottom.
        return g_num_stack_objs + i;
    }
}

// negative indices are from the top of the stack.
// positive indices are from the bottom.
// -1 is the top of the stack
// 0 is the bottom of the stack.
obj_t* obj_stack_get(int i)
{
    return g_stack[_stack_index(i)];
}

void obj_stack_set(int i, obj_t* obj)
{
    g_stack[_stack_index(i)] = obj;
}

void obj_stack_dump(const char* desc)
{
    printf("%s = \n", desc);
    printf("    stack_frames : [");
    int i = 0;
    for (i = 0; i < g_num_stack_frames; i++) {
        printf(" %d ", g_stack_frames[i]);
    }
    printf("]\n");

    printf("    stack_objs : [");
    for (i = 0; i < g_num_stack_objs; i++) {
        printf(" %p ", g_stack[i]);
    }
    printf("]\n");
}

//
// obj makers
//

obj_t* obj_make_symbol(const char* str)
{
    int len = strlen(str);
    int id = symbol_find(str, len);
    if (id < 0)
        id = symbol_add(str, len);

    obj_t* obj = _pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, symbol = %s\n", obj, symbol_get(id));
#endif
    return obj;
}

obj_t* obj_make_symbol2(const char* start, const char* end)
{
    int len = end - start;
    int id = symbol_find(start, len);
    if (id < 0)
        id = symbol_add(start, len);

    obj_t* obj = _pool_alloc();
    obj->type = SYMBOL_OBJ;
    obj->data.symbol = id;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, symbol = %s\n", obj, symbol_get(id));
#endif
    return obj;
}

obj_t* obj_make_number(double num)
{
    obj_t* obj = _pool_alloc();
    obj->type = NUMBER_OBJ;
    obj->data.number = num;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, number = %f\n", obj, num);
#endif
    return obj;
}

obj_t* obj_make_number2(const char* start, const char* end)
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
    return obj_make_number(num);
}

obj_t* obj_make_pair(obj_t* car, obj_t* cdr)
{
    obj_t* obj = _pool_alloc();
    obj->type = PAIR_OBJ;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, pair\n", obj);
#endif
    return obj;
}

obj_t* obj_make_environment(obj_t* plist, obj_t* parent)
{
    obj_t* obj = _pool_alloc();
    obj->type = ENV_OBJ;
    obj->data.env.plist = plist;
    obj->data.env.parent = parent;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, environment\n", obj);
#endif
    return obj;
}

obj_t* obj_make_prim_form(prim_func_t prim_func)
{
    obj_t* obj = _pool_alloc();
    obj->type = PRIM_FORM_OBJ;
    obj->data.prim_func = prim_func;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, prim_form\n", obj);
#endif
    return obj;
}

obj_t* obj_make_prim_proc(prim_func_t prim_func)
{
    obj_t* obj = _pool_alloc();
    obj->type = PRIM_PROC_OBJ;
    obj->data.prim_func = prim_func;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, prim_proc\n", obj);
#endif
    return obj;
}

obj_t* obj_make_comp_proc(obj_t* formals, obj_t* env, obj_t* body)
{
    obj_t* obj = _pool_alloc();
    obj->type = COMP_PROC_OBJ;
    obj->data.comp_proc.formals = formals;
    obj->data.comp_proc.env = env;
    obj->data.comp_proc.body = body;

#ifdef GC_DEBUG
    fprintf(stderr, "ALLOC obj %p, comp_proc\n", obj);
#endif
    return obj;
}

//
// obj type predicates
//

int obj_is_garbage(obj_t* obj) // for debugging gc
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == GARBAGE_OBJ;
}

int obj_is_immediate(obj_t* obj)
{
    assert(obj);
    return (long)obj & IMM_TAG;
}

int obj_is_boolean(obj_t* obj)
{
    assert(obj);
    return obj == KTRUE || obj == KFALSE;
}

int obj_is_null(obj_t* obj)
{
    assert(obj);
    return obj == KNULL;
}

int obj_is_symbol(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == SYMBOL_OBJ;
}

int obj_is_number(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == NUMBER_OBJ;
}

int obj_is_pair(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == PAIR_OBJ && !obj_is_null(obj);
}

int obj_is_environment(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == ENV_OBJ;
}

int obj_is_prim_form(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == PRIM_FORM_OBJ;
}

int obj_is_prim_proc(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == PRIM_PROC_OBJ;
}

int obj_is_comp_proc(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && obj->type == COMP_PROC_OBJ;
}

int obj_is_proc(obj_t* obj)
{
    assert(obj);
    return !obj_is_immediate(obj) && (obj->type == PRIM_PROC_OBJ || obj->type == COMP_PROC_OBJ);
}

obj_t* obj_cons(obj_t* a, obj_t* b)
{
    PUSHF();
    PUSH2(a, b);
    POPF_RET(obj_make_pair(a, b));
}

obj_t* obj_car(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.car;
}

obj_t* obj_cdr(obj_t* obj)
{
    assert(obj_is_pair(obj));
    return obj->data.pair.cdr;
}

obj_t* obj_cadr(obj_t* obj)
{
    return obj_car(obj_cdr(obj));
}

void obj_set_car(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj->data.pair.car = value;
}

void obj_set_cdr(obj_t* obj, obj_t* value)
{
    assert(obj_is_pair(obj));
    obj->data.pair.cdr = value;
}

int obj_is_eq(obj_t* a, obj_t* b)
{
    if (obj_is_immediate(a) && obj_is_immediate(b)) {
        return a == b;
    } else if (!obj_is_immediate(a) && !obj_is_immediate(b) && a->type == b->type) {
        switch (a->type) {
        case SYMBOL_OBJ:
            return a->data.symbol == b->data.symbol;
        case NUMBER_OBJ:
            return a->data.number == b->data.number;
        default:
            return a == b;
        }
    }
    return 0;
}

int obj_is_equal(obj_t* a, obj_t* b)
{
    if (obj_is_pair(a) && obj_is_pair(b)) {
        obj_t* a_head = obj_car(a);
        obj_t* a_tail = obj_cdr(a);
        obj_t* b_head = obj_car(b);
        obj_t* b_tail = obj_cdr(b);
        return obj_is_equal(a_head, b_head) && obj_is_equal(a_tail, b_tail);
    }
    else
        return obj_is_eq(a, b);
}

obj_t* obj_env_lookup(obj_t* env, obj_t* symbol)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    obj_t* pair = _assq(symbol, env->data.env.plist);
    if (!obj_is_null(pair)) {
        return obj_cdr(pair);
    }
    else {
        if (obj_is_environment(env->data.env.parent))
            return obj_env_lookup(env->data.env.parent, symbol);
        else {

            // AJT: REMOVE
            fprintf(stderr, "Warning: could not find symbol \"%s\" in env\n", symbol_get(symbol->data.symbol));

            return KNULL;
        }
    }
}

void obj_env_define(obj_t* env, obj_t* symbol, obj_t* value)
{
    assert(obj_is_symbol(symbol));
    assert(obj_is_environment(env));

    PUSHF();
    PUSH3(env, symbol, value);
    obj_t* pair = PUSH(_assq(symbol, env->data.env.plist));
    if (obj_is_null(pair)) {
        // did not find it. so add a new property to the beginning of the plist.
        pair = PUSH(obj_cons(symbol, value));
        env->data.env.plist = obj_cons(pair, env->data.env.plist);
    } else {
        // found it, change the value
        obj_set_cdr(pair, value);
    }
    POPF();
}

// no gc
static obj_t* _assq(obj_t* key, obj_t* plist)
{
    while (obj_is_pair(plist)) {
        if (obj_is_eq(key, obj_car(obj_car(plist))))
            return obj_car(plist);
        plist = obj_cdr(plist);
    }
    return KNULL;
}

//
// debug output
//

#define PRINTF(args...)                          \
    do {                                         \
        if (to_stderr)                           \
            fprintf(stderr, args);               \
        else                                     \
            printf(args);                        \
    } while(0)

void obj_dump(obj_t* obj, int to_stderr)
{
    if (obj_is_immediate(obj)) {
        if (obj == KTRUE)
            PRINTF("#t");
        else if (obj == KFALSE)
            PRINTF("#f");
        else if (obj == KNULL)
            PRINTF("()");
        else
            PRINTF("#<??? %p>", obj);
    } else {
        switch (obj->type) {
        case NUMBER_OBJ:
            PRINTF("%f", obj->data.number);
            break;
        case SYMBOL_OBJ:
            PRINTF("%s", symbol_get(obj->data.symbol));
            break;
        case PAIR_OBJ:
            PRINTF("(");
            while (obj_is_pair(obj)) {
                obj_dump(obj_car(obj), to_stderr);
                obj = obj_cdr(obj);
                if (!obj_is_null(obj) && !obj_is_pair(obj)) {
                    PRINTF(" . ");
                    obj_dump(obj, to_stderr);
                    break;
                }
                if (!obj_is_null(obj))
                    PRINTF(" ");
            }
            PRINTF(")");
            break;
        case ENV_OBJ:
            PRINTF("#<env 0x%p>", obj);
            break;
        case PRIM_FORM_OBJ:
            PRINTF("#<prim-form 0x%p>", obj);
            break;
        case PRIM_PROC_OBJ:
            PRINTF("#<prim-proc 0x%p>", obj);
            break;
        case COMP_PROC_OBJ:
            PRINTF("#<comp-proc 0x%p>", obj);
            break;
        case GARBAGE_OBJ:
            PRINTF("#<garbage %p>", obj);
            break;
        default:
            PRINTF("#<? 0x%x>", obj->type);
            break;
        }
    }
}

obj_t* obj_eval_expr(obj_t* obj, obj_t* env)
{
    PUSHF();
    PUSH2(obj, env); // 0, 1
    obj_t* args = PUSH(obj_cons(obj, KNULL)); // 2
    POPF_RET(proc_eval(args, env));
}

obj_t* obj_eval_str(const char* str, obj_t* env)
{
    PUSHF();
    PUSH(env);
    obj_t* expr = PUSH(read(str));
    POPF_RET(obj_eval_expr(expr, env));
}

//
// interpreter init, this needs happen before any thing else.
//

void obj_init()
{
    assert(sizeof(obj_t) == 64);

    _stack_init();
    _pool_init();
    g_env = obj_make_environment(KNULL, KNULL);
    prim_init();

    // bootstrap
    obj_eval_expr(read_file("bootstrap.scm"), g_env);
}
