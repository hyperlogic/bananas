#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

obj_t* $is_inert(obj_t* obj, obj_t* env);
obj_t* $is_ignore(obj_t* obj, obj_t* env);
obj_t* $is_boolean(obj_t* obj, obj_t* env);
obj_t* $is_null(obj_t* obj, obj_t* env);
obj_t* $is_symbol(obj_t* obj, obj_t* env);
obj_t* $is_number(obj_t* obj, obj_t* env);
obj_t* $is_inexact(obj_t* obj, obj_t* env);
obj_t* $is_pair(obj_t* obj, obj_t* env);
obj_t* $is_environment(obj_t* obj, obj_t* env);
obj_t* $is_operative(obj_t* obj, obj_t* env);
obj_t* $is_applicative(obj_t* obj, obj_t* env);
obj_t* $quote(obj_t* obj, obj_t* env);
obj_t* $eq(obj_t* obj, obj_t* env);
obj_t* $equal(obj_t* obj, obj_t* env);
obj_t* $define(obj_t* obj, obj_t* env);
obj_t* $eval(obj_t* obj, obj_t* env);
obj_t* $if(obj_t* obj, obj_t* env);
obj_t* $cons(obj_t* obj, obj_t* env);
obj_t* $set_car(obj_t* obj, obj_t* env);
obj_t* $set_cdr(obj_t* obj, obj_t* env);
obj_t* $make_environment(obj_t* obj, obj_t* env);
obj_t* $vau(obj_t* obj, obj_t* env);
obj_t* $wrap(obj_t* obj, obj_t* env);
obj_t* $unwrap(obj_t* obj, obj_t* env);
obj_t* $add(obj_t* obj, obj_t* env);
obj_t* $sub(obj_t* obj, obj_t* env);
obj_t* $mul(obj_t* obj, obj_t* env);
obj_t* $div(obj_t* obj, obj_t* env);
obj_t* $num_gt(obj_t* obj, obj_t* env);
obj_t* $num_gteq(obj_t* obj, obj_t* env);
obj_t* $num_eq(obj_t* obj, obj_t* env);
obj_t* $num_lt(obj_t* obj, obj_t* env);
obj_t* $num_lteq(obj_t* obj, obj_t* env);
obj_t* $num_abs(obj_t* obj, obj_t* env);
obj_t* $print(obj_t* obj, obj_t* env);

#endif
