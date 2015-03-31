The Lobster Type System

This document is about how to make most effective use of the type system..

State of the Type Checker
-------------------------

The type checker is now the only way to write Lobster code, even though it only
just excited "beta" state. There may be rough edges. If after reading this
document you still don't understand why you're getting type errors, contact me
and I see if I can improve the compiler or the documentation. If the VM crashes
due to a type related error, definitely contact me.

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
function add(x, y): x + y
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
a := nil      // a is a nilable of unknown type
if(..):
    a = "foo" // a is a nilable string
a += "bar"    // error: + not defined for nil
if(a):        // guaranteed not be nil inside block
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
    initialized with, such as an empty vector or string, or a struct filled with
    zeroes, that has a benign effect when used.

-   If you must start with `nil`, try to have the if-check for it as early on as
    possible, so all code deeper down the call chain can benefit from a non-nil
    type that doesn't need to be checked.

The flow based analysis generally understand logical expressions, so if you
write `if(!a):` instead, you'll be able to access `a` in the `else` block, but
not in the `then` block. Similarly, if you write `if (a & f(a)):` then `a` is
available as non-nil both as argument to `f` *and* in the following block.

De-referencing nillables as in `o & o.f` is so common that there's a shorthand
for it: `o?.f` (which can be chained). As useful as this looks, note that you
are just passing on the problem, as the result of such an expression is again a
nilable type. As such, this is best used inside conditionals or as statements
where the result is not needed.

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
`string?`. Alternatively you could write `nil & a`, which makes `nil` have the
type of whatever `a` has, which may be a generic variable. Similarly, `a | nil`
has the same nilable type, but now the value is actually `a` rather than `nil`.

More flow based types
---------------------

As we saw with `nil`, a variable may have a different type in certain parts of
the code, thanks to the information that a conditional provides.

Another example of this is the `is` operator:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(a is xyz): a.x
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can access fields of `xyz` regardless of what type `a` had outside this
scope.

Assignment can work similar to conditionals:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a := nil
if(..): a = xyz_0
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

Generic objects
---------------

Besides functions, objects can also be generic. If you define any field without
types:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct foo: [ a:string, b ]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You create a generic type.

Unlike functions however, Lobster is a bit more strict here in that it requires
you to explicitly define a specialization before you can use it[^1]:

[^1]: The initial typed version of Lobster would create specializations on the
fly, but this could cause very hard to track down type errors.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct foo_f: foo(float)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can now construct a foo with floats, either explicitly with `foo_f { "hi",
2.0 }`, or you can use the generic type: `foo { "hi", 2.0 }` and let the
compiler pick from all available specializations.

Functions written to accept `foo` arguments will of course work on any of the
specializations.

You can even specialize and subclass at the same time:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct bar : foo { c }
struct bar : foo(float) { c:int }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first version subclasses `foo` into another generic type, and the second
specializes and subclasses at the same time, creating a non-generic type
instead.
