// exception handling, implemented using the "return from" feature:

function try(body, catch):
    catch(exception_handler(body)) // catch takes thrown value as arg

function exception_handler(body):
    return body() from try // if no exception raised, try returns result of body and skips catch

function throw(v):
    return v from exception_handler // will invoke catch() of nearest try-catch on the callstack
    // if you get the runtime error: "return from exception_handler" outside of function, it means you're using throw() outside of try() body

function try(body):     // convenience function: empty catch
    try(body): _

function protect(body, finally):    // convenience function: ensure cleanup function "finally" always gets called regardless of wether an exception passes thru body
    try(body) v:
        finally()
        throw(v)
    finally()
