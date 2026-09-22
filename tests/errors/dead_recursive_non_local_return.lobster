// EXPECT-OK

// Functions that are never called still get typechecked, by themselves (see
// TypeCheckDeadCode). There, `a` has no call of `handler` to return from, which is not an
// error. Its recursive call must still reuse the specialization being typechecked: making a new
// one for it makes one for every recursive call, and the compiler never finishes.

def handler(body):
    body()
    return 0

def a(n:int) -> int:
    if n > 1000:
        return 1 from handler
    return if n <= 0: 0 else: a(n - 1)

def never_called(n:int):
    print a(n)

// Here a call of `handler` is active, so `a` needs a specialization that returns from it.
def never_called_either(n:int):
    let r = handler(): a(n)
    print r
