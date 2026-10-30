import {
	LobsterFunctionSignature,
	parseLobster,
	queryDefinition,
	splitList
} from './lobster';
import * as path from 'path';
import { LobsterSettings } from './lsp';
import { parseMessages } from './lobster';
import { Programs } from './program';
import { findNative } from './natives';
import { ProgramIndex } from './index';
import { locationsOf, resolve } from './features/references';
import * as fs from 'fs';
import { docComment, occurrences, wordAt } from './utils';
import { toDiagnostics } from './diagnostics';
import { callAt } from './features/signature';
import { programItems } from './features/completion';
import { DiagnosticSeverity, DiagnosticTag } from 'vscode-languageserver';

// Unlike console.assert, which only logs, a failed assertion also fails the run.
function assert(condition: unknown, ...data: unknown[]) {
	if (condition) return;
	console.error("Assertion failed:", ...data);
	process.exitCode = 1;
}

const settings: LobsterSettings = {
	executable: path.join('..', '..', 'bin', 'lobster'),
	imports: [],
	experimental: true
};

console.log("Parsing valid.lobster");
parseLobster(settings, 'testFixture/valid.lobster', path.resolve('out', 'valid-index.json'))
	.then(result => assert(result.messages.length == 0 && result.other == ""));

console.log("Parsing invalid.lobster");
parseLobster(settings, 'testFixture/invalid.lobster', path.resolve('out', 'invalid-index.json'))
	.then(({ messages: result }) => {
		assert(result.length == 2, "Incorrect amount of errors?", result);

		assert(result[0].message == "variable a is constant",
			"Message is not the same?", result);
		assert(result[0].line == 13, "Incorrect line?", result[0].line);
		assert(result[1].line == 17, "Incorrect line?", result[1].line);
	});

console.log("Querying valid.lobster");
queryDefinition(settings, 'testFixture/valid.lobster', 'valid.lobster', 5, 'b')
	.then(result => {
		assert(result.declartionLocation,
			"No declaration location?", result);
		assert(result.declartionLocation?.file == path.join('testFixture', 'valid.lobster'),
			"Incorrect file?", result);
		assert(result.declartionLocation?.line == 2,
			"Incorrect line?", result);

		assert(result.signature,
			"No signature?", result);

		const signature = result.signature as LobsterFunctionSignature;
		assert(signature.name == 'b',
			"Incorrect name for signature?", signature);
		assert(signature.parameters.length == 0,
			"Incorrect amount of parameters?", signature);
	});

console.log("Querying builtin valid.lobster");
queryDefinition(settings, 'testFixture/valid.lobster', 'valid.lobster', 12, 'print')
	.then(result => {
		assert(result.declartionLocation == null,
			"Has declaration location?", result);
		assert(result.signature,
			"No signature?", result);

		const signature = result.signature as LobsterFunctionSignature;
		assert(signature.name == 'print',
			"Incorrect name for signature?", result);
		assert(signature.parameters.length == 1,
			"Incorrect amount of parameters?", result);
		assert(signature.parameters[0].name == 'x',
			"Incorrect name for parameter?", signature.parameters[0]);
		assert(signature.parameters[0].type == 'string',
			"Incorrect type for parameter?", signature.parameters[0]);
	});

console.log("Splitting signatures");
assert(splitList("d:dictionary<K, V>, f:function(x:int, y:int) -> int").length == 2,
	"Split inside brackets?");
assert(splitList("f(g:function(x:int) -> int) -> [int]", "->")[1] == " [int]",
	"Split at an inner ->?");

console.log("Parsing messages");
{
	const messages = parseMessages(
		"lib/a.lobster(3): warning: unused variable `x`\n" +
		"b.lobster(10): error: f (1st argument) requires type `int`, got `string`\n" +
		"  in main.lobster(5): g() -> int\n");
	assert(messages.length == 2, "Incorrect amount of messages?", messages);
	assert(messages[0].file == "lib/a.lobster" && messages[0].line == 2, "Incorrect location?", messages[0]);
	assert(messages[1].context.length == 1 && messages[1].context[0].file == "main.lobster" &&
		messages[1].context[0].line == 4, "Incorrect context?", messages[1]);
}

