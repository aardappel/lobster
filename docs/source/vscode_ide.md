---
title: VSCode as Lobster IDE
---

VSCode extension
-------------------
The vscode extension includes syntax highlithing, 
a lobster lsp and breakpoints.

See [here](command_line_usage.html) what the lsp is capable of.

### How to install
-  Download the file `lobster.vsix` found in the github at `docs/vscode/lobster.vsix`.

-  Open your vscode command pallete and search for `Install from VSIX`. Then select the file you just downloaded.


Running Lobster programs from within the editor without the extension
-----------------------------------------------

-   Copy `docs/vscode/tasks.json` to a `.vscode` workspace folder that
    is in the root of where you keep your lobster code.

-   Load some lobster code. pressing `ctrl+shift+b` should run it. If there was
    an error, press `F8` to go to it, and/or `` ctrl+` `` (shell) to show
    program output.