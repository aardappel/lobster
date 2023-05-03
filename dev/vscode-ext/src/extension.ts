import * as path from 'path';
import { workspace, ExtensionContext, debug } from 'vscode';

import {
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';
import { registerTasks } from './task';
import { InlineLobsterDebugAdapterFactory } from './dap';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
	// The server is implemented in node
	const lspModule = context.asAbsolutePath(
		path.join('..', 'lsp', 'webpack-out', 'lobster_lsp.js') //TODO when debugging use this path
		// path.join('webpack-out', 'lobster_lsp.js')
	);

	const dapModule = context.asAbsolutePath(
		path.join('..', 'lsp', 'webpack-out', 'lobster_dap.js') //TODO when debugging use this path
		// path.join('webpack-out', 'lobster_dap.js')
	);

	// If the extension is launched in debug mode then the debug server options are used
	// Otherwise the run options are used
	const serverOptions: ServerOptions = {
		run: { module: lspModule, transport: TransportKind.ipc },
		debug: { module: lspModule, transport: TransportKind.ipc }
	};

	// Options to control the language clients
	const clientOptions: LanguageClientOptions = {
		// Register the server for lobster language file
		documentSelector: [{ scheme: 'file', language: 'lobster' }],
		synchronize: {
			// Notify the server about file changes to '.clientrc files contained in the workspace
			fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
		}
	};

	// Create the language client and start the client.
	client = new LanguageClient(
		'lobsterLSP',
		'Lobster Language Server',
		serverOptions,
		clientOptions
	);

	// Start the client. This will also launch the server
	client.start();

	context.subscriptions.push(debug.registerDebugAdapterDescriptorFactory(
		'lobster', 
		new InlineLobsterDebugAdapterFactory(dapModule)
	));

	registerTasks(context);
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
