import std
import match

// UTILITY AND HELPER FUNCTIONS
def char_in_string(str, char):
    let uchar = string_to_unicode(char) or [0]
    let ustr  = string_to_unicode(str) or [0]

    for(ustr): if _ == uchar[0]: return true
    return false

def replace_in_string(str, char, repl):
    let tokens = tokenize(str, char, " ")
    return concat_string(tokens, repl + " ")

def char_to_entity(char):
    return switch(char):
        case "|" : "&vert;"
        case "_" : "&lowbar;"
        case ";" : "&semi;"
        case "#" : "&num;"
        case "-" : "&dash;"
        case "<" : "&lt;"
        case ">" : "&gt;"
        case "\n": "<br />"
        case "\t", " ": "&nbsp;"
        default: char

// takes a string and converts (by char_to_entity) every character to html
// character entities.
def string_to_safe(str):
    let converted =
        str.map() t:
            char_to_entity(unicode_to_string([t]))

    return concat_string(converted, "")

// takes a header and returns a formatted string
def normalize_header(header):
    let unbroken = tokenize(concat_string(header, ""), "!", " ")
    let no_breaks =
        unbroken.filter() l:
            l != "" and
            l.length > 1 and
            substring(l, 0, 1) != "\n"

    // i know...
    return replace_in_string(replace_in_string(concat_string(no_breaks, ""), "\n", ""), "#", "")

// takes a description and returns a formatted string
def normalize_desc(desc):
    let unbroken = tokenize(concat_string(desc, ""), "\n", " ")
    let no_breaks =
        unbroken.filter() l:
            l != "" and
            l.length > 1 and
            l[0] != '\n'

    if no_breaks.length > 0:
        let formatted = []
        for(no_breaks) el:
            if el[0] == '#':
                let temp = string_to_unicode(el) or [35]
                remove(temp, 0)
                formatted.push(unicode_to_string(temp))

        return replace_in_string(concat_string(formatted, ""), "\n", "")

    return ""

// returns a new string without the document properties
def remove_file_properties(file):
    let tokens = file.map() char: char

    // pretty similar to get_file_properties
    // possibly refactor into get_file_properties?

    var len = 0
    if tokens[0] == 't' or tokens[0] == 'o':
        var m = 0
        while tokens[m] != '!' and tokens[m] != '#':
            m++

        len = m

    tokens.remove(0, len)
    return unicode_to_string(tokens)

// returns the document title and output filename as strings
def get_file_properties(file):
    var title = "New Document"
    var output = "new_documentation"
    let tokens = file.map() char: char

    var i = 0
    while i < tokens.length - 1:
        // 't' for title, 'o' for output
        // needs to be refactored in the future
        if tokens[i] == 't' or tokens[i] == 'o':
            var temp_str = ""

            var m = i
            while tokens[m] != ' ':
                temp_str += unicode_to_string([tokens[m]])
                m++

            // tell i to skip over characters we just saw
            i += (m - i)

            if temp_str == "title:":
                temp_str = ""
                m = i
                while tokens[m] != '\n':
                    temp_str += unicode_to_string([tokens[m]])
                    m++

                title = tokenize(temp_str, "", " ")[0]
                i += (m - i)

            if temp_str == "output:":
                temp_str = ""
                m = i
                while tokens[m] != '\n':
                    temp_str += unicode_to_string([tokens[m]])
                    m++

                output = tokenize(temp_str, "", " ")[0]
                i += (m - i)
        i++

    return title, output

// START

struct entry { type: string, desc: string, lang1: string, lang2: string }

let contents = []
let pre_render = []

// hardcoded doc file right now
let doc_file = read_file("new_doc.doc", 1) or ""
if doc_file.length < 1: fatal_exit("unable open file: \'./new_doc.doc\'")

let doc_title, doc_filename = get_file_properties(doc_file)

// filter out our 'code separation' character and any empty lines
let doc_file_contents =
    filter tokenize(remove_file_properties(doc_file), "`", " "):
        _ != "\n"

