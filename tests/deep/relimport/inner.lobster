// Part of the relative import test: a subdirectory with its own local import,
// which works both when imported from a parent project and when run directly:
//
//   bin/lobster tests/deep/relimport/inner.lobster

import .inner2

def relimport_inner(): return relimport_inner2() + 1
