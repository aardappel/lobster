---
title: The Lobster C++ Implementation
---

This document gives hints on how to work with the Lobster C++ code in terms of
building, extending, reusing, and compiling Lobster code to C++ or WASM.

Lobster has been released under the Apache 2 open source license.

Building Lobster
----------------

Lobster uses recent C++17 features, so will need
Visual Studio 2019 (the free community edition will do), Xcode 10.x, or a recent
GCC (7+) / Clang (6+) to be compiled.

Lobster uses OpenGL, SDL 2.x and FreeType, these are included in the repo, so should compile
out of the box with no further external dependencies.

All source code and other files related to building Lobster for all platforms
sit in the `dev` folder.

Lobster can be built for 32 or 64-bit on all platforms. On 64-bit, all
Lobster values will take double the size (and precision), and thus use more
memory. Despite that, for most programs, 64-bit is still a speed-up (the VM
benefits a lot from extra registers).

### Windows

Open up `dev\lobster\lobster.sln` with Visual Studio, and ensure `Release` mode is selected.
The project is set up to build lobster.exe in the `bin` folder, and will be ready for use
as described either from the [command line](command_line_usage.html) or [VS Code](vscode_ide.html) /
[SublimeText](sublime_ide.html) / [Notepad++](notepadpp_ide.html).

### OS X & iOS

Building for either one is easy using the single Xcode project (in
`dev/xcode/lobster`) which produces a .app bundle for either platform (the
`lobsterapp` and `lobster_ios` targets), or a command line version for OS X (the
`lobster` target) placed in the `bin` folder much like Windows.

To develop Lobster code on OS X, easiest probably is to use the command line
version. Many OS X editors support running a command line compiler, e.g.
VSCode, SublimeText, or Komodo Edit with Tools -\> Run Command.

Alternatively, you could add your lobster source (and extra data it might need)
to the Xcode project, and add it to the build rules such that these are copied
to the Resource location in the bundle, then running from Xcode with the main
lobster file as command line argument.

Distribution is currently a bit clumsier. You'll need to run lobster to produce
a pak file (see [command line](command_line_usage.html)), then make a copy
of the bundle, and stick the bytecode file (+data) in the Resource location, and
you should have something that can be distributed to users. For iOS you can
compile using the OS X exe, then run that same pak file using the iOS exe.

### Linux

You can build with CMake on Linux:

This requires a C++17 compiler, and the mesa dev files
should be installed (`apt-get install mesa-common-dev`).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cd dev
cmake -DCMAKE_BUILD_TYPE=Release && make -j8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It creates `bin/lobster`. Run it to access the
samples, e.g. `bin/lobster samples/pythtree.lobster`

Note the `LOBSTER_ENGINE` CMake option, which is by default on. You can turn this
off to get a command-line only version of Lobster that does not depend on OpenGL,
SDL, FreeType etc.

### Android

