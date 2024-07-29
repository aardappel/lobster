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
import { 
    getBuiltinsDoc, 
    LobsterBuiltinsDoc, 
    LobsterFunctionSignature 
} from "../lobster";

let builtinsDoc: LobsterBuiltinsDoc[];

export function getFunctionSignature(
  funcname: string,
): LobsterFunctionSignature | undefined {
  const doc: LobsterBuiltinsDoc = builtinsDoc.find((d) => d.funcname == funcname)!;
  const res: LobsterFunctionSignature = {
    name: doc.funcname,
    parameters: doc.args,
  };
  return res;
}

export default function setupFeature(lsp: LSPInstance) {
  lsp.connection.onCompletion(
    async (_textDocumentPosition: TextDocumentPositionParams) => {
      if (!builtinsDoc) {
        const settings = await lsp.getDocumentSettings(URI.parse(_textDocumentPosition.textDocument.uri));  
        builtinsDoc = await getBuiltinsDoc(settings);
        console.error("Got builtins doc");
      }

      const document = lsp.documents.get(
        _textDocumentPosition.textDocument.uri,
      )!;
      const startPos = Position.create(_textDocumentPosition.position.line, 0);
      const range = Range.create(startPos, _textDocumentPosition.position);
      const tgtText = document.getText(range).trimStart();
      if (lsp.errored()) return [];

      const items = builtinsDoc
        .filter((doc) => doc.funcname.substring(0, tgtText.length) == tgtText)
        .map((doc) => {
          const label = doc.funcname.split(".").at(-1);
          const x: CompletionItem = {
            label: label!,
            kind: CompletionItemKind.Function,
            labelDetails: { detail: " "+doc.doc },
            documentation: doc.doc,
            insertText: label! + "()",
          };
          return x;
        });

      return CompletionList.create(items, false);
    },
  );
}
