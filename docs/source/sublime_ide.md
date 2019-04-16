---
title: SublimeText as Lobster IDE
---

## Running Lobster programs from within the editor

-   This assumes SublimeText 3, though may also work with older
    versions.

-   Copy the files in the `docs/sublime` folder to your
    `Packages/User` folder (on Windows this is
    `\Users\YourUserName\AppData\Roaming\Sublime Text 3\Packages\User`).

-   Open up `lobster.sublime-build` and make sure the path to your
    lobster executable is correct. Also note that it refers to a custom
    version of `exec.py` called `lobster_exec.py`. The only difference
    is that the default script surpresses windows from popping up, which
    disables any graphical output from Lobster.

-   Load some lobster code. pressing `ctrl+b` should run it.

## Editing

By default, SublimeText comes set up insert tabs into your code, which
is against the Lobster [style guide][], and can result in errors if you
modify the existing examples. Add `"translate_tabs_to_spaces": true` to
your user settings.

## Error Highlighting

The build script parses Lobster error messages, so just pressing F4 will
get you to the line of the error.

## Syntax Highlighting

The `lobster.tmLanguage` file you copied should provide automatic syntax
highlighting for any .lobster files.

##

  [style guide]: style_guide.html
