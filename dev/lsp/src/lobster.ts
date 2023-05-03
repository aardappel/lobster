import { exec } from 'child_process';
import { Diagnostic, DiagnosticSeverity, integer, uinteger } from 'vscode-languageserver';
import { LobsterSettings } from './lsp';

export interface LobsterLocation {
	file: string,
	line: integer
}

export interface LobsterSignartureParameter {
	name: string,
	type: string
}

export interface LobsterVariableSignature {
	name: string,
	type: string
}

export interface LobsterFunctionSignarture {
	name: string,
	parameters: LobsterSignartureParameter[]
}

export type LobsterSignature = LobsterVariableSignature | LobsterFunctionSignarture;

export interface QueryDefinitionResult {
	declartionLocation?: LobsterLocation,
	signature?: LobsterSignature
}

function callLobster(
	settings: LobsterSettings,
	output: (output: string) => void,
	...params: string[]
) {
	const command =
		settings.executable + " " +
		settings.imports.map(i => "--import " + i).join(" ") + " " +
		params.join(" ");


	console.debug(command);
	exec(command, (error, stdout, stderr) => {
		if (stderr) {
			console.error(`Lobster (${settings.executable}) returned with output: ${stderr}`);
			return;
		}

		output(stdout);
	});
}

export async function parseLobster(
	settings: LobsterSettings,
	file: string,
	...args: string[]
): Promise<Diagnostic[]> {
	function parseOutput(input: string): Diagnostic[] {
		if (input.length == 0) return [];

		if (input.startsWith("can't open file")) {
			return []; // TODO make "can't open file" a normal error instead of an unformatted error
		}

		const regex = /^(.*)\((\d+)\): (warning|error): (.*)/g;
		let match;
		const output: Diagnostic[] = [];

		while ((match = regex.exec(input))) {
			const at = match[1];
			if (!file.endsWith(at)) return [];

			const line = parseInt(match[2]) - 1; // Its zero based
			const messageType = match[3].trim();
			const message = match[4].trim();

			const severity = messageType == "warning" ?
				DiagnosticSeverity.Warning :
				DiagnosticSeverity.Error;

			output.push({
				range: {
					start: { line, character: 0 },
					end: { line, character: uinteger.MAX_VALUE }
				},
				message,
				severity
			});
		}

		if (output.length == 0) throw new Error("Invalid output from lobster: " + input);

		return output;
	}


	return new Promise<Diagnostic[]>(back => callLobster(
		settings,
		i => back(parseOutput(i)),
		...(settings.experimental ? [ "--errors", "99" ] : []),
		"--compile-only",
		file,
		...args
	));
}

export async function queryDefinition(
	settings: LobsterSettings,
	file: string,
	line: integer,
	identifier: string,
	...args: string[]
): Promise<QueryDefinitionResult> {
	function parseOutput(input: string): QueryDefinitionResult {
		if (input.length == 0) return {};

		let declartionLocation: LobsterLocation | undefined;
		let signature: LobsterSignature | undefined;

		const queryLocationB = input.indexOf('query_location');
		if (queryLocationB != -1) {
			const queryLocationE = input.substring(queryLocationB).indexOf('\n');
			const sub = input.substring(queryLocationB, queryLocationE);
			const match = sub.match(/^query_location: (.+) (\d+) (.+)?/);

			if (!match) throw new Error("Invalid output from lobster: " + sub);

			declartionLocation = {
				file: match[1].trim(),
				line: parseInt(match[2]) - 1 // Its zero based
			};

			if (match[3]) {
				const def = match[3].trim();
				const defmatch = def.match(/^(.+)\((.*)\)$/);
				if (defmatch) {
					const parameters = readParameters(defmatch[2]);
					signature = {
						name: defmatch[1],
						parameters
					};
				} else {
					signature = {
						name: identifier,
						type: def
					};
				}
			}
		}

		const querySignatureB = input.indexOf('query_signature');
		if (querySignatureB != -1) {
			const querySignatureE = input.substring(querySignatureB).indexOf('\n');
			const sub = input.substring(querySignatureB, querySignatureE);
			const match = sub.match(/^query_signature: (.+)\((.*)\)/);

			if (!match) throw new Error("Invalid output from lobster: " + sub);


			signature = {
				name: match[1].trim(),
				parameters: readParameters(match[2].trim())
			};
		}

		return {
			declartionLocation,
			signature
		};
	}


	const fileName = file.split(/\\|\//).pop();
	if (!fileName) throw new Error("Invalid file name");

	return new Promise(back => callLobster(
		settings,
		i => back(parseOutput(i)),
		file,
		'--query', 'definition', fileName, (line + 1).toString(), identifier,
		...args));
}

function readParameters(input: string): LobsterSignartureParameter[] {
	const parameters = input == '' ? [] : input.split(',')
		.map(i => i.trim().match(/^(.+):(.+)$/) || [])
		.map(i => ({ name: i[1], type: i[2] }));

	if (parameters.some(i => i.name == undefined || i.type == undefined))
		throw new Error("Invalid output from lobster: " + input);

	return parameters;
}