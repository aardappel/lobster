---
title: "Lobster Design Philosophy, History, and Future"
---

This document tries to clarify why certain design choices were made in the
Lobster language and system.

NOTE: this may be a little out of date. Lobster has evolved a lot since I
wrote this.

### Primary Motivation

The most important over-arching reason behind everything in Lobster was to make
it a pleasant language to use for one single person: me, the author. I've
designed a fair number of languages in the past that had a particular goal or
should appeal to a particular target group, or were even commercial. Lobster is
not such a language.

Unlike past work, its goal from the start wasn't to try to appeal to anyone in
particular. If you happen to like Lobster, then congratulations, your taste in
programming is probably similar to mine.

Besides being useful to me, creating Lobster in itself was merely meant to be a
fun project. I love creating programming languages, but there's really no money
in it, and generally to most people new programming languages are not desirable
things, which is what had stopped me for a while creating any more of them.
Lobster was the "screw it, I am making one because it's fun" project. That, and
I had some new ideas about some subtle features that would make a language
suitable for my style of programming (heavily refactoring-based).

### What Lobster is not:

-   A popular / mainstream language. For that, I've made far too many peculiar
    choices already, in particular in relation to its syntax (very terse, not C
    based) and it's semantics (more functional than OO).

-   A beginner friendly system.

    -   The language: to use Lobster where it shines, you need to get very
        familiar with higher order functions for example.

    -   Its engine, while friendly for simple drawing, generally gives you a lot
        of control (immediate mode rendering), and requires you to understand
        the rendering pipeline moreso than simple retained mode object systems
        (e.g. Flash). If you create more complex games with it, be prepared to
        program your own "scenegraph" on top of Lobster's low level drawing
        primitives.

    -   It contains no game editor or friendly IDE. Everything is code.

-   A language for large scale / team programming. Its type system favors
    expressive power over building interfaces. It's best used for small to
    medium sized projects by programmers that know what they're doing.

-   A language that is all things to all people. While I would claim that
    Lobster is a *general purpose programming language*, it is strongly biased
    toward game programming and related fields. With a different set of built-in
    functions I am sure it would make a kick-ass server language, but I don't
    feel it is important to serve every niche.

### What Lobster is:

-   A language design motivated by *refactoring* and *compositionality*. It
    should be easier than in most languages to rip arbitrary pieces of code out
    and reorganize them and make them reusable, to keep the program
    *non-redundant*. Lobster's free variables ensure that your code keeps doing
    what it did before it was moved without needing to create tons of function
    arguments. They allow your new functionality to be as local as possible, and
    look just as terse as any built-in abstractions when using function value
    arguments. I am fanatic about well-factored code, so the design reflects my
    thinking on how give the programmer maximum refactoring fluidity.

-   A language that has some goodies built-in that streamline code typical in
    games, for example almost everything can operate on vectors of numbers
    (2D, 3D, 4D).

-   A language inspired more by functional programming than by object oriented
    programming. Lobster is of course an imperative language first: games are
    full of global state, and I believe that the most natural way to express
    that is directly, rather than indirectly by emulating state (such as with
    monads). That said, any program benefits from trying to write as much code
    as possible in a purely functional style, i.e. if it is easy to express
    something in terms of re-generating data instead of modifying it in place,
    you almost always should. Lobster has some very strong tools to make this
    possible, with a very terse and uniform higher order function call syntax,
    and optionally immutable objects.
    Lobster actually has very powerful object oriented features (its overloading
    and dynamic dispatch being more expressive than in most OO
    languages), but it doesn't push it's OO angle to the forefront, and unlike
    OO languages doesn't have an object heavy API. While OO brought us many
    important ideas in programming, it is also single-handedly responsible for
    the crippling over-engineering culture so prevalent nowadays, and Lobster is
    trying to go the opposite direction.

-   A type system that tries to get as close to the experience of using a
    dynamically typed language as possible while allowing maximum efficiency.

-   A language that tries to be very efficient with memory, thanks to
    compile time reference counting, inline structs, and a very fast allocator.

-   The belief that the ideal programming language is... TWO programming
    languages. Any larger project consists of a lot of high level and glue code
    that is infrequently executed and benefits from the simple and terse
    notation of a language like Lobster. However, any non trivial project
    (especially in games) also contains a few algorithms where complete control
    of memory layout and native code is desirable, and for that, there really is
    only one viable language, namely C++. Having access to both languages in a
    single project gives the most options for creating a fast, high quality
    result in reasonable time. Importantly, past systems/engines have not
    realized this potential because they make C++ the dominant language that is
    responsible for all the glue code, thus reducing the utility of the
    scripting language and complicating the overal project. Lobster instead
    wants Lobster code to be the main program, and relegate C++ to implement
    small libraries of speed sensitive code.

