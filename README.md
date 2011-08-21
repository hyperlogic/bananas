Bananas
==========

Monkeys love bananas, lisp interpreters, not so much.

Todo
-----------

### Envs should just be assocs.

Remove special case vector code.
Add prim assoc find

### Lambda Expressions

Add new closure node type.  { args, body, up }
Hit up apply with new logic.

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