console.log("Finding programs");
{
	const programs = new Programs();
	const helper = path.resolve('testFixture/program/sub/helper.lobster');
	const found = programs.programsFor(helper, [path.resolve('testFixture')]);
	assert(found.length == 1 && found[0].main == path.resolve('testFixture/program/main.lobster'),
		"Not found as part of its program?", found.map(p => p.main));
	assert(found[0].files.get(helper) == "sub/helper.lobster", "Incorrect name?", found[0].files);
	parseLobster(settings, found[0].main, path.resolve('out', 'program-index.json'))
		.then(result => assert(result.messages.length == 0 && result.other == "",
			"Program doesn't compile?", result));
	const editing = new Programs(file => file == helper ? "def helper(): return shared + 2\n" : undefined);
	const staged = editing.stage(found[0], path.resolve('out', 'staged'));
	parseLobster(settings, staged.main, path.resolve('out', 'staged-index.json'), staged)
		.then(result => assert(result.messages.length == 0 && result.other == "",
			"Staged program doesn't compile?", result));
	queryDefinition(settings, staged.main, 'sub/helper.lobster', 0, 'shared', staged)
		.then(result => assert(staged.real(path.resolve(staged.cwd, result.declartionLocation?.file ?? ""))
			== found[0].main, "Not found in the real main file?", result));
}

console.log("Finding builtins");
{
	const location = findNative(path.resolve('..', '..'), 'gl.window');
	assert(location?.file.endsWith(path.join('dev', 'src', 'graphics.cpp')), "Not found?", location);
	assert(findNative(path.resolve('..', '..'), 'print')?.file.endsWith('builtins.cpp'), "Not found?");
}

console.log("Finding references");
{
	const file = path.resolve('testFixture/references.lobster');
	const indexFile = path.resolve('out', 'references-index.json');
	parseLobster(settings, file, indexFile).then(() => {
		const programIndex = ProgramIndex.read(indexFile, name => path.resolve('testFixture', name))!;
		const lines = (f: string) => fs.readFileSync(f, 'utf8').split('\n');
		const at = (kind: string, name: string) =>
			locationsOf(programIndex.defs.find(d => d.kind == kind && d.name == name)!, [programIndex], lines)
				.map(l => `${l.location.range.start.line}:${l.location.range.start.character}`);
		assert(at('field', 'count').join() == "1:4,3:8,11:8", "Field uses?", at('field', 'count'));
		assert(at('enumval', 'green').join() == "7:4,12:6", "Enum value uses?", at('enumval', 'green'));
		assert(at('function', 'bump').join() == "2:8,10:2", "Method uses?", at('function', 'bump'));
	});
}

console.log("Reading doc comments");
{
	const lines = fs.readFileSync('testFixture/doc.lobster', 'utf8').split(/\r?\n/);
	const expected: [number, string, string | undefined][] = [
		[18, 'inc', "Returns x plus one.\nNever less."],
		[19, 'count', "The count so far."],
		[20, 'hidden', "Only for this file."],
		[21, 'bare', undefined],
	];
	for (const [line, name, doc] of expected) {
		queryDefinition(settings, 'testFixture/doc.lobster', 'doc.lobster', line, name)
			.then(result => {
				const location = result.declartionLocation;
				assert(location, "No declaration location?", name, result);
				const found = location && docComment(lines, location.line);
				assert(found == doc, "Incorrect doc comment?", name, found);
			});
	}
}