-   An engine that adheres to *make simple things simple, and complex things
    possible*. It requires very little code to start drawing, yet the system
    doesn't make any assumptions about what kind of thing you might want to
    program, and how you want to structure it.

-   A portable, cross platform system, targeting desktop and mobile OSes alike.

-   An implementation that cares about simplicity, speed, and robustness.

### What Lobster was (History):

Lobster development started in October of 2010.

Unlike code, normally a programming language design is so intricate, you benefit
from designing almost all of it up-front, before you write the first line of
code. Lobster wasn't designed like that, it's design was constantly refactored
much like it's implementation. In fact, normally the first thing I write before
I start writing a parser is the grammar, but Lobster's grammer was instead
derived from the parser code. Because initially it was simply a platform for fun
and experimenation of language features, it started out very different, both in
syntax and semantics.

Lobster started out fully dynamically scoped. I like the idea of dynamic
scoping, and wanted to experiment with it, to see if all the people on the net
claiming it is evil are a bunch of whiners. I found out very quickly they are
not whiners and reverted to default lexical scoping.

My other early experiment was a complete implementation of Icon-style
backtracking. For example, you could write things like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print #3 < 2 | #3 * 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

which would print `0 1 0 2 4`. In short, whereas in most programming language an
expression returns exactly 1 result, in Icon an EarlyLobster it would 0..N
results. `#` was an iteration operator that would generate values much like
`for` does nowadays, and `|` would simply concatenate streams of values. The
concept of something being false just meant no values were generated, i.e. the
`<` operator returns 0 or 1 results (the lhs). This backtracking would even work
across function definitions.

If that sounds complicated, it is certainly hard to get right, though it was
helped by the fanastic paper *The Implementation of Generators and
Goal-Directed-Evaluation in Icon* by Janalee O'Bagy whose implementation
technique I followed closely.

Sadly, though examples like the above look super terse and very powerful, more
general examples turned out less readable, less general, and not much shorter
than the equivalent code using higher order functions, which is when I got
inspired to instead focus on making higher order functions terser and more
similar to built-iteration, and make sure they have the same shortcutting power
as backtracking (using Lobster's cross-function `return`). I ripped out all
backtracking code at this point. I still have a copy of the last version that
has this functionality in, which at some point I can release if there's
interest.

Syntax wise, EarlyLobster had a more haskell-y / smalltalk-y bracketed syntax.
See for example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
factorial: [ n => 1 > n | (factorial n - 1) * n ]
print "factorial of 7: " + (factorial 7)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Function blocks were `[]`, and lists were `{}`. One of the first changes I made
post EarlyLobster was to switch those two, and make function calls more C-style,
as I had to admit I found I liked that better anyway, and made it easier to have
function blocks follow the function call instead of being inside of it. Then I
noticed I could denote function blocks entirely without brackets and just with
indentation, and those were removed too.

The implementation still had the C-stack based function calling implied by the
abovementioned paper, which now wasn't necessary anymore, and Lobster switched
to a more traditional VM stack.

Later, multi-methods, and more recently, coroutines, got added.

Most recently, Lobster transitioned from being a dynamically typed language to a
statically typed language. I prefer programming using dynamic typing, but I had
an idea how I could use specialization in combination with type inference and
flow-sensitive typing to get very far in being able to type-check all existing
Lobster code without having to litter it with types. That challenge took it much
further than I anticipated. The end result is impressively powerful, even though
I made it a bit more strict than I planned (i.e. making `nil` its own type).
More here: [type system](type_checker.html).

Another big change is moving to compile-time reference counts (lifetime
analysis) and in-line structs, about which you can read more in
[memory management](memory_management.html).

This signals the move of Lobster to a more efficiency/strong typing oriented language.
Strongly typed enums and bools got added, and multi-methods got replaced by a very
flexible overloading / dispatching system.
See the [language reference](language_reference.html).

In July 2020, co-routines were removed, along with the lesser known "frame log"
feature. A full explanation of why is
[here](https://github.com/aardappel/lobster/issues/100)
The summary: it is a feature that seems very attractive for game programming,
but in practice never got used (and in most cases works just as well with
classes), and they put a big complexity strain on the implementation that
makes it harder to make the language faster.
The last version of the language that does contain co-routines is marked
with the label `last_coroutine` in git, and the diff showing exactly what
was all removed (and thus gives a good picture on how it used to work) is
[here](https://github.com/aardappel/lobster/commit/9483b7026419fdce5421d22b2181e77e666daf82).
Similarly, the last version that has frame log functionality is `last_frame_log`
and the diff is
[here](https://github.com/aardappel/lobster/commit/274f76ca1f36742469090befbdb7d7eaa108af8f)

### What Lobster will be (Future Plans)

I've collected many random scribbles about what I think Lobster still needs in
`dev/TODO.txt`.
