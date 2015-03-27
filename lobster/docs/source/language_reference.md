Lobster Language Reference

This is the definition of the language that tries to be a more exact a
description of how the language works (the most exact description, however, will
always be the source code :). As such, this is not the easiest way to learn the
language, start with a tutorial first, and then use this for more difficult
questions.

Syntactically Lobster tries to follow a mix of Python and C conventions where
possible, but has a lot of unique syntax too.

Lexical definition
------------------

-   Whitespace is space, tab, carriage return, nested comments delimited by `/*`
    and `*/` or single line comments starting with `//`

-   Operator tokens are `( ) [ ] : ; , & | + ++ += - -- -= * *= / /= % %= == !=
    < > <= >= <- = := :== ! ? ?. . ->`

-   Strings delimited by `"` and character constants with `'` using escape codes
    `\n \t \r \" \' \ \x` (followed by 2 hex digits, e.g. `\xFF` is the
    character with value 255).

-   Numbers consisting of a series of digits to indicate an integer constant
    (e.g. `123`) or hex digits with a leading `0x` (e.g. `0xABADCAFE`) or digits
    with a single `.` (e.g. `.1` or `1.` or `1.1`) to indicate a floating point
    constant.

-   Identifiers made from alpha-numeric characters and the `_` (except no digits
    for the first character).

-   Keywords: `nil true false return from struct value include int float string
    vector function super is program private coroutine enum`

-   Linefeed is whitespace if it follows a token that indicates an incomplete
    expression (such as `+` or `,`) and an actual token otherwise (used to
    separate lists of expressions inside a block, see below).

-   `indent` and `dedent` tokens are generated whenever the indentation level
    changes. Indentation of any 2 adjacent lines must start with the same
    sequence of spaces and/or tabs. This ensures it is impossible to create
    indentation that behaves differently from the way it looks visually.

Grammar
-------

Below, `...` indicates a loop with exit point at that scope level, and `||` is
like `|` except indicates a precedence level difference.

program = stats end\_of\_file

stats = topexp … linefeed

topexp = `include` string\_constant \| [ `private` ] ( ident functiondef \|
struct \| vardef \| enumdef ) \| expstat

struct = ( `struct` \| `value` ) ident `:` [ ident ] `[` indlist( ident ) `]`

vardef = list( ident ) ( `:=` \| `:==` \| `<-` ) list( opexp )

enumdef = `enum` [ `+` \| `*` ] list( ident [ `=` integer\_constant ] )

functiondef = `(` args `) :` body

block = args `:` body \| functiondef

args = [ list( ident [ ( `:` \| `::` ) type ] ) ]

body = ( expstat \| indent stats dedent )

type = `int` \| `float` \| `string` \| `vector` \| `function` \| `coroutine` \|
`nil` \| ident

call = `(` [ list( exp ) ] `)` [ block [ ident block … ] ]

expstat = ( exp … `;` ) \| `return` ( [ list( opexp ) ] ) [ `from` ( `program`
\| ident ) ]

exp = opexp [ ( `=` \| `+=` \| `-=` \| `*=` \| `/=` \| `%=` ) exp ]

opexp = unary ( `*` \| `/` \| `%` \|\| `+` \| `-` \|\| `<` \| `>` \| `>=` \|
`<=` \|\| `==` \| `!=` \|\| `&` \| `|` ) unary

unary = ( `-` \| `!` \| `++` \| `--` ) unary \| deref

deref = factor [ `[` exp `]` \| `.` ident [ call ] \| `?.` ident \| `->` ident
\| `++` \| `--` \| call \| `is` type ]

factor = constant \| `(` exp `)` \| constructor \| `function` functiondef \|
`coroutine` ident call \| ident [ call ]

constructor = `[` ( `super` exp [ `,` indlist( exp ) ] ) \| [ indlist( exp ) ]
`]` [ `:` type ]

constant = numeric\_constant \| string\_constant \| character\_constant \| `nil`
\| `true` \| `false`

indlist(e) = list(e) \| indent list(e) [ linefeed ] dedent linefeed

list(e) = e ... `,`

Types
-----

