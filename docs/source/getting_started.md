---
title: Quick Getting Started Guide.
---


Get pre-built binaries (Windows, Mac, Linux)
-------------------------------------------

### Latest version from CI

If you don't want to deal with building it yourself, go to
[github CI](https://github.com/aardappel/lobster/actions?query=workflow%3ACI)
(click on the latest succesful run to get a list of binaries at the bottom).
These are 64-bit for all platforms.

Note: if you're NOT seeing a list of artifacts at the bottom of each run, that
may be because you're not logged in to github.

Now get https://github.com/aardappel/lobster (download zip).
This has all the data files and examples. Place the binary
you obtained in `bin` folder of the repo (not `/bin`).

### Releases

There may occasionally be builds on https://github.com/aardappel/lobster/releases
(`lobster_<date>_<platform>.zip` and `Source code (zip)`), but these are
usually way behind using the most recent builds above, so not recommended.


Or, build it yourself!
---------------------

### Linux

Pre-Requirements: Git, CMake, a C++17 compiler, Mesa dev files
(`apt-get install mesa-common-dev`).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone https://github.com/aardappel/lobster.git
cd lobster/dev
cmake -DCMAKE_BUILD_TYPE=Release
make -j8
cd ..
bin/lobster samples/pythtree.lobster
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Windows

Pre-Requirements: VS2022 Community edition or better, C++ desktop tools installed.

Get https://github.com/aardappel/lobster using your favorite git tool (best),
or just download a `.zip` from there otherwise.

Open `dev\lobster\lobster.sln`, ensure `Release` is selected in the top bar,
`Build -> Build Solution`, then close if no errors.

### Mac

Pre-Requirements: Latest XCode & CMake.

Get https://github.com/aardappel/lobster using your favorite git tool (best),
or just download a `.zip` otherwise.

From the terminal, in the `dev` folder, run `sh build_osx.sh` which will
generate an Xcode project in the `xcode-cmake` sub-folder.

Open `dev/xcode-cmake/Lobster.xcodeproj`, ensure `lobster > My Mac` is selected
as scheme in the top bar, `Product -> Build for -> Profiling` (to get a Release
build), then close if no errors.

Alternatively from the terminal from the `xcode-cmake` folder:

`xcodebuild -toolchain clang -configuration Release -target lobster`

Running it
----------

You now have a `lobster` executable in your `bin` folder.
You can run this with any Lobster file as argument to run it, for example
`bin/lobster.exe samples/pythtree.lobster` should work.

More Docs
---------

For more advanced usage, including how to run it from an editor,
check the rest of [the docs](README_FIRST.html)
