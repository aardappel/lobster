import { execFile } from 'child_process';
import * as path from 'path';
import { DiagnosticSeverity, integer } from 'vscode-languageserver';
import { LobsterSettings } from './lsp';

export interface LobsterLocation {
    file: string,
    line: integer
}

export interface LobsterSignatureParameter {
    name: string,
    type: string
}

export interface LobsterVariableSignature {
    name: string,
    type: string
}

export interface LobsterFunctionSignature {
    name: string,
    parameters: LobsterSignatureParameter[]
    returns?: string[]
}

export interface LobsterBuiltinsDoc {
    args: LobsterSignatureParameter[];
    doc: string;
    funcname: string;
    returns?: string[];
    subsystem: string;
}

export type LobsterSignature = LobsterVariableSignature | LobsterFunctionSignature;

export interface QueryDefinitionResult {
    declartionLocation?: LobsterLocation,
    signature?: LobsterSignature
}

// Where to run the compiler, and the directories to look for imports in after it.
export interface LobsterRun {
    cwd?: string,
    imports?: string[]
}

// More than execFile's default of 1 MB, which a large program's messages can take.
const MAX_OUTPUT = 64 * 1024 * 1024;

function callLobster(
    settings: LobsterSettings,
    run: LobsterRun,
    output: (output: string) => void,
    ...params: string[]
) {
    // Paths are relative to where the LSP runs, not `run.cwd`.
    const executable = settings.executable.includes('/') || settings.executable.includes('\\')
        ? path.resolve(settings.executable)
        : settings.executable;
    const imports = [...settings.imports, ...run.imports ?? []].map(i => path.resolve(i));
    const args = [...imports.flatMap(i => ["--import", i]), ...params];

    console.debug(executable, ...args);
    execFile(executable, args, { cwd: run.cwd, maxBuffer: MAX_OUTPUT }, (error, stdout, stderr) => {
        if (stderr) {
            console.error(`Lobster (${executable}) returned with output: ${stderr}`);
        }

        output(stdout);
    });
}

// A warning or error, in the file the compiler names (relative to the main file's directory,
// or absolute), at a zero based line.
export interface LobsterMessage {
    file: string,
    line: integer,
    severity: DiagnosticSeverity,
    message: string,
    // Where it was reached from, as in "in game.lobster(2061): bake() -> int".
    context: (LobsterLocation & { text: string })[]
}

// What a compile says: its messages, and what isn't about a place in a file, such as that the
// main file can't be read.
export interface LobsterOutput {
    messages: LobsterMessage[],
    other: string
}

export function parseMessages(input: string): LobsterMessage[] {
    const regex = /^(.*?)\((\d+)\): (warning|error): (.*)((?:\n[ \t]+.*)*)/gm;
    const regexInner = /^\s*in (.*?)\((\d+)\): ?(.*)$/;
    const output: LobsterMessage[] = [];
    let match;
    while ((match = regex.exec(input))) {
        const context: LobsterMessage['context'] = [];
        const details: string[] = [];
        for (const line of match[5].split("\n").slice(1)) {
            const inner = line.match(regexInner);
            if (inner) context.push({ file: inner[1], line: parseInt(inner[2]) - 1, text: inner[3] });
            else details.push(line.trim());
        }
        output.push({
            file: match[1],
            line: parseInt(match[2]) - 1,
            severity: match[3] == "warning" ? DiagnosticSeverity.Warning : DiagnosticSeverity.Error,
            message: [match[4].trim(), ...details].join("\n"),
            context
        });
    }
    return output;
}

// Compiles `main`.
export async function parseLobster(
    settings: LobsterSettings,
    main: string,
    run: LobsterRun = {}
): Promise<LobsterOutput> {
    return new Promise(back => callLobster(
        settings,
        run,
        stdout => {
            const messages = parseMessages(stdout);
            back({ messages, other: messages.length ? "" : stdout.trim() });
        },
        "--errors", "99",
        "--compile-only",
        "--warn-unused",
        main
    ));
}