Lobster is dynamically typed, and any variable, argument or vector element can
be a value of one of the following types:

-   `int` : a 32bit signed integer. Constructed using:

    -   integer constants : `123`

    -   hexadecimal constants : `0xABADCAFE`

    -   character constants : `'A'` (65)

    -   default boolean values `true` and `false` (same as `1` and `0`)

-   `float` : 32bit IEEE floating point number

-   `string` : a vector of byte sized elements, generally used to store text,
    but can store any byte array. The recommended format for text is UTF-8,
    though this is not enforced; indexing and size operations act on bytes. To
    properly manipulate unicode symbols, UTF-8 strings can be converted to
    vectors of uncompressed unicode values using the built-in functions
    string2unicode and unicode2string. Immutable: can be indexed into for
    reading but not writing.

-   `vector` : a dynamically sized array of any Lobster values, constructed with
    square brackets surrounding 0 or more comma separated values, e.g. `[ 1,
    "hello", [] ]`. May be dereferenced for reading/writing using indices (e.g.
    `a[0]`). Vectors may be typed by being suffixed by `: type`, which in the
    case of builtin types will require all elements to be of that type, or in
    the case of user defined types they can additionally be indexed by field
    names as opposed to just numeric indices (see `struct` / `value` below).

-   `nil` : a type with just one value, generally meant to indicate the absence
    of a value.

-   `function` : a function value, can be called just like a normal function.
    See below.

-   `coroutine` : a special object that contains a suspended computation, see
    the section on coroutines below.

-   undefined : a special value used for unitialized variables and functions
    that don't return a value. There are no legal operations on this type of
    value, attempting to use one will result in a runtime error.

Lobster does not have a separate boolean type. Instead, for boolean tests such
as the `! & |` operators (see below) or the builtin function `if`, the values `0
0.0 nil` (which includes the keyword `false`) are all considered to be false,
and all other values are true.

The `vector` and `coroutine` types are the only mutable objects (can change
after creation), and have reference semantics (multiple values can refer to the
same object in memory, and thus changes can be observed from each).

User Defined Types
------------------

The `struct` and `value` keywords allow you to define "typed vectors", that can
mostly be used similarly to vectors, but have some additional perks, like being
able to be indexed by name, and allowing you to write dynamically dispatched
functions for them (see MultiMethods, below). For example, from `vec.lobster`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
value xy: [ x, y ]
value xyz: xy [ z ]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use either `struct` or `value` to define these, both giving the same
result except for the latter being more restrictive: it does not allow fields to
be modified (assigned to) after it has been constructed. This makes sense for
small objects such as the one in this example, and can be used to enforce a more
functional style of programming.

Generally in Lobster `:` separates the thing you are defining by what it is
being defined as. Here it is followed by a vector of field names. Optionally, it
can be prefixed by the name of a supertype, which has the effect of adding all
the fields of the supertype to the current type, thus making it an extension of
the former.

You construct values of these types similarly to vectors, with an added type
specified at the end:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
v := [ 1, 0, 0 ]:xyz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The type ensures that the right amount of values are given, and they can now be
accessed as `v.x` etc. in addition to `v[0]`.

You can optionally construct one out of a supertype value using `super`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
v := [ super s, 0 ]:xyz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here, `s` is a vector of type `xy`, and both of its fields will be copied into
the newly constructed `xyz` vector.

Optionally, you may declare types of elements, which will cause these types to
be checked upon construction:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
value xy: [ x:float, y:float ]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Operators
---------

Lobster comes with a set of built-in operators mostly familiar from other
languages that attempt to work on as many of the above types as makes sense. In
particular, unlike most languages, many of them work on whole vectors, which
makes typical game code both convenient and fast.

### Assignment and Definition

The assignment operators simply copies the value on the left hand side into the
variable or vector index on the left hand side:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a = 1
v[0] = 1
v.x = 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All variables must already have been declared, or this will result in an error.
Instead, the `:=` operator defines and assigns in one go, and requires the
variable to not have been declared yet in this scope:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a := 1
b :== 1
enum x = 1, y, z
c \<- 1
d, e := 1, 2
f, g := 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The `:==` operator does the same as `:=`, but now forces the variable to never
be assigned to again in any of the following statements (makes it a constant).
Think of the `==` in `:==` as saying that the two will be kept equal as opposed
to merely assigned once.

