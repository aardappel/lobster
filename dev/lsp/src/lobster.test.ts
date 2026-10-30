import { assert } from 'console';
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
import { locationsOf } from './features/references';
import * as fs from 'fs';
import { docComment } from './utils';

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
