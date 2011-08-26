Bananas
==========

Monkeys love bananas, parenthesis, not so much.

TODO
-----------

### Bugs

* def should replace an existing binding.

### Bootstrap

Let's start writing lisp code in lisp.
Instead of all these c functions. Yuk.

What is the minimal set of prims that we need?

### first class envs 

eval and apply should take env argument, make-env

### Obj Pools

All nodes should come from a pool.  Some kind of memory reporting stats.

### Ref count

All objs should be ref-counted.
All prims should manange this stuff. so that "hopefully" all lisp code will too.

### Exceptions

Make this work. somehow. setjmp, longjmp?

### Immediate values

Is there some hacky way to use doubles or floats as immediate values?  Nans?

### Macros

These are nice to play with.

