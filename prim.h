#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

// these don't expect an argument list, nor do they eval their arguments.
#define CONS(_car, _cdr) make_pair(_car, _cdr)
#define LIST2(a, b) CONS(a, CONS(b, NULL))
#define LIST3(a, b, c) CONS(a, CONS(b, CONS(c, NULL)))
#define QUOTE(n) LIST2(make_symbol("quote"), n)
obj_t* DUMP(obj_t* n, int to_stderr);
obj_t* EQ(obj_t* a, obj_t* b);
obj_t* ASSOC(obj_t* key, obj_t* plist);
obj_t* MEMBER(obj_t* obj, obj_t* lst);

// prims - all of these expect an argument list.  Many of them eval their arguments.
obj_t* eval(obj_t* obj);
obj_t* apply(obj_t* n);
obj_t* cons(obj_t* n);
obj_t* car(obj_t* n);
obj_t* cdr(obj_t* n);
obj_t* cadr(obj_t* n);
obj_t* def(obj_t* n);
obj_t* quote(obj_t* n);
obj_t* add(obj_t* n);
obj_t* assoc(obj_t* n);
obj_t* eq(obj_t* n);
obj_t* lambda(obj_t* n);

#endif
