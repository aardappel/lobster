// Simple multi-threading example.

class fib_request:
    n:int

class fib_response:
    fr:int

def fib(n:int) -> int:
    return if n <= 1: n else: fib(n - 1) + fib(n - 2)

def time(name, f):
    let start = seconds_elapsed()
    f()
    print name + " took: " + (seconds_elapsed() - start)

if not is_worker_thread():
    let hwthreads, hwcores = thread_information()
    print "running on " + hwthreads + " hardware threads and " + hwcores + " cores"
    let fibn = 30
    let fibr = 832040
    time("single thread fib " + fibn): assert fib(fibn) == fibr
    // Let's launch some threads!
    start_worker_threads(hwthreads)
    time("parallel fib " + fibn + " x " + hwthreads):
        // Now Q up some work:
        for(hwthreads): thread_write(fib_request { fibn })
        // And wait for the responses.
        for(hwthreads):
            let r = thread_read(typeof fib_response)
            assert r && r.fr == fibr
else:
    // We're a worker.. process jobs.
    while workers_alive():
        let r = thread_read(typeof fib_request)
        if r: thread_write(fib_response { fib(r.n) })
