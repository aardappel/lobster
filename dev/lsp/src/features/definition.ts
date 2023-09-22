import { Location, uinteger } from "vscode-languageserver";
import { getWordOnCursor } from "../utils";
import { queryDefinition } from "../lobster";
import { LobsterDocument, LobsterDocumentState } from "../document";
import { LSPInstance } from "../lsp";
import { URI } from "vscode-uri";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onDefinition(async (params): Promise<Location[]> => {
        if (lsp.errored()) return [];

        const uri = URI.parse(params.textDocument.uri);
        const document = lsp.documents.get(uri.toString()) as LobsterDocument;
        if (document.state === LobsterDocumentState.HasErrors) return [];

        const settings = await lsp.getDocumentSettings(uri);

        const lineText = document.getText({
            start: { line: params.position.line, character: 0 },
            end: { line: params.position.line, character: uinteger.MAX_VALUE }
        });

        const [word, inFront] = getWordOnCursor(lineText, params.position.character);
        if (!word) return [];

        const temp = await document.writeToTmp(lsp);
        const result = await queryDefinition(settings, temp, params.position.line, word);

        if (result.declartionLocation) {
            const location = result.declartionLocation;

            // TODO wierd bug/feature? files from --import appear in the temp dir???
            const uri = location.file.startsWith(lsp.tempDir)
                ? location.file.replace(lsp.tempDir, (await lsp.getWorkspaceFolders())![0].uri)//(await findDoc(location.file))!.uri 
                : URI.file(location.file).toString();

            return [
                {
                    uri,
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