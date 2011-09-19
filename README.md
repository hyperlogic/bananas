Bananas
===================

Monkeys love bananas, parenthesis, not so much.

or

Kernel from scratch, stays crunchy even in milk.

Garbage-Collection stack
----------------------------
GC can happen anytime a new obj is allocated.
Therefore, it is important to reference temporary objects, or else they will be collected.
The obj_stack is the mechanism used for this.

It's really a stack of stacks.  A stack frame is a stack of objects.
A stack frame can be pushed and popped on the stack frame stack.

Usually, the stack frame is pushed in a C function scope.
Temporaries ore pushed on the stack frame, then the entire stack frame is popped on function exit.

In between the return of a function and the point where that return value is stored
the returned value has no reference.  So, It is important, when calling a function to 
immediately push the value on the stack, otherwise it might be lost on the next allocation.

  1) Anytime you call a C function which returns an obj. You must push it on the stack.
     Or immediately return it as well.

  2) obj and env values must immediately be pushed onto the stack on entry. and popped on return.

If these two invariants are maintained, there should not be any leaks.
Now, this isn't the optimal approach, but lets just get it working first, then optimize it later.

Stack operations are O(1).

Exceptions
----------------
Make this work. somehow. setjmp, longjmp?
How to make this work with ref-counting?

Immediate values
-------------------
Is there some hacky way to use doubles or floats as immediate values?  Nans?

Optimization
------------------------
Why so slow?  I see _eval, $eval, in call stack alot, perhaps I should merge the two?
And use TCO.

eq? is a beast, it eats nodes for breakfast.

Kernel Notes
====================

Immediate Values
--------------------
* #t
* #f
* #inert
* #ignore
* ()  -- evaluates to ()

Core types and primitive features
------------------------------------
* boolean? - #t and #f
* eq?
* equal? - I'm gonna skip the cycle handling part, tho.
* symbol?
* inert? - #inert
* $if
* pair?
* null?
* cons
* set-car!
* set-cdr!
* TODO: copy-es-immutable - lets see how far I can get without this...
* environment?
* ignore? - #ignore
* eval
* make-environement - my environments only have one parent, and they chain.
* $define!
* operative?
* applicative?
* $vau
* wrap
* unwrap

Core library features
------------------------------
$sequence - in obj.c

See bootstrap.ooo for the rest.

* list
* list*
* $vau - this version accepts multiple expressions, like a sequence.
* $lambda
* car
* cdr
* caar, cdar etc.
* apply
* $cond
* get-list-metrics
* list-tail
* encycle!
* max
* lcm
* map
* $let
* not?
* and?
* or?
* $and?
* combiner?
* length
* list-ref
* append
* list-neighbors
* filter
* assoc
* member?
* finite-list?
* countable-list?
* reduce
* append!
* copy-es
* assq
* memq?
* eq? - this versions takes zero or more arguments
* equal? - zero or more arguments
* $binds?
* get-current-environment
* make-kernel-standard-environment
* $let*
* $letrec
* $let-redirect
* $let-safe
* $remote-eval
* $bindings->environment
* $set!
* $provide!
* $import!
* for-each
* min
* get-module

