import { MarkupKind, ParameterInformation, SignatureHelp, SignatureInformation } from "vscode-languageserver";
import { URI } from "vscode-uri";
import { shortName } from "../index";
import { splitList } from "../lobster";
import { LSPInstance } from "../lsp";
import { codeLength, docComment, memo } from "../utils";

// The name a call before its `(` is made with, as in `f`, `x.f` or `f<int>`.
const CALLEE = /([A-Za-z_][A-Za-z0-9_.]*)\s*(?:<[^<>()]*>)?$/;

// A signature as `name(a:int, b) -> type`, with where each parameter is in it.
function signature(label: string): SignatureInformation {
    const open = label.indexOf("("), close = label.lastIndexOf(")");
    const parameters: ParameterInformation[] = [];
    if (open >= 0 && close > open + 1) {
        let at = open + 1;
        for (const p of splitList(label.substring(open + 1, close))) {
            const start = at + p.length - p.trimStart().length;
            parameters.push({ label: [start, start + p.trim().length] });
            at += p.length + 1;
        }
    }
    return { label, parameters };
}

// The call whose arguments `offset` in `text` is in: the unmatched `(` before it, and the
// commas between at its level.
function callAt(text: string, offset: number): { open: number, commas: number } | undefined {
    // Comments are blanked out, keeping offsets.
    const code = text.substring(0, offset).split("\n")
        .map(line => line.substring(0, codeLength(line)).padEnd(line.length)).join("\n");
    let depth = 0, commas = 0;
    for (let i = offset - 1; i >= 0; i--) {
        const c = code[i];
        if (c == '"' || c == "'") {
            for (i--; i >= 0 && (code[i] != c || code[i - 1] == '\\'); i--);
        } else if (c == ')' || c == ']' || c == '}') {
            depth++;
        } else if (c == '(' || c == '[' || c == '{') {
            if (depth == 0) return c == '(' ? { open: i, commas } : undefined;
            depth--;
        } else if (c == ',' && depth == 0) {
            commas++;
        }
    }
    return undefined;
}

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onSignatureHelp((params): SignatureHelp | null => {
        const document = lsp.documents.get(params.textDocument.uri)!;
        const text = document.getText();
        const call = callAt(text, document.offsetAt(params.position));
        if (!call) return null;
        const name = text.substring(0, call.open).match(CALLEE)?.[1];
        if (!name) return null;
        const word = name.substring(name.lastIndexOf('.') + 1);

        // Each signature, with what describes it.
        const labels = new Map<string, string | undefined>();
        const file = URI.parse(params.textDocument.uri).fsPath;
        const indexes = lsp.checker.indexesFor(file);
        const lines = memo(f => lsp.programs.lines(f));
        for (const index of indexes) {
            for (const def of index.defs) {
                if (def.kind == "function" && shortName(def) == word && !labels.has(def.type!)) {
                    labels.set(def.type!, docComment(lines(def.file), def.line));
                }
            }
        }
        for (const doc of lsp.builtinsDoc) {
            if (doc.funcname == name || doc.funcname == word) {
                const args = doc.args.map(a => a.type ? `${a.name}:${a.type}` : a.name).join(", ");
                const returns = doc.returns?.length ? " -> " + doc.returns.join(", ") : "";
                labels.set(`${doc.funcname}(${args})${returns}`, doc.doc);
            }
        }
        const signatures = [...labels].map(([label, doc]) =>
            ({ ...signature(label), documentation: doc && { kind: MarkupKind.Markdown, value: doc } }));
        if (!signatures.length) return null;
        // `x.f(a)` passes x as the first argument, unless x is a namespace.
        const namespaced = lsp.builtinsDoc.some(d => d.funcname == name) ||
            indexes.some(index => index.defs.some(d => d.name == name));
        const method = name.includes('.') && !namespaced;
        const activeParameter = call.commas + (method ? 1 : 0);
        // The first that takes that many arguments, else the first.
        const fitting = signatures.findIndex(s => s.parameters!.length > activeParameter);
        return { signatures, activeSignature: fitting < 0 ? 0 : fitting, activeParameter };
    });
}
