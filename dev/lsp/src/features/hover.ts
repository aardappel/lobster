import { URI } from "vscode-uri";
import { LSPInstance } from "../lsp";
import { LobsterDocumentState } from "../document";
import { getWordOnCursor, markupSignature } from "../utils";
import { LobsterSignature, queryDefinition } from "../lobster";
import { uinteger } from "vscode-languageserver";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onHover(async (params) => {
        if (lsp.errored()) return null;

        const uri = URI.parse(params.textDocument.uri);
        const document = lsp.documents.get(uri.toString())!;

        const lineText = document.getText({
          start: { line: params.position.line, character: 0 },
          end: { line: params.position.line, character: uinteger.MAX_VALUE },
        });
        const [word, inFront] = getWordOnCursor(
          lineText,
          params.position.character,
        );
        if (!word) return null;

        let signature: LobsterSignature | undefined;

        if (document.state === LobsterDocumentState.HasErrors) {
          signature = lsp.getFunctionSignature(word);
        } else {
          const settings = await lsp.getDocumentSettings(uri);

          const temp = await document.writeToTmp(lsp);
          const result = await queryDefinition(
            settings,
            temp,
            params.position.line,
            word,
          );

          signature = result.signature;
        }

        if (signature) {
          return {
            contents: markupSignature(signature),
            range: {
              start: {
                line: params.position.line,
                character: params.position.character - inFront,
              },
              end: {
                line: params.position.line,
                character: params.position.character + word.length - inFront,
              },
            },
          };
        }

        return null;
    });
}
