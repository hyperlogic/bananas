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
--------------------
* boolean? - #t and #f, I'm just gonna skip this...
* eq? - pretty much like mine.
* equal? - needs to handle cycles, I'm just gonna skip this too.
* symbol?
* #inert - wtf? - i guess i'll use nil
* inert? - uh, lets use nil?
* $if - sweet, lets just use if.
* pair? - got it
* null? - hmm, nil? instead?
* cons - got it.
* set-car! - DONE, return nil, instead of #inert
* set-cdr! - DONE, return nil, instead of #inert
* copy-es-immutable - uh... 
* environment? - env?
* ignore? - #ignore is a type, different from #inert. Skip it.
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