`enum` is merely a shorthand for `:==`, e.g. the above example is equivalent to
`x :== 1; y :== 2; z :== 3`. If you leave out the `= 1`, the sequence will start
at `0` instead. You may specify `+` or `*` after `enum` to indicate wether you
want the sequence to continue using addition (default, by 1) or multiplication
(by 2), the latter useful for flags.

`<-` is another assignment operator, that re-defines an existing variable using
*lexically bounded dynamic scope* instead of the default lexical scope (see
example below). As you can see in the last 2 lines, all of these operators also
allow multiple values to be assigned at once (see also multiple return values
below).

### Indexing

As indicated, square brackets can be used to index into vectors, with the .
notation being a convenient shorthand for constant indices when field names are
available. These may be chained arbitrarily (to index into a vector of vectors
for example).

The `?.` operator will only dereference if its argument is not `nil`, otherwise
return `nil`.

You may even use a vector as index, e.g.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mat := [ [ 1, 2 ],  [ 3, 4 ] ]
pos := [ 0, 1 ]
print(mat[pos])
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This prints `3`, and is a convenient shortcut for `mat[pos.y][pos.x]`. Note how
it uses the last element to index with first, this is because in code an other
places 2d arrays are usually most naturally thought of as row-major.

### Mathematical Operators

The 5 binary mathematical operators `+ - * / %` (the last 3 having higher
precedence) work on `int`, `float` (or a combination, where the end result will
be `float`). They also work on vectors containing ints or floats with either
another vector or a single int or float. The resulting vector will have the
length of the shortest of all input vectors, and will contain `float` elements
unless *all* values involved were `int`. The type of the shortest vector (or
left hand side if both equal) is preserved in the result:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
[ 1, 2, 3 ] * [ 4, 5.5 ]:xy // results in [ 4.0, 11.0 ]:xy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All 5 also have have a combined assignment operator version, `+= -= *= /= %=`,
which are implemented to have exactly the same effect as their expanded form,
e.g. `a += 1` is always exactly the same as `a = a + 1`.

This matters for vectors, where `+=` does *not* overwrite the left hand side
vector with new values, but instead assigns the variable the newly constructed
vector that is the result of the `+` operation. This is vital given that vectors
use reference semantics, because otherwise assigning a constant vector to a
variable followed by `+=` would also modify that constant, which would cause
subtle bugs and generally go against Lobster's functional programming oriented
design. The downside of this is that if you pass a vector to a function, and
then use += to modify it, you are modifying your local variable only, and the
caller's vector will be unaffected. Instead, you'll have to use (multiple)
return values to give the modified value back to the caller.

