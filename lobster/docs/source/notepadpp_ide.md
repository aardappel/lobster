% Notepad++ as Lobster IDE

Running Lobster programs from within the editor
-----------------------------------------------

-   Install [Notepad++][2] (default install) and the NppExec plugin
    ([download][1], unzip the .dll and other files into the plugins folder of
    where you installed Notepad++).

[1]: <http://sourceforge.net/projects/npp-plugins/files/NppExec/>

[2]: <http://notepad-plus-plus.org/>

-   Open menu Plugins -\> NppExec -\> Execute and enter:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    NPP_SAVE
    cmd /C C:\path_to_lobster_goes_here\lobster.exe "$(FULL_CURRENT_PATH)"
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   Save script under name "`lobster`"

-   Load some lobster code. pressing `F6` and then "ok" should run it. Next time
    press `Ctrl+F6` to bypass the dialog.



Editing
-------

By default, Notepad++ comes set up insert tabs into your code, which is against
the Lobster [style guide][3], and can result in errors if you modify the
existing examples. In Settings -\> Preferences -\> Language Menu / Tab Settings
you can change this.

[3]: <style_guide.html>



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

-   Language -\> Define your language.., then Import... and choose
    `notepadpp_udl_lobster.xml` file in docs directory.

-   Save As... "`lobster`"

-   It should now show `.lobster` files with correct highlighting, or you can
    set it explicitly from the Language menu

-   The colors were chosen to work well with "Zenburn" (Settings -\> Style
    Configurator), you may need to change them if you use a bright background
    instead.



Optionally
----------

-   in Plugins -\> NppExec -\> Advanced Options, you can add the lobster script
    as a menu item, or toolbar button, or special hotkey.
