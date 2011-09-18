Bananas
===================

Monkeys love bananas, parenthesis, not so much.

Ref Counting Notes
--------------------
All objects are ref-counted.

The functions in obj.c have different ref-counting behaviors.
Function with the own suffix implies that the caller owns the return value, 
and must unref it when they are finished with it.
The deny suffix means the caller does not own the value directly. 
For example: just reading a value, or passing it on to another function which 
will retain ownership.

Vau circular ref problem...
-----------------------------
There is a circular dependency, between a $vau and the environment it is defined within.
This means that enviornments with vau's in them will always leak.
In and of itself, this is not a big deal.  Except, that many vau's have local defines with in them.
Which means they will leak every time they are evaluated!  
Sadly, this is a common ideom in the the bootstrap library code.  
So, even basic primitives like and? and list* leak.

Here are some potential solutions:

  * Ditch ref-counting and garbage-collect objs instead.  This should handle
    the cycles no problem.

  * Don't ref the static_env within the compound-op.  This, prevents the cycle.
    But may lead to a dangling reference, when a $vau is returned from a $vau.

  * Pre-evaluate all (non-formal) (non-eformal) symbols in the static-env when
    the $vau expression is evaluated.  This will also prevent mutations of the static_env
    after the $vau expression to affect the results.
    But there's a definite difference. You loose the flexibilty of having access to the
    static env. You will lose the ability to lookup an argument in the static_env.

  * Live with the leaks

Well, i guess that means I'm going to switch to a gc.

Garbage-Collection (Mark and sweep)
----------------------
* Replace ref_count in the obj structure with mark.
* When pool-alloc occurs and the free list is empty. Call the gc. Then check again.
* Keep track of ALL temp stack objects for gc.
* Root Objs - global-env, stack-root
* Mark Phase - traverse from all the 'root' objects, and mark reachable objs.
* Sweep Phase - iterate over used objs and moved unmarked objs into the free list.

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

Code Cleanup
----------------
Adding a obj_cadr() would make the code easier to read.

Exceptions
----------------
Make this work. somehow. setjmp, longjmp?
How to make this work with ref-counting?

Immediate values
-------------------
Is there some hacky way to use doubles or floats as immediate values?  Nans?

GC Bugs
--------------------
When gc is "hammered" i.e. gc before every alloc.
There is strange behavior....

$num_gt gets invoked with a 0.0 and #e-infinity during bootstrap.
and an assertion is hit.  Which seems correct. except.. why is this not occuring
when gc is less frequent?

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


