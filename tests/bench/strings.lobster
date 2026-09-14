// A string-heavy benchmark shaped like a code generator: it formats many lines out of names
// and numbers, passes them thru helpers, and accumulates them into one output, which is what
// generating source text or a report looks like. Everything a line is built from is a
// temporary, so this is the workload the string temporary arena is for.
//
//     bin/lobster tests/bench/strings.lobster
//
// It prints the seconds it took and a checksum that must not change between builds.

import std

def indent(depth:int) -> string:
    var s = ""
    for(depth): s += "    "
    return s

def emit_expr(i:int, f:float) -> string:
    return "v{i} = ({i} * {f}) + f{i % 7}(v{i - 1}, \"{i}\")"

def emit_call(name:string, args:[string]) -> string:
    return name + "(" + concat_string(args, ", ") + ")"

def gen(nfuncs:int, nstmts:int) -> string:
    var out = ""
    var checksum = 0
    for(nfuncs) fi:
        let name = "func_{fi}"
        out += "def " + name + "(a:int, b:float) -> string:\n"
        for(nstmts) si:
            let pre = indent(1 + si % 3)
            let line = pre + emit_expr(si, si * 0.5) + "  // stmt {si} of {name}"
            if line.length > 40 and line.find_string("f3(") >= 0:
                out += line + "\n"
            else:
                out += pre + emit_call("call_{si}", [ "a", "b + {si}", name ]) + "\n"
            checksum += line.length
        out += indent(1) + "return \"{name} done\"\n\n"
    return out + "// total {checksum}\n"

let start = seconds_elapsed()
var total = 0
for(400) i:
    let src = gen(40, 60)
    total += src.length + i
let took = seconds_elapsed() - start
print "strings time taken: {took}"
print "checksum: {total}"
