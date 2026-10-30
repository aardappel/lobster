import { Diagnostic, DiagnosticSeverity } from "vscode-languageserver";
import * as path from 'path';
import { toDiagnostics } from "./diagnostics";
import { QueryDefinitionResult, parseLobster, queryDefinition } from "./lobster";
import { LobsterSettings } from "./lsp";
import { Program, Programs } from "./program";
import { memo } from "./utils";

// What a program last compiled said: its diagnostics, by file.
interface Compiled {
    program: Program,
    diagnostics: Map<string, Diagnostic[]>
}

// The same diagnostic, from different programs.
function key(d: Diagnostic): string {
    return `${d.range.start.line} ${d.severity} ${d.message}`;
}

// With errors the compiler stops warning, so what it said `before` stays, moved along with the
// edits since (see Checker.shift).
function keepWarnings(diagnostics: Map<string, Diagnostic[]>, before: Map<string, Diagnostic[]>) {
    for (const [file, old] of before) {
        const now = diagnostics.get(file) ?? [];
        const said = new Set(now.map(key));
        const kept = old.filter(d => d.severity != DiagnosticSeverity.Error && !said.has(key(d)));
        if (kept.length) diagnostics.set(file, [...now, ...kept]);
    }
}

// Compiles programs, and gives what they say to `publish`, for each of their files.
export class Checker {
    private programs: Programs;
    private tempDir: string;
    private publishDiagnostics: (file: string, diagnostics: Diagnostic[]) => void;
    // By main file.
    private compiled = new Map<string, Compiled>();
    // Of each file checked to the end, all the programs it is part of, by their main file.
    private checked = new Map<string, string[]>();
    // Compiles run one at a time, as programs share files.
    private queue: Promise<unknown> = Promise.resolve();
    // Counts the checks of each file, so a newer one stops an older one.
    private generations = new Map<string, number>();

    constructor(programs: Programs, tempDir: string,
                publish: (file: string, diagnostics: Diagnostic[]) => void) {
        this.programs = programs;
        this.tempDir = tempDir;
        this.publishDiagnostics = publish;
    }

    // Compiles `programs`, those `file` is part of. A newer check of the file stops it after the
    // program it is compiling.
    async check(file: string, programs: Program[], settings: LobsterSettings) {
        const generation = (this.generations.get(file) ?? 0) + 1;
        this.generations.set(file, generation);
        // What programs that no longer include the file said, such as the file on its own before
        // something imported it, is out of date.
        for (const [main, { program }] of this.compiled) {
            if (program.files.has(file) && !programs.some(p => p.main == main)) this.forget(main);
        }
        for (const [i, program] of programs.entries()) {
            if (i && this.generations.get(file) != generation) return;
            await this.enqueue(() => this.compile(program, settings));
        }
        this.checked.set(file, programs.map(p => p.main));
        this.publish(file);
    }

    // What `identifier` at `line` of `file` refers to, in `program`.
    query(program: Program, file: string, line: number, identifier: string,
          settings: LobsterSettings): Promise<QueryDefinitionResult> {
        return this.enqueue(async () => {
            const staged = this.programs.stage(program, this.tempDir);
            const result = await queryDefinition(settings, staged.main, program.files.get(file)!, line,
                                                 identifier, staged);
            const location = result.declartionLocation;
            if (location) location.file = staged.real(path.resolve(staged.cwd, location.file));
            return result;
        });
    }

    // Moves what was said about a file along with an edit that replaced its lines `start` to `end`
    // with ones with `added` line breaks, dropping the diagnostics on lines it removed.
    shift(file: string, start: number, end: number, added: number) {
        const delta = added - (end - start);
        if (!delta) return;
        const removed = (line: number) => delta < 0 && start < line && line <= end;
        const move = (line: number) => line <= start ? line : removed(line) ? start : line + delta;
        for (const { diagnostics } of this.compiled.values()) {
            const old = diagnostics.get(file);
            if (!old) continue;
            diagnostics.set(file, old.filter(d => !removed(d.range.start.line)).map(d => ({
                ...d,
                range: { start: { ...d.range.start, line: move(d.range.start.line) },
                         end: { ...d.range.end, line: move(d.range.end.line) } }
            })));
        }
    }

    // Runs `job` once the compiles before it are done.
    private enqueue<T>(job: () => Promise<T>): Promise<T> {
        const result = this.queue.then(job);
        this.queue = result.catch(() => undefined);
        return result;
    }

    private forget(main: string) {
        const files = this.compiled.get(main)!.diagnostics.keys();
        this.compiled.delete(main);
        for (const file of files) this.publish(file);
    }

    private async compile(program: Program, settings: LobsterSettings) {
        const staged = this.programs.stage(program, this.tempDir);
        const output = await parseLobster(settings, staged.main, staged);
        const locate = (name: string) =>
            staged.real(this.programs.locate(program, name) ?? path.join(staged.cwd, name));
        const diagnostics = toDiagnostics(output, program, locate, memo(file => this.programs.lines(file)));
        const before = this.compiled.get(program.main);
        if (output.other || output.messages.some(m => m.severity == DiagnosticSeverity.Error)) {
            keepWarnings(diagnostics, before?.diagnostics ?? new Map());
        }
        this.compiled.set(program.main, { program, diagnostics });
        for (const file of new Set([...before?.diagnostics.keys() ?? [], ...diagnostics.keys()])) {
            this.publish(file);
        }
    }

    // Publishes a file's diagnostics, from all programs it is part of. What one program doesn't
    // use, another may: that is only shown for a file all of whose programs were compiled, when
    // all of them say so (which they don't with errors).
    private publish(file: string) {
        const shown = new Map<string, Diagnostic>();
        for (const { diagnostics } of this.compiled.values()) {
            for (const d of diagnostics.get(file) ?? []) {
                if (d.severity != DiagnosticSeverity.Hint || this.unusedInAll(file, d)) shown.set(key(d), d);
            }
        }
        this.publishDiagnostics(file, [...shown.values()]);
    }

    private unusedInAll(file: string, hint: Diagnostic): boolean {
        const mains = this.checked.get(file) ?? [];
        return mains.length > 0 && mains.every(main =>
            this.compiled.get(main)?.diagnostics.get(file)?.some(d => key(d) == key(hint)));
    }
}
