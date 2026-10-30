import * as fs from 'fs';
import * as path from 'path';
import { LobsterLocation } from './lobster';

// Where the builtins are, in the C++ sources of a Lobster checkout: each file defines the
// BUILTIN_GROUP its BUILTIN(name, ...) go in, which RegisterBuiltin(nfr, "gl", ...) puts in a
// namespace.

// The name a builtin-defining macro gives: BUILTIN(name, ...), BUILTIN_OUTS(name, ...),
// BUILTIN_CODEGEN(codegen, name, ...), and otherwise the first string, as in
// VECTOROPVM1234(sym, "name", ...).
function builtinName(line: string): string | undefined {
    const m = line.match(/^([A-Z][A-Z0-9_]*)\((.*)/);
    if (!m) return undefined;
    const args = m[2].split(',').map(a => a.trim());
    if (m[1] == 'BUILTIN' || m[1] == 'BUILTIN_OUTS') return args[0].match(/^\w+$/)?.[0];
    if (m[1] == 'BUILTIN_CODEGEN') return args[1]?.match(/^\w+$/)?.[0];
    return m[2].match(/"(\w+)"/)?.[1];
}

function buildIndex(srcDir: string): Map<string, LobsterLocation> {
    const index = new Map<string, LobsterLocation>();
    // A checkout without its sources.
    if (!fs.existsSync(srcDir)) return index;
    const sources = fs.readdirSync(srcDir).filter(f => f.endsWith('.cpp')).map(f => ({
        file: path.join(srcDir, f),
        lines: fs.readFileSync(path.join(srcDir, f), 'utf8').split('\n')
    }));
    const namespaces = new Map<string, string>();
    for (const { lines } of sources) {
        for (const line of lines) {
            const m = line.match(/RegisterBuiltin\(\s*\w+\s*,\s*"([^"]*)"\s*,\s*"[^"]*"\s*,\s*(\w+)\s*\)/);
            if (m) namespaces.set(m[2], m[1]);
        }
    }
    for (const { file, lines } of sources) {
        let ns: string | undefined;
        lines.forEach((line, i) => {
            const group = line.match(/^\s*#define\s+BUILTIN_GROUP\s+(\w+)/);
            if (group) ns = namespaces.get(group[1]);
            const builtin = ns !== undefined && builtinName(line);
            if (builtin) {
                const name = ns ? `${ns}.${builtin}` : builtin;
                if (!index.has(name)) index.set(name, { file, line: i });
            }
        });
    }
    return index;
}

const indexes = new Map<string, Map<string, LobsterLocation>>();

// The C++ definition of builtin `name`, as in `gl.read_texture`.
export function findNative(lobsterDir: string, name: string): LobsterLocation | undefined {
    const srcDir = path.join(lobsterDir, 'dev', 'src');
    let index = indexes.get(srcDir);
    if (!index) indexes.set(srcDir, index = buildIndex(srcDir));
    return index.get(name);
}
