import testing

// This includes all tests in default run-once mode, as a simple unit test.

current_test_runner = fn(name:string, t:testf):
    print "testing: {name}"
    t()

import structtest
import misctest
import typetest
import astartest
import goaptest
import knightstest
import parsertest
import floodtest
import watertest
import gradienttest
import springstest
import smallpttest
import stringtest

// Not added to speedtest yet.
import mischtest
import ccodetest
import stringtest
import lifetimetest
import builtintest
import operators
import matrixtest
import typeswitch
import freevars

// These are more exhaustive tests, that all run regardless of error.
import deep.test

"the end"  // FIXME: a file can not be only import files?
