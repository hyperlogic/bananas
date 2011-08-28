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

Type predicates
---------------------
* boolean? - #t and #f
* ignore? - #ignore
* inert? - #inert
* symbol?
* pair?
* null?

* eq?
* equal? - needs to handle cycles, I'm just gonna skip this too.

* $if

* cons - got it.
* set-car! - DONE, return nil, instead of #inert
* set-cdr! - DONE, return nil, instead of #inert
* copy-es-immutable - uh... 
* environment? - env?
* eval - take an environment argument.
* make-env - create an env with an optional parent env
* $define! - in kernel, the first argument is a "formal-parameter-tree"....
  this is what gives kernel it's destructuring-bind behavior.
  let's see how far i can get without it.
* operative?   - TODO
* applicitive? - TODO
* $vau
* wrap
* unwrap
* *everything* else can be defined with the above...



