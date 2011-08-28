Bananas
==========

Monkeys love bananas, parenthesis, not so much.

TODO
-----------

* sequence
* eval bootstrap.ooo
* recursive def

Code Review
----------------
* What is the minimal set of prims that we need?
* should we try kernal style $vau?

Exceptions
----------------
Make this work. somehow. setjmp, longjmp?

Immediate values
-------------------
Is there some hacky way to use doubles or floats as immediate values?  Nans?

Macros
-------------------
These are nice to play with.

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
* TODO: $vau
* TODO: wrap
* TODO: unwrap
* *everything* else can be defined with the above...



