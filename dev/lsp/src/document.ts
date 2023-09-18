import {
	Diagnostic,
	DiagnosticSeverity,
	DocumentUri,
	Position,
	Range,
	TextDocumentContentChangeEvent,
	TextEdit
} from 'vscode-languageserver';
import { TextDocument } from 'vscode-languageserver-textdocument';
import { URI } from 'vscode-uri';
import { mergeSort } from './utils';
import * as path from 'path';
import * as fs from 'fs';
import { parseLobster } from './lobster';
import { LSPInstance } from './lsp';


// Based of https://github.com/microsoft/vscode-languageserver-node/blob/main/textDocument/src/main.ts
// Wich is licensed under MIT

export enum LobsterDocumentState {
	NoErrors,
	HasWarnings,
	HasErrors
}

export class LobsterDocument implements TextDocument {

	private _uri: URI;
	private _languageId: string;
	private _version: number;
	private _content: string;
	private _lineOffsets: number[] | undefined;
	private _state: LobsterDocumentState = LobsterDocumentState.NoErrors;
	private _relative: string | undefined;

	public constructor(uri: URI, languageId: string, version: number, content: string) {
		this._uri = uri;
		this._languageId = languageId;
		this._version = version;
		this._content = content;
		this._lineOffsets = undefined;
	}

	public get uri(): string {
		return this._uri.toString();
	}

	public get languageId(): string {
		return this._languageId;
	}

	public get version(): number {
		return this._version;
	}

	public get state(): LobsterDocumentState {
		return this._state;
	}

	public getText(range?: Range): string {
		if (range) {
			const start = this.offsetAt(range.start);
			const end = this.offsetAt(range.end);
			return this._content.substring(start, end);
		}
		return this._content;
	}

	public async getRelative(lsp: LSPInstance): Promise<string> {
		if (this._relative) return this._relative;

		const folders = await lsp.getWorkspaceFolders();
		const folder = folders?.filter(
			(folder) => this._uri.fsPath.startsWith(URI.parse(folder.uri).fsPath)
		)[0];

		return this._relative = path.relative(
			URI.parse(folder?.uri || '').fsPath,
			this._uri.fsPath
		);
	}

	public async writeToTmp(lsp: LSPInstance): Promise<string> {
		if (lsp.errored()) throw "LSP has errored";

		const relative = await this.getRelative(lsp);
		const join = path.join(lsp.tempDir, relative);
		const reldir = path.parse(relative).dir;

		if (reldir.length != 0) {
			await new Promise(
				(resolve, fail) => fs.mkdir(path.join(lsp.tempDir, reldir), { recursive: true },
					(err, path) => {
						if (err) {
							console.error(err);
							fail(err);
						} else {
							resolve(path);
						}
					})
			);
		}

		await new Promise((resolve, fail) => {
			fs.open(join, 'w', (err, fd) => {
				if (err) {
					fail(err);
					return;
				} else {
					fs.writeFile(fd, this._content, (err) => {
						if (err) {
							fs.close(fd, (err) => err && console.error(err));
							fail(err);
							return;
						} else {
							fs.close(fd, (err) => {
								if (err) {
									fail(err);
									return;
								} else {
									resolve(null);
								}
							});
						}
					});
				}
			});
		});

		return join;
	}

	public async parse(lsp: LSPInstance): Promise<Diagnostic[]> {
		const settings = await lsp.getDocumentSettings(this._uri);
		if (lsp.errored()) return [];

		const temp = await this.writeToTmp(lsp);
		const result = await parseLobster(settings, temp);

		this._state = LobsterDocumentState.NoErrors;
		result.forEach((diag) => {
			if (diag.severity === DiagnosticSeverity.Warning &&
				this._state === LobsterDocumentState.NoErrors) {
				this._state = LobsterDocumentState.HasWarnings;
			} else if (diag.severity === DiagnosticSeverity.Error) {
				this._state = LobsterDocumentState.HasErrors;
			}
		});

		return result;
	}

	public update(changes: TextDocumentContentChangeEvent[], version: number): void {
		for (const change of changes) {
			if (LobsterDocument.isIncremental(change)) {
				// makes sure start is before end
				const range = getWellformedRange(change.range);

				// update content
				const startOffset = this.offsetAt(range.start);
				const endOffset = this.offsetAt(range.end);
				this._content = this._content.substring(0, startOffset)
					+ change.text
					+ this._content.substring(endOffset, this._content.length);

				// update the offsets
				const startLine = Math.max(range.start.line, 0);
				const endLine = Math.max(range.end.line, 0);
				let lineOffsets = this._lineOffsets!;
				const addedLineOffsets = computeLineOffsets(change.text, false, startOffset);

				if (endLine - startLine === addedLineOffsets.length) {
					for (let i = 0, len = addedLineOffsets.length; i < len; i++) {
						lineOffsets[i + startLine + 1] = addedLineOffsets[i];
					}
				} else {
					if (addedLineOffsets.length < 10000) {
						lineOffsets.splice(startLine + 1, endLine - startLine, ...addedLineOffsets);
					} else { // avoid too many arguments for splice
						this._lineOffsets = lineOffsets =
							lineOffsets.slice(0, startLine + 1)
								.concat(addedLineOffsets, lineOffsets.slice(endLine + 1));
					}
				}

				const diff = change.text.length - (endOffset - startOffset);
				const start = startLine + 1 + addedLineOffsets.length;
				if (diff !== 0) {
					for (let i = start, len = lineOffsets.length; i < len; i++) {
						lineOffsets[i] = lineOffsets[i] + diff;
					}
				}
			} else if (LobsterDocument.isFull(change)) {
				this._content = change.text;
				this._lineOffsets = undefined;
			} else {
				throw new Error('Unknown change event received');
			}
		}
		this._version = version;
	}

