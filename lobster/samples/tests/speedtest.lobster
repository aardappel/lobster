include "unittest.lobster"

starttime := seconds_elapsed()
for(1000):
    run_test_cases(true)
time_taken := seconds_elapsed() - starttime
if time_taken:
    msg := "time taken: "
    print msg + time_taken
