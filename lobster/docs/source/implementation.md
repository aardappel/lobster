---
title: The Lobster C++ Implementation
...

This document gives hints on how to work with the Lobster C++ code in terms of
building, extending, and reusing.

Lobster has been released under the Apache 2 open source license.

Building Lobster
----------------

Lobster uses recent C++11 features (auto, lambda, range-for), so will need
Visual Studio 2012 (the free desktop edition will do), Xcode 4.6, or a recent
GCC to be compiled.

Lobster uses OpenGL, SDL 2.0 and FreeType. Currently for Windows/OS X/iOS
SDL/Freetype precompiled libs are supplied with the project, so should compile
out of the box with no further external dependencies.

All source code and other files related to building Lobster for all platforms
sit in the `dev` folder, which is usually parallel to the main lobster folder.

Lobster must be build in 32bit mode on all platforms. For a high speed
interpreter, sizes of data are a bit more critical than most programs, and while
mobile platforms are still stuck in the 32bit era, this uniformity is helpful.

### Windows

This platform is definitely best supported and easiest to use for now. Open up
`dev\lobster\lobster.sln` with Visual Studio. The project is set up to build
lobster.exe in the main lobster folder, and will be ready for use as described
either from the [command line][1] or [notepad++][2].

[1]: <command_line_usage.html>

[2]: <notepadpp_ide.html>

### OS X & iOS

Building for either one is easy using the single Xcode project (in
`dev/xcode/lobster`) which produces a .app bundle for either platform (the
`lobsterapp` and `lobster_ios` targets), or a command line version for OS X (the
`lobster` target) placed in the main lobster folder much like Windows.

To develop Lobster code on OS X, easiest probably is to use the command line
version (use the -c option to tell it it's not running from an app bundle). Many
OS X editors support running a command line compiler, e.g. Komodo Edit with
Tools -\> Run Command.

Alternatively, you could add your lobster source (and extra data it might need)
to the Xcode project, and add it to the build rules such that these are copied
to the Resource location in the bundle, then running from Xcode with the main
lobster file as command line argument.

Distribution is currently a bit clumsier. You'll need to run lobster to produce
a bytecode file (see [command line][3]), then make a copy of the bundle, and
stick the bytecode file (+data) in the Resource location, and you should have
something that can be distributed to users. For iOS you can compile using the OS
X exe, then run that same bytecode using the iOS exe. Versioning of the Lobster
bytecode is currently very simplistic and tied to the day the exe was compiled,
so if iOS exe complains that it can't read the bytecode, make sure the OS X exe
you used to produce it was compiled on the same day. This will improve in the
future.

[3]: <command_line_usage.html>

### Linux

There is a makefile directly in the src folder, however unlike some of the other
platforms this isn't self-contained, you'll need 32bit (!) versions of SDL 2.0
and Freetype 2 installed.

### Android

I've made a preliminary port to Android which worked at that time, but hasn't
been updated in a while. Will get back to making this work more smoothly soon.

Extending Lobster
-----------------

Besides using Lobster as a stand-alone programming language as-is, there are 2
ways of extending Lobster, by adding your code to Lobster, or adding Lobster to
your project.

Note that unlike other scripting languages, Lobster has been designed as a
stand-alone language first, rather than a plug-in scripting system (more like
Python and Ruby, not like Lua and UnrealScript). You use Lobster code as your
"main program", with the "engine" being the library you call into. Most game
engines are the opposite: the engine code is the main program, and the scripting
language is being called into. For that reason, adding your own code to Lobster
is by the far the preferable way of building an application that uses Lobster,
and will generally be a much more productive environment.

The thinking here is that you use C++ purely to write performance critical code,
which can usually be contained in libraries. For the non-performance critical
code, which includes the general setup of your main program determining how
things fit together, you are much better off using a friendlier language, like
Lobster. It means that changing the structure of your project is much quicker,
and it is easier to experiment with new game ideas based on your C++ libraries.
Iterations in Lobster can be done more rapidly and more safely, often in less
code, than C++.

### Adding your code to Lobster

Depending on what you want to write, the current engine functionality of Lobster
may not be sufficient. Lobster adds C++ functions to the language in a modular
fashion, in the Visual Studio project you can see all things added to Lobster in
2 places:

-   The "builtins" folder, which should really be part of any Lobster
    implementation. `buitins.cpp` is the most important one that adds control
    structures, vector & math operations etc., without which the language would
    be hard to use. `file.cpp` adds file I/O related functions, and
    `lobsterreader.cpp` allows you to parse data structures in lobster syntax
    from a running program.

-   The engine folder. This contains all graphics/font/audio etc functionality.
    `graphics.cpp` / `font.cpp` build on top of the above mentioned 3 libraries,
    and can either be kept, or replaced entirely by functions that use a
    different rendering system. You can turn Lobster into a console-only
    language in one step by simply removing the engine folder from the project.

You can always run Lobster with the `-r` option to get an overview of all
functions currently added to the system (the current list is [here][4]). To
add/remove functionality is generally as easy as adding/removing the
corresponding `.cpp` file.

[4]: <builtin_functions_reference.html>

Lobster uses some macros to allow you to define a native function in one
location without declarations needed elsewhere. To learn how to write your own
.cpp of native functions, best to start with a simple example, such as
`audio.cpp`, then browse through more complex examples in `builtin.cpp` and
`graphics.cpp`.

Here's a simple example of a self-contained Lobster extension:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

void MyNativeOps()
{
    STARTDECL(add) (Value &x, Value &y)
    {
        return Value(x.ival + y.ival);
    }
    ENDDECL2(add, "x,y", "II", "I", "adds two integers.");

    // more such declarations here
}

AutoRegister __mno("name", MyNativeOps);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You'll need to become somewhat familiar with the Lobster internals to write
these functions succesfully, in particular with the `Value` type (see
`vmbase.h`), which is a union of all possible lobster types. If you specify
specific types (such as `I` for `int`, `F` for `float`, `S` for `string`, `V`
for `vector`, `C` for a `function` value, `R` for a `coroutine` and `A` for any
type, lowercase of any of them for an optional value that will be `nil` if not
specified) in the declaration, then the `Value` will already have been
typechecked and guaranteed to be that type, such that you can directly access
the component (e.g. `.ival`) of the union without checking the type.

As you can see, even the help text is included in the declaration, so everything
related to the function is in one location.

Important is dealing with reference counting, all of your
string/vector/coroutine arguments will have the proper reference count before
your function is called, and if you're not returning this value, you need to
decrement them when you're done with them (look for functions that use these
types as an example). If you fail to do this, the person writing Lobster code in
your dialect will get memory leaks he can't fix.

