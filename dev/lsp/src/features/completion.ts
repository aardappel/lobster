import {
  TextDocumentPositionParams,
  CompletionList,
  CompletionItem,
  CompletionItemKind,
  Range,
  Position,
  CompletionItemLabelDetails,
} from "vscode-languageserver";
import { URI } from "vscode-uri";
import { LSPInstance } from "../lsp";


export default function setupFeature(lsp: LSPInstance) {
  lsp.connection.onCompletion(
    async (_textDocumentPosition: TextDocumentPositionParams) => {
      const document = lsp.documents.get(
        _textDocumentPosition.textDocument.uri,
      )!;
      const startPos = Position.create(_textDocumentPosition.position.line, 0);
      const range = Range.create(startPos, _textDocumentPosition.position);
      const tgtText = document.getText(range).trimStart();
      if (lsp.errored()) return [];

      const items = lsp.builtinsDoc
        .filter((doc) => doc.funcname.substring(0, tgtText.length) == tgtText)
        .map((doc) => {
          const label = doc.funcname.split(".").at(-1)!;
          const x: CompletionItem = {
            label: label,
            kind: CompletionItemKind.Function,
            labelDetails: { detail: " "+doc.doc },
            documentation: doc.doc,
            insertText: label + "()",
          };
          return x;
        });

      return CompletionList.create(items, false);
    },
  );
}
