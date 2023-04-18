import { assert } from 'console';
import { LobsterFunctionSignarture, LobsterSettings, parseLobster, queryDefinition }  from './lobster';

const settings: LobsterSettings = {
	executable: "D:\\Clones\\lobster\\bin\\lobster.exe", // HAS TO BE CHANGED TO TEST
	imports: []
};


console.log("Parsing valid.lobster");
parseLobster(settings, 'server\\testFixture\\valid.lobster')
	.then(result => assert(result.length == 0));

console.log("Parsing invalid.lobster");
parseLobster(settings, 'server\\testFixture\\invalid.lobster')
	.then(result => {
		assert(result.length == 1);

		assert(result[0].message == "variable a is constant", "Message is not the same?", result);
		assert(result[0].range.start.line == 13, "Incorrect line?", result[0].range.start.line);
		assert(result[0].range.end.line == 13, "Incorrect line?", result[0].range.end.line);
	});

console.log("Querying valid.lobster");
queryDefinition(settings, 'server\\testFixture\\valid.lobster', 5, 'b')
	.then(result => {
		assert(result.declartionLocation, "No declaration location?", result);
		assert(result.declartionLocation?.file == 'server\\testFixture\\valid.lobster', "Incorrect file?", result);
		assert(result.declartionLocation?.line == 2, "Incorrect line?", result);

		assert(result.signature, "No signature?", result);

		const signature = result.signature as LobsterFunctionSignarture;
		assert(signature.name == 'b', "Incorrect name for signature?", signature);
		assert(signature.parameters.length == 0, "Incorrect amount of parameters?", signature);
	});

console.log("Querying builtin valid.lobster");
queryDefinition(settings, 'server\\testFixture\\valid.lobster', 12, 'print')
	.then(result => {
		assert(result.declartionLocation == null, "Has declaration location?", result);
		assert(result.signature, "No signature?", result);

		const signature = result.signature as LobsterFunctionSignarture;
		assert(signature.name == 'print', "Incorrect name for signature?", result);
		assert(signature.parameters.length == 1, "Incorrect amount of parameters?", result);
		assert(signature.parameters[0].name == 'x', "Incorrect name for parameter?", signature.parameters[0]);
		assert(signature.parameters[0].type == 'string', "Incorrect type for parameter?", signature.parameters[0]);
	});