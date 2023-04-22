---
title: Using Lobster from the command line
---

Basic usage
-----------

If you have a file `helloworld.lobster` that contains

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Hello, World!"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

then running it like so will compile and run it:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
bin/lobster helloworld.lobster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Command line options
--------------------

Format: `lobster [ OPTIONS ] [ FILE ] [ -- ARGS ]`

-   `FILE` : main Lobster file to compile & run.

-   `-- ARGS` pass args to the running Lobster program, available from
    `command_line_arguments()`. Must be last on the command-line.

-   `--pak` : generates a pakfile (currently always called "`default.lpak`") in the
    same folder as the `.lobster` file it reads, and doesn't run the program
    afterwards. If you run lobster with no arguments at all, it will try to load
    "`default.lpak`" from the same folder it resides in. Thus distributing
    programs created in lobster is as simple as packaging up the lobster
    executable with a pakfile. The pakfile contains the bytecode, and any data
    files you have specified with the `pakfile` keyword, see “Distributing
    Lobster programs” in the [implementation](implementation.html) documentation.

-   `--cpp` : compiles to a .cpp file. Please make sure to follow
    [implementation](implementation.html) on how to use this feature.
    Useful if you’ve created something in Lobster that could use a bit more speed,
    for a shipping build. Not recommend to be used during development.

-   `--import RELDIR` specifies a dir relative to FILE from which `import`
    (and `pakfile`) statements can be resolved, or any loading the running
    program does. Does the same as adding `import from "RELDIR"` to the top
    of your program.

-   `--runtime-shipping` : Compile with asserts off.
-   `--runtime-asserts` : Compile with asserts on (default).
-   `--runtime-verbose` : Compile with asserts on + line info + function tracking
    for better runtime errors (stack traces) and profiling. Small effect on speed.
    `--runtime-debug` : Same as verbose, but now tones inlining way down to get
    better stack traces. Larger effect on speed. Automatically breaks into the
    graphical debugger on assertion failures.

-   `--main MAIN` : after compiling FILE, if present, compile this file and
    run it. This is useful when running Lobster from an editor, where you
    may be editing a module that is not the main file, would like to see that it
    compiles by itself, but to run it need to be launching the main Lobster
    program it belongs to (e.g. `main.lobster` in the same folder).

-   `--wait` : makes the compiler wait for commandline input before it exits. Useful
    in Windows batch files.
-   `--noconsole` : Close console window (Windows)

-   `--verbose` : verbose mode, outputs additional stats about the program being
    compiled/run. `--debug` outputs even more, only useful for working on the compiler.
-   `--silent` : Only output errors.

-   `--gen-builtins-html` : dumps a help file of all builtin functions the
    compiler knows about to `builtin_functions_reference.html`.
    `--gen-builtins-names` dumps a plain text list of functions, useful for
    adding to syntax highlighting files etc.

-   `--parsedump` : dumps internal representations of the program as AST, and
    `--disasm` for a readable bytecode dump. Only useful for compiler
    development or if you are really curious.

-   `--non-interactive-test` : Quit after running 1 frame. Useful for running graphical
    programs as part of a test suite.

-   `--query`: Ask the compiler to answer a query about definitions in the program being
    compiled. When is this mode, the compiler does not try to do a full compilation,
    but simply tries to answer the query, including ignoring errors or aborting half-way
    if it has to.
    This option is for use by an IDE or LSP server implementation.
    All further args are passed to the query.
    The first arg is the kind of query (e.g. `definition` or `complete`).
    The second arg is the file this query should take place in (as a relative path, i.e
    `foo/bar.lobster`).
    The 3rd arg is the line where the query should happen.
    The 4th arg is the identifier the query is about.
    The rest of the args depend on the query type, if any.
    For `definition` a response will be the location of the definition.
    For `complete`, it will return a list of possible fields/methods.

Lobster source code may start with a shebang `#!` so you can embed the command-line in
a Lobster script.

Default directories
-------------------

It's useful to understand the directories lobster uses, both for reading source
code files and any data files the program may use:

-   the root repo directory: This is the main folder that has the default
    `bin modules data docs samples tests` folders inside of it.

-   the auxiliary directory: this is where the main `.lobster` file being
    compiled resides.

-   the directory for writing files: the same as auxiliary on desktop platforms,
    but often a special directory on mobile platforms.

-   On Linux additionally it can load files from `/usr/share/lobster/` (or
    whatever path was configured for install by CMake, see `DATADIR`) if
    the above paths don't work. This is to allow package managers to install
    Lobster in the system directories.

Additionally, if any of these folders contains a `modules` directory, it will
load source code from there as well.

Any of the Lobster builtin commands that load data files specify paths relative
to either the main or auxiliary directories (and either / or  may be used as
path separators). If you package up a Lobster program for distribution, all
these files can be packed into a pakfile, see `--pak`.

Output
------

Running lobster may result in a compiler error, that generally look something
like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mygame.lobster(960): error: unknown identifier: highscor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If compiled correctly, running will give you output from your own print
statements, and additionally at some point may cause a runtime error, which can
look something like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is called a stack trace. Besides the error and the line it happened on, it
will show all functions and blocks that called that code, in reverse order, with
any local variables and their values. This helps you get an idea where the
problem came from and helps in debugging.
