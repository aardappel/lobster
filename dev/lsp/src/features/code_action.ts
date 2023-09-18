import { CodeAction, CodeActionKind } from "vscode-languageserver";
import { LobsterDocumentState } from "../document";
import { LSPInstance } from "../lsp";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onCodeAction((params) => {
        if (lsp.errored()) return [];

        const document = lsp.documents.get(params.textDocument.uri)!;
        if (document.state === LobsterDocumentState.HasErrors) return [];

        const result: CodeAction[] = [];

        params.context.diagnostics.forEach(diagnostic => {
            if (diagnostic.message.startsWith("use `let` to declare")) {
                const text = document.getText(diagnostic.range);
                const location = text.search("var");

                if (location == -1) return; // Can't find?

                const range = {
                    start: { line: params.range.start.line, character: location },
                    end: { line: params.range.start.line, character: location + 3 }
                };

                result.push({
                    title: "Make variable constant",
                    kind: CodeActionKind.QuickFix,
                    diagnostics: [diagnostic],
                    isPreferred: true,
                    edit: {
                        changes: {
                            [document.uri]: [{
                                range,
                                newText: "let"
                            }]
                        }
                    }
                });
            }
        });

        return result;
    });
}