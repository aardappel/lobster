import { MarkupContent, MarkupKind } from 'vscode-languageserver';
import { LobsterSignature } from './lobster';

export function getWordOnCursor(text: string, character: number): [string | null, number] {
	const part1 = text.substring(0, character).match(/[a-zA-Z0-9-_\\.]+$/);
	const part2 = text.substring(character).match(/^[a-zA-Z0-9-_\\.]+/);

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
