import { Connection, TextDocuments, WorkspaceFolder } from "vscode-languageserver";
import { exec } from 'child_process';
import { URI } from "vscode-uri";
import { LobsterDocument, LobsterDocumentState } from "./document";

export interface LobsterSettings {
    executable: string,
    imports: string[],
    experimental: boolean
}

export enum ErroredState {
    None,
    InvalidExecutable,
}

const defaultSettings: LobsterSettings = { executable: '', imports: [], experimental: false };

export class LSPInstance {
    connection: Connection;
    tempDir: string;
    globalSettings: LobsterSettings = defaultSettings;
    errorState: ErroredState = ErroredState.None;

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

    readConfiguration(uri: URI): Promise<LobsterSettings> {
        const config = this.connection.workspace.getConfiguration({
            scopeUri: uri.toString(),
            section: 'lobster'
        }) as Promise<LobsterSettings>;

        //Validate config
        config.then(c => {
            const throwError = (msg: string) => {
                this.connection.window.showErrorMessage(msg);
                this.errorState = ErroredState.InvalidExecutable;
                return config;
            };
            
            if (c.executable.length == 0)
                return throwError("Lobster executable path is not set.");

            exec(c.executable, (error, stdout, stderr) => {
                if (stderr) {
                    return throwError(`Lobster (${c.executable}) returned with output: ${stderr}`);
                } else if (error && error.code != 1) { // If errorcode 1 then its the help message
                    return throwError(`Lobster (${c.executable}) failed to execute: ${error.message}`);
                } else if (!stdout.startsWith("Lobster programming language")) {
                    return throwError(`Lobster (${c.executable}) returned with unexpected output (is this a lobster binary?): ${stdout}`);
                }
            });
        });

        this.documentSettings.set(uri, config);
        return config;
    }

    async getDocumentSettings(uri: URI): Promise<LobsterSettings> {
        if (!this.hasConfigurationCapability) {
            return Promise.resolve(this.globalSettings);
        }

        let result = this.documentSettings.get(uri);
        if (!result) {
            result = this.readConfiguration(uri);
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
        if (this.errored()) return null;

        return this.connection.workspace.getWorkspaceFolders();
    }

    async getWorkspaceFoldersPaths(): Promise<string[]> {
        if (this.errored()) return [];

        const folders = await this.getWorkspaceFolders();
        if (!folders) return [];
        return folders.map(f => URI.parse(f.uri).fsPath);
    }

    errored(): boolean {
        return this.errorState != ErroredState.None;
    }
}