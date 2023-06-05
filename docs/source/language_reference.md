---
title: Lobster Language Reference
---

This is the definition of the language that tries to be a more exact a
description of how the language works (the most exact description, however, will
always be the source code :) ). As such, this is not the easiest way to learn the
language, start with a tutorial first, and then use this for more difficult
questions.

Syntactically Lobster tries to follow a mix of Python and C conventions where
possible, but has a lot of unique syntax too.

Lexical definition
------------------

-   Whitespace is space, tab, carriage return, nested comments delimited by `/*`
    and `*/` or single line comments starting with `//`

-   Operator tokens are `( ) [ ] { } : ; , & | + ++ += - -- -= * *= / /= % %= ==
    != < > <= >= <- = ? . -> ^ << >>`

-   Strings delimited by `"` and character constants with `'` using escape codes
    `\n \t \r \" \' \ \x` (followed by 2 hex digits, e.g. `\xFF` is the
    character with value 255). Alternatively, strings can be enclosed in
    triple-quotes (`"""`) and may then contain any characters verbatim,
    including new-lines, quotes, and escape codes (which are all left
    untouched).

-   Numbers consisting of a series of digits to indicate an integer constant
    (e.g. `123`) or hex digits with a leading `0x` (e.g. `0xABADCAFE`) or digits
    with a single `.` (e.g. `.1` or `1.` or `1.1`) to indicate a floating point
    constant.

-   Identifiers made from alpha-numeric characters and the `_` (except no digits
    for the first character).

-   Keywords: `nil return class struct import int float string any void
    def fn is from program private resource enum enum_flags typeof
    var let pakfile switch case default namespace not and or attribute
    if for while super`

-   Linefeed is whitespace if it follows a token that indicates an incomplete
    expression (such as `+` or `,`) and an actual token otherwise (used to
    separate lists of expressions inside a block, see below).

-   `indent` and `dedent` tokens are generated whenever the indentation level
    changes. Indentation of any 2 adjacent lines must start with the same
    sequence of spaces and/or tabs. This ensures it is impossible to create
    indentation that behaves differently from the way it looks visually.

Grammar
-------

Below, `...` indicates a loop with exit point at that scope level (ex. `(ident ... ,)` -> `(ident (, ident)*)`, * meaning optionaly repeating), 
and `||` is like `|` except indicates a precedence level difference. `[rule]` Means optional. 

program = stats end\_of\_file

stats = topexp … linefeed

topexp = `namespace` ident
      \|`import` [ `from` ] ( string\_constant \| ( ident ... `.` ) )
      \| [ `private` ] ( functiondef \| class \| vardef \| enumdef )
      \| expstat \| attrdef

class = ( `class` \| `struct` ) ident
        ( `=` ident specializers
       \| [ generics ] `:` [ ident [ specializers ] ] 
          indlist( ident [ `:` type ] [ `=` exp ] \| functiondef ) )

specializers = `<` list( type ) `>`

generics = `<` list( ident ) `>`

vardef = ( `var` \| `let` ) list( ident [ `:` type ] ) `=` opexp

enumdef = ( `enum` | `enum_flags` ) ident `:` indlist( ident [ `=` integer\_constant ] )

functiondef = `def` ident [ generics ] functionargsbody

functionargsbody = `(` args `)` `:` body

block = [ args ] `:` body \| functionargsbody

args = [ list( ident [ ( `:` \| `::` ) type ] [ `=` exp ] ) ]

body = ( expstat \| indent stats dedent )

type = `int` \| `float` \| `string` \| `[` type `]` \| `resource` `<` ident `>` \| `void` 
\| ident [ specializers ]

call = [ specializers ] `(` [ list( exp ) ] `)` [ block [ `fn` block … ] ]

expstat = ( exp … `;` ) \| `return` ( [ list( opexp ) ] ) [ `from` ( `program` \| ident ) ]

exp = opexp [ ( `=` \| `+=` \| `-=` \| `*=` \| `/=` \| `%=` ) exp ]

opexp = unary [ ( `*` \| `/` \| `%` \|\| `+` \| `-` \|\| `<` \| `>` \| `>=` \|
`<=` \|\| `==` \| `!=` \|\| `&` \| `|` \| `and` \| `or` \| \^ \|
`<<` \| `>>`) unary ]

unary = ( `-` \| `++` \| `--` \| \~ \| `not` ) unary \| deref

deref = factor [ `[` exp `]` \| `.` ident [ call ] \| `->` ident
\| `++` \| `--` \| `is` type ]

factor = constant \| `(` exp `)` \| `super` \| ctrlflow \| pakfile string\_constant \| constructor \| `fn` functionargsbody \| ident [ call ]

ctrlflow = `if` ifpart \| (`for` \| `while`) exp `:` body

ifpart = exp `:` body (`else` `:` body \| `elif` `:` ifpart) 

