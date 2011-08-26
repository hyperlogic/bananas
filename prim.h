#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

// prims
obj_t* prim_is_nil(obj_t* obj);
obj_t* prim_is_symbol(obj_t* obj);
obj_t* prim_is_number(obj_t* obj);
obj_t* prim_is_pair(obj_t* obj);
obj_t* prim_is_prim(obj_t* obj);
obj_t* prim_is_closure(obj_t* obj);
obj_t* prim_is_env(obj_t* obj);
obj_t* prim_eval(obj_t* obj);
obj_t* prim_apply(obj_t* obj);
obj_t* prim_cons(obj_t* obj);
obj_t* prim_car(obj_t* obj);
obj_t* prim_cdr(obj_t* obj);
obj_t* prim_cadr(obj_t* obj);
obj_t* prim_def(obj_t* obj);
obj_t* prim_defined(obj_t* obj);
obj_t* prim_quote(obj_t* obj);
obj_t* prim_add(obj_t* obj);
obj_t* prim_assoc(obj_t* obj);
obj_t* prim_is_eq(obj_t* obj);
obj_t* prim_lambda(obj_t* obj);

#endif
