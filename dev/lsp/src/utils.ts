import { MarkupContent, MarkupKind } from 'vscode-languageserver';
import { LobsterSignature } from './lobster';

export function getWordOnCursor(text: string, character: number): [string | null, number] {
	const part1 = text.substring(0, character).match(/[a-zA-Z0-9_\\.]+$/);
	const part2 = text.substring(character).match(/^[a-zA-Z0-9_]+/);

	if (!part1 && !part2) return [null, 0];
	return [
		((part1 && part1[0]) || '') + ((part2 && part2[0]) || ''),
		part1 ? part1[0].length : 0
	];
}

export function markupSignature(signature: LobsterSignature): MarkupContent {
	if ('parameters' in signature) return {
		kind: MarkupKind.PlainText,
		value:
			signature.name +
			'(' + signature.parameters.map(p => `${p.name}: ${p.type}`).join(', ') + ')' +
			(signature.returns ? " -> " + signature.returns.join(", ") : "")
	};

	if ('type' in signature) return {
		kind: MarkupKind.PlainText,
		value: signature.name + ': ' + signature.type
	};

	throw new Error("Unknown signature type");
}

// What `map` has for `key`, set to `make()` first if it has nothing.
export function getOrSet<K, V>(map: Map<K, V>, key: K, make: () => V): V {
	let value = map.get(key);
	if (value === undefined) map.set(key, value = make());
	return value;
}

// `f`, reading what it gives for each file once.
export function memo<T>(f: (file: string) => T): (file: string) => T {
	const results = new Map<string, T>();
	return file => getOrSet(results, file, () => f(file));
}

export function mergeSort<T>(data: T[], compare: (a: T, b: T) => number): T[] {
	if (data.length <= 1) {
		// sorted
		return data;
	}
	const p = (data.length / 2) | 0;
	const left = data.slice(0, p);
	const right = data.slice(p);

	mergeSort(left, compare);
	mergeSort(right, compare);

	let leftIdx = 0;
	let rightIdx = 0;
	let i = 0;
	while (leftIdx < left.length && rightIdx < right.length) {
		const ret = compare(left[leftIdx], right[rightIdx]);
		if (ret <= 0) {
			// smaller_equal -> take left to preserve order
			data[i++] = left[leftIdx++];
		} else {
			// greater -> take right
			data[i++] = right[rightIdx++];
		}
	}
	while (leftIdx < left.length) {
		data[i++] = left[leftIdx++];
	}
	while (rightIdx < right.length) {
		data[i++] = right[rightIdx++];
	}
	return data;
}

const WORD = /[A-Za-z0-9_]/;

// The length of `line` without its comment.
export function codeLength(line: string): number {
	for (let i = 0; i < line.length; i++) {
		const c = line[i];
		if (c == '/' && line[i + 1] == '/') return i;
		if (c == '"' || c == "'") i = stringEnd(line, i);
	}
	return line.length;
}

// Where the string or character constant starting at `start` of `line` ends, at its quote.
function stringEnd(line: string, start: number): number {
	let i = start + 1;
	for (; i < line.length && line[i] != line[start]; i++) if (line[i] == '\\') i++;
	return i;
}

// Where `name` is in `text` (one line) as a whole word outside strings and comments, as
// [start, end) columns: all of them, or with `dot` only those after a dot (true) or not (false).
export function occurrences(text: string, name: string, dot?: boolean): [number, number][] {
	const result: [number, number][] = [];
	const end = codeLength(text);
	for (let i = 0; i < end; i++) {
		const c = text[i];
		if (c == '"' || c == "'") {
			i = stringEnd(text, i);
			continue;
		}
		if (!WORD.test(c) || (i > 0 && WORD.test(text[i - 1]))) continue;
		let j = i;
		while (j < end && WORD.test(text[j])) j++;
		if (text.substring(i, j) == name && (dot === undefined || dot == afterDot(text, i))) {
			result.push([i, j]);
		}
		i = j - 1;
	}
	return result;
}

// Whether what starts at `start` in `text` follows a dot (but not `..`).
function afterDot(text: string, start: number): boolean {
	let k = start - 1;
	while (k >= 0 && (text[k] == ' ' || text[k] == '\t')) k--;
	return k >= 0 && text[k] == '.' && text[k - 1] != '.';
}
