import { TextDocumentPositionParams } from "vscode-languageserver";
import { LSPInstance } from "../lsp";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onCompletion(
        async (_textDocumentPosition: TextDocumentPositionParams) => {
            if (lsp.errored()) return [];
            
            // TODO
            return [];
        }
    );
}