constructor = `[` [ list( exp ) ] `]` [ `::` type ] \| ident `{` [ list( ident `:` exp \| exp ] `}`

constant = numeric\_constant \| string\_constant \| character\_constant \| `nil` [ `::` type ]

attrdef = `attribute` ident [ `=` ( string\_constant \| numeric\_constant \| ident ) ]

indlist(e) = indent list(e) [ linefeed ] dedent linefeed

list(e) = e ... `,`

Types
-----

Lobster is statically typed, and any variable, argument or vector element can be
a value of one of the following types:

-   Scalar types (64-bit on all platforms):

    -   `int` : a signed integer. Constructed using:

        -   integer constants : `123`

        -   hexadecimal constants : `0xABADCAFE`

        -   character constants : `'A'` (65)

        -   default boolean values `true` and `false` (same as `1` and `0`)

    -   `float` : an IEEE floating point number

    -   a function value, can be called just like normal functions. See below.

-   Reference values:

    -   `string` : a vector of byte sized elements, generally used to store
        text, but can store any byte array. The recommended format for text is
        UTF-8, though this is not enforced; indexing and size operations act on
        bytes. To properly manipulate unicode symbols, UTF-8 strings can be
        converted to vectors of uncompressed unicode values using the built-in
        functions string_to_unicode and unicode_to_string. Immutable: can be indexed
        into for reading but not writing.

    -   vector : a dynamically sized array of any Lobster values, constructed
        with square brackets surrounding 0 or more comma separated values, e.g.
        `[ 1, 2, 3 ]`. May be dereferenced for reading/writing using indices
        (e.g. `a[0]`). Vectors may be typed by being suffixed by `::type`, which
        will require all elements to be of that type (also useful to force a type
        on an empty list).

    -   `class` / `struct` : a user defined data structure, see below.

    -   `nil` : a special value of any reference type above, that indicates the
        absence of a legal value. `nil` is only allowed if the type is
        "nilable", for more on that see the document on type checking,
        [here](type_checker.html). Since just `nil` doesn't make it obvious
        what type you are referring to, you can use e.g. `nil::string` to
        specify a value of type `string?` (a type that denotes the value can be
        a string or nil), though in most cases type inference makes writing
        just `nil` sufficient.

    -   `resource<T>` : an opaque object managed by the engine / C++ code.
        `T` is the type, e.g. `texture` or `mesh`, the availability of these
        depends on what engine functionality is linked in to Lobster, see
        `builtin_functions_reference.html` for what functions create and use
        these resources. Their lifetime is managed by the language much like
        other reference types above.

Lobster does not have a built-in boolean type, though it does have a pre-defined
`bool` enum (see enums below). In general, for boolean tests such
as the `not and or` operators (see below) or the builtin function `if`, the values
`0 0.0 nil` (which includes the enum value `false`) are all considered to be false,
and all other values are true.

The vector and `class` types are the only mutable objects (can
change after creation), and have reference semantics (multiple values can refer
to the same object in memory, and thus changes can be observed from each).

User Defined Types
------------------

The `class` and `struct` keywords allow you to define a user defined type. For
example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct xy:
    x:int
    y:int

struct xyz : xy
    z:int
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use either `class` or `struct` to define these, with the latter being more
restrictive: it does not allow fields to be modified (assigned to) after it has
been constructed, they are stored in-inline in their parent and copied.
This makes sense for small objects such as the one in this example, and can be used
to enforce a more functional style of programming.

You specify a list of fields using indentation.

Optionally, you specify a supertype, which has the effect of adding all the
fields of the supertype to the current type, thus making it an extension of the
former.

The above example uses ints directly, but you
can also define types more generically, and then define named specializations of them:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct xy<T>:
    x:T
    y:T

struct xy_i = xy<int>
struct xy_f = xy<float>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You construct values of these types you use a similar syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let v = xy_i { 1, 0 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(Or use `xy_f { 1, 0 }` / `xy<bool> { true, false }` to explicitly pick a different
specialization).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct xy:
    x = 0.0
    y = 0.0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Additionally, you may specify default values, if these are given, then these
values are not arguments to the constructor, e.g. `xy {}`.

For more complex structs, you can use field names as "tags" in a constructor
call, for example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xy { x: 1, y: 2 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Besides being more readable, it allows you to specify the fields in any order,
and to override fields that have defaults.

To declare a type whose only purpose is to serve as a superclass for other
types and is not to be instantiated, declare it with `abstract`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
abstract class Node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Operators
---------

Lobster comes with a set of built-in operators mostly familiar from other
languages that attempt to work on as many of the above types as makes sense. In
particular, unlike most languages, many of them work on (numeric) structs, which
makes typical game code both convenient and fast.

### Assignment and Definition

The assignment operators simply copies the value on the right hand side into the
variable or vector index on the left hand side:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a = 1
v[0] = 1
v.x = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All variables must already have been declared, or this will result in an error.
`var` defines and assigns in one go, and requires the variable to not
have been declared yet in this scope. `let` does the same for variables which
cannot be modified afterwards:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var a = 1
let b = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var d, e = 1, 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As you can see in the last 2 lines, all of these operators also
allow multiple values to be assigned at once (see also multiple return values
below).

### Indexing

As indicated, square brackets can be used to index into vectors, and similarly
`.` can dereference fields of a `class` or `struct`.
These may be chained arbitrarily.

You may even use a vector as index, e.g.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let mat = [ [ 1, 2 ],  [ 3, 4 ] ]
let pos = xy_i { 0, 1 }
print mat[pos]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This prints `3`, and is a convenient shortcut for `mat[pos.y][pos.x]`. Note how
it uses the last element to index with first, this is because in code and other
places 2d arrays are usually most naturally thought of as row-major.

### Mathematical Operators

The 5 binary mathematical operators `+ - * / %` (the last 3 having higher
precedence) work on `int`, `float` (or a combination, where the end result will
be `float`). They also work on structs containing ints or floats with either
another struct or a single int or float. These structs must be the same type.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
xy_i { 1, 2 } * xy_i { 4, 5 }  // results in xy_i { 4, 10 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All 5 also have have a combined assignment operator version, `+= -= *= /= %=`,
which are implemented to have exactly the same effect as their expanded form,
e.g. `a += 1` is always exactly the same as `a = a + 1`.

In addition, the `+` operator also works as string concatenation if either side
is of string type (the other side will be converted to a string representation
if it wasn't already), though in most cases you should prefer to use
string interpolation (see below).

Unary minus (`-`) acts on the same types in the same way as the binary version.

The increment and decrement operators `++` and `--` work either as prefix
(increment, return new value) and as postfix (increment, return old value) on
any lvalues.

Bitwise operators `& | ^ ~ << >>` behave like they do in any other language.

### Comparison and Logical Operators

The next lower level of precedence are the comparison operators `< > <= >=`
which work on `int`, `float` and `string` and structs (returning a struct of
ints, use builtin functions `any` and `all` to test these), and then the
equality operators `==` and `!=` which additionally work on all other types, but
in particular for `vector` and `class` compare *by reference*, i.e they will
give true only if both sides refer to the same object (*object identity*). To
test for *structural identity* instead, use the built-in function `equal`.

The logical operators `and` and `or` are the next lower level of precedence,
and both short-circuit: `a or b` returns `a` if it is not a false value (one
of `0 0.0 nil`), and `b` otherwise. `a and b` returns `a` if it is a false
value, `b` otherwise. The unary operator `not` turns false values into `1`
and others into `0`.

The `is` operator returns true if the left hand side value is of the type
specified on the right, e.g. `x is float`.

Function Definitions
--------------------

Lobster's entire design centers around functions and how they can be composed.
It has both *named functions* and *function values*.

### Named Functions

Named functions can be declared at any scope level (may be local), like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def name(arg1, arg2): body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`body` can either be a single expression, multiple expressions all on a single
line separated by `;`, or, most commonly, an indentation (start of code on the
next line further than the previous line, in this case the `def` keyword), and
then any number of expressions on their own line separated by linefeeds, until a
de-dedentation occurs (return to the indentation level of the parent, in this
case again the `def` keyword). It is an error to de-dedent less than the parent
level. For example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def name(arg1, arg2):
    exp1
    exp2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The return value of a function is determined by its `return` statements (see
below) or `void` (no return value) otherwise, except for anonymous functions,
which don't need an explicit `return` (it is automatically the last expression
evaluated).

Arguments can be just an argument name (which will be available as a lexically
scoped local variable inside body), or a typed name (e.g. `s:string`).
If you don't specify types, the function is generic,
meaning it will receive types from the caller. If called with multiple
combinations of incompatible arguments, you automatically get multiple
"specializations" of the same function, meaning working with different types
is very easy. Alternatively, you can specify generic types explicitly as well
(more in [type system](type_checker.html)).

You can use :: instead of : for typed class/struct arguments, which allows you to
access all fields / functions of that object directly, without having to prefix
them with the argument name, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def magnitude(v::xy): return sqrt(x * x + y * y)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can also leave out the `v::xy` entirely if you define this function as part
of a `class` / `struct` definition of type `xy` (see above). Both types of
definition are equivalent.

You can specify an explicit return type, like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def f(a:int, b:int) -> float: return a + b
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is typically not necessary, but may be helpful when coercing to a more
general type.

### Function calls

Functions can be called as part of an expression with a similar syntax to its
definition, e.g. `name(1, 2)`. Alternatively, you may use the `.` notation to
place the first argument ahead of the call, for example `1.name(2)`. If you are
using the `.` notation with a function that has just one argument, the `()` may
be omitted, `v.length` being a common example. You are encouraged to only do
this for simple functions that return a property of the argument, and don't
modify the argument.

Optionally, you may also call functions without any parentheses at all, e.g.
`print "hi!"`. This is only allowed for known functions (that are not ambiguous
with variables) that have 1 expression argument (followed by any number of
function value arguments that don't take arguments themselves, see below). It is
up to the programmer to use good judgement on when to use this, the recommended
use case is for calls used as statements (no nesting) that don't cause
additional parentheses to be needed elsewhere. In case of doubt, use the
standard call syntax.

### Function Values

You can also create anonymous (nameless) functions as values. In the most
general case, this has the syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let f = fn(a:type, n:type): body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Or, if you are not specifying any types:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let f = fn a, b: body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You call these just like any other function, e.g. `f(1, 2)`. You currently
must call them using a variable (not any expression, not even a field).

The full `fn` syntax is infrequently used however, because most function
values are created to be passed to other functions, and Lobster has a special
syntax for this situation that is meant to mimic control structures in other
languages. Any function call may be followed by one or more function values,
where the `fn` keyword is omitted:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
g(10) i: print(i)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here, the function `g` is called with 2 arguments, the first is `10`, and the
second is the function value `fn i: print(i)` (as before, we left out the
the `()` around the arguments). Lobster allows yet further simplification of
the syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
g(10): print(_)
g 10: print _
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You may use anonymous arguments, which
are variable names starting with an `_` that will automatically be declared as
arguments for you. If you use multiple such arguments (e.g. `_a` and `_b`), they
will become arguments in the order they appear in the body. Using anonymous
variables is only recommended for very simple function bodies.

As mentioned above, you may drop the parentheses entirely if the body doesn't
have any argument declarations.

This style of syntax is intended to make each function that takes a function as
argument (a *higher order function*) have the convenient syntax of a control
structure, since that's what those functions usually are meant to be anyway.
Lobster's  built-in control structures `if` `for` and `while` have syntax
that is closely compatible with this function call syntax (and in the case of
`for` allow the same argument simplifications).

The return value of these functions is the last expression in `body`. You don't
use explicit `return` statements, as those are used with the enclosing named
function instead, see "Explicit Returns" below.

Though not recommend (as readability suffers), it is even possible to pass
multiple function values to a function, but then every function value except the
first can't omit the `fn` keyword:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
g(10) i:
    print i
fn:
    print "reached the end"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For example, to pass an additional function to do something special at the end of
an iteration.

Writing your own functions that take function values is the key to getting the
most out of Lobster. It allows you to refactor pretty much any code into
something that has no redundancy yet is easy to create, use and modify.

Typically, to write a function that takes a function value argument, simply
use an argument with no type:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def twice(f):
    for(2): f()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes, you may want to be explicit about the function type. You can declare
new function types, and then use them as a type:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def function_type(i:int) -> int
def g(f:function_type): return 1 + f(2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Besides enforcing the type of function that can be passed (which makes for more
readable errors when it fails), it also forces the function passed to not be inlined
(which may reduce code-bloat if functions taking this function type are called
many times, at a minimal hit in speed). Function values called over a generic
variable are always inlined (since they represent a unique function type even
if its signature is accidentally the same as another function value), to ensure
higher order functions are competitive in speed with hard-coded equivalents.

Lobster really wants you to be able to use function values everywhere at no
cost, so besides guaranteed inlining, the other way they differ from other
languages is that they are always *non-escaping*. What that means is, that
while function values may use free variables (refer to variables from enclosing
scopes), they are not "closures", i.e. they do not close over (capture) these
variables, they merely refer to them. This means these function values cannot
be called from an environment where those free variables are not available
anymore (this will typically result in a compile-time error). This makes a
function value in Lobster a single code pointer without any variable information
attached, and thus extremely cheap. The downside is that they can't be used
for things like certain kinds of callbacks, though you probably shouldn't be
using those in game-like contexts anyway ;)

### Explicit Returns

Using `return` you return from the closest lexically enclosing named function,
e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def find(list, x):
    for list:
        if x == _:
            return true
    return false
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`find`'s return value is `false` unless the nested `return` gets evaluated, and
then it becomes `true`. Importantly, `return true` is an expression that sits
inside a function value being passed to `if` (which in turn sits in a function
value passed to `for`), but bypasses all of this (unlike most programming
languages with function values). This is essential for functions to be able to
work as proper control structures.

If this feels like it is similar to exception handling in other programming
languages, that's because it is. Lobster even allows you to specify the name of
the function to return from (e.g. `return "expression expected" from parse`),
which is extremely handy when you want to be able to return errors from a bunch
of helper functions without having to pass it back through all intermediate
functions, such as when writing a parser. This is a simple form of exception
handling, that is also powerful enough to allow
you to implement fully general exception handling in Lobster code, see
`exception.lobster`.

You can use the keyword `program` instead of a function name to force returning
from the entire program.

If you need to return from a function value with an explicit return, you need
to turn that particular function into a named function instead.

### Multiple Return Values

return can specify more than one value to be returned, which can then be
received by the multiple assignment syntax introduced above:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def m(): return 1, 2
let a, b = m()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All return statements for any function must all return the same number of return
values.

When `m` returns multiple values, they get assigned to each variable in turn.
If there are more return values than there are variables, additionally
values are thrown away, and if there are more variables than there are return
values, this is an error.

Other than functions (and in assignments statements, see above), expressions
can return multiple values in other contexts too, but there may need to
be placed in parentheses to disambiguate them from other uses of `,`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let x, y = if foo: m() else: (1, 2)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


### Scope

Functions and variables declared there-in always obey lexical scope: any use of
a variable always refers to the closest enclosing definition of it.

Since functions and function values can be defined at any scope level, this
means they can access variables from enclosing scopes, called *free variables*.
Free variables are essential to make Lobster's higher order functions
convenient. References to free variables are only valid within the scope they
are defined, which luckily is almost always the case, but can be broken by
storing a function value and then calling it at a later time outside of the
context where its free variables were valid, which will result in a runtime
error. Other languages use *closures* to ensure availability in all cases, which
are very costly (parent stackframe(s) may have to be dynamically allocated) as
opposed to Lobster's approach which makes function values and free variables
have no overhead compared to regular functions and variables.

### Overloading and dynamic dispatch

Overloading and dynamic dispatch are part of the same system, the only difference
being wether choosing the right function is done at compile time or runtime.

You can define these overloads anywhere, either as part of a class, or outside
of them, regardless of wether you wrote the original class. For example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def f(a::A): return 1
def f(b::B): return 2
def f(c::C): return 3
def f(i:int): return 4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(as we note above, using `::` instead of `:` merely means all fields of that type
become directly available, saving you having to type `a.`, but is otherwise
equivalent.)

What happens when you call `f` depends on the types above, and the type you call
it with. If all 4 types are unrelated, then you guaranteed get static dispatch
(a normal function call). If `B` inherits from `A`, but `C` is unrelated, and you call
with either a `B`, `C` or `int` argument type, you still get static dispatch, since
there is statically only one option.

Only if you call with an `A` argument however, you get dynamic dispatch,
since the argument may point to a `B` value, and `B` has a different function
implementation. A dynamic dispatch goes thru a "virtual table", and while slower than
static dispatch, is still very fast. As you can see, wether something is "virtual" gets
decided per call, and with knowledge of the whole program (all types and functions that
can possibly exist), so typically less calls result in dynamic dispatch than in other
languages.

Types like `int` never participate in dynamic dispatch, since they don't have a
sub-class relation to any other type.

Defining these functions can also be done "in line" in a class declaration, like so:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class A:
    def f(): return 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This definition of `f` for `A` is entirely equivalent to the one above, except
the name of the first argument is now `this` instead of `a` above.

Only the first argument to a function is used to resolve dynamic dispatch, but
for static overloading, all arguments will be taken into consideration.
Lobster used to have the ability to dispatch on all arguments, called "multi-methods",
which at least academically seem very elegant.
In practice however, these are slow (require complicated look-up tables) and ambiguous
(hard to tell which function will get called, sometimes accidentally combine unrelated
functions into a multimethod and get unexpected errors or slow-down). Single dispatch
gives predictable, fast polymorphism that seems to work well for most languages,
so for the moment, multi-methods are removed from the language.

Overloading and dynamic dispatch can even be mixed with type specialization (see
[type system](type_checker.html)), meaning you can generate multiple versions of
a polymorphic call that do different things. Simply leave out the type of any
arguments beyond the first:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class A:
    def g(c): return c + 1
class B : A
    def g(c): return c + 2
x : A = B {}  // Type is A, but dynamic value is a B!
assert x.g("hi") == "hi2"
assert x.g(3) == 5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here, the call to `g` is dynamically dispatched for `A` or `B`, but choosing the
`int` or `string` specialization is entirely static.

You can force calling a superclass method with the `super` keyword:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class B : A
    def f(): return 1 + super f()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

That `f()` is statically dispatched to call `A`'s version of `f` (or its
superclass, if it doesn't have one).


### Functions with different number of arguments / default arguments.

You can define functions with the same name but different number of
arguments. These are essentially treated as independent functions, in the sense
that which is being called is always determined completely statically.

Functions can even have default arguments, as long as the default arguments
don't cause ambiguity with other functions of the same name:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def da(a:X, b, c = x + 1): return c
def da(a:Y, b, c = x + 1): return c
def da(a): return a
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This defines 2 complete separate functions, one that has 3 arguments (and can
be called with 2, since it has one default argument), and one with 1 argument.
As you can see, since there is no overlap in number of arguments, it is always
clear which variant is being called.

The version with 3 arguments has 2 overloads. Overloads must have exactly the
same default arguments, if any.

Default arguments are simple substition, writing `da(1, 2)` is exactly the same
as writing `da(1, 2, x + 1)`. That's why you can even use variables in these
default arguments, as long as they're in scope when called.


### Operator overloading.

You can overload what operators do on `class` and `struct` types, by writing
a function that defines what the `operator` should do:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct A:
    x:int
    def operator+(o:A): return A { x + o.x }
    def operator-(): return A { -x }

print - A { 2 } + A { 3 }  // A { 1 }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You may currently overload: `+` `-` `*` `/` `%` `++` `--` `==` `!=` `<` `>`
`<=` `>=` `&` `|` `^` `~` `<<` `>>` `=` `+=` `-=` `*=` `/=` `%=` `&=` `|=`
`^=` `<<=` `>>=` `[]` (indexing).

Overloaded operators are parsed with the same operator precedence as for the
built-in operators, but when it comes to typechecking, they are handled as
regular function calls, with all that endtails. The above expression is thus
handled internally as `operator+(operator-(A { 2 }), A { 3 })` where `operator+`
is just an ordinary function name.

The assignment operators make most sense to overload on `class` types, since
there you can overwrite the members of the `this` (or left) argument of the
operator. There is currently no way to do this with `struct` which are always
copied by value.

See more examples in `modules/quaternion.lobster` and `tests/operators.lobster`.


### Global and Class member variables with function scope.

You may declare a (what appears to be) a local variable with the `member` keyword
instead of `let`, which automatically stores it in the surrounding class:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Foo:
    a = 1
    def bar():
        member b = 2
        b += a
        return b
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Calling `bar` will give different results each time, it really is almost the same
as writing:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Foo:
    a = 1
    b = 2
    def bar():
        b += a
        return b
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The difference is that `b` is not available outside of `bar`, trying to access it
will result in an error.

But why would you want to do this, if it so similar? A small example doesn't do
this justice, but in games it is very common to need a lot of variables that
track state of things that happen across frames, which are often used in just
a single method, and end up cluttering the class they are in. By using `member`,
you put the declaration closer to where it is used, which makes it easier to
view all occurrences in larger classes. It also makes it easy to see that the
variable is used only in a single method. This reduces "cognitive load" in
understanding the code, compared to seeing a long list of member variables at the
top of a class and not knowing what their relationship is. Seeing that the
variable is local to, say, `render()` also gives additional information what
the variable may be used for.

Currently, `member` must occur in a function declaration that is declared
inside a class (not a struct), restrictions that may be lifted in the future.

You can do the same for functions outside of classes with the keyword `static`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let a = 1
def bar():
    static b = 2
    b += a
    return b
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Which again is pretty much the same as writing:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let a = 1
var b = 2
def bar():
    b += a
    return b
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With again the main difference being that in the former, `b`'s scope is restricted
to `bar`.
This is not as useful as `member` because there is only ever one instance possible,
but still nice to use when you can to simplify the complexity of global scope
elements.

But these two features are just the start of the real fun: the `member_frame` and
`static_frame` keywords. These behave like their counterparts above, with one
subtle difference: they will be reset to their initializer whenever a frame does
NOT use the variable. Or rather, their state is persisted as long as frames
keep using the state. It's like "immediate mode", but for state. For example using
`static_frame` (`member_frame` works the same other than where the variable is stored):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def foo():
    static_frame b = 0
    print b++

gl_window("foo", 100, 100)
while gl_frame():
    static a = 0
    a++
    if a % 4:
        foo()
    else:
        print "/"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will print an endless sequence of `0 1 2 / 0 1 2 / 0 1 2 / ..` until you
close the window. Why is this special? Notice that `foo` didn't need to be told
when to reset its sequence, that was automatic. In a game, you often have a lot
of systems and states those systems can be in, and when something changes (the
user goes into a menu or a particular gameplay or animation state), all these
systems need to be "updated" to be aware of the new situation. Here, much like
with an immediate mode gui where you don't need to worry about creating and
deleting widgets, simple control flow can dictate what systems are "active",
and their associated state automatically gets reset when the situation changes.
It's as if the variable `b` doesn't exist when `foo` is not being used, and
it automatically gets created/deleted for you.

variables declared this way use more space and an extra check (an extra variable
to check the frame count) but otherwise function just like normal variables,
so you should feel free to use them wherever they make the code simpler/clearer.

Typing
------

Lobster is statically typed, though most of the time you don’t notice, since
most types can be inferred. You specify types:

-   To define overloaded / dynamic dispatched functions (see earlier).

-   To provide coercion (`int` -\> `float`, anything -\> `string`)

-   As documentation.

-   To get simpler/earlier type errors.

As we've seen, you can type function arguments and UDT fields.

For more detail, see the [type system](type_checker.html).

Enums
-----

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum example:
    foo = 1
    bar
    baz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An `enum` defines a "strongly typed alias" for the `int` type. What this means is
that these values are fully compatible with `int` in any use, but a regular `int`
can't be passed to a context where an enum type is explicitly requested.

You can convert integers explicitly to an enum with a coercion function, e.g.
`example(1)` will create a value equivalent in type and value to `foo`.

If you leave out the `= 1`, the sequence will start
at `0` instead. Values automatically increment from the last explicitly specified value,
so `bar` will be `2` here. Instead of `enum` you can use `enum_flags`, which changes the
default first value to `1` and uses `* 2` to get to next value instead of `+ 1`.

Functions like `string`, `print` will get you the name of an enum value, and likewise
`parse_data` can turn these names into enum values (when part of a data structure).

When you use an enum in a `switch`, it is an error to not test all values of an enum
(if there is no default case).

### Booleans

A bool is not a built-in type, rather it is defined as an `enum` in `stdtype.lobster`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum bool:
    false
    true
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because they are enums, they have the same typing rules: a `bool` can be used anywhere
an `int` is expected, but not the other way around. Similarly, you can use e.g. `bool(1)`
to convert ints.

Programs Structure
------------------

A lobster program is like the body of a function: a list of expressions on
separate lines, defined by a single file, the main file of your program. At this
top level of a file, you can additionally use the `import` keyword to bring
additional code into your program:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
import std
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The contents of that file will be merged into your main file at the location of
the `import` for the purpose of compilation. If you you import the same file
twice, the second occurrence will be ignored.

An identifier like `std` is the same as specifying `"std.lobster"`, similarly `a.b`
is short for `"a/b.lobster"`.

Modules will typically be loaded relative to 2 locations: the current
main `.lobster` file being compiled, and whereever the lobster compiler is installed.
In both those locations, files may be optionally be found under an `modules`
sub-directory.
You can use `import from "path/to/"` to provide additional such starting directories
(relative to the current main `.lobster` file being compiled) that any following import
statements (recursively) can use.

You may use the keyword `private` at the top level in a file to prefix structs,
variables, functions, enums, fields and methods that you don't want to be
visible outside that file.

Memory Management
-----------------

Lobster uses (compile time) reference counting as its form of management for many
reasons. Besides simplicity and space efficiency, reference counting makes a
language more predictable in how much time is spent for a given amount of code,
since memory management cost is spread equally through all code, instead of
causing possibly long pauses like with garbage collection. Lobster has a custom
allocator that is very fast.

Most reference counting happens at compile time using a "lifetime analysis"
algorithm, details [here](memory_management.html).

Reference counting has one problem, which is that it can't deallocate cycles.
For example, this code:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class rec:
    r:rec?

var x = nil
x = rec { nil }
x.r = x
x = nil
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

will cause a memory leak, since initially the object that `x` points at has a
reference count of 1, then that count increases to 2 because it now points to
itself, and then when the count is reduced to 1 because of `x`'s reference going
away, we now have an object with no outside references that still thinks its
being referenced, thus not deallocated. That is a leak. Now this is a simple
example, but in the general case with complex data structures, it is not
generally possible for a programming language to ensure this never happens.

Leaks like these are not common, as they only occur with graph-like structures
or "parent reference" common in more complicated data structures. An example in
a game might be if two game units refer to eachother as their "enemy", and then
both die at the same time with the programmer forgetting to reset the enemy
field before they die.

Lobster deals with this by detecting that such objects are left over at the end
of the program, and alerting the programmer that there are leaks. It then outputs
a "leak report" with all leaks in somewhat readable form (with types and values),
making it easier for the programmer to figure out what caused the leak. The
programmer can then easily fix the leak by setting the reference causing the
cycle to `nil`, like clearing the enemy field when a unit dies, or by writing
`x.r = nil` in the above simplified example.

More details on Lobster's [memory management](memory_management.html).


Control Structures
-----------------

As noted, all of these follow closely the function call syntax introduced above
as much as possible, but are otherwise treated specially by the language.

`if` may be followed by multiple `elif` blocks and a single `else` block:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if a < 0:
    print "negativity not allowed!"
elif a < 10:
    print "single digit!"
elif a < 100:
    print "double digit!"
else:
    print "way too big!"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`elif` is simply short for writing `else: if`. You can also write these on a single
line, which is only recommended when very short, e.g. `if a < 0: 0 else: a`

`for` is the only built-in construct taking 0 to 2 arguments to the block: the element
being iterated over, and iteration index.

We can iterate over vectors (each element), strings (each byte), or integers (values
0..N-1):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
for("hello") a, i:
    print "{i}: {a}"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here `a` will contain the 5 characters and `i` will be just `0` to `4`.

The module `std` contains further useful loop constructs on top of `for`, like
`map`, `filter`, and `exists` etc.

`while` is an odd function, since it is an exception to the rule of Lobster
syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
while a < 10: a++
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

That looks perfectly normal, but one thing should stand out: while takes not
one, but 2 function values. Normally, in Lobster, this would make more sense:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
while():
    a < 10
do:
    a++
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

but since people are so used to the way while loops work from other languages,
the first argument has a special type that automatically converts an expression
into a function value. When you think about it, even in languages like C the
condition of a while is the only part of the language that can be executed more
than once yet does not use / cannot use the block `{}` syntax. This exception is
carried over in Lobster. This is not great for readability so isn't generally
used elsewhere.

`while` returns void. A similar function int module `std` called `collectwhile`
returns a vector of all body return values.

Both `for` and `while` can have a break statement inside of them, that
exits the enclosing loop. Alternatively, use `return` or `return from` (see
above) for more complex cases.

Many other functions that look like regular functions are actually also control
structures, like many of the graphics function that change the current rendering
state. An example is `gl_translate`, that optionally takes a body, and will
run the body and restore the previous transform afterwards.

`switch` has special syntax, since it does a lot of things different:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var st = switch i:
        case 1: "no"
        case 2, 3:
            var x = i
            "yes{x}"
        case 4..6, 8: "maybe"
        default: "what?"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The value you switch on may be int, float or string. Cases may test for multiple
values, even ranges (which are inclusive)


Modules and Name Spaces
----------------------
A module is simply a single `.lobster` file, that can be imported into another using the
`import` keyword. You can `import` a module from multiple files and it will only be compiled
once.

You can prefix any top-level declaration by `private` to cause it not be available to
users of the module.

Lobster has a very simple namespacing mechanism that uses `_` for separating namespaces,
effectively generating just a longer identifier:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace foo

class bar:
    x = 1

def baz(): return bar {}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Names `bar` and `baz` can be used as-is inside this module, but must be referred
to as `foo_bar` and `foo_baz` outside of this module. Non-top level items like `x` are not
affected.

Most built-in functions come with a namespace, such as `gl` etc.

Namespaces are indistinguishable from identifiers that already have a `_` baked into them,
except from the fact that these have to be referred to by their full name everywhere, even
inside the module that defines them.


Declaration order
-----------------
Lobster is a language that relies heavily on type inference and generic types, and generally
not requiring you to specify types, the order in which things get type-checked sometimes
matters.

Lobster type-checks function calls in call order, but type declaration in the order in which
they are specified in the source code, or imported.

As such, to allow the maximum amount of freedom it what can refer to what, it is recommended
to import files and declare types (structs and classes) as much as possible in the order of
dependencies (least dependent things first), and call functions from top level (which triggers
a lot of use of these types) only after all have been declared.

This is not always possible, so there are ways to declare things ahead of definition, for
example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Monster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(note the lack of `:` introducing the definition) pre-declares this type, so it can
be referred to by types defined before `Monster` is finally defined. This allows for
circular dependancies.

Similarly, you may declare variables before you define them:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class World
var world:World

// Lots of types that may want to access `world` as a global in their methods goes here.
// Then finally `World` gets defined based on the earlier types.

var world:World = World {}
world.init()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is an error to access `world` by code in between its declaration and definition.
Typically, all code accessing `world` in any methods in between only gets called
below, so this works out fine.


Type Checking
-------------

This has its own document, [here](type_checker.html).


Built-in Functions
------------------

Please refer to the [built-in function reference](builtin_functions_reference.html).


String Interpolation
-------------------
This is a convenience feature that allows you to write arguments to string
conversion inline in the string, instead of endless amounts of `" + "`
separators, which tends to be less readable for longer sequences.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"a = {a} and b = {b}"  // Same as: "a = " + a + " and b = " + b
"a = {f(a) + 1}"       // Same as: "a = " + (f(a) + 1)
"{a}"                  // Same as: string(a)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Because of this feature, if you actually want to put `{` or `}` in a string
constant, they must be escaped (by using 2 of them, or prefixing with a `\`),
e.g.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
let a = 42
print "\{ {a} \}"  // Prints: "{ 42 }"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Multi-threading
---------------

Lobster has built-in multi-threading functionality, that is in its early stages.

It is different from multi-threading in most other languages, in that it does not
allow threads to share memory or any other VM state. It essentially runs one Lobster
VM per hardware thread / core, each running an independent and isolated copy of
your Lobster program. This prevents entire classes of potential concurrency bugs,
like race conditions.

Communication/synchronisation between the threads is explicit through the use of a
"tuple space" (a bag of Lobster objects used as messages) which are copied rather
than shared. While copying is potentially slower than sharing, it allows each
Lobster VM to run as-if it was single-threaded, with no synchronisation primitives
to slow it down (for example, memory allocators in Lobster are single-threaded),
no GIL (global interpreter lock) or other multi-threading overhead, so overal
performance can easily be higher than shared memory concurrency systems.

The use of tuple spaces and 1 VM per thread suits "worker" style concurrency, for
example if you have 100 parallel tasks to perform, you throw each of those in
the tuple space, and the individual VMs grab and complete them, given automatic
load balancing for however many cores are available.

For now, the easiest way to get a feel for how this works is to read
`samples/threads.lobster`.
