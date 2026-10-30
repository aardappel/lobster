import { Diagnostic, DiagnosticSeverity, uinteger } from "vscode-languageserver";
import { URI } from "vscode-uri";
import { LobsterOutput } from "./lobster";
import { Program } from "./program";
import { getOrSet } from "./utils";

export function wholeLine(line: number) {
    return { start: { line, character: 0 }, end: { line, character: uinteger.MAX_VALUE } };
}

// The diagnostics of a compile of `program`, by file, where `locate` gives the file a name the
// compiler gives is.
export function toDiagnostics(
    { messages, other }: LobsterOutput,
    program: Program,
    locate: (name: string) => string
): Map<string, Diagnostic[]> {
    const found = new Map<string, Diagnostic[]>();
    const add = (file: string, diagnostic: Diagnostic) => getOrSet(found, file, () => []).push(diagnostic);
    for (const m of messages) {
        const file = locate(m.file);
        add(file, {
            range: wholeLine(m.line),
            severity: m.severity,
            source: "lobster",
            message: [m.message, ...m.context.map(c => `in ${c.file}(${c.line + 1}): ${c.text}`)]
                .join("\n"),
            relatedInformation: m.context.map(c => ({
                message: c.text,
                location: { uri: URI.file(locate(c.file)).toString(), range: wholeLine(c.line) }
            }))
        });
    }
    if (other) {
        add(program.main, { range: wholeLine(0), severity: DiagnosticSeverity.Error, source: "lobster",
                            message: other });
    }
    return found;
}
