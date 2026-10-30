// lobsterfmt: formats Lobster source files, like gofmt.
//
//     lobster tools/lobsterfmt/lobsterfmt.lobster -- [-l] [-w] path ...
//
// A directory stands for the .lobster files in it, recursively. Paths must be absolute
// (`"$PWD/file.lobster"`), since Lobster reads relative ones from where this program is. By
// default the formatted files are printed:
//     -l  list the files whose formatting differs instead
//     -w  write the result to the files instead
// Exits with 2 if a file can't be formatted safely.

import std
import .compilercheck
import .files

// Formats `file` as the flags say, returning whether that was possible.
def formatfile(file:string, list:bool, write:bool) -> bool:
    let src = read_file(file)
    if not src:
        print "{file}: can\'t read"
        return false
    let result, err = format_checked(src, file)
    if err:
        print "{file}:{err.line}:{err.column}: {err.message}"
        return false
    if list and result != src: print file
    if write and result != src and not write_file(file, result, false, true):
        print "{file}: can\'t write"
        return false
    // print adds the last line break.
    if not list and not write and result.length: print result.substring(0, result.length - 1)
    return true

let usage = "usage: lobsterfmt [-l] [-w] path ..."
set_print_length(1 << 40)
let args = command_line_arguments()
let list = exists(args): _ == "-l"
let write = exists(args): _ == "-w"
let paths = filter(args): _ != "-l" and _ != "-w"
var status = 0
let flag = find(paths): _.length > 0 and _[0] == '-'
let relative = find(paths): not is_absolute(_)
if flag >= 0:
    print "unknown flag {paths[flag]}\n{usage}"
    status = 2
elif not paths.length:
    print usage
    status = 2
elif relative >= 0:
    print "{paths[relative]}: paths must be absolute, such as \"$PWD/{paths[relative]}\"\n{usage}"
    status = 2
else:
    for(paths) path:
        let files = lobster_files(path)
        if not files.length:
            print "{path}: no .lobster files"
            status = 2
        for(files) file:
            if not formatfile(file, list, write): status = 2
set_exit_code(status)
