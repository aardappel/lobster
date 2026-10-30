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

const settings: LobsterSettings = {
	executable: path.join('..', '..', 'bin', 'lobster'),
	imports: [],
	experimental: true
};

console.log("Parsing valid.lobster");
parseLobster(settings, 'testFixture/valid.lobster')
	.then(result => assert(result.messages.length == 0 && result.other == ""));

console.log("Parsing invalid.lobster");
parseLobster(settings, 'testFixture/invalid.lobster')
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
	parseLobster(settings, found[0].main)
		.then(result => assert(result.messages.length == 0 && result.other == "",
			"Program doesn't compile?", result));
	const editing = new Programs(file => file == helper ? "def helper(): return shared + 2\n" : undefined);
	const staged = editing.stage(found[0], path.resolve('out', 'staged'));
	parseLobster(settings, staged.main, staged)
		.then(result => assert(result.messages.length == 0 && result.other == "",
			"Staged program doesn't compile?", result));
	queryDefinition(settings, staged.main, 'sub/helper.lobster', 0, 'shared', staged)
		.then(result => assert(staged.real(path.resolve(staged.cwd, result.declartionLocation?.file ?? ""))
			== found[0].main, "Not found in the real main file?", result));
}
