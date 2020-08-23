---
title: The Lobster Type System
---

This document is about how to make most effective use of the type system..

The nature of the type system
-----------------------------

Lobster was originally dynamically typed. Not only is this easier to implement,
it bring tremendous expressive power "for free", particularly in the way it
allows you to write generic code which often requires arcane typing annotations
in static languages. The downsides are obvious too: type errors come late or
stay hidden, and execution speed suffers.

The type system in Lobster attempts to achieve the best of both worlds, with a
type system that retains as much as possible of the dynamic style of writing
(little to no type annotations needed, while able to type check a much broader
range of code than other languages). It achieves this by using 3 techniques that
build on eachother:

1.  **Type Inference**. A well known technique by now, as many language employ
    it one way or another, and it is an obvious way to reduce type annotations.
    By itself it is rather restrictive however, as it is limited in the kind of
    generic code it can correctly type.

2.  **Specialization**. Instead of struggling to infer or define generic types
    for everything, we specialize functions to the exact parameter types they
    are called with. This allows a much wider range of possible code inside
    generic functions, since they are only ever type-checked with specific
    types. This of course requires there to be multiple copies of functions, but
    in practice the code expansion is minimal. Specialized code can be optimized
    and inlined more aggressively, resulting in significant speed advantages
    besides the expressivity gains.

3.  **Flow Based**: Type checking happens in order of function calls, i.e. it is
    much like evaluating the code, but with types instead of values. This means
    that when type checking a particular function, it not just knows the exact
    types of parameters (thanks to specialization), but also the types of free
    variables, and it knows its callers (a compile time call stack). This means
    that when a type error happens, you can see in what context the wrong type
    got created and passed down. Think of it as an earlier dynamic type error.
    In addition, it takes into account the state of conditional statements,
    allowing you to avoid unnecessary type errors.

As a result, you are less constrained in the kind kind of code you can write
that still type checks correctly, while using less annotations. But with great
power comes great responsibility: the less annotations you write, the more a
compile time error indeed looks like a run-time error: they are almost as hard
to track down. It is certainly easy to start with no annotations, and then
gradually add them in places to force type errors to come earlier and be easier
to understand. Find your own trade-off.

Writing generic code
--------------------

In the simplest cases, there is no difference between writing dynamically typed
code and typed code. For example a simple function like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def add(x, y): x + y
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

is how you create a generic functions in either case. You create a generic
parameter by.. not specifying a type!

Now this is already a non-trivial example that will trip up many languages with
type inference, since you cannot have the arguments be generic unless the `+`
operation is defined on them, requiring them to have a way to specify the set of
types that have this operation specified (see e.g. type classes in Haskell), or
simply not allowing it at all (see e.g. generics in Java).

Lobster does not have this problem, since it simply type checks a copy of `add`
for each combination of types it is called with. This then deals correctly with
allowing to write `add("number: ", 4)`, taking care of converting `4` to a
string and calling string concatenation on the result, without making simpler
cases like `add(1, 2)` slower because of type inspection. In fact, the
opportunities for statically optimizing these operations are now greatly
increased. This is very similar to C++ templates, but without the clunky syntax.

It is typically "good style" in Lobster to use untyped arguments when a)
multiple types are possible/useful, and b) there is no relation between the
types of the arguments. Sometimes it is useful to use explicit generic
annotations, for example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def add<T>(x:T, y:T): x + y
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Would enforce that both parameters are of the same type, unlike the example above.
The type `T` is automatically deduced from the first argument that matches it (in
this case, the first), and then any further uses are required to be that same type,
so for example `add(1.0, 1)` would make `T` a `float`, and thus coerce the second
argument to `float` before the function call. In the untyped case above, it would
specialize `add` for `(float, int)` arguments, and the coercion to `float`
(required by `+`) would now happen inside the function.

You can even specify the generic type explicitly, for example `add<float>(1, 1)`
would actually coerce both arguments ahead of time. If the function has
multiple generic arguments, you don't need to specify all of them, if the
remaining ones can be inferred. Caveat: this currently only works where the `<`
directly follows a known function name (to distinguish it from uses of the
`<` operator).

And all of this also works on untyped function declarations equally, since they
internally get converted to explicit generics by simply adding generic types
`A, B, C,..` to them for their untypes arguments.

You can use generic types from surrounding functions, and from any class
this function is a "method" of (either declared inside that class, or with the
class type as first argument).

The trouble with nil
--------------------

So far, the type system tries to preserve as much freedom of dynamic typing as
possible. One area where it is however *more* strict than most existing
languages is how it deals with `nil`.

`nil` (or `null` in other languages) is usually a default value of all reference
types, which is very convenient in expressing uninitialized or optional values.
But this convenience comes at a high cost: if you forget to check for `nil`
before you access a value, your program may terminate with a run-time error.
Very often we know a value cannot be `nil`, but we cannot guarantee this, or are
lazy to do so. Result: random errors popping up in random places. Not good.

Lobster takes the obvious approach: `nil` is a separate type that is *not a
valid value* of a reference type. You may still create "nilable" types, but
these cannot be accessed (e.g. dereferenced) unless they are first converted to
their underlying type:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var a = nil      // a is a nilable of unknown type
if ..:
    a = "foo" // a is a nilable string
a += "bar"    // error: + not defined for nil
if a:         // guaranteed not be nil inside block
    a += "!"  // ok: a is of type string here
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As you can see, we can use conditionals to guard code that wants to access a
nilable. This seems like a hassle maybe, but think of the benefit: you will
never see a `nil` related run-time error *ever again*.

