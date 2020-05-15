---
title: Memory Management in Lobster
---

This is a more in-depth explanation of how memory management in Lobster works, and
is typically not needed to be understood fully to use the language.

It may be interesting to those wanting to implement a similar scheme in another language.

Introduction
------------
Memory management is an aspect of a language that has one of the biggest influences
on how a language turns out: it affects the type system and the kinds of types you can
have, it affects efficiency in both time and space, it affects the cognitive model of
the programmer in what data structures can be represented, it affects latency, interoperability,
and much more.

Yet, it is often ignored and almost invisble at the same time. Many consider it to be
a "solved" problem with probably 95% of programming languages out there using some
form of garbage collection: allocate, then worry about reclaiming unreachable objects
later.

I think it is far from solved. If I'd have to make a top-10 of "least elegant
algorithms in computer science", the #1 would be an easy pick. The idea that just
because "who is pointing to this object" is a hard question we are not going to
even try to track it, and instead repeatedly scan large parts of the heap to recover
that information later, really makes me cringe.

The problem is maybe that for the longest time, the alternatives have not been great.
Low level languages would use manual memory management (efficient but error prone),
and some have used reference counting. Then there were languages based on linear
types or regions, but those never entered the mainstream.

I've personally always thought of reference counting as the "least bad" solution,
which is why it was initially used for Lobster. Many people dismiss it out-right
because it can't collect cycles, but I've found that, in practical use, getting a
"cycle report" at program exit that helps the programmer know where to break those
cycles, is largely sufficient.

I mentioned linearity (every object can only have one pointer to it) and regions
(every object is assigned a memory pool that outlives them all), and I have implemented
languages in the past that implemented both of those strategies. While viable, I felt
at the time they required too much programmer assistence to be mainstream.

Ownership models
---------------
There is a memory management design space which I'll collectively call "Ownership"
which is a generalization of the linearity models: one pointer always owns an
object (and is responsible for its deallocation), and others merely "borrow" it.

This now poses new questions: How do we determine who is the owner? When is
borrowing allowed? How do we detect a borrower that outlives an owner? What
about an owner being modifier while there are borrowers? Do we check any
of these at compile time, runtime, or a mix?

There have been many ownership models in the past, but only recently they are
becoming more mainstream:

* Rust is the poster child, which tries to solve all of the above questions at
  compile time, at the cost of much programmer assistence.
* Modern C++ with `std::unique_ptr` is essentially an ownership model, except one
  that answers questions like "How do we detect that a borrower outlives an
  owner" with "We don't". But even with such caveats, it largely works well,
  and is a huge improvement over the manual memory management of C and even
  the manual RAII of earlier C++.
