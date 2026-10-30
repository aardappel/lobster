import {
  TextDocumentPositionParams,
  CompletionList,
  CompletionItem,
  CompletionItemKind,
  Range,
  Position,
  MarkupKind
} from "vscode-languageserver";
import { URI } from "vscode-uri";
import { Def, ProgramIndex, shortName } from "../index";
import { LobsterFunctionSignature, parseDefinition } from "../lobster";
import { LSPInstance } from "../lsp";
import { docComment, memo } from "../utils";
const isAlphaNumeric = (str: string) => /^[a-z0-9]*$/gi.test(str);

export default function setupFeature(lsp: LSPInstance) {
  lsp.connection.onCompletion(
    async (_textDocumentPosition: TextDocumentPositionParams) => {
      const document = lsp.documents.get(
        _textDocumentPosition.textDocument.uri,
      )!;
      const content = document.getText();
      let offset = document.offsetAt(_textDocumentPosition.position);

      do {
        var c = content.charAt(--offset);
      } while(isAlphaNumeric(c) || c == '.' || c == '_');

      const range = Range.create(document.positionAt(offset + 1), _textDocumentPosition.position);
      const tgtText = document.getText(range).trimStart();

      if (lsp.errored()) return [];
      const items = lsp.builtinsDoc
        .filter((doc) => doc.funcname.substring(0, tgtText.length) == tgtText)
        .map((doc) => {
          const x: CompletionItem = {
            label: doc.funcname,
            kind: CompletionItemKind.Function,
            labelDetails: { detail: " "+doc.doc },
            documentation: doc.doc,
            // If no arguments are expected then just autofill ) too
            textEdit: { range: range, newText: doc.funcname + (doc.args.length == 0 ? '()' : '(') } 
          };
          return x;
        });

      const file = URI.parse(document.uri).fsPath;
      items.push(...programItems(lsp.checker.indexesFor(file), file, _textDocumentPosition.position, tgtText,
                                 memo(f => lsp.programs.lines(f))));
      return CompletionList.create(items, true);
    },
  );
}

const kinds: { [kind: string]: CompletionItemKind } = {
  function: CompletionItemKind.Function,
  var: CompletionItemKind.Variable,
  class: CompletionItemKind.Class,
  struct: CompletionItemKind.Struct,
  field: CompletionItemKind.Field,
  enum: CompletionItemKind.Enum,
  enumval: CompletionItemKind.EnumMember,
};

// The class a type names, as in `Foo`, `Foo?` or `Foo<int>`.
const typeName = (type: string) => type.replace(/\?$/, "").replace(/<.*$/, "");

// The class a field or method is of: its container, or the type of a function's first parameter,
// as `T` in `f(this:T, x)`.
function ownerOf(def: Def): string | undefined {
  if (def.container) return def.container.name;
  const signature = parseDefinition(def.name, def.type!) as LobsterFunctionSignature;
  return signature.parameters[0]?.type;
}

// What the programs a file is part of declare that fits `typed`, the text before `position`:
// after a dot the fields and methods of the variable before it (all of them if its type isn't
// known), otherwise the variables in scope and everything declared outside functions. Functions
// are described by the comment above them in `lines`.
function programItems(indexes: ProgramIndex[], file: string, position: Position, typed: string,
                      lines: (file: string) => string[]): CompletionItem[] {
  const dot = typed.lastIndexOf('.');
  const prefix = typed.substring(dot + 1);
  const range = Range.create({ line: position.line, character: position.character - prefix.length }, position);
  const items = new Map<string, CompletionItem>();
  const add = (def: Def, label = shortName(def)) => {
    const key = `${label} ${def.kind}`;
    if (!label.startsWith(prefix) || items.has(key)) return;
    const doc = def.kind == "function" ? docComment(lines(def.file), def.line) : undefined;
    items.set(key, {
      label,
      kind: def.kind == "function" && dot >= 0 ? CompletionItemKind.Method : kinds[def.kind],
      labelDetails: { detail: def.type && " " + def.type },
      documentation: doc && { kind: MarkupKind.Markdown, value: doc },
      textEdit: { range, newText: label + (def.kind == "function" ? "(" : "") }
    });
  };
  const defs = indexes.flatMap(index => index.defs);
  // Whether `position` is in the body of function or lambda `fun`.
  const within = (fun: Def) => fun.file == file && fun.line <= position.line && position.line <= fun.end!;
  // Globals, and the locals of the functions `position` is in, declared before it.
  const inScope = (v: Def) => !v.container || (within(v.container) && v.line <= position.line);
  // Innermost first, as those shadow the rest.
  const depth = (v: Def) => v.container?.line ?? -1;
  const visible = defs.filter(d => d.kind == "var" && inScope(d)).sort((a, b) => depth(b) - depth(a));
  if (dot >= 0) {
    const receiver = typed.substring(0, dot).split('.').pop();
    const type = visible.find(v => v.name == receiver)?.type;
    for (const def of defs) {
      if (def.kind != "field" && def.kind != "function") continue;
      const owner = ownerOf(def);
      if (!type || (owner && typeName(owner) == typeName(type))) add(def);
    }
  } else {
    for (const def of visible) add(def);
    for (const def of defs) {
      if (def.kind != "var" && def.kind != "lambda" && def.kind != "field") add(def, def.name);
    }
  }
  return [...items.values()];
}
