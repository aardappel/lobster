// Finding the .lobster files to format, for lobsterfmt and its tests.

import std

def is_lobster_file(name:string) -> bool:
    return name.length > 8 and substring(name, name.length - 8, 8) == ".lobster"

// Lobster reads a relative path from where the program is, not the working directory.
def is_absolute(path:string) -> bool:
    return (path.length > 0 and (path[0] == '/' or path[0] == '\\')) or
        (path.length > 2 and path[1] == ':' and (path[2] == '/' or path[2] == '\\'))

// The .lobster files `path` stands for: itself, or those in it if it's a directory, recursively
// (but not in hidden ones).
def lobster_files(path:string) -> [string]:
    let names, sizes = scan_folder(path, false)
    if not names or not sizes:
        return if is_lobster_file(path): [ path ] else: []
    let result = []
    for(names) name, i:
        let p = path + "/" + name
        if sizes[i] < 0:
            if name[0] != '.': append_into(result, lobster_files(p))
        elif is_lobster_file(name):
            result.push(p)
    return result
