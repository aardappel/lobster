Using Lobster from the command line

## Basic usage

If you have a file `helloworld.lobster` that contains

    print("Hello, World!")

then running it like so will compile and run it:

    lobster helloworld.lobster

## Command line options

These can be passed to lobster anywhere on the command line.

-   `-b` : generates a bytecode file (currently always called
    "`default.lbc`") in the same folder as the `.lobster` file it reads,
    and doesn't run the program afterwards. If you run lobster with no
    arguments at all, it will try to load "`default.lbc`" from the same
    folder it resides in. Thus distributing programs created in lobster
    is as simple as packaging up the lobster executable with a bytecode
    file and any data files it may use.

-   `-t` : run the typechecker (& optimizer)

-   `-w` : makes the compiler wait for commandline input before it
    exits. Useful on Windows.

-   `-c` : (deprecated, this should now be automatically detected).
    *forces lobster into "command line" mode. This is useful on Apple
    platforms where by default lobster expects to be run from within an
    app bundle. With this option, it will not try to look for files in
    an app bundle, but instead functions much like Windows & Linux.*

-   `--gen-builtins-html` : dumps a help file of all builtin functions
    the compiler knows about to `builtin_functions_reference.html`.
    `--gen-builtins-names` dumps a plain text list of functions, useful
    for adding to syntax highlighting files etc.

-   `--verbose` : verbose mode, outputs additional stats about the
    program being compiled

-   `--parsedump` : dumps internal representations of the program as
    AST, and `--disasm` for a readable bytecode dump. Only useful for
    compiler development or if you are really curious.

## Default directories

It's useful to understand the directories lobster uses, both for reading
source code files and any data files the program may use:

-   the main directory (`lobster`): this is where the lobster exe
    resides, or the "Resource" folder on Apple platforms (when run from
    an app bundle instead of the command line). This is the folder this
    has the default `include shaders data docs samples` folders inside
    of it.

-   the auxiliary directory: this is where the main `.lobster` file
    being compiled resides.

-   the directory for writing files: the same as auxiliary on desktop
    platforms, but often a special directory on mobile platforms.

Additionally, if any of these folders contains an "`include`" directory,
it will load source code from there as well.

Any of the Lobster builtin commands that load data files specify paths
relative to either the main or auxiliary directories (and either / or
 may be used as path separators). If you package up a Lobster program
for distribution, in addition to the executable, the bytecode file and
any data files referenced in this way, you'll need
`shaders/default.materials` which is implicitly loaded by `gl_window`.

## Output

Running lobster may result in a compiler error, that generally look
something like this:

    mygame.lobster(960): error: unknown identifier: highscor

If compiled correctly, running will give you output from your own print
statements, and additionally at some point may cause a runtime error,
which can look something like this:

    pythtree.lobster(15): VM error: division by zero
    in block -> pythtree.lobster(16)
       i = 0
    in block -> pythtree.lobster(16)
    in function: branch -> pythtree.lobster(29)
       poly = [[1.000000, 0.000000, 0.000000, 0.000000]:xyzw, [-1.000000, ...]:xyzw, ....]
       len = 4
       scale = 0.700000
       max = 11.000000
       n = 0
    in block -> pythtree.lobster(29)

This is called a stack trace. Besides the error and the line it happened
on, it will show all functions and blocks that called that code, in
reverse order, with any local variables and their values. This helps you
get an idea where the problem came from and helps in debugging.

