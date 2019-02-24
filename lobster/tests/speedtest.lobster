include testing

// This is a speed test to help optimize the implementation.

// Define a custom testrunner:
// Just collect all tests so we can run them interleaved.
class test { name:string, t:testf, runs:int, min:float }
tests := []
number_of_runs := 1
current_test_runner = def(name:string, t:testf):
    tests.push(test { name, t, number_of_runs, 999999.9 })



//trace_bytecode(true, false)

// Balance these numbers to get a good blend of what kind of code we're optimizing for.

number_of_runs = 5000
include misctest

number_of_runs = 4000
include typetest

number_of_runs = 4000
include corotest

number_of_runs = 100
include astartest

number_of_runs = 200
include goaptest

number_of_runs = 100
include knightstest

number_of_runs = 500
include parsertest

number_of_runs = 100
include floodtest

number_of_runs = 10
include watertest

number_of_runs = 10
include gradienttest

number_of_runs = 10
include springstest

number_of_runs = 10
include smallpttest


// We interleave all tests, and take the min of all meta runs, to eliminate variance as best as
// possible.
meta_runs :== 17
total_time_taken := 0.0
for meta_runs:
    for(tests) t:
        starttime := seconds_elapsed()
        for t.runs:
            f := t.t
            f()
        time_taken := seconds_elapsed() - starttime
        t.min = min(t.min, time_taken)
for(tests) t:
    total_time_taken += meta_runs * t.min
    print t.name + " time taken for " + meta_runs * t.runs + " runs: " + meta_runs * t.min

print "TOTAL time taken: " + total_time_taken