Make sure you have the latest [Android Studio](https://developer.android.com/studio)
installed, and follow instructions to add the
[NDK](https://developer.android.com/studio/projects/install-ndk.md)

In Android Studio, use "Open" to open the `dev/android-project` dir.
It may complain about not knowing where the NDK lives, either let it fix this
automatically, or manually modify the path in `local.properties`

Using the desktop lobster exe, build your desired Lobster program using the `--pak`
option, such that all assets end up in a single file (see below for more information).
Place the result in `dev/android-project/app/src/main/assets/default.lpak` so
it will automatically be picked up by the build process and added to the APK.

You should now be able to just press run. Wait for the build, and see it launch
on an attached Android device. Note that Lobster requires a device that supports
GLES 3.0 and Android version 4.3 (Jellybean). Emulators often do not support ES3,
you'll see a shader compile error in logcat if this happens.

If there are errors running, check logcat.

Things to change if you want to release your app in the Google Play store (these
instructions may be out of date):

-   Generate your own signing key with `keytool`. Make sure to also delete any
    old debug copy of the app from your device, or you’ll get a signature mismatch error.

-   Change the name to something else in `app/src/main/res/values/strings.xml`.

-   Change the .png files in `app/src/main/res/` with your own application icon.

-   You may want to change the package name from `org.libsdl.app` to your
    own. This needs to be done in 3 places, `app/build.gradle`, and
    `app/src/main/java/org/libsdl/app/SDLActivity.java` (both at the top of that file
    and the directory path itself!). Alternatively create a new class and inherit from
    `SDLActivity`.

### WebAssembly / Emscripten

You need the [emscripten
toolchain](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html)
installed, as well as GNU make (on windows that means installing
[this](http://gnuwin32.sourceforge.net/packages/make.htm)).

Before you build, gather your lobster distribution files (see below) and place
them in `dev/emscripten/assets`. They will be automatically picked up by the
build process this way.

To build, go to `dev/emscripten`, and type `make -j8`. This should produce a
lobster.[wasm\|js\|html\|data] in the same directory (the latter containing whatever
you placed in `assets`).

You can now run it with `emrun --browser chrome lobster.html --verbose` or
similar. Note that just loading up the html in your browser directly may not
work because of security restrictions. Alternatively place all the generated
files on a webserver, and load from there.

Note: the above compiles the bytecode *interpreter* to wasm. To instead
compile the lobster bytecode to wasm directly, see
"Compiling Lobster code to WebAssembly" below.

Distributing Lobster programs.
------------------------------

While the above instructions will build you the lobster executable, to
distribute a Lobster program to others, you will need to distribution files.
These must be (including correct paths):

-   `default.lpak`. This is the Lobster pakfile file you obtain from compiling
    your program with the `--pak` option, it includes:

    -   The bytecode.

    -   `data/shaders/default.materials` (these are the minimum shader definitions
        needed for to render anything, and is implicitly loaded by `gl_window`).

    -   Any other files/directories you have specified with `pakfile`, e.g:
        `gl_load_texture(pakfile ”mypath/myfile.png”)`. `pakfile` can prefix
        filenames or directories (ending in `/`), in which case all files in the
        directory will (non-recursively) be added. When running with `--verbose`
        you can see what files are added/loaded from a pakfile, and which are
        loaded individually.

-   Any files your code references that are not in the pakfile (e.g.
    `gl_load_texture(”mypath/myfile.png”)` ).

-   On Windows, you’ll need to include `bin\openvr_api.dll` and/or `bin\steam_api.dll`
    next to `lobster.exe` ONLY if you use functions starting with `vr_` or
    `steam_` respectively.

Where you place these files depends on the platform, on Windows / Linux it is
next to the lobster executable, on OS X / iOS it is the application bundle under
Contents, on Android it’s under assets in the .apk, and with emscripten there’s
an assets directory also.

Compiling Lobster code to C++
-----------------------------

Rather than directly executing or compiling to bytecode, Lobster can also be
translated to C++, for a further speed boost. This is useful when releasing a
shipping build to customers, but hopefully not necessary during development.

With the `--cpp` option on the command-line, the compiler will generate a
`compiled_lobster.cpp` file. This file contains a `main()` and is otherwise
self-contained such that when you compile it with the build files for any
platform (see instructions above) substituting it for the standard `main.cpp`,
you’ll end up with an executable that runs only that specific program.

Currently, this process is a bit clunky, you must run the compiler with
the repo root as you current directory, and the compiled file will always be
written to `dev/compiled_lobster/`

On Windows, there are project files in that same directory that will
automatically pick up the compiled lobster code.

On Linux, create a `build` directory anywhere, for example in
`dev/compiled_lobster/build`, go there, then
`cmake -DLOBSTER_TOCPP=ON -DCMAKE_BUILD_TYPE=Release ../..` will automatically
substitute the compiled lobster main program. Build with `make -j8` or similar.


Compiling Lobster code to WebAssembly
-------------------------------------
Note: this is about *generating wasm code from lobster bytecode* directly.
If you just want to build the bytecode interpreter for wasm, see the
"WebAssembly / Emscripten" section above.

Similarly to compiling to C++, with `--wasm` the compiler with generate
`dev/emscripten/compiled_lobster_wasm.o` file, from e.g.
`bin/lobster --wasm samples/pythtree.lobster`

Though this file has the same format as a `.wasm` file (and can be inspected
by the tools in e.g. WABT), this is not a WebAssembly module ready to be run
however, it is a file that still needs to be linked against
the rest of the Lobster runtime, much like in the C++ case. The build files in
`dev/emscripten` can pick up this file.

To compile the project and link in the `.o` we just generated, we build
similarly to described in the `WebAssembly / Emscripten` section above,
except we use a special make target to indicate it should link in our
compiled code as the main program instead of the default:
`make -j8 lobster_wasmout`.

Don't forget to place needed files in the `assets` dir as described above.
You can even place an `lpak` file there (which currently will contain bytecode,
which will be unused).

You now run this much like described above.

If you're interested in the details of how Lobster is translated to wasm,
or how you can generate wasm yourself, read more [here](implementation_wasm.html).

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
    implementation. `buitins.cpp` is the most important one that adds
    vector & math operations etc., without which the language would
    be hard to use. `file.cpp` adds file I/O related functions, and
    `lobsterreader.cpp` allows you to parse data structures in lobster syntax
    from a running program.

-   The engine folder. This contains all graphics/font/audio etc functionality.
    `graphics.cpp` / `font.cpp` build on top of the above mentioned 3 libraries,
    and can either be kept, or replaced entirely by functions that use a
    different rendering system. You can turn Lobster into a console-only
    language in one step by simply removing the engine folder from the project.

You can always run Lobster with the `-r` option to get an overview of all
functions currently added to the system (the current list is
[here](builtin_functions_reference.html)). To add/remove functionality is
generally as easy as adding/removing the corresponding `.cpp` file.

Lobster uses some macros to allow you to define a native function in one
location without declarations needed elsewhere. To learn how to write your own
.cpp of native functions, best to start with a simple example, such as
`audio.cpp`, then browse through more complex examples in `builtin.cpp` and
`graphics.cpp`.

Here's a simple example of a self-contained Lobster extension:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "stdafx.h"

#include "natreg.h"

void MyNativeOps(NativeRegistry &nfr) {

    nfr("add", "x,y", "II", "I",
        "Adds two integers.",
        [](VM &, Value &x, Value &y) {
            return x.ival() + y.ival();
        });

    // more such declarations here
}

AutoRegister __mno("name", MyNativeOps);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You'll need to become somewhat familiar with the Lobster internals to write
these functions succesfully, in particular with the `Value` type (see
`vmbase.h`), which is a union of all possible lobster types. If you specify
specific types (such as `I` for `int`, `F` for `float`, `S` for `string`, `V`
for `vector`, `L` for a function value etc (more details in `natreg.h`),
then the `Value` will already have been typechecked and
guaranteed to be that type, such that you can directly access the component
(e.g. `.ival()`) without checking the type (you'll get an assert if you get this
wrong).

As you can see, even the help text is included in the declaration, so everything
related to the function is in one location.

Important is dealing with memory management: by default, you *borrow* all
arguments, meaning you're not supposed to keep a pointer to them after the
function ends. If you return a value that was allocated (see e.g. `VM::NewString`)
then the caller will own this value. This is typically what you want.
If for whatever reason you want to hold on to a value, have a look at functions
that do so, like `push`.

In designing your extension library, if you intend to add a lot of functions, it
is a good idea to choose a namespace (similar to `gl_` for all the graphics
functionality) to all your functions. Lobster uses `_` for namespacing also in
the language. The burden on making sure there are no name clashes is on the
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

For now, look at `main.cpp` for an example of how to call scripts from your code
base (replace `main()` with something else). You'll want to add your own
functionality as native functions as described in the previous section.

Some of Lobster relies on it's own math library (`geom.h`), but it should be
very easy to make convenient functions to convert Lobster vectors into your own
math types (you could wrap around the `ToValue` and `ValueTo` functions).
