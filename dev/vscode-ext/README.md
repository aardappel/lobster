# Lobster VSCode Extension
This extension makes use of the LSP of lobster together with syntax highlighting.

> Warning this extension is still very WIP

## How to install?
In the repo under docs/vscode will be a vsix file wich you can install the extension from.

You can use the command `code --install-extension lobster.vsix`

## Contributing
Using the included .vscode/launch.json you can run a test editor for testing this extension.

To make a final packaged vsix file you need to have `vsce` installed.
To install it `npm install -g @vscode/vsce`

In this dir, run `npm install webpack webpack-cli --save-dev`

In the `lsp` dir, run `npm install`.

Then in this dir run `vsce package` and it will generate the packaged vsix file.

To contribute it back to the repo, overwrite the version in `docs/vscode`.