In addition, the `+` operator also works as string concatenation if either side
is of string type (the other side will be converted to a string representation
if it wasn't already).

Unary minus (`-`) acts on the same types in the same way as the binary version.

The increment and decrement operators `++` and `--` work either as prefix
(increment, return new value) and as postfix (increment, return old value) on
variables and vector index.

### Comparison and Logical Operators

The next lower level of precedence are the comparison operators `< > <= >=`
which work on `int`, `float` and `string` and `vector` (returning a vector of
booleans, use builtin functions `any` and `all` to test these), and then the
equality operators `==` and `!=` which additionally work on all other types, but
in particular for `vector` and `coroutine` compare *by reference*, i.e they will
give true only if both sides refer to the same object (*object identity*). To
test for *structural identity* instead, use the built-in function `equal`.

The logical operators `|` and `&` (or, and) are the next lower level of
precedence, and both short-circuit: `a | b` returns `a` if it is not a false
value (one of `0 0.0 nil`), and `b` otherwise. `a & b` returns `a` if it is a
false value, `b` otherwise. The unary operator `!` turns false values into `1`
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
function name(arg1, arg2): body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`body` can either be a single expression, multiple expressions all on a single
line separated by `;`, or, most commonly, an indentation (start of code on the
next line further than the previous line, in this case the `function` keyword),
and then any number of expressions on their own line separated by linefeeds,
until a de-dedentation occurs (return to the indentation level of the parent, in
this case again the `function` keyword). It is an error to de-dedent less than
the parent level. For example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function name(arg1, arg2):
    exp1
    exp2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The return value of a function is always that of the last expression evaluated,
or given by `return` (see below).

Functions can be called as part of an expression with a similar syntax to its
definition, e.g. `name(1, 2)`. Alternatively, you may use the `.` notation to
place the first argument ahead of the call, for example `1.name(2)`. If you are
using the `.` notation with a function that has just one argument, the `()` may
be omitted, `v.length` being a common example. You are encouraged to only do
this for simple functions that return a property of the argument, and don't
modify the argument.

Arguments can be just an argument name (which will be available as a lexically
scoped local variable inside body), or a typed name (e.g. `s:string`). Types
will be checked at run-time, and cause the incoming value to either be converted
to that type, or rejected with a run-time error.

You can use :: instead of : for typed vector arguments, which allows you to
access all fields of that vector directly, without having to prefix them with
the argument name, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function magnitude(v::xy): sqrt(x * x + y * y)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Additionally, types allow the definition of multimethods, whereby all functions
of the same name and number of arguments inside the program (not necessarily
adjacent in code, in any order) act as a single function:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function intersect(a, b): "no idea how to intersect these two!"
function intersect(c:circle, p:xy): "point in circle"
function intersect(c:circle, r:ray): "ray vs circle"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Calling intersect with any 2 arguments will automatically call the most
appropriate function. If 2 functions apply to the argument types, the most
specific one (the one where the exact types match) will be preferred, determined
by the arguments from left to right. If no function satisfies the argument
types, this is a run-time error (the above example will never have a runtime
error, since at least one function always matches).

### Function Values

You can also create anonymous (nameless) functions as values. In the most
general case, this has the syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
f := function(arg1, arg2): body
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You call these just like any other function, e.g. `f(1, 2)`.

The full `function` syntax is infrequently used however, because most function
values are created to be passed to other functions, and Lobster has a special
syntax for this situation that is meant to mimic control structures in other
languages. Any function call may be followed by one or more function values,
where the `function` keyword is omitted:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
for(10) (i): print(i)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here, the function `for` is called with 2 arguments, the first is `10`, and the
second is the function value `function(i): print(i)`. Lobster allows two more
levels of further simplification of the syntax if the arguments do not contain
type annotations:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
for(10) i: print(i)
for(10): print(_)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can leave out the parentheses, and you may use anonymous arguments, which
are variable names starting with an `_` that will automatically be declared as
arguments for you. If you use multiple such arguments (e.g. `_a` and `_b`), they
will become arguments in lexical order, irrespective of what order they appear
in the body. Using anonymous variables is only recommended for very simple
function bodies.

This style of syntax is intended to make each function that takes a function as
argument (a *higher order function*) have the convenient syntax of a control
structure, since that's what those functions usually are meant to be anyway. In
fact, Lobster does not have built-in control structures as part of the language,
`for` and `if` are examples of built-in functions, and have no different syntax
from any such function the user could define.

As an example of how to pass more than one function value, let's see an example
for `if`:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
nat := if(a < 0): 0 else: a
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here we see that all except the first function value must be preceded by the
name of the argument they're specifying. In Lobster, `else` is not a keyword, it
simply is the name of the 3rd argument of `if`. Similarly, with indentation:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
if(a < 0):
    print("negative numbers are scary!")
else:
    print("a = " + a)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Writing your own functions that take function values is the key to getting the
most out of Lobster. It allows you to refactor pretty much any code into
something that has no redundancy yet is easy to create, use and modify.

### Explicit Returns

Using return, we can bypass the default return value, and return from the
closest lexically enclosing named function, e.g.:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function find(list, x):
    for(list):
        if(x == \_):
            return true
    false
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`find`'s return value appears to be `false`, but by using `return`, we can
override it to be `true`. Importantly, `return true` is an expression that sits
inside a function value being passed to `if` (which in turn sits in a function
value passed to `for`), but bypasses all of this (unlike most programming
languages with function values). This is essential for functions to be able to
work as proper control structures.

If this feels like it is similar to exception handling in other programming
languages, that's because it is. Lobster even allows you to specify the name of
the function to return from (e.g. `return "expression expected" from parse`),
which is extremely handy when you want to be able to return errors from a bunch
of helper functions without having to pass it back through all intermediate
functions, such as when writing a parser. It is also powerful enough to allow
you to implement fully general exception handling in Lobster code, see
`exception.lobster`.

You can use the keyword `program` instead of a function name to force returning
from the entire program.

### Multiple Return Values

return can specify more than one value to be returned, which can then be
received by the multiple assignment syntax introduced above:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function m(): return 1, 2
a, b := m()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All return statements for any function must all return the same amount of return
values.

When `m` returns multiple values, they get assigned to each variable in turn (if
there was only 1 return value in this example, they would all receive the same
value). If there are more return values than there are variables, additionally
values are thrown away, and if there are more variables than there are return
values, this is an error.

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

Lobster also supports a safe way of optionally using *dynamic scope* by making
it *lexically bounded*, i.e. you can use dynamic scoping only on variables that
are already defined in lexical scope. As an example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
x, y := 0
function f():
    print(x + " " + y)
function g():
    x := 1
    y <- 1
    f()
g()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will print `0 1`. This makes sense for `x`, after all, the second `x` is a
completely separate variable that doesn't influence the print statement. The
`<-` operator however, changes an existing variable for the duration of it's
lexical scope, meaning that inside `g` (and thus all functions called from it
that can see the original variable) `y` has a different value. At the end of
`g`, the old value is automatically restored.

Alternatively, you can understand the part that does `y <- 1; f()` as doing
something like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
oldy := y
y = 1
f()
y = oldy
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is very useful for when you want to change a variable for the duration of a
function, which is a common programming pattern. It is also safe because unlike
other programming languages with dynamic scope, it can't function without a
governing lexically scoped variable.

Typing
------

While Lobster is dynamically typed by default, it does allow optional type
annotations that have many uses:

-   to cause type errors to happen earlier (where the values are created rather
    than used)

-   used to create multi-methods (see earlier)

-   to provide coercion (`int` -\> `float`, anything -\> `string`)

-   as documentation

-   (future) to receive some type errors at compile time

-   (future) for faster execution

As we've seen, you can type function arguments and UDT fields. Important to note
is that these are checked when called / constructed, there are still cases when
the exact types may change afterwards (though not in the case of `value`, whose
fields are immutable).

Lobster performs type checking when variables/fields are overwritten, and
requires the new value to be the same as the old one, with some exceptions:

-   `nil` may overwrite anything, and anything may overwrite `nil`

-   an `int` may overwrite a `float` (and will become a `float`)

-   any UDT may overwrite any other UDT type

Note that a type annotation specifying a type only allows that type initially,
it does not accept `nil` also. If you want to accept `nil` additionally,
currently you just have to leave it typeless, though there may be a way to
express that you accept a type or `nil` in the future.

Coroutines
----------

The higher order functions (function that take a function argument) we saw above
often perform iteration, and then call the function value back 0..N times. But
what if you want to iterate, but you don't want to run all iterations all at
once? For example, what if you want the iteration to happen across frames in a
game? In most languages, that means writing iteration code that is a lot
clumsier than the nice functions you already have. In Lobster however, you can
turn any such function into a coroutine, which is a higher order function that
can be resumed on demand. Kind of like a separate thread that only runs when you
want to (*cooperative multitasking*). For example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
co := coroutine cofor(10)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Normally, `cofor` takes two arguments, the range to iterate over, and the
function value to call for each value. When we prefix a call to for by
`coroutine`, it transforms the call into an object that can have its iteration
triggered manually. A coroutine *suspends* (yields) when it would normally call
the function value (which here is supplied by `coroutine` automatically). We can
check the last value the object produced using `co.returnvalue` (which after the
above call should be `0`) and we can cause the next iteration step to happen
with `co.resume` (or `co.resume(x)`, where `x` is the value to be returned from
the function value call inside the coroutine). At some point, the coroutine may
end naturally (when the loop is over), at which point it can't be resumed
anymore, and trying to resume it would be an error. To test whether you can
still iterate a coroutine further, you can call `co.active`. Putting that
together, a typical loop to exhaust a coroutine looks like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
while(co.active):
    print(co.returnvalue)
    co.resume
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Which would print 0..9. However, noone would ever write such a loop, since in
that case it would have been easier to just call the function this coroutine is
based on! Typical use of a coroutine object is therefore to resume it once per
frame in a game loop (for programming animations or game objects that act over
time), or once every time new data arrives from a file, a network, or another
thread.

Calling co.returnvalue when co.active has turned false will get you the return
value of the coroutine call as a whole.

If you create a coroutine object based on a function that contains local
variables, you can access those local variables even when the coroutine is not
running, using the `->` operator: `co->local` accesses a particular local
variable in a coroutine object. You can use these variables just like you can
any other variable (assign to them etc.).

Programs Structure
------------------

A lobster program is like the body of a function: a list of expressions on
separate lines, defined by a single file, the main file of your program. At this
top level of a file, you can additionally use the `include` keyword to bring
additional code into your program:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
include "std.lobster"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The contents of that file will be merged into your main file at the location of
the `include` for the purpose of compilation. If you you include the same file
twice, the second occurrence will be ignored.

You may use the keyword `private` at the top level in a file to prefix structs,
variables, and functions that you don't want to be visible outside that file.

Memory Management
-----------------

Lobster uses reference counting as its basic form of management for many
reasons. Besides simplicity and space efficiency, reference counting makes a
language more predictable in how much time is spent for a given amount of code,
since memory management cost is spread equally through all code, instead of
causing possibly long pauses like with garbage collection. Lobster has a custom
allocator for its vector object that is very fast.

Reference counting has one problem, which is that it can't deallocate cycles.
For example, this code:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
a := [ nil ]
a[0] = a
a = nil
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

will cause a memory leak, since initially the vector that a points at has a
reference count of 1, then that count increases to 2 because it now points to
itself, and then when the count is reduced to 1 because of `a`'s reference going
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
of the program, and alerting the programmer that there are leaks. It then writes
a text file with all leaks in somewhat readable form (with types and values),
making it easier for the programmer to figure out what caused the leak. The
programmer can then easily fix the leak by setting the reference causing the
cycle to `nil`, like clearing the enemy field when a unit dies, or by writing
`a[0] = nil` in the above simplified example.

Manually fixing leaks is certainly preferable, but for the cases where a Lobster
program may run for a long time, and leak-free code cannot be guaranteed,
Lobster optionally provides a garbage collector that can be run periodically to
clean up left over objects:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
amount_of_leaks := collect_garbage()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Like most garbage collectors, this function can be slow depending on the amount
of memory in use, so call it infrequently.

Built-in Functions
------------------

Built-in functions are not strictly part of the language, but since Lobster
relegates so much core functionality to them, it is useful to have a look how
the important ones work. For a complete list, please refer to the [built-in
function reference][1].

[1]: <builtin_functions_reference.html>

### Control Structures

We've already seen `if`, and it functions quite like you expect. The second
function argument is optional.

The loop constructs `for`, `map`, `filter`, and `exists` all function similarly
in that the iteration argument can be an int N (iterate 0..N-1), a string (each
byte value), or a vector (each element). They all supply 0, 1 (the element) or 2
(element, index) values to the function value, depending on the function value.
`for` returns the amount of calls that returned a true value, `map` simply
returns all return values in a vector, `filter` returns a vector of all elements
for which the call returned a true value, and exists returns the first element
for which the call returns true (and doesn't iterate further!) or `false`
otherwise.

`while` is an odd function, since it is an exception to the rule of Lobster
syntax:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
while(a < 10): a++
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

`while` returns the last return value of the body, or nil if never executed. A
similar function `collectwhile` returns a vector of all body return values.

Many other functions that look like regular functions are actually also control
structures, like many of the graphics function that change the current rendering
state. An example is `gl_translate()`, that optionally takes a body, and will
run the body and restore the previous transform afterwards.

Type Checking
-------------

This has its own document, [here][2].

[2]: <type_checker.html>
