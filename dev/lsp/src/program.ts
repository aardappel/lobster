import * as crypto from 'crypto';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';

// A file can use what its program declared before importing it, so on its own it may not
// compile: it is checked as part of the programs that import it, found by following the imports
// of the .lobster files around it.

// `import a.b`, `import .a.b`, `import "a/b.lobster"` and `import from "dir"`.
const IMPORT =
    /^[ \t]*import[ \t]+(?:from[ \t]+"([^"]*)"|"([^"]*)"|(\.?)([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*))/gm;

interface Imports {
    files: { name: string, relative: boolean }[],
    dirs: string[]
}

export interface Program {
    main: string,
    // Each of its files found, to the name the compiler gives it (relative to the main file's
    // directory, unless found elsewhere). Not the standard modules.
    files: Map<string, string>,
    // The directories it imports from, as in `import from "dir"`.
    dirs: string[]
}

function parseImports(source: string): Imports {
    const imports: Imports = { files: [], dirs: [] };
    for (const m of source.matchAll(IMPORT)) {
        if (m[1] !== undefined) imports.dirs.push(m[1]);
        else if (m[2] !== undefined) imports.files.push({ name: m[2], relative: false });
        else imports.files.push({ name: m[4].replace(/\./g, '/') + '.lobster', relative: m[3] == '.' });
    }
    return imports;
}

function isFile(file: string): boolean {
    return fs.statSync(file, { throwIfNoEntry: false })?.isFile() ?? false;
}

function inside(dir: string, file: string): boolean {
    const rel = path.relative(dir, file);
    return !rel.startsWith('..') && !path.isAbsolute(rel);
}

// The topmost directory above `file` that, like each one between, holds .lobster files, as a
// file opened on its own may be in a subdirectory of its program.
function projectDir(file: string): string {
    let dir = path.dirname(file);
    for (;;) {
        const up = path.dirname(dir);
        if (up == dir || up == os.homedir() || !fs.readdirSync(up).some(f => f.endsWith('.lobster'))) {
            return dir;
        }
        dir = up;
    }
}

export class Programs {
    // The Lobster checkout, with the standard modules/.
    lobsterDir = '';
    // The directories given with --import.
    importDirs: string[] = [];
    // The text of open documents, which may not be saved.
    private contents: (file: string) => string | undefined;

    private imports = new Map<string, { mtime: number, imports: Imports }>();
    private scans = new Map<string, string[]>();
    private cache = new Map<string, Program>();

    constructor(contents: (file: string) => string | undefined = () => undefined) {
        this.contents = contents;
    }

    // Where the compiler is, and what it is given with --import.
    configure(lobsterDir: string, importDirs: string[]) {
        if (lobsterDir == this.lobsterDir && importDirs.join('\n') == this.importDirs.join('\n')) return;
        this.lobsterDir = lobsterDir;
        this.importDirs = importDirs;
        this.cache.clear();
    }

    // After files were created, deleted or saved.
    rescan() {
        this.scans.clear();
        this.cache.clear();
    }

    // After a document changed, which may import other files.
    changed() {
        this.cache.clear();
    }

    // The lines of a file: an open document's text, else what is saved.
    lines(file: string): string[] {
        const text = this.contents(file) ?? (isFile(file) ? fs.readFileSync(file, 'utf8') : '');
        return text.split(/\r?\n/);
    }

    isModule(file: string): boolean {
        return !!this.lobsterDir && inside(path.join(this.lobsterDir, 'modules'), file);
    }

    private importsOf(file: string): Imports {
        const open = this.contents(file);
        if (open !== undefined) return parseImports(open);
        const mtime = fs.statSync(file).mtimeMs;
        const cached = this.imports.get(file);
        if (cached?.mtime == mtime) return cached.imports;
        const imports = parseImports(fs.readFileSync(file, 'utf8'));
        this.imports.set(file, { mtime, imports });
        return imports;
    }

    // The .lobster files under `dir`, but not in hidden directories or node_modules.
    private scan(dir: string): string[] {
        const cached = this.scans.get(dir);
        if (cached) return cached;
        const files: string[] = [];
        const walk = (d: string) => {
            for (const e of fs.readdirSync(d, { withFileTypes: true })) {
                const full = path.join(d, e.name);
                if (e.isDirectory()) {
                    if (!e.name.startsWith('.') && e.name != 'node_modules') walk(full);
                } else if (e.name.endsWith('.lobster')) {
                    files.push(full);
                }
            }
        };
        walk(dir);
        this.scans.set(dir, files);
        return files;
    }

    // Where the compiler looks for what `program` imports.
    private dataDirs(program: Program): string[] {
        return [path.dirname(program.main), this.lobsterDir, ...this.importDirs, ...program.dirs];
    }

    // The file a name the compiler gives refers to, as `std.lobster` does to modules/std.lobster,
    // unless it is `relative` to the file importing it.
    locate(program: Program, name: string, relative = false): string | undefined {
        if (path.isAbsolute(name)) return isFile(name) ? name : undefined;
        for (const [file, n] of program.files) if (n == name) return file;
        for (const prefix of relative ? [''] : ['modules', '']) {
            for (const dir of this.dataDirs(program)) {
                const file = path.join(dir, prefix, name);
                if (isFile(file)) return file;
            }
        }
        return undefined;
    }

    // The program compiled from `main`, as far as its imports can be followed.
    program(main: string): Program {
        const cached = this.cache.get(main);
        if (cached) return cached;
        const program: Program = { main, files: new Map([[main, path.basename(main)]]), dirs: [] };
        const todo = [main];
        while (todo.length) {
            const file = todo.pop()!;
            const imports = this.importsOf(file);
            for (const dir of imports.dirs) program.dirs.push(path.resolve(path.dirname(main), dir));
            for (const imp of imports.files) {
                // A relative import is named from the importing file's own name.
                const name = imp.relative
                    ? path.posix.join(path.posix.dirname(program.files.get(file)!), imp.name)
                    : imp.name;
                const found = this.locate(program, name, imp.relative);
                if (!found || program.files.has(found) || this.isModule(found)) continue;
                program.files.set(found, name);
                todo.push(found);
            }
        }
        this.cache.set(main, program);
        return program;
    }

    // The programs `file` is part of, largest first: those that import it that no other one
    // does, or else the file on its own. Looked for in `folders`, and in the file's project.
    programsFor(file: string, folders: string[]): Program[] {
        const dirs = [...new Set([...folders, projectDir(file)])];
        const including = dirs.filter(d => !dirs.some(o => o != d && inside(o, d)))
            .flatMap(d => this.scan(d))
            .filter(main => main != file)
            .map(main => this.program(main))
            .filter(p => p.files.has(file));
        if (!including.length) return [this.program(file)];
        const roots = including.filter(p => !including.some(o => o != p && o.files.has(p.main)));
        // Programs that import each other are all roots.
        return (roots.length ? roots : including).sort((a, b) => b.files.size - a.files.size);
    }

    private unsaved(file: string): boolean {
        const open = this.contents(file);
        return open !== undefined && (!isFile(file) || open != fs.readFileSync(file, 'utf8'));
    }

    // Where to compile `program` from. The compiler runs in `cwd` and looks in `imports` after
    // it, so for a program with unsaved files that is a directory in `tempDir` with just those,
    // where they are in the program; `real` gives the file a path in it stands for.
    stage(program: Program, tempDir: string):
        { cwd: string, main: string, imports: string[], real: (file: string) => string } {
        const mainDir = path.dirname(program.main);
        const unsaved = [...program.files.keys()].filter(file => this.unsaved(file));
        const main = path.basename(program.main);
        if (!unsaved.length) return { cwd: mainDir, main, imports: [], real: file => file };
        const root = path.join(tempDir, 'programs',
                               crypto.createHash('sha1').update(mainDir).digest('hex'));
        fs.rmSync(root, { recursive: true, force: true });
        // FIXME: the unsaved text of a file outside the main file's directory isn't seen.
        for (const file of unsaved.filter(f => inside(mainDir, f))) {
            const to = path.join(root, path.relative(mainDir, file));
            fs.mkdirSync(path.dirname(to), { recursive: true });
            fs.writeFileSync(to, this.contents(file)!);
        }
        return {
            cwd: root,
            main,
            imports: [mainDir, ...program.dirs],
            real: file => inside(root, file) ? path.join(mainDir, path.relative(root, file)) : file
        };
    }
}
