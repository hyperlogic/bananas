Bananas
==========

Monkeys love bananas, lisp interpreters, not so much.

Todo
-----------

### Refactor

Split eval and parse stuff.  Split prims from internal functions?

### Envs should be nodes

for debugging, for modules, and to fold into ref counting.

### Node Pools

All nodes should come from a pool.  Some kind of memory reporting stats.

### Ref count

All nodes should be ref-counted.

### Exceptions

Make this work.

### Immediate values

Is there some hacky way to use doubles or floats as immediate values?  Nans?

### Macros

macro-expand

### Boot-strap

Start implementing lisp code in lisp.

