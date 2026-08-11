// Root file for the deep test suite: runs every test file in this directory.
//
//   bin/lobster tests/deep/test.lobster
//
// Unlike tests/unittest.lobster these tests aim for exhaustive coverage of
// language + builtin features using many small cheap checks, and they do not
// stop at the first failure: failing checks print a FAIL line, and the exit
// code is 1 if any check failed.
//
// Test cases that expose currently-known compiler/VM bugs are commented out
// with a FIXME comment above them describing the problem.

import deep.test_helpers

import deep.lexical
import deep.operators
import deep.strings
import deep.math
import deep.vectors
import deep.structs
import deep.vecutils
import deep.control
import deep.functions
import deep.classes
import deep.generics
import deep.enums
import deep.nilable
import deep.stdlib
import deep.lifetime
import deep.binary
import deep.flatbuffers
import deep.serialization
import deep.reflection
import deep.exceptions
import deep.geometry
import deep.misc
import deep.errors

print "deep tests: {num_checks} checks, {num_failures} failures"
if num_failures:
    set_exit_code(1)
