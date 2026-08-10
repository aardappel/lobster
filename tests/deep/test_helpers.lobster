// Minimal test helpers for the deep test suite (tests/deep/).
// Unlike assert, failures are counted rather than aborting, so a single run
// reports all failures. The non-failure path is kept as cheap as possible.

var num_checks = 0
var num_failures = 0
var current_suite = ""

// Called at the top of each test file / section, so failures can name it.
def suite(name:string):
    current_suite = name

def fail(msg:string):
    num_failures++
    print "FAIL [{current_suite}]: {msg}"
    // Non-empty only when running with --runtime-stack-traces.
    let st = get_stack_trace()
    if st.length: print st

// Generic truthiness check (anything false-y fails: 0, 0.0, nil, false).
def test_true(c):
    num_checks++
    if not c: fail("condition is false")

def test_true(c, msg:string):
    num_checks++
    if not c: fail(msg)

// Equality via ==: value equality for scalars/strings/structs,
// reference equality for vectors/classes (use test_seq for those).
def test_eq(a, b):
    num_checks++
    if a != b: fail("{a} != {b}")

def test_neq(a, b):
    num_checks++
    if a == b: fail("{a} == {b}")

// Structural equality, for vectors/classes.
def test_seq(a, b):
    num_checks++
    if not equal(a, b): fail("{a} !equal {b}")

// Approximate equality, for floats produced by math that may not be exact.
def test_approx(a:float, b:float):
    num_checks++
    if abs(a - b) > 1.0e-6: fail("{a} !~ {b}")

def test_approx(a:float2, b:float2):
    num_checks++
    if max(abs(a - b)) > 1.0e-6: fail("{a} !~ {b}")

def test_approx(a:float3, b:float3):
    num_checks++
    if max(abs(a - b)) > 1.0e-6: fail("{a} !~ {b}")

def test_approx(a:float4, b:float4):
    num_checks++
    if max(abs(a - b)) > 1.0e-6: fail("{a} !~ {b}")
