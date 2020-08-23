// A way to define tests which then can be run in different contexts, adding
// functionality like timing and/or running multiple times etc.

def testf() -> void
def testrunnerf(name:string, t:testf) -> void

// This variable can be set to run the tests differently, the default just executes once.
var current_test_runner : testrunnerf = fn(name:string, t:testf): t()

def run_test(name:string, t:testf):
    current_test_runner(name, t)

// An example of what a test looks like.
run_test("example"):
    assert 1 + 2 == 3
