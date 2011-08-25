Bananas
==========

Monkeys love bananas, lisp interpreters, not so much.

Todo
-----------

### Refactor

* prims - Holds primitive forms
  * _ prefix - for internal c use only.  i.e. function takes operands directly, instead of a list.
  * $ prefix - for "operatives". i.e. functions that do not evaluate their operands.
  * no prefix - for "applicatives". i.e. functions that do evaluate their operands.

### Vau and wrap

Try going Kernal style and implenting $vau and wrap style lamda expressions.

### Envs should be objs

for debugging, for modules, and to fold into ref counting.

### Obj Pools

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

### Dotted lists

Need to modify parser. and DUMP

