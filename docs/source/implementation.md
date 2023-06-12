---
title: The Lobster C++ Implementation
---

This document gives hints on how to work with the Lobster C++ code in terms of
building, extending, reusing, and compiling Lobster code to C++.

Lobster has been released under the Apache 2 open source license.

Building Lobster
----------------

Lobster uses recent C++17 features, so will need
Visual Studio 2022 (the free community edition will do), lastest Xcode, or a very
recent GCC / Clang to be compiled.

Lobster uses OpenGL, SDL 2.x and FreeType, these are included in the repo, so should compile
out of the box with no further external dependencies.

All source code and other files related to building Lobster for all platforms
sit in the `dev` folder.

Lobster should be built for 64-bit platforms where possible. It will still also
build on 32-bit Wasm & Android. This does not affect Lobster
data type sizes, e.g. `int` and `float` types are 64-bit on all platforms.

Lobster can be built with a JIT (on desktop platforms, convenient during
development, as it can run Lobster source "instantly"), or by compiling to C++
(possible on all platforms, required on non-desktop), see below.

### Windows

Open up `dev\lobster\lobster.sln` with Visual Studio, and ensure `Release` mode is selected.
The project is set up to build lobster.exe in the `bin` folder, and will be ready for use
as described either from the [command line](command_line_usage.html) or [VS Code](vscode_ide.html) /
[SublimeText](sublime_ide.html) / [Notepad++](notepadpp_ide.html).

### OS X (& iOS)

You need to first generate the Xcode project using CMake, run `sh build_osx.sh`
from the `dev` folder, to generate an Xcode project in the `xcode-cmake`
sub-folder.

Open `dev/xcode-cmake/Lobster.xcodeproj`, ensure `lobster > My Mac` is selected
as scheme in the top bar, `Product -> Build for -> Profiling` (to get a Release
build), then close if no errors.

Alternatively from the terminal from the `xcode-cmake` folder:

`xcodebuild -toolchain clang -configuration Release -target lobster`

This results in a `bin/lobster` you can use.

To develop Lobster code on OS X, use the command line
version. Many OS X editors support running a command line compiler, e.g.
VSCode, SublimeText, or Komodo Edit with Tools -\> Run Command.

How to turn a Lobster program into an App Bundle for distribution: TBD
Need to see how to set this up using CMake.
You'll likely want to produce
a pak file (see [command line](command_line_usage.html)) to make sticking
this extra data in a bundle easier.

For iOS be sure to read how to compile to C++ below, since iOS doesn't
support in Lobster's default JIT mode.
How to make the above CMake project work for iOS: TBD.

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
Also compile your Lobster code to C++ as described below.

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

The Wasm implementation does not support the JIT, so you should first
compile your `.lobster` code to C++, as described in the
section "Compiling Lobster code to C++" below.

To build, go to `dev/emscripten`, and type `make -j8`. This should produce a
lobster.[wasm\|js\|html\|data] in the same directory (the latter containing whatever
you placed in `assets`).

You can now run it with `emrun --browser chrome lobster.html --verbose` or
if that doesn't work, `emrun --no_browser lobster.html --verbose` and
manually navigate to `http://localhost:6931/lobster.html?--verbose` in your
browser. Note that just loading up the html in your browser directly may not
work because of security restrictions. Alternatively place all the generated
files on a webserver, and load from there.

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

Rather than directly executing with a JIT, Lobster can also be
translated to C++, for a further speed boost. This is useful when releasing a
shipping build to customers, but hopefully not necessary during development.
It is necessary for building for mobile/web platforms.

With the `--cpp` option on the command-line, the compiler will generate
`dev/compiled_lobster/src/compiled_lobster.cpp` (currently, you MUST compile
it from the root of the repo, e.g. `bin/lobster --cpp somepath/my.lobster`,
otherwise this will likely fail).
This file contains a `main()` and is otherwise
self-contained such that when you compile it with the build files for any
platform (see instructions above) substituting it for the standard `main.cpp`,
you’ll end up with an executable that runs only that specific program.

On Windows, there are project files in `dev/compiled_lobster` that will
automatically pick up the compiled lobster code.

On Linux, building in `dev` like above, then instead
`cmake -DLOBSTER_TOCPP=ON -DCMAKE_BUILD_TYPE=Release .` will automatically
substitute the compiled lobster main program. Build with `make -j8` or similar.

For Emscripten, there's a `cpp` make target (which is the default) that works
similar to the WebAssembly mode described below.

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

Here's a simple example of a Lobster extension:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "lobster/stdafx.h"

#include "lobster/natreg.h"

using namespace lobster;

void MyNativeOps(NativeRegistry &nfr) {

    nfr("add", "x,y", "II", "I",
        "Adds two integers.",
        [](StackPtr &, VM &, Value &x, Value &y) {
            return Value(x.ival() + y.ival());
        });

    // more such declarations here
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You'll need to become somewhat familiar with the Lobster internals to write
these functions succesfully, in particular with the `Value` type (see
`vmdata.h`), which is a union of all possible lobster types. If you specify
specific types (such as `I` for `int`, `F` for `float`, `S` for `string`, `V`
for `vector` etc (more details in `natreg.h`),
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

Now from your main program, you'll need to call
`RegisterBuiltin(nfr, "name", MyNativeOps)` or similar before you invoke the
Lobster compiler, such that these new functions are available during all
compiler passes.

You can add the above file to whatever build system you're using, but that
may involve modifying the Lobster project files which you may not want to do.
Instead, if your extension is small enough to sit in a header file, you can
make use of the automatic inclusion of `projects/include/lobster_engine_plugins.h`
(see comments in `engine.cpp`) which will then automatically be picked up as
part of any Lobster build.

### Adding Lobster to your project

This should still be fairly easy, as Lobster was made to be fairly modular, but
is a bit more work than above. I will strive to make this path easier in the
future.

With the CMake project, this is easy, as all `LOBSTER_ENGINE=OFF` will get you
a Lobster build without the built-in engine, the result you can add to your
own projects. You'll likely want to replace `main.cpp` with something that
runs Lobster from your own code.

Similarly, in the visual studio project, there is the `language` project,
which is what you'd want to include in your own projects, and the `engine`
project which you are replacing. Finally there's again `main.cpp` to adapt.

Some of Lobster relies on it's own math library (`geom.h`), but it should be
very easy to make convenient functions to convert Lobster vectors into your own
math types (you could wrap around the `ToValue` and `ValueTo` functions).
