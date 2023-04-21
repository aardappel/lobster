// exception handling, implemented using the "return from" feature:

def try(body, catch):
    let err, stack_trace = exception_handler(body)
    if err:
        catch(err, stack_trace)

def exception_handler(body):
    body()
    return nil, ""

// Throw an exception, which must be a non-scalar value.
// It will return thru exception_handler then execute the catch function.
// if you get the error: "return from exception_handler" outside of function,
// it means you're using throw() outside of try() body
def throw(v):
    return v, get_stack_trace() from exception_handler

// This can conveniently wrap error-returning functions.
def throw_if(v):
    if v:
        return v, get_stack_trace() from exception_handler

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
