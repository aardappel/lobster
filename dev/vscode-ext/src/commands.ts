import * as vscode from 'vscode';

export function registerCommands(context: vscode.ExtensionContext) {
    context.subscriptions.push(
		vscode.commands.registerCommand('extension.lobster.runEditorContents', (resource: vscode.Uri) => {
			let targetResource = resource;
			if (!targetResource && vscode.window.activeTextEditor) {
				targetResource = vscode.window.activeTextEditor.document.uri;
			}
			if (targetResource) {
				vscode.debug.startDebugging(undefined, {
					type: 'lobster',
					name: 'Run File',
					request: 'launch',
					program: targetResource.fsPath
				},
					{ noDebug: true }
				);
			}
		}),
        vscode.commands.registerCommand('extension.lobster.debugEditorContents', (resource: vscode.Uri) => {
			let targetResource = resource;
			if (!targetResource && vscode.window.activeTextEditor) {
				targetResource = vscode.window.activeTextEditor.document.uri;
			}
			if (targetResource) {
				vscode.debug.startDebugging(undefined, {
					type: 'lobster',
					name: 'Run File',
					request: 'launch',
					program: targetResource.fsPath,
                    stopOnEntry: true
				});
			}
		}),
        vscode.commands.registerCommand('extension.lobster.getProgramName', config => {
            return vscode.window.showInputBox({
                placeHolder: "Please enter the name of a markdown file in the workspace folder",
                value: "someFile.lobster"
            });
        })
    );
}