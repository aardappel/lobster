import * as path from 'path';
import { workspace, ExtensionContext, debug, DebugConfiguration, ProviderResult, WorkspaceFolder, DebugConfigurationProviderTriggerKind, window } from 'vscode';

import {
	CancellationToken,
	LanguageClient,
	LanguageClientOptions,
	ServerOptions,
	TransportKind
} from 'vscode-languageclient/node';
import { registerTasks } from './task';
import { InlineLobsterDebugAdapterFactory } from './dap';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
	// If the extension is launched in debug mode then the debug server options are used
	// Otherwise the run options are used
	const serverOptions: ServerOptions = {
		run: { 
			module: context.asAbsolutePath(path.join('webpack-out', 'lobster_lsp.js')), 
			transport: TransportKind.ipc 
		},
		debug: { 
			module: context.asAbsolutePath(path.join('..', 'lsp', 'webpack-out', 'lobster_lsp.js')), 
			transport: TransportKind.ipc 
		}
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
		new InlineLobsterDebugAdapterFactory("TODO")
	));

	context.subscriptions.push(debug.registerDebugConfigurationProvider('lobster', {

		resolveDebugConfiguration(
			folder: WorkspaceFolder, 
			config: DebugConfiguration, 
			token?: CancellationToken
		): ProviderResult<DebugConfiguration> {
			if (!config.type && !config.request && !config.name) {
				const editor = window.activeTextEditor;
				if (editor && editor.document.languageId === 'lobster') {
					config.type = 'lobster';
					config.name = 'Launch';
					config.request = 'launch';
					config.program = '${file}';
					config.stopOnEntry = true;
				}
			}
	
			if (!config.program) {
				return window.showInformationMessage("Cannot find a program to debug")
				.then(_ => {
					return undefined;	// abort launch
				});
			}

			return config;
		},
		
		provideDebugConfigurations(
			folder: WorkspaceFolder | undefined
		): ProviderResult<DebugConfiguration[]> {
			return [
				{
					name: "Debug lobster",
					request: "launch",
					type: "lobster",
					program: "${file}"
				}
			];
		}
	}, DebugConfigurationProviderTriggerKind.Dynamic));

	registerTasks(context);
}

export function deactivate(): Thenable<void> | undefined {
	if (!client) {
		return undefined;
	}
	return client.stop();
}
