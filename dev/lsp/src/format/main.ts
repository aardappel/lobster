// lobsterfmt: formats Lobster source files, like gofmt.
//
//     lobsterfmt [-l] [-w] [path ...]
//
// Without paths it formats standard input to standard output. A directory stands for the
// .lobster files in it, recursively. By default the formatted files are printed:
//     -l  list the files whose formatting differs instead
//     -w  write the result to the files instead
// Exits with 2 if a file can't be formatted (it doesn't lex, or indents inconsistently).

import * as fs from "fs";
import * as path from "path";
import { format, FormatError, position } from "./format";

function files(p: string): string[] {
    if (!fs.statSync(p).isDirectory()) return [p];
    return fs.readdirSync(p, { withFileTypes: true })
        .filter(e => e.isDirectory() ? !e.name.startsWith(".") : e.name.endsWith(".lobster"))
        .flatMap(e => files(path.join(p, e.name)));
}

function main(args: string[]): number {
    const list = args.includes("-l"), write = args.includes("-w");
    const paths = args.filter(a => a != "-l" && a != "-w");
    const bad = paths.find(a => a.startsWith("-"));
    if (bad) {
        process.stderr.write(`unknown flag ${bad}\nusage: lobsterfmt [-l] [-w] [path ...]\n`);
        return 2;
    }
    if (!paths.length) {
        const src = fs.readFileSync(0, "utf8");
        try {
            process.stdout.write(format(src));
        } catch (e) {
            return report("<standard input>", src, e);
        }
        return 0;
    }
    let status = 0;
    for (const file of paths.flatMap(p => {
        try {
            return files(p);
        } catch (e) {
            process.stderr.write(`${p}: ${e instanceof Error ? e.message : e}\n`);
            status = 2;
            return [];
        }
    })) {
        const src = fs.readFileSync(file, "utf8");
        let result: string;
        try {
            result = format(src);
        } catch (e) {
            status = report(file, src, e);
            continue;
        }
        if (list && result != src) process.stdout.write(file + "\n");
        if (write && result != src) fs.writeFileSync(file, result);
        if (!list && !write) process.stdout.write(result);
    }
    return status;
}

function report(file: string, src: string, e: unknown): number {
    if (!(e instanceof FormatError)) throw e;
    const { line, character } = position(src, e.offset);
    process.stderr.write(`${file}:${line + 1}:${character + 1}: ${e.message}\n`);
    return 2;
}

process.exitCode = main(process.argv.slice(2));
