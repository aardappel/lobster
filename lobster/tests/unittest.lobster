include testing

// This includes all tests in default run-once mode, as a simple unit test.

//trace_bytecode(true, false)

current_test_runner = def(name:string, t:testf):
    print "testing: " + name
    t()

include structtest
include misctest
include mischtest
include typetest
include corotest
include astartest
include goaptest
include knightstest
include parsertest
include floodtest
include watertest
include gradienttest
include springstest
include smallpttest

"the end"  // FIXME: a file can not be only include files?
