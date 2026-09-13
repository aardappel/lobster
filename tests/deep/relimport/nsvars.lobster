// Part of the namespace test (see modules.lobster): variables declared at file scope in a
// namespace are reachable as nsvars.x from other files, and unqualified in this one. A
// private one is only reachable here.

namespace nsvars

let x = 1
var y = 2
private let z = 3

def sum(): return x + y + z
def bump(): y++
