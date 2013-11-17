% Lobster Design Philosophy, History, and Future



This document tries to clarify why certain design choices were made in the
Lobster language and system.

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
        familiar with higher order functions and coroutines for example.

    -   Its engine, while friendly for simple drawing, generally gives you a lot
        of control (immediate mode rendering), and requires you to understand
        the rendering pipeline moreso than simple retained mode object systems
        (e.g. Flash). If you create more complex games with it, be prepared to
        program your own "scenegraph" on top of Lobster's low level drawing
        primitives.

    -   It contains no game editor or friendly IDE. Everything is code.

-   A language for large scale / team programming. It's a dynamically typed
    language that favors expressive power over protecting the programmer. It's
    best used for small to medium sized projects by programmers that know what
    they're doing.

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
    games:

    -   Almost everything can operate on vectors of numbers (2D, 3D, AnyD).

    -   Coroutines that can greatly simplify code that runs over time (across
        frames).

-   A language inspired more by functional programming than by object oriented
    programming. Lobster is of course an imperative language first: games are
    full of global state, and I believe that the most natural way to express
    that is directly, rather than indirectly by emulating state (such as with
    monads). That said, any program benefits from trying to write as much code
    as possible in a purely functional style, i.e. if it is easy to express
    something in terms of re-generating data instead of modifying it in place,
    you almost always should. Lobster has some very strong tools to make this
    possible, with a very terse and uniform higher order function call syntax,
    optionally immutable objects and coroutines that reduce the need for global
    state in the first place.  
    Lobster actually has very powerful object oriented features (multimethods
    being more expressive than traditional single dispatch in most OO
    languages), but it doesn't push it's OO angle to the forefront, and unlike
    OO languages doesn't have an object heavy API. While OO brought us many
    important ideas in programming, it is also single-handedly responsible for
    the crippling over-engineering culture so prevalent nowadays, and Lobster is
    trying to go the opposite direction.

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
    scripting language and complicating the overall project. Lobster instead
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
I start writing a parser is the grammar, but Lobster's grammar was instead
derived from the parser code. Because initially it was simply a platform for fun
and experimentation of language features, it started out very different, both in
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
helped by the fantastic paper *The Implementation of Generators and
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

Later, multi-methods, and most recently, coroutines, got added.



### What Lobster will be (Future Plans)

I've collected many random scribbles about what I think Lobster still needs in
`dev/TODO.TXT`, but here are some broad ideas as to what I think Lobster still
needs, roughly in this order:

-   More documentation, tutorials, examples, and reusable Lobster code for
    games.

-   More built-in / graphics functionality. So far I've just added what I needed
    for my own game prototypes, but it is pretty barren.

-   Generally more standard sub-systems. Related to sound, threading,
    networking, physics etc etc. I want to be careful however to not expand the
    amount of library dependencies unless I absolutely have to, since that makes
    building and integrating so much more painful, and gives a lot of people
    bloat they don't need. I am hoping that I can make a nice set of
    functionality with minimal code that can serve most people for small to
    medium projects, and for those who want extensive engine functionality to be
    linking their own additional libraries themselves (or at least make them
    optional components).

-   Platform features: better testing / building / integration on some of the
    neglected platforms.

-   A way to automatically package assets into a package for distribution
    purposes.

-   Debugging: some form of interactive debugging. I'm not missing it myself
    much, but I know others will.

-   Separate out the Lobster language from the engine even better, so it's
    easier for people to use Lobster for entirely different purposes.

-   Allowing for multiple simultaneous VMs and simultaneous windows (cleaning up
    global state for that), which relates to being able to add threading
    features and potentially write a visual debugger in Lobster itself.

-   A typechecker to catch more errors at compile time.

-   A high-level optimizer

-   An LLVM backend

-   Make the VM file format simpler, smaller, and more self-contained.


