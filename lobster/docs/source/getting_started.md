---
title: Quick Getting Started Guide.
---

Linux
-----

Pre-Requirements: Git, CMake, a C++17 compiler, Mesa dev files
(`apt-get install mesa-common-dev`).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone https://github.com/aardappel/lobster.git
cd lobster/dev
cmake -DCMAKE_BUILD_TYPE=Release
make -j8
cd ../lobster
./lobster samples/pythtree.lobster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For more advanced usage check the rest of [the docs](README_FIRST.html)


Windows
-------

### Building it yourself (best)

Pre-Requirements: VS2019 Community edition or better, C++ desktop tools installed.

Get https://github.com/aardappel/lobster using your favorite git tool (best), or just download
a `.zip` from there otherwise.

Open `dev\lobster\lobster.sln`, ensure `Release` is selected in the top bar,
`Build -> Build Solution`, then close if no errors.

### Alternatively: getting latest pre-built

If you don't want to deal with building it yourself, you may be able to use
a pre-built executable, though these can be out of date.

On https://github.com/aardappel/lobster/releases, for the last release that has a
`lobster_<date>_windows.zip` also get `Source code (zip)` (this has all the data files
and examples). Unpack both, and place the exe you find in the first inside the top
`lobster` dir you find in the second.

### Running it

You now have a `lobster\lobster.exe`. You can run this with any Lobster file
as argument to run it, for example `lobster\lobster.exe lobster\samples\pythtree.lobster`
should work.

For more advanced usage, including how to run it from an editor,
check the rest of [the docs](README_FIRST.html)


OS X
----

### Building it yourself (best)

Pre-Requirements: Latest XCode.

Get https://github.com/aardappel/lobster using your favorite git tool (best), or just download
a `.zip` otherwise.

Open `dev/xcode/lobster/lobster.xcodeproj`, ensure `lobster_release > My Mac` is selected
as scheme in the top bar, `Product -> Build`, then close if no errors.

### Alternatively: getting latest pre-built

If you don't want to deal with building it yourself, you may be able to use
a pre-built executable, though these can be out of date.

On https://github.com/aardappel/lobster/releases, for the last release that has a
`lobster_<date>_osx.zip` also get `Source code (zip)` (this has all the data files
and examples). Unpack both, and place the executable you find in the first inside the top
`lobster` dir you find in the second.

### Running it

You now have a `lobster/lobster` executable. You can run this with any Lobster file
as argument to run it, for example `lobster/lobster lobster/samples/pythtree.lobster`

For more advanced usage, including how to run it from an editor,
check the rest of [the docs](README_FIRST.html)
