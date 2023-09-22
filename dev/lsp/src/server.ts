/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
import {
	createConnection,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	TextDocumentSyncKind,
	InitializeResult,
} from 'vscode-languageserver/node';

import * as temp from 'temp';
import { LSPInstance } from './lsp';

temp.track();

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);
const instance = new LSPInstance(connection, temp.mkdirSync('lobster-lsp'));

// Create a simple text document manager.
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;
let hasCodeActionLiteralSupportCapability = false;

connection.onInitialize((params: InitializeParams) => {
	const capabilities = params.capabilities;

	// Does the client support the `workspace/configuration` request?
	// If not, we fall back using global settings.
	instance.hasConfigurationCapability = !!(
		capabilities.workspace && !!capabilities.workspace.configuration
	);
	hasWorkspaceFolderCapability = !!(
		capabilities.workspace && !!capabilities.workspace.workspaceFolders
	);
	hasDiagnosticRelatedInformationCapability = !!(
		capabilities.textDocument &&
		capabilities.textDocument.publishDiagnostics &&
		capabilities.textDocument.publishDiagnostics.relatedInformation
	);

	hasCodeActionLiteralSupportCapability =
		capabilities.textDocument?.codeAction?.codeActionLiteralSupport != undefined;

	const result: InitializeResult = {
		capabilities: {
			textDocumentSync: TextDocumentSyncKind.Incremental,
			// Tell the client that this server supports code completion.
			completionProvider: {
				resolveProvider: true
			},

			codeActionProvider: hasCodeActionLiteralSupportCapability,
			definitionProvider: true,
			hoverProvider: true
		}
	};
	if (hasWorkspaceFolderCapability) {
		result.capabilities.workspace = {
			workspaceFolders: {
				supported: true
			}
		};
	}
	return result;
});

connection.onInitialized(() => {
	if (instance.hasConfigurationCapability) {
		// Register for all configuration changes.
		connection.client.register(DidChangeConfigurationNotification.type, undefined);
	}
	if (hasWorkspaceFolderCapability) {
		connection.workspace.onDidChangeWorkspaceFolders(_event => {
			console.log('Workspace folder change event received.');
		});
	}
});

connection.onDidChangeConfiguration(change => {
	if (instance.hasConfigurationCapability) {
		// Reset all cached document settings
		instance.documentSettings.clear();
	} else {
		instance.globalSettings = change.settings;
	}

	// Revalidate all open text documents
	instance.documents.all().forEach(instance.validateDocument);
});


connection.onDidChangeWatchedFiles(_change => {
	// Monitored files have change in VSCode
	connection.console.log('We received an file change event');
});

import registerCodeAction from './features/code_action';
import registerCompletion from './features/completion';
import registerDefinition from './features/definition';
import registerHover from './features/hover';

registerCodeAction(instance);
registerCompletion(instance);
registerDefinition(instance);
registerHover(instance);

// Listen on the connection
connection.listen();