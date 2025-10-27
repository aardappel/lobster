import {
  TextDocumentPositionParams,
  CompletionList,
  CompletionItem,
  CompletionItemKind,
  Range
} from "vscode-languageserver";
import { LSPInstance } from "../lsp";
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
      } while(isAlphaNumeric(c) || c == '.');

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

      return CompletionList.create(items, true);
    },
  );
}
