/*
    This file takes every entry in the "examples" vector, converts the special
    characters into HTML Entities, and (optionally) outputs each example into a
    single markdown table row. If outputting to a table, the "description"
    string is used as the last entry.

    Also note, the output is not very human readable or easily reformatted, so a
    well made markdown table "reformatter" is recommended.

    Tool created by Judah Caruso Rodriguez (github.com/kyoto-shift) :)
*/

import std

def strip_trailing_breaks(arr):
    if arr.top() == "<br />": arr.pop()
    return arr

def char_to_entity(char):
    return switch(char):
        case "\n": "<br />"
        case "\t", " ": "&nbsp;"
        case "\"": "&quot;"
        case "\'": "&apos;"
        case "<" : "&lt;"
        case ">" : "&gt;"
        case "|" : "&vert;"
        case "_" : "&lowbar;"
        case ";" : "&semi;"
        case "-" : "&dash;"
        case "#" : "&num;"
        case "*" : "&ast;"
        default: char

def string_to_safe(str):
    let converted =
        str.map() t: char_to_entity(unicode_to_string([t]))

    return "<code class=\'example\'>" + concat_string(strip_trailing_breaks(converted), "") + "</code>"

let description = """This is an example. Use multiline strings so you don't have to escape ' characters"""
let examples = [
// First example
"""
def example():
    print "It works!"

example()
""",
// Second example
"""
int example() {
    printf("It works!\n");
    return 0;
}

example();
"""
]

let converted_code = examples.map() l: string_to_safe(l)

// To output a table without the description entry, remove: + description + " |"
let table_string =
    "| " + converted_code.concat_string(" | ") + " | " + description + " |"

// print code.concat_string("") // Just converted code (no table or description)
print table_string

// Uncomment to write the output to a file (useful for workflow / further processing)
// if write_file("./output.md", table_string, 1):
//   print table_string
//   print "\nWrote markdown table output to \'./output.md\'"
// else:
//   fatal_exit "Unable to write file \'./output.md\'"
