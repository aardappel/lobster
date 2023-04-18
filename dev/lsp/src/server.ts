/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */
import {
	createConnection,
	TextDocuments,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	CompletionItem,
	TextDocumentPositionParams,
	TextDocumentSyncKind,
	InitializeResult,
	CodeAction,
	CodeActionKind,
	uinteger,
	Location,
	Hover
} from 'vscode-languageserver/node';


import { URI } from 'vscode-uri';
import { LobsterSettings, queryDefinition } from './lobster';
import { LobsterDocument, LobsterDocumentState } from './document';
import { getWordOnCursor, getWorkspaceFolders, markupSignature, setupConnection } from './utils';
import * as temp from 'temp';

temp.track();
const currentTempDir = temp.mkdirSync('lobster-lsp');

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
export const connection = createConnection(ProposedFeatures.all);
setupConnection(connection);

// Create a simple text document manager.
const documents: TextDocuments<LobsterDocument> = new TextDocuments(LobsterDocument);

let hasConfigurationCapability = false;
let hasWorkspaceFolderCapability = false;
let hasDiagnosticRelatedInformationCapability = false;
let hasCodeActionLiteralSupportCapability = false;

connection.onInitialize((params: InitializeParams) => {
	const capabilities = params.capabilities;

	// Does the client support the `workspace/configuration` request?
	// If not, we fall back using global settings.
	hasConfigurationCapability = !!(
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
	if (hasConfigurationCapability) {
		// Register for all configuration changes.
		connection.client.register(DidChangeConfigurationNotification.type, undefined);
	}
	if (hasWorkspaceFolderCapability) {
		connection.workspace.onDidChangeWorkspaceFolders(_event => {
			console.log('Workspace folder change event received.');
		});
	}
});

// The global settings, used when the `workspace/configuration` request is not supported by the client.
// Please note that this is not the case when using this server with the client provided in this example
// but could happen with other clients.
const defaultSettings: LobsterSettings = { executable: '', imports: [] };
let globalSettings: LobsterSettings = defaultSettings;

// Cache the settings of all open documents
const documentSettings: Map<string, Thenable<LobsterSettings>> = new Map();

connection.onDidChangeConfiguration(change => {
	if (hasConfigurationCapability) {
		// Reset all cached document settings
		documentSettings.clear();
	} else {
		globalSettings = <LobsterSettings>(
			(change.settings || defaultSettings)
		);
	}

	// Revalidate all open text documents
	documents.all().forEach(validateTextDocument);
});

export async function getDocumentSettings(resource: string): Promise<LobsterSettings> {
	if (!hasConfigurationCapability) {
		return Promise.resolve(globalSettings);
	}
	let result = documentSettings.get(resource);
	if (!result) {
		result = connection.workspace.getConfiguration({
			scopeUri: resource,
			section: 'lobster'
		});
		documentSettings.set(resource, result);
	}

	return result;
}

// Only keep settings for open documents
documents.onDidClose(e => {
	documentSettings.delete(e.document.uri);
});

// The content of a text document has changed. This event is emitted
// when the text document first opened or when its content has changed.
documents.onDidChangeContent(change => {
	validateTextDocument(change.document);
});

connection.onDidChangeWatchedFiles(_change => {
	// Monitored files have change in VSCode
	connection.console.log('We received an file change event');
});

// This handler provides the initial list of the completion items.
connection.onCompletion(
	(_textDocumentPosition: TextDocumentPositionParams): CompletionItem[] => {
		// TODO
		return [];
	}
);

// This handler resolves additional information for the item selected in
// the completion list.
connection.onCompletionResolve(
	(item: CompletionItem): CompletionItem => {
		return item;
	}
);

connection.onCodeAction((params): CodeAction[] => {
	const document = documents.get(params.textDocument.uri) as LobsterDocument;
	if (document.state === LobsterDocumentState.HasErrors) return [];

	let result: CodeAction[] = [];

	params.context.diagnostics.forEach(diagnostic => {
		if (diagnostic.message.startsWith("use `let` to declare")) {
			const text = document.getText(diagnostic.range); 
			const location = text.search("var");

			if (location == -1) return; // Can't find?

			const range = {
				start: { line: params.range.start.line, character: location },
				end: { line: params.range.start.line, character: location + 3 }
			};

			result = [{
				title: "Make variable constant",
				kind: CodeActionKind.QuickFix,
				diagnostics: [diagnostic],
				isPreferred: true,
				edit: {
					changes: {
						[document.uri]: [{
							range,
							newText: "let"
						}]
					}
				}
			}];
		}
	});

	return result;
});

async function validateTextDocument(document: LobsterDocument): Promise<void> {
	// Send the computed diagnostics to VSCode.
	connection.sendDiagnostics({ 
		uri: document.uri, 
		diagnostics: await document.parse(currentTempDir) 
	});
}


connection.onDefinition(async (params): Promise<Location[]> => {
	const uri = URI.parse(params.textDocument.uri);
	const document = documents.get(uri.toString()) as LobsterDocument;
	if (document.state === LobsterDocumentState.HasErrors) return [];

	const settings = await getDocumentSettings(uri.toString());
	
	const lineText = document.getText({
		start: { line: params.position.line, character: 0},
		end: { line: params.position.line, character: uinteger.MAX_VALUE}
	});

	const [ word, inFront ] = getWordOnCursor(lineText, params.position.character);
	if (!word) return [];

	const temp = await document.writeToTmp(currentTempDir);
	const result = await queryDefinition(settings, temp, params.position.line, word);

	if (result.declartionLocation) {
		const location = result.declartionLocation;

		// TODO wierd bug/feature? files from --import appear in the temp dir???
		const uri = location.file.startsWith(currentTempDir) 
		? location.file.replace(currentTempDir, (await getWorkspaceFolders())![0].uri)//(await findDoc(location.file))!.uri 
		: URI.file(location.file).toString();

		return [
			{
				uri,
				range: {
					start: { line: location.line, character: 0 },
					end: { line: location.line, character: uinteger.MAX_VALUE }
				}
			}
		];
	}

	return [];
});

connection.onHover(async (params): Promise<Hover | null> => {
	const uri = URI.parse(params.textDocument.uri);
	const document = documents.get(uri.toString()) as LobsterDocument;
	if (document.state === LobsterDocumentState.HasErrors) return null;

	const settings = await getDocumentSettings(uri.toString());

	const lineText = document.getText({
		start: { line: params.position.line, character: 0},
		end: { line: params.position.line, character: uinteger.MAX_VALUE}
	});

	const [ word, inFront ] = getWordOnCursor(lineText, params.position.character);
	if (!word) return null;

	const temp = await document.writeToTmp(currentTempDir);
	const result = await queryDefinition(settings, temp, params.position.line, word);

	if (result.signature) {
		return {
			contents: markupSignature(result.signature),
			range: {
				start: { 
					line: params.position.line, 
					character: params.position.character - inFront 
				},
				end: { 
					line: params.position.line, 
					character: params.position.character + word.length - inFront 
				}
			}
		};
	}

	return null;
});

async function findDoc(path: string): Promise<LobsterDocument | undefined> {
	for (const doc of documents.all()) {
		if (path.endsWith(await doc.getRelative())) return doc;
	}

	return undefined;
}

// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

// Listen on the connection
connection.listen();