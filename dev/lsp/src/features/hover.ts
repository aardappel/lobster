import { URI } from "vscode-uri";
import { LSPInstance } from "../lsp";
import { docComment, getWordOnCursor, markupSignature } from "../utils";
import { LobsterSignature } from "../lobster";
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

        // The compiler answers queries despite errors, but may not reach an expression after
        // one, so builtins have their documentation to fall back on.
        const result = await lsp.queryDefinition(uri, params.position.line, word);
        const signature: LobsterSignature | undefined =
          result.signature ?? lsp.getFunctionSignature(word);

        if (signature) {
          // Functions are described by the comment above their def, builtins by their docs.
          const location = result.declartionLocation;
          const doc = location
            ? docComment(lsp.programs.lines(location.file), location.line)
            : lsp.builtinsDoc.find((d) => d.funcname == signature.name)?.doc;
          return {
            contents: markupSignature(signature, doc),
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
