import { URI } from "vscode-uri";
import { LSPInstance } from "../lsp";
import { LobsterDocumentState } from "../document";
import { getWordOnCursor, markupSignature } from "../utils";
import { queryDefinition } from "../lobster";
import { uinteger } from "vscode-languageserver";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onHover(async (params) => {
        if (lsp.errored()) return null;

        const uri = URI.parse(params.textDocument.uri);
        const document = lsp.documents.get(uri.toString())!;
        if (document.state === LobsterDocumentState.HasErrors) return null;

        const settings = await lsp.getDocumentSettings(uri);

        const lineText = document.getText({
            start: { line: params.position.line, character: 0 },
            end: { line: params.position.line, character: uinteger.MAX_VALUE }
        });

        const [word, inFront] = getWordOnCursor(lineText, params.position.character);
        if (!word) return null;

        const temp = await document.writeToTmp(lsp);
        const result = await queryDefinition(settings, temp, params.position.line, word);

        if (result.signature) {
            return {
                contents: markupSignature(result.signature),
                range: {
                    start: {
                        line: params.position.line,
                        character: params.position.character - inFront
                    },
                    end: {
                        line: params.position.line,
                        character: params.position.character + word.length - inFront
                    }
                }
            };
        }

        return null;
    });
}