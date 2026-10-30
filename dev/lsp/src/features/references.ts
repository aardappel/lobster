import { LSPErrorCodes, Location, Position, Range, ResponseError, TextEdit, WorkspaceEdit } from "vscode-languageserver";
import { URI } from "vscode-uri";
import { Def, DefKind, ProgramIndex, defKey, shortName } from "../index";
import { LSPInstance } from "../lsp";
import { getOrSet, memo, occurrences, wordAt } from "../utils";

// What a name refers to when several on its line have it, best first.
const preference: DefKind[] = ['var', 'field', 'function', 'enumval', 'class', 'struct', 'enum', 'lambda'];

function isType(def: Def): boolean {
    return def.kind == 'class' || def.kind == 'struct' || def.kind == 'enum';
}

// The definition the identifier at `character` of `line` in `file` refers to: the one of that
// name used or declared on the line, preferring what fits the dot before it (or its absence),
// then the innermost.
export function resolve(index: ProgramIndex, file: string, line: number, text: string,
                        character: number): Def | undefined {
    const at = wordAt(text, character);
    if (!at) return undefined;
    const candidates = index.on(file, line).filter(d => shortName(d) == at.word);
    // A field is named after a dot, a function either way.
    const fits = (d: Def) => d.kind == 'function' || (at.dot ? d.kind == 'field' : d.kind != 'field');
    const fitting = candidates.filter(fits);
    const best = (fitting.length ? fitting : candidates)
        .sort((a, b) => preference.indexOf(a.kind) - preference.indexOf(b.kind) || b.line - a.line)[0];
    if (best || at.dot) return best;
    // Types used in annotations are not in the index.
    return index.defs.find(d => isType(d) && d.name == at.word);
}

// The definition at a position in a document, from the indexes of the programs it is in.
export function definitionAt(lsp: LSPInstance, uri: string, position: Position): Def | undefined {
    const file = URI.parse(uri).fsPath;
    const text = lsp.programs.lines(file)[position.line] ?? "";
    for (const index of lsp.checker.indexesFor(file)) {
        const def = resolve(index, file, position.line, text, position.character);
        if (def) return def;
    }
    return undefined;
}

// Where on a line `def` is written: a field after a dot, or without one in a method of its
// class; a function either way; anything else, and any declaration, without one.
function columns(def: Def, text: string, declaration: boolean): [number, number][] {
    const name = shortName(def);
    if (declaration) return occurrences(text, name, false);
    if (def.kind == "function") return occurrences(text, name);
    if (def.kind != "field") return occurrences(text, name, false);
    const dotted = occurrences(text, name, true);
    return dotted.length ? dotted : occurrences(text, name, false);
}

// Everywhere a definition is written, in all `indexes` of the programs it is part of, with
// `lines` the text of a file.
export function locationsOf(def: Def, indexes: ProgramIndex[], lines: (file: string) => string[]):
    { location: Location, declaration: boolean }[] {
    const found = new Map<string, Set<number>>([[def.file, new Set([def.line])]]);
    const add = (file: string, line: number) => getOrSet(found, file, () => new Set()).add(line);
    const key = defKey(def);
    for (const index of indexes) {
        for (const use of index.find(key)?.uses ?? []) add(use.file, use.line);
    }
    // Types used in annotations aren't in the index: any word that is the type's name, not after
    // a dot, is one.
    if (isType(def)) {
        for (const file of new Set(indexes.flatMap(index => [...index.files]))) {
            lines(file).forEach((text, l) => {
                if (occurrences(text, def.name, false).length) add(file, l);
            });
        }
    }
    const result: { location: Location, declaration: boolean }[] = [];
    for (const [file, ls] of found) {
        const text = lines(file);
        for (const l of [...ls].sort((a, b) => a - b)) {
            const declaration = file == def.file && l == def.line;
            for (const [start, end] of columns(def, text[l] ?? "", declaration)) {
                result.push({
                    location: {
                        uri: URI.file(file).toString(),
                        range: { start: { line: l, character: start }, end: { line: l, character: end } }
                    },
                    declaration
                });
            }
        }
    }
    return result;
}

// Whether `position` is in `range`, which is on one line.
function covers(range: Range, position: Position): boolean {
    return range.start.line == position.line && range.start.character <= position.character &&
           position.character <= range.end.character;
}

export default function setupFeature(lsp: LSPInstance) {
    const locations = (def: Def) =>
        locationsOf(def, lsp.checker.indexesFor(def.file), memo(file => lsp.programs.lines(file)));

    lsp.connection.onReferences(params => {
        const def = definitionAt(lsp, params.textDocument.uri, params.position);
        if (!def) return [];
        return locations(def)
            .filter(l => params.context.includeDeclaration || !l.declaration)
            .map(l => l.location);
    });

    // Only what the program itself declares can be renamed, not what Lobster's modules do.
    const renamable = (def: Def) => def.kind != "lambda" && !lsp.programs.isModule(def.file);

    lsp.connection.onPrepareRename(params => {
        const def = definitionAt(lsp, params.textDocument.uri, params.position);
        if (!def || !renamable(def)) return null;
        const here = locations(def).find(({ location }) =>
            location.uri == params.textDocument.uri && covers(location.range, params.position));
        return here ? { range: here.location.range, placeholder: shortName(def) } : null;
    });

    lsp.connection.onRenameRequest(params => {
        if (!/^[A-Za-z_][A-Za-z0-9_]*$/.test(params.newName)) {
            throw new ResponseError(LSPErrorCodes.RequestFailed, `not a valid name: ${params.newName}`);
        }
        const def = definitionAt(lsp, params.textDocument.uri, params.position);
        if (!def || !renamable(def)) throw new ResponseError(LSPErrorCodes.RequestFailed, "nothing to rename here");
        const changes: WorkspaceEdit["changes"] = {};
        for (const { location } of locations(def)) {
            (changes[location.uri] ??= []).push(TextEdit.replace(location.range, params.newName));
        }
        return { changes };
    });
}