In designing your extension library, if you intend to add a lot of functions, it
is a good idea to choose a small prefix (similar to `gl_` for all the graphics
functionality) to all your functions. Lobster does not have a namespace facility
currently, so the burden on making sure there are no name clashes is on the
programmer integrating new libraries (you will get an assert if 2 names ever
clash).

`AutoRegister` in the example above simply adds the function that contains your
native function implementations to a list, so that the compiler can bind them.
This means that the above .cpp file doesn't need any extra mechanism to be added
to a Lobster implementation, simply link in the file and the functions will be
available.

### Adding Lobster to your project

This should still be fairly easy, as Lobster was made to be fairly modular, but
is a bit more work than above. I will strive to make this path easier in the
future.

First step would be to strip Lobster down from external dependencies. As
mentioned above, if you remove the graphics related `.cpp` builtin files
(`graphics.cpp`, `font.cpp` and others) from the lobster project (again, see
`PopulateNativeRegistry()` for a list of what it is currently using), you should
then also be able to remove all source code files that are prefixed with `gl`,
`sdl`, `ft`, and `stb`, and be left with a version of Lobster that has no
external dependencies.

You should be able to stick the remaining code somewhere in your project and
have it compile. I might make this easier in the future by making this part of
the code a separate library.

For now, look at lobster.cpp for an example of how to call scripts from your
code base (replace `main()` with something else). You'll want to add your own
functionality as native functions as described in the previous section.

Some of Lobster relies on it's own math library (`geom.h`), but it should be
very easy to make convenient functions to convert Lobster vectors into your own
math types (you could wrap around the `ToValue` and `ValueTo` functions).
