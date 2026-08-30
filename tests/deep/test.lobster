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

import .test_helpers

import .lexical
import .operators
import .strings
import .math
import .vectors
import .structs
import .vecutils
import .control
import .functions
import .classes
import .overloads
import .generics
import .enums
import .nilable
import .stdlib
import .lifetime
import .binary
import .flatbuffers
import .serialization
import .reflection
import .exceptions
import .geometry
import .misc
import .modules
import .noeffect
import .errors
import .declcheck

print "deep tests: {num_checks} checks, {num_failures} failures"
if num_failures:
    set_exit_code(1)
