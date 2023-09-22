---
title: VSCode as Lobster IDE
---

VSCode extension
-------------------
The vscode extension includes syntax highlighting, 
a lobster LSP (Language Server Protocol).

See [here](lsp.html) for more info about the lobster LSP.

### How to install
-  Get the file `lobster.vsix` found in the repo at `docs/vscode/lobster.vsix`.

-  Open your vscode command pallete and search for `Install from VSIX`. Then select the file mentioned above.


Running Lobster programs from within the editor without the extension
-----------------------------------------------

-   Copy `docs/vscode/tasks.json` to a `.vscode` workspace folder that
    is in the root of where you keep your lobster code.

-   Load some lobster code. pressing `ctrl+shift+b` should run it. If there was
    an error, press `F8` to go to it, and/or `` ctrl+` `` (shell) to show
    program output.