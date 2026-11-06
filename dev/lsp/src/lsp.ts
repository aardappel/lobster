import { Connection, Diagnostic, DiagnosticSeverity, TextDocuments, WorkspaceFolder } from "vscode-languageserver";
import { exec } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import { URI } from "vscode-uri";
import { Checker } from "./checker";
import { LobsterDocument, LobsterDocumentState } from "./document";
import {
    LobsterBuiltinsDoc, LobsterFunctionSignature, QueryDefinitionResult, getBuiltinsDoc
} from "./lobster";
import { Program, Programs } from "./program";

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

// How long typing pauses before a document is checked.
const CHECK_DELAY_MS = 300;

// The Lobster checkout an executable is in, as its bin/lobster, if it is found.
function checkoutOf(executable: string): string {
    const candidates = /[\\/]/.test(executable) ? [executable]
        : (process.env.PATH ?? '').split(path.delimiter).map(dir => path.join(dir, executable));
    const found = candidates.find(file => fs.existsSync(file));
    return found ? path.dirname(path.dirname(fs.realpathSync(found))) : '';
}

// The line breaks in `text`, as an editor counts them.
function lineBreaks(text: string): number {
    return text.match(/\r\n|\r|\n/g)?.length ?? 0;
}

export class LSPInstance {
    connection: Connection;
    tempDir: string;
    globalSettings: LobsterSettings = defaultSettings;
    errorState: ErroredState = ErroredState.None;
    builtinsDoc: LobsterBuiltinsDoc[] = [];

    readonly documents: TextDocuments<LobsterDocument> = new TextDocuments({
        create: LobsterDocument.create,
        update: (document, changes, version) => {
            // What the document's lines were said to have moves along with them.
            const file = URI.parse(document.uri).fsPath;
            for (const change of changes) {
                if (!('range' in change)) continue;
                this.checker.shift(file, change.range.start.line, change.range.end.line,
                                   lineBreaks(change.text));
            }
            return LobsterDocument.update(document, changes, version);
        }
    });
    // Cached per document settings, by URI.
    readonly documentSettings: Map<string, Thenable<LobsterSettings>> = new Map();

    hasConfigurationCapability = false;

    readonly programs = new Programs(file => this.documents.get(URI.file(file).toString())?.getText());
    readonly checker: Checker;
    // The documents being typed in, to check once that pauses.
    private pending = new Map<string, ReturnType<typeof setTimeout>>();

    constructor(connection: Connection, tempDir: string) {
        this.tempDir = tempDir;
        this.connection = connection;
        this.checker = new Checker(this.programs, tempDir, (file, diagnostics) =>
            this.publish(file, diagnostics));
        this.documents.listen(this.connection);

        this.documents.onDidClose(e => {
            this.documentSettings.delete(e.document.uri);
        });

        this.documents.onDidSave(e => {
            this.programs.rescan();
            if (!this.pending.has(e.document.uri)) this.validateDocument(e.document);
        });
        // Opening a document changes its content too.
        this.documents.onDidChangeContent(change => {
            const uri = change.document.uri;
            this.programs.changed();
            clearTimeout(this.pending.get(uri));
            this.pending.set(uri, setTimeout(() => {
                this.pending.delete(uri);
                this.validateDocument(change.document);
            }, CHECK_DELAY_MS));
        });
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

            if (this.builtinsDoc.length == 0) {
                getBuiltinsDoc(c).then((d) => this.builtinsDoc = d)
                    .catch(e => this.connection.console.error(`can't read the builtins: ${e}`));
            }

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

        this.documentSettings.set(uri.toString(), config);
        return config;
    }

    async getDocumentSettings(uri: URI): Promise<LobsterSettings> {
        if (!this.hasConfigurationCapability) {
            return Promise.resolve(this.globalSettings);
        }

        let result = this.documentSettings.get(uri.toString());
        if (!result) {
            result = this.readConfiguration(uri);
        }

        return result.then(s => ({
            executable: s.executable || this.globalSettings.executable,
            imports: s.imports,
            experimental: s.experimental || this.globalSettings.experimental
        }));
    }

    // The programs a document is part of, largest first.
    async programsFor(uri: URI, settings: LobsterSettings): Promise<Program[]> {
        this.programs.configure(checkoutOf(settings.executable), settings.imports);
        return this.programs.programsFor(uri.fsPath, await this.getWorkspaceFoldersPaths());
    }

    // Compiles the programs a document is part of, and shows what they say in each of their
    // files.
    async validateDocument(document: LobsterDocument) {
        const uri = URI.parse(document.uri);
        const settings = await this.getDocumentSettings(uri);
        if (this.errored()) return;
        // Files can go away while they are read, which shouldn't stop the server.
        try {
            await this.checker.check(uri.fsPath, await this.programsFor(uri, settings), settings);
        } catch (e) {
            this.connection.console.error(`checking ${document.uri} failed: ${e}`);
        }
    }

    // What `identifier` at `line` of a document refers to, in the largest program it is part of.
    async queryDefinition(uri: URI, line: number, identifier: string): Promise<QueryDefinitionResult> {
        const settings = await this.getDocumentSettings(uri);
        const [program] = await this.programsFor(uri, settings);
        return this.checker.query(program, uri.fsPath, line, identifier, settings);
    }

    private publish(file: string, diagnostics: Diagnostic[]) {
        const uri = URI.file(file).toString();
        const document = this.documents.get(uri);
        if (document) {
            document.state = diagnostics.some(d => d.severity == DiagnosticSeverity.Error)
                ? LobsterDocumentState.HasErrors
                : diagnostics.length ? LobsterDocumentState.HasWarnings : LobsterDocumentState.NoErrors;
        }
        this.connection.sendDiagnostics({ uri, diagnostics });
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

    getFunctionSignature(funcname: string): LobsterFunctionSignature | undefined {
        const doc: LobsterBuiltinsDoc | undefined = this.builtinsDoc
            .find((d) => d.funcname == funcname);
        if (!doc) return undefined;
        const res: LobsterFunctionSignature = {
            name: doc.funcname,
            parameters: doc.args,
            returns: doc.returns
        };
        return res;
    }

    errored(): boolean {
        return this.errorState != ErroredState.None;
    }
}
