VSCode as Lobster IDE

Running Lobster programs from within the editor
-----------------------------------------------

-   Copy `lobster/docs/vscode/tasks.json` to a `.vscode` workspace folder that
    is in the root of where you keep your lobster code.

-   Load some lobster code. pressing `ctrl+shift+b` should run it. Press
    `ctrl+shift+m` (or click on the error icon in the status bar) to get problem
    pane, which allows you to click on the error to go to it.

    -   Output is set to `silent` in `tasks.json` because otherwise the output
        window obscures the problem pane, and errors in the output are not
        clickable in VSCode. If you want to see any `print` output however,
        change this setting to `always`.

Syntax Highlighting
-------------------

Copy the folder `lobster/docs/vscode/lobster` to `.vscode/extensions` *within
your user folder*, not your workspace folder. This will enable syntax
highlighting next time VSCode is run.

 