console.log("Making diagnostics");
{
	const lines: { [file: string]: string[] } = {
		"/p/main.lobster": ["import lib", "def unused_fn(): return 1", "    let x = 1", "print helper(unknown)"],
		"/p/lib.lobster": ["def helper(a): return a"],
		"/l/modules/std.lobster": ["def map(xs, f): return f(xs)"],
	};
	const locate = (name: string) => name.startsWith("std") ? `/l/modules/${name}` : `/p/${name}`;
	const messages = parseMessages(
		"main.lobster(2): warning: unused function `unused_fn`\n" +
		"main.lobster(3): warning: unused variable `x`\n" +
		"main.lobster(4): error: unknown identifier `unknown`\n" +
		"std.lobster(1): error: `f` requires type `int`\n" +
		"  in main.lobster(4): helper(a)\n");
	const program = {
		main: "/p/main.lobster",
		files: new Map([["/p/main.lobster", "main.lobster"], ["/p/lib.lobster", "lib.lobster"]]),
		dirs: []
	};
	const found = toDiagnostics({ messages, other: "" }, program, locate, file => lines[file] ?? []);
	const main = found.get("/p/main.lobster")!;
	const fn = main.find(d => d.message.startsWith("unused function"))!;
	assert(fn.severity == DiagnosticSeverity.Hint && fn.tags?.includes(DiagnosticTag.Unnecessary),
		"Unused function not faded?", fn);
	assert(fn.range.start.character == 4 && fn.range.end.character == 13, "Not the quoted name?", fn.range);
	const x = main.find(d => d.message.startsWith("unused variable"))!;
	assert(x.severity == DiagnosticSeverity.Warning && x.tags?.includes(DiagnosticTag.Unnecessary),
		"Unused variable not a faded warning?", x);
	const unknown = main.find(d => d.message.startsWith("unknown identifier"))!;
	assert(unknown.range.start.character == 13, "Not the quoted name?", unknown.range);
	assert(main.some(d => d.range.start.line == 3 && d.message.includes("in std.lobster(1)")),
		"Module error not shown where the program got to it?", main);
	assert(found.get("/l/modules/std.lobster")?.length == 1, "Module error not in the module?", found);
	const other = toDiagnostics({ messages: [], other: "can't open file: main.lobster" }, program, locate,
		() => []);
	assert(other.get("/p/main.lobster")?.[0].range.start.line == 0, "Other output not on the main file?", other);
	const quoted = toDiagnostics({ messages: parseMessages("main.lobster(1): warning: unused variable `x`\n"), other: "" },
		program, locate, () => ['let s = "x" + y; let x = 1']);
	assert(quoted.get("/p/main.lobster")?.[0].range.start.character == 21, "Underlined in a string?", quoted);
}

console.log("Resolving names");
{
	// class point: x:int / def len(): return x / let x = 1 / print p.x + x / let c:point
	const index = new ProgramIndex({
		files: ["a.lobster"],
		defs: [
			{ k: "class", n: "point", f: 0, l: 1 },
			{ k: "field", n: "x", f: 0, l: 2, t: "int", c: 0 },
			{ k: "function", n: "len", f: 0, l: 3, e: 3, t: "len(this:point) -> int", c: 0 },
			{ k: "var", n: "x", f: 0, l: 4, t: "int" },
		],
		refs: [[0, 3, 1], [0, 5, 1], [0, 5, 3]]
	}, name => "/" + name);
	const at = (line: number, text: string, character: number) =>
		resolve(index, "/a.lobster", line, text, character);
	assert(at(4, "print p.x + x", 8)?.kind == "field", "Not the field after a dot?");
	assert(at(4, "print p.x + x", 12)?.kind == "var", "Not the variable without a dot?");
	assert(at(2, "    def len(): return x", 22)?.kind == "field", "Not the field used bare in a method?");
	assert(at(6, "let c:point", 7)?.kind == "class", "Not the type in an annotation?");
	assert(at(4, "print p.x + x", 3) === undefined, "Resolved what isn't defined?");
	index.shift("/a.lobster", line => line >= 4 ? line + 1 : line);
	assert(at(5, "print p.x + x", 12)?.kind == "var", "Not moved along with an edit?");
}

console.log("Finding the call being typed");
{
	const call = (text: string) => callAt(text, text.length);
	assert(call("f(a, g(b, c), ")?.commas == 2, "Commas outside the inner call?", call("f(a, g(b, c), "));
	assert(call("f(\"a, b\", ")?.commas == 1, "Commas in a string?", call("f(\"a, b\", "));
	assert(call("f('\\'', ")?.commas == 1, "Escaped quote?", call("f('\\'', "));
	assert(call("let v = [ a, ") === undefined, "A vector isn't a call?");
	assert(call("f(x) + y") === undefined, "Outside a call?");
	assert(call("f(a, // don't (\nb, ")?.commas == 2, "Brackets in a comment?", call("f(a, // don't (\nb, "));
}

console.log("Completing what the program declares");
{
	// class point: x:int / def outer(p:point): let inner = 1 ... / let global = 1
	const index = new ProgramIndex({
		files: ["a.lobster"],
		defs: [
			{ k: "class", n: "point", f: 0, l: 1 },
			{ k: "field", n: "x", f: 0, l: 2, t: "int", c: 0 },
			{ k: "function", n: "outer", f: 0, l: 3, e: 6, t: "outer(p:point)" },
			{ k: "var", n: "p", f: 0, l: 3, t: "point", c: 2 },
			{ k: "var", n: "inner", f: 0, l: 4, t: "int", c: 2 },
			{ k: "var", n: "global", f: 0, l: 8, t: "int" },
			{ k: "function", n: "unrelated", f: 0, l: 9, e: 9, t: "unrelated(s:string)" },
		],
		refs: []
	}, name => "/" + name);
	const labels = (line: number, typed: string) =>
		programItems([index], "/a.lobster", { line, character: 20 }, typed, () => []).map(i => i.label).sort();
	// A function whose first parameter is a point is a method of it.
	assert(labels(4, "p.").join() == "outer,x", "Fields and methods of the receiver's class?", labels(4, "p."));
	assert(labels(4, "in").join() == "inner", "Locals in scope?", labels(4, "in"));
	assert(labels(8, "in").join() == "", "Locals out of scope?", labels(8, "in"));
	assert(labels(8, "o").join() == "outer", "Functions?", labels(8, "o"));
}

