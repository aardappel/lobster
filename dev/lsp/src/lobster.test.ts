import { assert } from 'console';
import {
	LobsterFunctionSignature,
	parseLobster,
	queryDefinition
} from './lobster';
import * as path from 'path';
import { LobsterSettings } from './lsp';

const settings: LobsterSettings = {
	executable: path.join('..', '..', 'bin', 'lobster'),
	imports: [],
	experimental: true
};

console.log("Parsing valid.lobster");
parseLobster(settings, 'testFixture/valid.lobster')
	.then(result => assert(result.length == 0));

console.log("Parsing invalid.lobster");
parseLobster(settings, 'testFixture/invalid.lobster')
	.then(result => {
		assert(result.length == 1);

		assert(result[0].message == "variable a is constant",
			"Message is not the same?", result);
		assert(result[0].range.start.line == 13,
			"Incorrect line?", result[0].range.start.line);
		assert(result[0].range.end.line == 13,
			"Incorrect line?", result[0].range.end.line);
	});

console.log("Querying valid.lobster");
queryDefinition(settings, 'testFixture/valid.lobster', 5, 'b')
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
queryDefinition(settings, 'testFixture/valid.lobster', 12, 'print')
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