* Swift, one of the few languages that uses reference counting (inherited from
  objective C) has intentions to (optionally) move to
  [ownership](https://github.com/apple/swift/blob/master/docs/OwnershipManifesto.md).
* Nim has intention to move away from GC to ownership (see post
  [1](https://nim-lang.org/araq/destructors.html)
  [2](https://nim-lang.org/araq/ownedrefs.html)) with a model that is partially
  programmer assisted and partially runtime.
* D has [plans](https://dlang.org/blog/2019/07/15/ownership-and-borrowing-in-d/).
* Many more research-y or less known languages, e.g.
  [Gel](https://pdfs.semanticscholar.org/d0f2/d28962d2a50d1914f0af8243d3f382fe077c.pdf),
  [Dyon](https://github.com/PistonDevelopers/dyon/issues/173),
  [CoSy](http://liu.diva-portal.org/smash/get/diva2:20899/FULLTEXT01.pdf),
  [ParaSail](https://drive.google.com/file/d/0B6Vq5QaY4U7ubm5qVkFpMEtmN2s/view)
  [Cone](http://cone.jondgoodwin.com/memory.html)
  [Scopes](https://gist.github.com/paniq/71251083aa52c1577f2d1b22be0ac6e1)
  [Pure RC GC](https://researcher.watson.ibm.com/researcher/files/us-bacon/Bacon03Pure.pdf)
  [Mitten](https://github.com/doctorn/micro-mitten)

I personally think this general direction is the future of memory management.
It just needs to be made more friendly than Rust for most programmers to
want to come on board. Lobster offers a possible direction.

Lobster's Memory Management Strategy
-----------------------------------
This has two parts, its by-value model and lifetime analysis.

### In-line, by-value structs

The best memory management is.. not to have to manage it at all!
You can make an incredibly fast memory manager, but it will never beat simply
having less objects to manage. This is the thinking behind Lobster's
by-value structs, which offer similar "zero abstraction cost" objects
to ones available in C/C++, Rust, and even C#, but surprisingly not
in most other languages. It is the most effective and cheapest way towards
more efficient memory use that frankly every language should have.

Essentially, you declare an object with the `struct` keyword instead of the
usual `class`, which causes Lobster to always allocate this object inline
in the parent, where "parent" can be another struct or class, a vector, or
simply the stack (as a local variable). On an implementation level, it is
as if you added all the fields of the struct as fields/variables to the
parent, so writing `s.x` (where `s` is a struct) is just as cheap as
writing `x`. The struct disappears.

This of course has limitations, in that you're completely dependent on
the lifetime of the parent, and that you assign these kinds of struct by
copy, rather than by pointer. That means it is not great for large
objects, but usually up to 4 fields or so this is still more efficient
than heap allocation.

Currently, such structs are immutable, since that simply makes sense for
small, copied objects. This is likely to be relaxed in the future to also
allow mutation.

There are situations where rather than copying the implementation could
use a short-lived reference instead, like is common in C++. This is
however not actually faster in many cases, and more importantly, a
reference would be subject to the lifetime analysis below. By making
them copies, we essentially "reduce pressure" on that algorithm, meaning
it is more likely to produce optimal lifetime assignments and less
errors.

Having these by-value structs is especially important in a language like
Lobster that has so far focussed on the areas of games and computer
graphics, which make intensive use of 2D/3D vectors. Having such types
be as cheap as possible is important.

### Lifetime Analysis

Lobster combines its original (runtime) reference counting with a lifetime
analysis algorithm, to get "compile time reference counting".

This is a fully automatic algorithm that mostly does not require programmer
intervention. In my personal testing, working with several dozens of Lobster
programs (many of which medium sized game prototypes) I needed to make minor
changes in only 2 locations when transitioning from runtime to compile-time
reference counting, the rest "just worked". Or rather, I was able to make
the algorithm jump through hoops such that code changes weren't necessary.

Essentially, the algorithm picks a single owner for each new heap allocation,
which is usually the first variable, field, or vector element it gets
assigned to, and then tries as best as possible to make all uses from there
on "borrows". The initial ownership and all borrows do not require any
runtime reference counting. If somewhere else in the code something wants to
own that same value, it will insert a reference count increase only for this
particular use (in Rust, this would cause an error instead). Using this
analysis was able to remove around 95% of runtime reference count operations.

This is the default behavior, since it is "good enough" of an optimisation
for most code, and very convenient for the programmer. For those cases where
the programmer wants maximum control however, they can opt-in to a more Rust-like
model on a per variable basis, which will guarantee there will only be one
owner ever, and error out if the analysis is not able to guarantee this. This
will also be able to remove the reference count field from objects entirely,
a further optimisation. This has not been implemented at the time of writing,
however.

### Errors

These are errors that may happen as a consequence of lifetime analysis, though
all of them have proven to be very rare.

~~~~
cannot assign to x while borrowed
~~~~

This can be caused by code like:

~~~~
var x = "hello"
def f(y):
    x = "world"
    print y
f(x)
~~~~

Here, `y` borrows `x`, then `f` tries to overwrite `x` while still being borrowed,
which would cause it to deallocate `"hello"` while `y` is still pointing at it.
It is pretty rare for code that uses a borrowed value to be accessing the original
value also. As I mentioned, this only happened twice in a large volume of test code,
and is easy to avoid.

~~~~
cannot assign to borrowed argument: x
~~~~

This typically never triggers, as arguments that are assigned to are typically forced to
be owners, but there are currently some exceptions related to dynamic dispatch where this may still
happen. Likely will be eliminated entirely in the future.

~~~~
variable x still has 1 borrowers

x used in <feature> without being borrowed
~~~~

These two I've never even seen happen, they largely exist because I am not smart enough
to write a formal proof that they can't happen, and then ensure that my code matches the
proof :)

The algorithm in detail
----------------------

The "ground truth" for how the algorithm is to be found in `typecheck.h`,
this section is merely a description of it. If you were hoping for pages of
equations like the typical PL paper, I am going to have to disappoint.

More on the type checker [here](type_checker.html).

Wait, what, the type checker? Yes, as much as I would have preferred to make
the lifetime checker a standalone algorithm, it is interwoven with type checking.
The reason for this is that a lot of the power of Lobster centers around its
"Flow Sensitive Type Specialization", meaning it type checks functions in
call-graph order, and specializes them based on types. As it turns out, to
most optimally remove reference count operations, *we want to specialize on
lifetimes as well*. Making it a separate algorithm would mean duplicating a lot
of this logic, so I decided to interleave them.

### AST lifetime matching

The lifetime analysis works differently from most other languages in that rather
than working just on variables and other storage locations, it works on all
values in the language.

To be more precise, every AST node has a lifetime it expects of its children,
and a lifetime it passes to its parent. The core of the algorithm is thus the
matching that takes place between child and parent:

* When the two agree, nothing happens. This is most of the time. For example in
  `let a = [ 1, 2, 3 ]` the vector wants to be owned, and the variable wants to own,
  everybody is happy. No Reference counts touched.
* When the parent wants to own, but the child wants to borrow, then either a
  a reference count increase has to be inserted (by default), or it is an error (optionally).
  For example, in `let a = b`, `b` wants to borrow (lend) itself, but `a` wants to own,
  so this has to be reconciled.
* When a parent wants to borrow, but a child wants to be owned, we insert an anonymous
  variable in the current scope that will delete the value later, much like what happens
  to R-values in C++. For example in `print [ 1, 2, 3 ]` like most built-in functions
  `print` defaults to only wanting to borrow the value, and the vector wants to be owned.
  So instead it gets held on to and deleted at the end of the scope.

### L-value borrowing

Owned values are simple.. these are values that someone needs to be responsible for
at all times.

Borrowed values are a little more complicated. Whenever an expression returns the
value of a variable, field or vector element we want to default to borrowing these
values, since the L-value already owns it. We have a stack of currently active L-values,
and the lifetime returned by the expression refers to these stack elements. This
lifetime is also reference counted, as multiple borrows may be active at once.
This reference count decreases as borrowed values get "consumed". Mutating a
borrowed L-value with a reference count > 0 produces the above error. The
reference count not being 0 at the end is a bug in the lifetime checker :)

Note that variables currently always own. This was done for simplicity, as in
theory a variable could be made to either own or borrow depending on the needs
of its initial RHS, but this was awkward in cases where the definition and
subsequent assignments disagreed, and generally made checking lifetimes more
difficult. This could be revisited in the future once the consequences of
the algorithm are better understood, or maybe when more accurate dataflow
information is present.

The end of a variable's lifetime is typically the end of the scope, though
making it its "last use" is being worked on, see `return` below.

### Function specialization by lifetime.

Function lifetime specialization is probably quite unique to Lobster, and solves an
important problem: if different callers to a function have different lifetimes
that would be ideal for their arguments, the compiler (or worse, the programmer)
would have to pick one, and let the others be suboptimal. Not only does this
produce unnecessary reference counting, it reduces the amount of code that
could otherwise work error-free in the context of strict ownership. For example:

~~~~
def f(x): print x
f([ 1, 2, 3 ])
f(a)
~~~~

If `x` had been fixed to own because of the first call, the second call
would have been less efficient, or even an error. Now they both get to be
maximally efficient because of specialization. Now in this simple example
it is easy to see that `x` should really borrow, but it isn't always that
trivial, and would need a more complex lifetime analysis that employs
"logical variables" much like type inference currently does (assuming
you want to do this without help of the programmer).

There are currently some exceptions to this:

* Arguments that are assigned to are always owned.
* Coroutines always own (because they are longer lived).
* The return value of a function is currently always owned.

Much like variables, some of these could be relaxed/improved in the future.

### Lifetimes for AST types

These are currently how the different AST types interact lifetime wise
with their children (wants) and parent (results).

Note that most of code also uses a lifetime of "any", which means either
that the lifetime doesn't matter (for e.g. scalars) or that the recipient
is cool with any kind of lifetime.

* Assignment wants to own, and results in borrow.
* `if` results in the union of the lifetime of both branches. The union
  is simple if both are equal or either side doesn't care. If they differ,
  it defaults to own. It has specialized code to detect if one of the
  branches never returns (has a return statement in it that escapes the
  `if`) in which the lifetime is that of the other branch.
  The condition wants to borrow. `if` with one branch
  of course does not result in anything.
* `switch` is similar to `if`, but currently always defaults to owning the
  value returned by the cases, since doing a correct lifetime union
  between many cases in the context of branches not returning is harder.
  This should be improved.
* `and` and `or` are again similar to `if`, but have some special cases
  because sometimes a value is guaranteed ignored (`a and b or c` never
  results in `a`) or values are coerced to `bool` when they are
  incompatible (`"a" or "b"` results in a string with an own lifetime,
  but `[ 1 ] or "hello"` results in a `bool` with the values immediately
  deleted).
* `while` borrows its condition and doesn't care about the body.
* `for` borrows what it iterates over and doesn't care about its body.
  It produces an element variable that wants to be owned.
  Note: this may change in the future to also allow borrowing.
* String constants conceptually result in an owned value (they are a heap
  object), but currently they actually result in a borrow. This is
  because strings are most frequently passed to contexts that would prefer
  to borrow (like `+` or many builtin functions) and thus produce a lot
  of reference count churn. To avoid this, the VM allocates strings on
  first use, but never lets the reference count drop to 0, meaning they
  can safely be borrowed, and reduce total reference counting.
  Or more simply: it's as if there exists an anonymous global variable
  for each string constant that's actually used, and which you borrow
  from.
* `+` on strings wants to borrow, and returns owned. This is the same
  in principle for all binary operators, but thanks to "inline structs"
  this doesn't matter for most of them anymore.
* Built-in functions by default want to borrow all their args, and
  return an owned value. There are exceptions to this rule, which can
  be marked when declaring a built-in, mainly for functions that
  directly operate on a vector or other data structure. For example `push`
  wants to own its second argument, and `top` returns a borrowed value.
* As noted above, `return` typically wants to own what it is returning,
  but currently makes an exception for the frequent case of returning
  a single variable, where it pretends to borrow it. This is because
  otherwise it would typically cause a reference count increase followed
  by a decrease as the variable goes out of scope.
  Extending the cases where this happens, and also generally allowing the
  last use of a variable to be owned by its last use rather than waiting
  for the end of the scope, is planned as a future improvement.
* Any data structure constructors (objects, vectors, coroutines) of course
  result in own.
* Any L-value results in borrow.