// don't really like how this works, will use custom printf later
let html_body_start =
"""
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "https://www.w3.org/TR/html4/strict.dtd">
<html><head><meta content="text/html; charset=ISO-8859-1" http-equiv="content-type"><title>""" +
doc_title +
"""</title>
<!--<link rel="stylesheet" type="text/css" href="docs/github.css">-->
<style type="text/css">
body { width: 90%; margin: 0 auto; padding-top: 50px; padding-bottom: 50px; }
table, tr, td { border: 1pt solid #DDDDDD; border-Collapse: collapse; max-width:1200px; vertical-align:top }
code { font-family: Consolas; background-color: #DDDDDD; padding: 1px 2px 1px 2px; }

td { font-family: Consolas; font-size: 0.8em; }

.table-doc { width: 100%; text-align: left; margin-left: auto; margin-right: auto; }
.table-header { font-weight: bold; font-size: 1em; }
.table-entry { font-family: Consolas; white-space: nowrap; }

.header-section { text-align: center; padding: 20pt; }
.header-section span { font-family: Consolas; font-size: 2em; }

.subheader-section { text-align: center; padding: 20pt; }
.subheader-section span { font-weight: bold; font-family: Consolas; font-size: 1.25em; }

.subtext { font-family: Consolas; font-size: 0.85em; text-align: justify; margin: 0 auto; padding-bottom: 20px; max-width: 1200px; }
</style>
</head>


<!-- DISCLAIMER: This document was generated by a program. It is not intended to be human readable, hopefully the docs are though :) -->
<!-- Tool created by: Judah Caruso Rodriguez (github.com/kyoto-shift) --> 

<body>
<!-- doc start -->
"""
let html_body_end    = "<!-- doc end --></body></html>"
let html_table_start =
"""
<!-- doc table start -->
<table class="table-doc" border="1" cellpadding="5" cellspacing="2"><tbody>
<tr>
<td class="table-header">Lobster Version</td>
<td class="table-header">C-style Version</td>
<td class="table-header">Notes</td>
</tr>
"""
let html_table_end   = "</tbody></table><!-- doc table end -->"

let html_header_start = "<!-- header start --><div class=\'header-section\'><span>"
let html_header_end   = "</span></div><!-- header end -->"

let html_subheader_start = "<!-- subheader start --><div class=\'subheader-section\'><span>"
let html_subheader_end   = "</span></div><!-- subheader end -->"

let html_header_subtext_start = "<!-- header subtext start --><p class=\'subtext\'>"
let html_header_subtext_end   = "</p><!-- header subtext end -->"

var i = 0
while i < doc_file_contents.length:
    // if we find a header '!'
    if char_in_string(doc_file_contents[i], "!"):
        let header = []
        let subtext = []

        // quick fix
        if char_in_string(doc_file_contents[i], "!"):
            var cleaned = tokenize(doc_file_contents[i], "\n", " ")

            // filter out any blank elements
            if cleaned[0] == "": cleaned = cleaned.filter(): _ != ""

            // push our header
            if cleaned[0][0] == '!': header.push(cleaned[0])

            // if we find something that's not the start of a doc
            if cleaned[1][0] != '#':
                var m = 1
                while m < cleaned.length:
                    if cleaned[m][0] != '#':
                        subtext.push(cleaned[m])
                    m++

        pre_render.push(
            entry {
                "subheader",
                normalize_header(header),
                "",
                ""})

        if subtext.length > 0:
            pre_render.push(
                entry {
                    "subtext",
                    concat_string(subtext, " "),
                    "",
                    ""})

    // if we find a description '#'
    if char_in_string(doc_file_contents[i], "#"):
        let description = []

        // quick fix for headers and subtexts being included in descriptions
        if char_in_string(doc_file_contents[i], "!"):
            var cleaned = tokenize(doc_file_contents[i], "\n", " ")
            // filter out any blank elements
            if cleaned[0] == "": cleaned = cleaned.filter(): _ != ""

            // remove any caught headers
            if cleaned[0][0] == '!': cleaned = cleaned.filter(): _[0] == '#'

            // finally push any headers
            if cleaned[0][0] == '#':
                var m = 0
                while m < cleaned.length and cleaned[m][0] == '#':
                    description.push(cleaned[m] + "\n")
                    m++
        else:
            var m = i
            while char_in_string(doc_file_contents[m], "#"):
                description.push(doc_file_contents[m] + "\n")
                m++


        // if any of our examples have less than 2 "arguments"
        if i+2 >= doc_file_contents.length:
            fatal_exit("missing argument(s) for example:\n" + doc_file_contents[i])

        pre_render.push(
            entry {
                "example",
                normalize_desc(description),
                string_to_safe(doc_file_contents[i+1]),
                string_to_safe(doc_file_contents[i+2])})
    i++

i = 0
while i < pre_render.length:
    var doc_object = ""

    if pre_render[i].type == "subheader":
        doc_object +=
            html_subheader_start + pre_render[i].desc + html_subheader_end

    if pre_render[i].type == "subtext":
        doc_object +=
            html_header_subtext_start + pre_render[i].desc + html_header_subtext_end

    if pre_render[i].type == "example":
        var table = html_table_start

        var m = i
        while m < pre_render.length and pre_render[m].type == "example":
            table +=
                "<tr><td class=\'table-entry\'>" +
                pre_render[m].lang1 +
                "</td><td class=\'table-entry\'>" +
                pre_render[m].lang2 +
                "</td><td>" +
                pre_render[m].desc +
                "</td></tr>"
            m++

        i += (m - i) - 1

        table += html_table_end
        doc_object += table

    contents.push(doc_object)
    i++

let file =
    html_body_start +                                   // html, title, body
    html_header_start + doc_title + html_header_end +   // header above docs
    concat_string(contents, "") +                       // actual docs
    html_body_end                                       // body, html

if write_file("../" + doc_filename + ".html", file, 0):
    print("Created documentation file: \'" +  doc_title + "\'" + " (" + doc_filename + ".html)")
    return from program
