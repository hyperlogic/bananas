Bananas
==========

Monkeys love bananas, parenthesis, not so much.


Ref Counting Notes
----------------
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
This causes a huge leaks, every time a non-anonymous compound-prim is evaluated, because the
ref-counter cannot clean up the local-env.

There are three solutions:
  * Ditch ref-counting and garbage-collect objs instead.  This should handle
    the cycles no problem.
  * Pre-evaluate all (non-formal) (non-eformal) symbols in the static-env when
    the $vau expression is evaluated.  This will also prevent mutations of the static_env
    after the $vau expression to affect the results.  
    But there's a definite difference. You loose the flexibilty of having access to the
    static env. You will lose the ability to lookup an argument in the static_env.
    (Not-an option)
  * Live with the leaks (not an option)
  
Well, i guess that means I'm going to switch to a gc.

Fix these Leaks/Bugs
-----------------
* (get-list-metrics) - 18 nodes
* (list*) - 8 nodes! wtf?
* (list* 1) - 10 nodes!
* (max 1 2)
* (or? #f) - leaks 784 nodes! wtf?!?
* (length '(1 2 3 4 5 6 7))  - runs out of memory wtf?!??
* (length '(1 2 3 4 5 6) - leaks 4329 nodes?!?!!?
* (eq? #t #f) - 196 nodes?!!?!?!?!?!  advanced version in bootstrap.ooo


Exceptions
----------------
Make this work. somehow. setjmp, longjmp?
How to make this work with ref-counting?

Immediate values
-------------------
Is there some hacky way to use doubles or floats as immediate values?  Nans?

Kernel Notes
====================

Immediate Values
--------------------
* #t
* #f
* #inert
* #ignore 
* ()

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
* $sequence
* list
* list*
