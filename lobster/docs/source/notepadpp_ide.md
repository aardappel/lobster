---
title: Notepad++ as Lobster IDE
---

Running Lobster programs from within the editor
-----------------------------------------------

-   Install [Notepad++](<http://notepad-plus-plus.org/>) (default install) and
    the NppExec plugin
    ([download](<http://sourceforge.net/projects/npp-plugins/files/NppExec/>),
    unzip the .dll and other files into the plugins folder of where you
    installed Notepad++).

-   Open menu Plugins -\> NppExec -\> Execute and enter:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    NPP_SAVE
    cmd /C C:\path_to_lobster_goes_here\lobster.exe "$(FULL_CURRENT_PATH)"
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   Save script under name "`lobster`"

-   Load some lobster code. pressing `F6` and then "ok" should run it. Next time
    press `Ctrl+F6` to bypass the dialog.

-   NOTE: running from within Notepad++ doesn't work for programs that do
    console input thru e.g. `get_line()`. Output (thru `print()`) does work, but
    is sometimes delayed. Graphical programs work well.

Editing
-------

By default, Notepad++ comes set up insert tabs into your code, which is against
the Lobster [style guide](<style_guide.html>), and can result in errors if you
modify the existing examples. In Settings -\> Preferences -\> Language Menu /
Tab Settings you can change this.

Error Highlighting
------------------

-   in Plugins -\> NppExec -\> Console Output Filters, add these on individual
    lines:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    %FILE%(%LINE%): error:*
    %FILE%(%LINE%): VM error:*
    in block -> %FILE%(%LINE%)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   make sure to checkmark them to on, and check bold, or change the color to
    red (`FF 00 00`) or something.

-   This will highlight errors you get from Lobster, and allow you to double
    click on them to go directly to the line of the error. The first one is for
    compile errors, the second for runtime errors, and the third for the
    stacktrace part of runtime errors.

Syntax Highlighting
-------------------

-   NOTE: this file currently only seems to work with a dark background theme,
    so be sure to choose in menu Settings -\> Style Configurator under "Select
    Theme" something like Zenburn first.

-   Language -\> Define your language.., then Import... and choose the
    `docs/notepad++/notepadpp_udl_lobster.xml` file.

-   Save As... "`lobster`"

-   It should now show `.lobster` files with correct highlighting, or you can
    set it explicitly from the Language menu.

Optionally
----------

-   in Plugins -\> NppExec -\> Advanced Options, you can add the lobster script
    as a menu item, or toolbar button, or special hotkey.
