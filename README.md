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

Fix these Leaks
-----------------
* (list* 1)
* (max 1 2)
* (or? #f) - leaks 784 nodes! wtf?!?

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