And now that `nil` is a separate type, we can improve our code by not using
`nil` unless absolutely necessary. Many uses of `nil` in legacy code are lazy,
and can be avoided:

-   If at all possible, try to reorder code such that variables can be
    initialized to a valid value right where they are defined. This is often
    possible.

-   If the variable cannot be initialized where it must be defined, see if
    there's a "empty" version of the reference type that the variable can be
    initialized with, such as an empty vector or string, or a class filled with
    zeroes, that has a benign effect when used.

-   If you must start with `nil`, try to have the if-check for it as early on as
    possible, so all code deeper down the call chain can benefit from a non-nil
    type that doesn't need to be checked.

The flow based analysis generally understand logical expressions, so if you
write `if not a:` instead, you'll be able to access `a` in the `else` block, but
not in the `then` block. Similarly, if you write `if a and f(a):` then `a` is
available as non-nil both as argument to `f` *and* in the following block.

`?` is generally the way to indicate a nilable type, e.g. `string?` is a nilable
string.

One more thing to notice about `nil` is that it only knows what exact nilable
type it represents once it comes "in contact with" its reference type. That
means that if you tried to access `a` right after it was defined, it would have
still have type `nil` rather than `string?`. Similarly, if you tried to assign a
vector to `a` after the first assignment, it would complain that only a
`string?` is valid, i.e. a `string` or `nil` value.

To bypass that process and force `nil` to represent a particular nilable type,
you can attach a type, like `nil:string`, which creates a value of type
`string?`. Alternatively you could write `nil and a`, which makes `nil` have the
type of whatever `a` has, which may be a generic variable. Similarly, `a or nil`
has the same nilable type, but now the value is actually `a` rather than `nil`.

More flow based types
---------------------

As we saw with `nil`, a variable may have a different type in certain parts of
the code, thanks to the information that a conditional provides.

Another example of this is the `is` operator:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if a is xyz: a.x
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can access fields of `xyz` regardless of what type `a` had outside this
scope.

Assignment can work similar to conditionals:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var a = nil
if ..: a = xyz_0
a.x        // error
a = xyz_0  // a is guaranteed non-nil below
a.x        // ok
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note how there are two different cases here: with an assignment inside a
conditional, the type checker has to be conservative, and can't guarantee the
value is non-nil. But with an un-conditional assignment, we can change the type
of `a` for the rest of the block.

An additional way is to use `assert`. Use this when you know for sure a value
must be non-nil or have a particular type, but you can't structure your code
such that this is already obvious to the type system.

Compile-time if-then's.
-----------------------

The flow-based type-checking above gives branches more specific types. But we
can do even better: if the condition is statically known, we can avoid
type-checking alltogether:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def compile_time_if(x):
    if x is int or x is float:
        1 / x
    else:
        x
assert(compile_time_if(1) is int)
assert(compile_time_if("") is string)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The division would be a type error if x is a string, but it compiles anyway
since this if is compile-time constant, and the type checker ignores this
branch.

If you're used to dynamic languages, you might think: what's the fuss? This
would be a compile error in almost all statically typed languages, since they
don't cull code before the optimizer. For example with C++ templates can't
handle this situation, and that's already more powerful than most forms of
generics.

Also note the return type is just the else branch in the string case, it doesn't
need to make a type union between the branches anymore.

This is powerful, because it allows you to write generic functions that do
subtly different things depending on the input, with no overhead from the
conditional, and the ability to operate differently on each case.

You can do something similar with overloading / dynamic dispatch, but that may
require more code or may be less efficient.

Generic classes and structs
--------------------------

Besides functions, classes can also be generic, but here we only use explicit
generic parameters to do so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class foo<T>:
    a:string
    b:T
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can now create named specializations using this syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class foo_f = foo<float>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can now construct a foo with floats, either explicitly with `foo_f { "hi",
2.0 }`, or you can use the generic type: `foo { "hi", 2.0 }` and let the
compiler pick from all available named specializations. If you wish, you can
also construct a specialization directly without naming it: `foo<float> { "hi", 2.0 }`

Functions written to accept `foo` arguments will of course work on any of the
specializations.

You can even specialize and subclass at the same time:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class bar : foo<float>
    c:int

class bar<T, U> : foo<T>
    c:U
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first specializes and subclasses at the same time, creating a non-generic type.
the second version subclasses `foo` into another generic type, passing on one generic
parameter and introducing another.


Numeric Structs (math vector types)
-----------------------------------
Structs whose fields are either all int or all float, and have between 2-4
fields have a special status in the type system, as they are accepted as
arguments to vector versions of the built-in operators (`+ - * /` etc) as
well as many vector operations part of the built-in functions (e.g. `normalize`).

You can see the default types for these (`xy`, `xyz`, and `xyzw`) defined in
`modules/stdtype.lobster`. Other types also work with these operations, though
(e.g. `color`).

Confusingly, in math, such 2-4 dimensional things are commonly referred to as
"vectors", whereas Lobster calls its resizable arrays "vectors". So in Lobster
these things are "numeric structs", but on occasion may still also be called
"vectors", sorry for the confusion.

Originally, vector operations would work on any size structs/vectors, but to
make the type system more precise (clearly distinguish between structs and vectors),
make the VM faster, allow SIMD use, and generally simplify things, they are focussed
on the common dimensions of 2-4.