	private getLineOffsets(): number[] {
		if (this._lineOffsets === undefined) {
			this._lineOffsets = computeLineOffsets(this._content, true);
		}
		return this._lineOffsets;
	}

	public positionAt(offset: number): Position {
		offset = Math.max(Math.min(offset, this._content.length), 0);

		const lineOffsets = this.getLineOffsets();
		let low = 0, high = lineOffsets.length;
		if (high === 0) {
			return { line: 0, character: offset };
		}
		while (low < high) {
			const mid = Math.floor((low + high) / 2);
			if (lineOffsets[mid] > offset) {
				high = mid;
			} else {
				low = mid + 1;
			}
		}
		// low is the least x for which the line offset is larger than the current offset
		// or array.length if no line offset is larger than the current offset
		const line = low - 1;
		return { line, character: offset - lineOffsets[line] };
	}

	public offsetAt(position: Position) {
		const lineOffsets = this.getLineOffsets();
		if (position.line >= lineOffsets.length) {
			return this._content.length;
		} else if (position.line < 0) {
			return 0;
		}
		const lineOffset = lineOffsets[position.line];
		const nextLineOffset = (position.line + 1 < lineOffsets.length) ?
			lineOffsets[position.line + 1] :
			this._content.length;

		return Math.max(Math.min(lineOffset + position.character, nextLineOffset), lineOffset);
	}

	public get lineCount() {
		return this.getLineOffsets().length;
	}

	private static isIncremental(event: TextDocumentContentChangeEvent):
		event is { range: Range; rangeLength?: number; text: string } {
		const candidate: { range: Range; rangeLength?: number; text: string } = event as any;
		return candidate !== undefined && candidate !== null &&
			typeof candidate.text === 'string' && candidate.range !== undefined &&
			(candidate.rangeLength === undefined || typeof candidate.rangeLength === 'number');
	}

	private static isFull(event: TextDocumentContentChangeEvent): event is { text: string } {
		const candidate: { range?: Range; rangeLength?: number; text: string } = event as any;
		return candidate !== undefined &&
			candidate !== null &&
			typeof candidate.text === 'string' &&
			candidate.range === undefined &&
			candidate.rangeLength === undefined;
	}
}

// idk how to do this properly, but this is the only way I could get it to work
// eslint-disable-next-line @typescript-eslint/no-namespace
export namespace LobsterDocument {
	export function create(
		uri: DocumentUri,
		languageId: string,
		version: number,
		content: string
	): LobsterDocument {
		return new LobsterDocument(URI.parse(uri), languageId, version, content);
	}

	export function update(
		document: LobsterDocument,
		changes: TextDocumentContentChangeEvent[],
		version: number
	): LobsterDocument {
		document.update(changes, version);
		return document;
	}

	export function applyEdits(document: LobsterDocument, edits: TextEdit[]): string {
		const text = document.getText();
		const sortedEdits = mergeSort(edits.map(getWellformedEdit), (a, b) => {
			const diff = a.range.start.line - b.range.start.line;
			if (diff === 0) {
				return a.range.start.character - b.range.start.character;
			}
			return diff;
		});
		let lastModifiedOffset = 0;
		const spans = [];
		for (const e of sortedEdits) {
			const startOffset = document.offsetAt(e.range.start);
			if (startOffset < lastModifiedOffset) {
				throw new Error('Overlapping edit');
			} else if (startOffset > lastModifiedOffset) {
				spans.push(text.substring(lastModifiedOffset, startOffset));
			}
			if (e.newText.length) {
				spans.push(e.newText);
			}
			lastModifiedOffset = document.offsetAt(e.range.end);
		}
		spans.push(text.substr(lastModifiedOffset));
		return spans.join('');
	}
}


function computeLineOffsets(text: string, isAtLineStart: boolean, textOffset = 0): number[] {
	const result: number[] = isAtLineStart ? [textOffset] : [];
	for (let i = 0; i < text.length; i++) {
		const ch = text.charCodeAt(i);
		if (ch === 13 || ch === 10) { // line feed or carriage return
			if (ch === 13 && i + 1 < text.length && text.charCodeAt(i + 1) === 10) {
				i++;
			}
			result.push(textOffset + i + 1);
		}
	}
	return result;
}

function getWellformedRange(range: Range): Range {
	const start = range.start;
	const end = range.end;
	if (start.line > end.line || (start.line === end.line && start.character > end.character)) {
		return { start: end, end: start };
	}
	return range;
}

function getWellformedEdit(textEdit: TextEdit): TextEdit {
	const range = getWellformedRange(textEdit.range);
	if (range !== textEdit.range) {
		return { newText: textEdit.newText, range };
	}
	return textEdit;
}