console.log("Finding words");
{
	assert(occurrences('x = "x" + x.x // x', "x").length == 3, "Not outside strings and comments?");
	assert(occurrences("a.b + b", "b", true).join() == "2,3", "Not after a dot?");
	assert(occurrences("a..b", "b", true).length == 0, "A range isn't a dot?");
	assert(wordAt("foo.bar", 5)?.word == "bar" && wordAt("foo.bar", 5)?.dot, "Word after a dot?");
	assert(docComment(["// a", "", "def f():"], 2) === undefined, "A blank line ends a description?");
	assert(docComment(["    // a", "    //b", "    def f():"], 2) == "a\nb", "Indented description?");
}

console.log("Warning about what nothing uses");
{
	const dir = path.resolve('testFixture/unused');
	const warnings = (main: string) =>
		parseLobster(settings, path.join(dir, main), path.resolve('out', `unused-${main}.json`))
			.then(r => r.messages.map(m => `${m.file}:${m.line + 1}: ${m.message}`).sort());
	warnings('functions.lobster').then(w => assert(w.join("\n") ==
		"functions.lobster:3: unused global `never`\n" +
		"functions.lobster:6: unused function `uncalled`", "Unused functions and globals?", w));
	warnings('imports.lobster').then(w => assert(w.join("\n") ==
		"imports.lobster:2: unused import of `unused.lobster`: nothing it declares is used\n" +
		"unused.lobster:1: unused function `unused_fn`", "Unused imports?", w));
	// The import is used, by a function value nothing calls.
	warnings('lambda.lobster').then(w => assert(!w.some(m => m.includes("unused import")),
		"Import used by an uncalled function value?", w));
}

console.log("Querying functions and their parameters");
{
	const main = path.resolve('testFixture/unused/functions.lobster');
	const query = (line: number, name: string) =>
		queryDefinition(settings, main, 'functions.lobster', line, name);
	query(13, 'uncalled').then(r => assert(r.declartionLocation?.line == 5 &&
		(r.signature as LobsterFunctionSignature).parameters.map(p => `${p.name}:${p.type}`).join() == "b:any,c:string",
		"Declared signature, at the def?", r));
	query(13, 'called').then(r => assert((r.signature as LobsterFunctionSignature).returns?.join() == "int",
		"Signature of a called function?", r));
	query(4, 'a').then(r => assert(r.signature && 'type' in r.signature && r.signature.type == "int",
		"Parameter of a called function?", r));
	query(6, 'c').then(r => assert(r.declartionLocation?.line == 6, "Parameter of an uncalled function?", r));
}

console.log("Indexing definitions");
{
	const dir = path.resolve('testFixture/unused');
	const indexFile = path.resolve('out', 'functions-index.json');
	parseLobster(settings, path.join(dir, 'functions.lobster'), indexFile).then(() => {
		const programIndex = ProgramIndex.read(indexFile, name => path.join(dir, name))!;
		const def = (kind: string, name: string) => programIndex.defs.find(d => d.kind == kind && d.name == name);
		assert(def('function', 'called')?.type == "called(a:int) -> int", "Declared signature?", def('function', 'called'));
		assert(def('function', 'uncalled')?.line == 5 && def('function', 'uncalled')?.end == 7,
			"Lines of a function?", def('function', 'uncalled'));
		assert(def('var', 'shown')?.uses.some(u => u.line == 4), "Use of a global?", def('var', 'shown'));
		assert(def('enumval', 'high')?.line == 11 && def('enumval', 'high')?.container?.name == "level",
			"Enum value's line and enum?", def('enumval', 'high'));
		assert(def('var', 'c')?.container?.name == "uncalled", "Local of its function?", def('var', 'c'));
	});
}
