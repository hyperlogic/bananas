#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

// prim forms
obj_t* form_define(obj_t* obj, obj_t* env);
obj_t* form_if(obj_t* obj, obj_t* env);
obj_t* form_quote(obj_t* obj, obj_t* env);
obj_t* form_set(obj_t* obj, obj_t* env);
obj_t* form_begin(obj_t* obj, obj_t* env);
obj_t* form_lambda(obj_t* obj, obj_t* env);

// prim procs
obj_t* proc_is_boolean(obj_t* obj, obj_t* env);
obj_t* proc_is_null(obj_t* obj, obj_t* env);
obj_t* proc_is_symbol(obj_t* obj, obj_t* env);
obj_t* proc_is_number(obj_t* obj, obj_t* env);
obj_t* proc_is_pair(obj_t* obj, obj_t* env);
obj_t* proc_is_environment(obj_t* obj, obj_t* env);
obj_t* proc_is_procedure(obj_t* obj, obj_t* env);
obj_t* proc_is_eq(obj_t* obj, obj_t* env);
obj_t* proc_is_equal(obj_t* obj, obj_t* env);
obj_t* proc_cons(obj_t* obj, obj_t* env);
obj_t* proc_car(obj_t* obj, obj_t* env);
obj_t* proc_cdr(obj_t* obj, obj_t* env);
obj_t* proc_set_car(obj_t* obj, obj_t* env);
obj_t* proc_set_cdr(obj_t* obj, obj_t* env);
obj_t* proc_add(obj_t* obj, obj_t* env);
obj_t* proc_sub(obj_t* obj, obj_t* env);
obj_t* proc_mul(obj_t* obj, obj_t* env);
obj_t* proc_div(obj_t* obj, obj_t* env);
obj_t* proc_num_gt(obj_t* obj, obj_t* env);
obj_t* proc_num_gteq(obj_t* obj, obj_t* env);
obj_t* proc_num_eq(obj_t* obj, obj_t* env);
obj_t* proc_num_lt(obj_t* obj, obj_t* env);
obj_t* proc_num_lteq(obj_t* obj, obj_t* env);
obj_t* proc_num_abs(obj_t* obj, obj_t* env);
obj_t* proc_eval(obj_t* obj, obj_t* env);
obj_t* proc_print(obj_t* obj, obj_t* env);
obj_t* proc_not(obj_t* obj, obj_t* env);
obj_t* proc_make_environment(obj_t* obj, obj_t* env);

#endif
