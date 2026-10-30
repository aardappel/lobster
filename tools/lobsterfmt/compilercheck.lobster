// The compiler's own check of format_lobster: whether its parser reads the result as it reads
// the source, with the same errors. Needs the parsed_tokens builtin.

import std
import .format

// The messages of the compiler's errors, without where they are (which removing blank lines
// moves).
private def messages(err:string?):
    let r = []
    for(tokenize(err or "", "\n", "")) l:
        let at = find_string(l, ": error: ")
        if at >= 0: r.push(l.substring(at + 9, l.length - at - 9))
    return r

// What the compiler's parser reads of src as the file filename (whose directory its imports are
// found from), the same for sources it parses the same, and its errors.
def compiler_parse(filename:string, src:string) -> string, [string]:
    let log, err = parsed_tokens(filename, src)
    // A line break before a dedent the parser takes as none, other than to recover from an error.
    return replace_string(log, "linefeed\nde-indentation\n", "de-indentation\n"), messages(err)

// The compiler's error number n in src, at the line it names and the column marked under it.
private def compilererror(filename:string, src:string, n:int):
    let _, err = parsed_tokens(filename, src)
    let lines = tokenize(err or "", "\n", "")
    var e = -1
    let at = find(lines) l:
        if find_string(l, ": error: ") >= 0: e++
        e == n
    let message = messages(err)[n]
    let prefix = filename + "("
    guard find_string(lines[at], prefix) == 0:
        return format_error { message, 1, 1 }
    let close = find_string(lines[at], ")", prefix.length)
    let line = string_to_int(lines[at].substring(prefix.length, close - prefix.length))
    var offset = 0
    var l = 1
    while l < line and offset < src.length:
        if src[offset] == '\n': l++
        offset++
    if at + 2 < lines.length and find_string(lines[at + 2], "at: ") == 0:
        offset += max(0, find_string(lines[at + 2], "^") - 4)
    let ln, column = format_position(src, offset)
    return format_error { message, ln, column }

// Why the compiler would read result, src formatted (both with LF line ends), differently, or
// nil.
def compiler_check(filename:string, src:string, result:string) -> format_error?:
    if result == src: return nil
    let before, beforeerrors = compiler_parse(filename, src)
    let after, aftererrors = compiler_parse(filename, result)
    for(beforeerrors) e, n:
        // An error that depends on whitespace (tabs mixed with spaces) would go.
        if n >= aftererrors.length or e != aftererrors[n]: return compilererror(filename, src, n)
    if aftererrors.length == beforeerrors.length and before == after: return nil
    // How the parser recovers from an error can depend on whitespace, so only in code without
    // any is this a bug.
    if beforeerrors.length: return compilererror(filename, src, 0)
    let what = if aftererrors.length: "make it not parse" else: "change how the compiler parses it"
    return format_error { "formatting would {what} (this is a formatter bug)", 1, 1 }

// format_lobster, also refusing what the compiler would read differently formatted, as the file
// filename.
def format_checked(source:string, filename:string) -> string, format_error?:
    let result, err = format_lobster(source)
    if err: return result, err
    let e = compiler_check(filename, replace_string(source, "\r\n", "\n"), result)
    if e: return "", e
    return result, nil
