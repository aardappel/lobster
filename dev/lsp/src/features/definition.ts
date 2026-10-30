import { Location, uinteger } from "vscode-languageserver";
import { getWordOnCursor } from "../utils";
import { LobsterDocument } from "../document";
import { LSPInstance } from "../lsp";
import { findNative } from "../natives";
import { URI } from "vscode-uri";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onDefinition(async (params): Promise<Location[]> => {
        if (lsp.errored()) return [];

        const uri = URI.parse(params.textDocument.uri);
        const document = lsp.documents.get(uri.toString()) as LobsterDocument;

        const lineText = document.getText({
            start: { line: params.position.line, character: 0 },
            end: { line: params.position.line, character: uinteger.MAX_VALUE }
        });

        const [word, inFront] = getWordOnCursor(lineText, params.position.character);
        if (!word) return [];

        const result = await lsp.queryDefinition(uri, params.position.line, word);
        // A builtin has no Lobster source, but may have its C++ one in the checkout.
        const location = result.declartionLocation ?? (result.signature &&
            findNative(lsp.programs.lobsterDir, result.signature.name));
        if (location) {
            return [
                {
                    uri: URI.file(location.file).toString(),
                    range: {
                        start: { line: location.line, character: 0 },
                        end: { line: location.line, character: uinteger.MAX_VALUE }
                    }
                }
            ];
        }

        return [];
    });
}