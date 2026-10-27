import { LSPErrorCodes, ResponseError, TextEdit } from "vscode-languageserver";
import { format, FormatError, position } from "../format/format";
import { LSPInstance } from "../lsp";

export default function setupFeature(lsp: LSPInstance) {
    lsp.connection.onDocumentFormatting(params => {
        const document = lsp.documents.get(params.textDocument.uri);
        if (!document) return null;
        const text = document.getText();
        let result: string;
        try {
            result = format(text);
        } catch (e) {
            if (!(e instanceof FormatError)) throw e;
            const { line, character } = position(text, e.offset);
            return new ResponseError(LSPErrorCodes.RequestFailed, `can't format: ${line + 1}:${character + 1}: ${e.message}`);
        }
        // One edit, of the lines between those that stay the same, so the rest of the
        // document (and what's selected in it) is left alone. All of it for CRLF, which
        // becomes LF.
        const before = text.split("\n"), after = result.split("\n");
        const crlf = text.includes("\r");
        let first = 0;
        while (!crlf && first < before.length && first < after.length && before[first] == after[first]) first++;
        if (first == before.length && first == after.length) return [];
        let last = 0;
        while (!crlf && last < before.length - first && last < after.length - first &&
               before[before.length - 1 - last] == after[after.length - 1 - last]) last++;
        const end = before.length - last;
        return [TextEdit.replace(
            { start: { line: first, character: 0 },
              end: end < before.length ? { line: end, character: 0 } : document.positionAt(text.length) },
            after.slice(first, after.length - last).map(l => l + "\n").join("").slice(0, end < before.length ? undefined : -1))];
    });
}
