import { DocumentSymbol, Range, SymbolInformation, SymbolKind, uinteger } from "vscode-languageserver";
import { URI } from "vscode-uri";
import { Def, defKey, shortName } from "../index";
import { LSPInstance } from "../lsp";
import { memo, occurrences } from "../utils";

function kind(def: Def): SymbolKind {
    switch (def.kind) {
        case "function": return def.container ? SymbolKind.Method : SymbolKind.Function;
        case "var": return SymbolKind.Variable;
        case "class": return SymbolKind.Class;
        case "struct": return SymbolKind.Struct;
        case "field": return SymbolKind.Field;
        case "enum": return SymbolKind.Enum;
        default: return SymbolKind.EnumMember;
    }
}

// What is worth listing: not locals and function values.
function listed(def: Def): boolean {
    return def.kind != "lambda" && !(def.kind == "var" && def.container);
}

// Whether `name` has the letters of `query` in order, as most editors match.
function matches(name: string, query: string): boolean {
    let i = 0;
    for (const c of name.toLowerCase()) if (c == query[i]) i++;
    return i == query.length;
}

// Where a definition's name is on its line, in `lines` of its file.
function nameRange(def: Def, lines: (file: string) => string[]): Range {
    const [start, end] = occurrences(lines(def.file)[def.line] ?? "", shortName(def))[0] ?? [0, 0];
    return { start: { line: def.line, character: start }, end: { line: def.line, character: end } };
}

export default function setupFeature(lsp: LSPInstance) {

    lsp.connection.onDocumentSymbol(params => {
        const file = URI.parse(params.textDocument.uri).fsPath;
        const [index] = lsp.checker.indexesFor(file);
        if (!index) return [];
        const lines = memo(f => lsp.programs.lines(f));
        const symbols = new Map<Def, DocumentSymbol>();
        for (const def of index.defs) {
            if (def.file != file || !listed(def)) continue;
            const end = def.end ?? def.line;
            symbols.set(def, {
                name: def.name,
                detail: def.type,
                kind: kind(def),
                range: {
                    start: { line: def.line, character: 0 },
                    end: { line: end, character: uinteger.MAX_VALUE }
                },
                selectionRange: nameRange(def, lines),
                children: []
            });
        }
        const roots: DocumentSymbol[] = [];
        for (const [def, symbol] of symbols) {
            const parent = def.container && symbols.get(def.container);
            if (!parent) {
                roots.push(symbol);
                continue;
            }
            parent.children!.push(symbol);
            // A class's range covers its fields and methods declared in this file.
            if (symbol.range.end.line > parent.range.end.line) parent.range.end = symbol.range.end;
        }
        return roots;
    });

    lsp.connection.onWorkspaceSymbol(params => {
        const query = params.query.toLowerCase();
        const lines = memo(f => lsp.programs.lines(f));
        const found = new Map<string, SymbolInformation>();
        for (const index of lsp.checker.indexes()) {
            for (const def of index.defs) {
                if (!listed(def) || lsp.programs.isModule(def.file) || !matches(def.name, query)) continue;
                found.set(defKey(def), {
                    name: def.name,
                    kind: kind(def),
                    containerName: def.container?.name,
                    location: { uri: URI.file(def.file).toString(), range: nameRange(def, lines) }
                });
            }
        }
        return [...found.values()];
    });
}
