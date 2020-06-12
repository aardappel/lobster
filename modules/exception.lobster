// exception handling, implemented using the "return from" feature:

def try(body, catch):
    catch(exception_handler(body)) // catch takes thrown value as arg

def exception_handler(body):
    return body() from try // if no exception raised, try returns result of body and skips catch

def throw(v):
    return v from exception_handler // will invoke catch() of nearest try-catch on the callstack
    // if you get the runtime error: "return from exception_handler" outside of function,
    // it means you're using throw() outside of try() body

// convenience function: empty catch
def try(body):
    try body: _

// convenience function: ensure cleanup function "finally" always gets called regardless of whether
// an exception passes thru body
def protect(body, finally):
    try(body) v:
        finally()
        throw v
    finally()
