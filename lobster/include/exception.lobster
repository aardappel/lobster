// exception handling, implemented using the "return from" feature:

def try(body, catch):
    catch(exception_handler(body)) // catch takes thrown value as arg

def exception_handler(body):
    return body() from try // if no exception raised, try returns result of body and skips catch
    // FIXME: If there is no throw, then this function will be of type void, which we can't pass
    // to catch. So we have to return something here. But if there is a throw, it has to be
    // compatible with that type, which we don't know, even though it never gets executed.
    // So as a total hack, we return a type variable, which is compatible with anything. We don't
    // have syntax for that, so we return the first element of a list of unknown type, which does
    // exactly that. Would be nice if there was a way to do this without such a hack.
    // We could just make the thrown value "any", but it is nice to keep this statically typed.
    return [][0]  // if no throw, make sure this function isn't void.

def throw(v):
    return v from exception_handler // will invoke catch() of nearest try-catch on the callstack
    // if you get the runtime error: "return from exception_handler" outside of function,
    // it means you're using throw() outside of try() body

// convenience function: empty catch
def try(body):
    try body: _

// convenience function: ensure cleanup function "finally" always gets called regardless of wether
// an exception passes thru body
def protect(body, finally):
    try(body) v:
        finally()
        throw v
    finally()
