include "unittest.lobster"

starttime := seconds_elapsed()
for(1001):
    run_test_cases(true)
time_taken := seconds_elapsed() - starttime
if time_taken:
    msg := "time taken: "
    print msg + time_taken
