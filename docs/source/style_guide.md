---
title: Lobster Style Guide
---

This document defines the style that ideally should be used for all lobster
programs.

Why this document?
------------------

There is no such thing as the provably best style for writing code: it all comes
down to human preference. But one thing is extremely detrimental to reading
code: inconsistent source code formatting. Hence why it is good to have a fixed
style.

You are free to pick any style the compiler will accept for your own projects,
of course. But if you wish to cooperate with other Lobster programmers, using
the style set forth in this document will be helpful. Most code you will find
will use this style, so trying to enforce another style may be tricky.

Python, being the language closest in syntax to Lobster, has a well defined
[style guide](<http://www.python.org/dev/peps/pep-0008/>), and so do other
languages, like Java. C/C++ in particular has failed to establish a clear style,
which is a constant source of confusion and disagreement.

Default Style
-------------

Unless noted explicitly in this document, the style to follow is that of the
samples in the Lobster documentation and distribution. Let's look at a typical
example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def qsort(xs, lt):
    if xs.length <= 1:
        xs
    else:
        var pivot = xs[0]
        var tail = xs.slice(1, -1)
        append(append(qsort(tail.filter():  lt(_, pivot), lt), [ pivot ]),
                      qsort(tail.filter(): !lt(_, pivot), lt))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Indentation: Tabs vs Spaces

By default, you should use 4 spaces to indent (same as Python). The compiler
will allow you to use tabs (discouraged), and in some cases even a mix (highly
discouraged).

In theory, tabs are superior for editing, but in practice, different editors and
different methods of displaying code can make tab based code work
inconsistently, which is more of a problem for a language with significant
indentation like Lobster and Python.

### Whitespace

Typical use of whitespace is around any binary operator (in the example `<=`
`=`), after a `,` and inside `[]` when used as vector constructor or definition
(but not when used for indexing, and not inside `()`).

### Alignment

As seen in the `append` call, you are encouraged to make similar code line up
using extra spaces. Because the first part of the `append` call ended in a `,`
the second line is not counted as an indentation in the same way as the
indentation blocks above it.

### Identifiers

Lobster uses lowercase for almost all indentifiers. If an identifier consists of
multiple words, they are preferably just written consecutively, though for cases
where that becomes unreadable using an `_` to separate them is ok. Do not uses
`_` to start or end regular identifiers (see the reference manual about
anonymously declared variables).

### Indentation vs one-liners

Prefer indentation over one liners unless the block is very short, e.g. `if a <
0: 10 else: a - 5` is better in-line than indented. Avoid the use of `;`
generally, if you need multiple expressions in a block, indentation is superior.

### Prefer the shortest form

Whenever short forms are available for a particular syntax, prefer to use that,
unless readability would suffer.

### Syntax alternatives

You should prefer `and or not` over `&& || !` where possible. The latter are
there to ease the transition for those coming from C style languages, but the
former are the language default.
