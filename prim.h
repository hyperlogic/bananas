#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

// type predicates
obj_t* prim_is_nil(obj_t* obj, obj_t* env);
obj_t* prim_is_true(obj_t* obj, obj_t* env);
obj_t* prim_is_symbol(obj_t* obj, obj_t* env);
obj_t* prim_is_number(obj_t* obj, obj_t* env);
obj_t* prim_is_pair(obj_t* obj, obj_t* env);
obj_t* prim_is_prim(obj_t* obj, obj_t* env);
obj_t* prim_is_closure(obj_t* obj, obj_t* env);
obj_t* prim_is_env(obj_t* obj, obj_t* env);

// pair stuff
obj_t* prim_cons(obj_t* obj, obj_t* env);
obj_t* prim_car(obj_t* obj, obj_t* env);
obj_t* prim_cdr(obj_t* obj, obj_t* env);
obj_t* prim_cadr(obj_t* obj, obj_t* env);
obj_t* prim_assoc(obj_t* obj, obj_t* env);
obj_t* prim_set_car(obj_t* obj, obj_t* env);
obj_t* prim_set_cdr(obj_t* obj, obj_t* env);

// env stuff
obj_t* prim_def(obj_t* obj, obj_t* env);
obj_t* prim_defined(obj_t* obj, obj_t* env);

// equality
obj_t* prim_is_eq(obj_t* obj, obj_t* env);
obj_t* prim_is_equal(obj_t* obj, obj_t* env);

// math
obj_t* prim_add(obj_t* obj, obj_t* env);
obj_t* prim_sub(obj_t* obj, obj_t* env);
obj_t* prim_mul(obj_t* obj, obj_t* env);
obj_t* prim_div(obj_t* obj, obj_t* env);

// special forms
obj_t* prim_eval(obj_t* obj, obj_t* env);
obj_t* prim_apply(obj_t* obj, obj_t* env);
obj_t* prim_quote(obj_t* obj, obj_t* env);
obj_t* prim_lambda(obj_t* obj, obj_t* env);
obj_t* prim_if(obj_t* obj, obj_t* env);

#endif
