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
* ignore? - #ignore
* inert? - #inert
* symbol?
* pair?
* null?
* environment?
* applicative?
* operative?
* $define!
* eval
* eq?
* equal? - I'm gonna skip the cycle handling part, tho.
* $if
* TODO: copy-es-immutable - lets see how far I can get without this...




* TODO: set-car! - DONE, return nil, instead of #inert
* TODO: set-cdr! - DONE, return nil, instead of #inert
* TODO: cons - got it.
* TODO: environment?
* TODO: make-env - create an env with an optional parent env

* TODO: $vau
* TODO: wrap
* TODO: unwrap
* *everything* else can be defined with the above...



