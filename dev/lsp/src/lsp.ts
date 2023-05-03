import { Connection, TextDocuments, WorkspaceFolder } from "vscode-languageserver";
import { URI } from "vscode-uri";
import { LobsterDocument, LobsterDocumentState } from "./document";

export interface LobsterSettings {
    executable: string,
    imports: string[],
    experimental: boolean
}

const defaultSettings: LobsterSettings = { executable: '', imports: [], experimental: false };

export class LSPInstance {
    connection: Connection;
    tempDir: string;
    globalSettings: LobsterSettings = defaultSettings;

    readonly documents: TextDocuments<LobsterDocument> = new TextDocuments(LobsterDocument);
    // Cached per document settings
    readonly documentSettings: Map<URI, Thenable<LobsterSettings>> = new Map();

    hasConfigurationCapability = false;

    constructor(connection: Connection, tempDir: string) {
        this.tempDir = tempDir;
        this.connection = connection;

        this.documents.onDidClose(e => {
            this.documentSettings.delete(URI.parse(e.document.uri));
        });

        this.documents.onDidChangeContent(change => {
            this.validateDocument(change.document);
        });

        this.documents.listen(this.connection);
    }

    async getDocumentSettings(uri: URI): Promise<LobsterSettings> {
        if (!this.hasConfigurationCapability) {
            return Promise.resolve(this.globalSettings);
        }

        let result = this.documentSettings.get(uri);
        if (!result) {
            result = this.connection.workspace.getConfiguration({
                scopeUri: uri.toString(),
                section: 'lobster'
            });
            this.documentSettings.set(uri, result);
        }

        const folders = await this.getWorkspaceFoldersPaths();
        return result.then(s => ({
            executable: s.executable || this.globalSettings.executable,
            imports: [...s.imports, ...folders],
            experimental: s.experimental || this.globalSettings.experimental
        }));
    }

    async validateDocument(document: LobsterDocument) {
        const noErrBefore = document.state === LobsterDocumentState.NoErrors;
        const diagnostics = await document.parse(this);
        if (!noErrBefore || diagnostics.length > 0) {
            this.connection.sendDiagnostics({
                uri: document.uri,
                diagnostics
            });
        }
    }

    async getWorkspaceFolders(): Promise<WorkspaceFolder[] | null> {
        if (!this.connection) return null;
        return this.connection.workspace.getWorkspaceFolders();
    }

    async getWorkspaceFoldersPaths(): Promise<string[]> {
        const folders = await this.getWorkspaceFolders();
        if (!folders) return [];
        return folders.map(f => URI.parse(f.uri).fsPath);
    }
}