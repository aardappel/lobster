import * as fs from 'fs';
import { getOrSet } from './utils';

// A program's definitions and where each is used, from the compiler's --index (see
// dev/src/lobster/index.h), with files absolute and lines zero based.

export type DefKind = 'function' | 'lambda' | 'var' | 'class' | 'struct' | 'field' | 'enum' | 'enumval';

export interface Def {
    kind: DefKind,
    name: string,
    file: string,
    line: number,
    // The last line of a function's or lambda's body.
    end?: number,
    // A function's signature, or the type of a variable or field.
    type?: string,
    // The class of a field or method, the function or lambda of a local, the enum of a value.
    container?: Def,
    uses: { file: string, line: number }[]
}

interface IndexJson {
    files: string[],
    defs: { k: DefKind, n: string, f: number, l: number, e?: number, t?: string, c?: number }[],
    refs: [number, number, number][]
}

export class ProgramIndex {
    readonly defs: Def[];
    readonly files: Set<string>;
    // The definitions used or declared on each line, by `${file}:${line}`.
    private byLine = new Map<string, Def[]>();
    private byKey: Map<string, Def>;

    // `locate` gives the file a name the compiler gives is.
    constructor(json: IndexJson, locate: (name: string) => string) {
        const files = json.files.map(locate);
        this.files = new Set(files);
        this.defs = json.defs.map(d => ({
            kind: d.k,
            name: d.n,
            file: files[d.f],
            line: d.l - 1,
            end: d.e === undefined ? undefined : d.e - 1,
            type: d.t,
            uses: []
        }));
        json.defs.forEach((d, i) => {
            if (d.c !== undefined) this.defs[i].container = this.defs[d.c];
        });
        for (const [f, l, d] of json.refs) {
            const def = this.defs[d];
            const use = { file: files[f], line: l - 1 };
            if (use.file != def.file || use.line != def.line) def.uses.push(use);
        }
        this.byKey = new Map(this.defs.map(d => [defKey(d), d]));
        this.place();
    }

    // The index the compiler wrote to `file`, if it got that far.
    static read(file: string, locate: (name: string) => string): ProgramIndex | undefined {
        if (!fs.existsSync(file)) return undefined;
        return new ProgramIndex(JSON.parse(fs.readFileSync(file, 'utf8')), locate);
    }

    // Moves what is on the lines of `file` to the lines `move` gives, as after an edit.
    shift(file: string, move: (line: number) => number) {
        for (const def of this.defs) {
            if (def.file == file) {
                def.line = move(def.line);
                if (def.end !== undefined) def.end = move(def.end);
            }
            for (const use of def.uses) if (use.file == file) use.line = move(use.line);
        }
        this.place();
    }

    private place() {
        this.byLine.clear();
        const at = (file: string, line: number) => getOrSet(this.byLine, `${file}:${line}`, () => []);
        for (const def of this.defs) {
            at(def.file, def.line).push(def);
            for (const use of def.uses) at(use.file, use.line).push(def);
        }
    }

    // The definition with the same key (see defKey) in this index.
    find(key: string): Def | undefined {
        return this.byKey.get(key);
    }

    // The definitions used or declared on a line.
    on(file: string, line: number): Def[] {
        return this.byLine.get(`${file}:${line}`) ?? [];
    }
}

// The name a definition is written with: functions in a namespace are named `ns.f`, but
// declared and often used as `f`.
export function shortName(def: Def): string {
    return def.name.substring(def.name.lastIndexOf('.') + 1);
}

// Identifies a definition across the indexes of different programs.
export function defKey(def: Def): string {
    return `${def.kind} ${def.file} ${def.line} ${def.name}`;
}
