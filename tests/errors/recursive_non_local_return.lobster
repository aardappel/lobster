// Bug that appears if a non local return goes out of a recursive dynamic dispatch:

class A:
    def recur() -> int:
        return 0

class B : A
    inner:A
    def recur() -> int:
        // This also does not get unwinding code, even though it should.
        return recur2()
    def recur2() -> int:
        // This call does get unwinding code, because it only relies on frecur getting marked
        // as needing it eventually.
        return frecur(inner) + 1

def nonlocaltarget():
    let t = B { A {} }
    let r = frecur(t)
    return string(r)

def err(a:A) -> int:
    if true:
        return "foo" from nonlocaltarget
    return 1

def frecur(a:A) -> int:
    // The problem is here: frecur is recursive, and we just started typechecking it,
    // and no non-local returns yet. Typechecking recur uses incomplete information
    // that frecur has no non-local returns, so it itself doesn't have any either.
    // So this dynamic dispatch to recur is compiled without any unwinding code..
    let r = a.recur()
    if r:
        return r
    return err(a)

print nonlocaltarget()
