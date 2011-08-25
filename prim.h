#ifndef PRIM_H
#define PRIM_H

#include "obj.h"

void prim_init();

// these don't expect an argument list, nor do they eval their arguments.
#define CONS(_car, _cdr) make_cell_node(_car, _cdr)
#define LIST2(a, b) CONS(a, CONS(b, NULL))
#define LIST3(a, b, c) CONS(a, CONS(b, CONS(c, NULL)))
#define QUOTE(n) LIST2(make_symbol_node("quote"), n)
node_t* DUMP(node_t* n, int to_stderr);
node_t* EQ(node_t* a, node_t* b);
node_t* ASSOC(node_t* key, node_t* plist);
node_t* MEMBER(node_t* obj, node_t* lst);

// prims - all of these expect an argument list.  Many of them eval their arguments.
node_t* eval(node_t* n);
node_t* apply(node_t* n);
node_t* cons(node_t* n);
node_t* car(node_t* n);
node_t* cdr(node_t* n);
node_t* cadr(node_t* n);
node_t* def(node_t* n);
node_t* quote(node_t* n);
node_t* add(node_t* n);
node_t* assoc(node_t* n);
node_t* eq(node_t* n);
node_t* lambda(node_t* n);

#endif
