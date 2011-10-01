Bananas
===================

Monkeys love bananas, parenthesis, not so much.

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
tco

Scheme Notes
====================

Chapter 2 - Lexical Conventions
----------------------------------

Probably not going to implment:

* strings "foo"
* character constants #\a
* [ ] { } | 

Might implement at some point:

* quasi-quote support ` , ,@
* vector constants #(1 2 3)

Chapter 3 - Variables, syntactic keywords and regions
-----------------------------------------------------------

There is a distinction between syntactic keywords and identifiers.

Type predicates, I don't have.

* char?
* string?
* port?
* vector? - i might want this at some point.

environment? is not listed.

Tail recursion.
-------------------------
lambdas implicitly can have more then one expression, and the last one occurs in "tail context"
true and false alternatives of if statements occur in "tail context"
the else expression of a case statement is in "tail context"
The last expression in an and statment is in "tail context"
The last expression in an or statment is in "tail context"
The body of a let, let* or letrec
The body of let-syntax, letrec-syntax
The last in a begin
The exit expression of a loop?

Chapter 4 - Expressions
---------------------------

TODO: lambda bodies can have more then one statment.
TODO: lambda formals of the form x
TODO: lambda formals of the form (a b c . d)

TODO: cond, case, and, or: can apparently be specified as macros.

TODO: let, let*, letrec: can also be implemented as macros
TODO: begin as a macro
TODO: do as a macro
TODO: named let as a macro
TODO: delay as a macro

TODO: quasi-quote

TODO: let-syntax

TODO: letrec-syntax

Chapter 5 - Program Structure
----------------------------------

TODO: define alternate form (define (plus a b) (+ a b))

...