export async function getBuiltinsDoc(
    settings: LobsterSettings,
): Promise<LobsterBuiltinsDoc[]> {
    function parseOutput(input: string): LobsterBuiltinsDoc[] {
        const parsedData: LobsterBuiltinsDoc[] = JSON.parse(input);
        return parsedData;
    }

    return new Promise<LobsterBuiltinsDoc[]>((back) =>
        callLobster(settings, {}, (i) => back(parseOutput(i)), "--gen-builtins-json"),
    );
}

function parseDefinition(
    identifier: string,
    definition: string,
): LobsterSignature {
    let signature: LobsterSignature;
    const [def, ret] = splitList(definition, "->").map(s => s.trim());
    const defmatch = def.match(/^(.+)\((.*)\)$/);
    if (defmatch) {
        const parameters = readParameters(defmatch[2]);
        signature = {
            name: defmatch[1],
            parameters,
            returns: ret
                ? splitList(ret.replace(/^\((.*)\)$/, "$1")).map(r => r.trim())
                : undefined,
        };
    } else {
        signature = {
            name: identifier,
            type: def,
        };
    }
    return signature;
}

// What `identifier` at `line` of `file` (named as the compiler names it) refers to, in the
// program compiled from `main`.
export async function queryDefinition(
    settings: LobsterSettings,
    main: string,
    file: string,
    line: integer,
    identifier: string,
    run: LobsterRun = {}
): Promise<QueryDefinitionResult> {
    function parseOutput(input: string): QueryDefinitionResult {
        if (input.length == 0) return {};

        let declartionLocation: LobsterLocation | undefined;
        let signature: LobsterSignature | undefined;

        const queryLocationB = input.indexOf("query_location");
        if (queryLocationB != -1) {
            const sub = input.substring(queryLocationB).split("\n")[0];
            const match = sub.match(/^query_location: (.+) (\d+) (.+)?/);

            if (!match) {
                console.error("Invalid query_location output from lobster: " + sub);
            } else {
                declartionLocation = {
                    file: match[1].trim(),
                    line: parseInt(match[2]) - 1, // Its zero based
                };
                if (match[3]) {
                    signature = parseDefinition(identifier, match[3]);
                }
            }
        }

        const querySignatureB = input.indexOf("query_signature");
        if (querySignatureB != -1) {
            const sub = input.substring(querySignatureB).split("\n")[0];
            const match = sub.match(/^query_signature: (.*)/);

            if (!match) {
                console.error("Invalid query_signature output from lobster: " + sub);
            } else {
                signature = parseDefinition(identifier, match[1]);
            }
        }

        return {
            declartionLocation,
            signature,
        };
    }

    return new Promise((back) =>
        callLobster(
            settings,
            run,
            (i) => back(parseOutput(i)),
            // Errors elsewhere in the program don't stop the query.
            "--errors", "99",
            main,
            "--query",
            "definition",
            file,
            (line + 1).toString(),
            identifier,
        ),
    );
}

// Splits at the separators that aren't inside a type's brackets, as in `dictionary<K, V>` or
// `function(x:int) -> int`.
export function splitList(input: string, separator = ","): string[] {
    const parts: string[] = [];
    let depth = 0, start = 0;
    for (let i = 0; i < input.length; i++) {
        if (depth == 0 && input.startsWith(separator, i)) {
            parts.push(input.substring(start, i));
            start = i + separator.length;
            i = start - 1;
        } else if (input.startsWith("->", i)) {
            i++;
        } else if ("<([{".includes(input[i])) {
            depth++;
        } else if (">)]}".includes(input[i])) {
            depth--;
        }
    }
    parts.push(input.substring(start));
    return parts;
}

function readParameters(input: string): LobsterSignatureParameter[] {
    const parameters = input == '' ? [] : splitList(input)
        .map(i => i.trim().match(/^([^:]+)(?::(.+))?$/) || [])
        .map(i => ({ name: i[1], type: i[2] || "any" }));

    if (parameters.some(i => i.name == undefined || i.type == undefined))
        throw new Error("Invalid output from lobster: " + input);

    return parameters;
}
