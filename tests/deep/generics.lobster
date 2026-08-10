import std

import dictionary

// Generic types & functions in depth: type variables in classes, nested
// generic use, dispatch mixed with specialization, explicit types.

class DGen<T>:
    ts:[T]                    // Type var nested inside a vector type.
    next:DGen<T>? = nil       // Recursive generic reference.
    nexti:DGen<int>? = nil    // Reference to an unnamed specialization.
    def push_t(t:T):          // Generic from surrounding class.
        ts.push(t)
    def push_u<U>(u:U):       // Method with its own generic.
        ts.push(u)
    def first(): return ts[0]

class DGenInt = DGen<int>     // Named specialization.

// Non-inline "method" on the generic.
def dgen_add<T>(g::DGen<T>, t:T):
    ts.push(t)

def generics_tests():
    suite("generics")

    // Named / unnamed specializations behave the same.
    let gi = DGenInt { [ 1 ] }
    gi.push_t(2)
    gi.push_u(3)
    gi.dgen_add(4)
    test_seq(gi.ts, [ 1, 2, 3, 4 ])
    test_eq(gi.first(), 1)
    let gs = DGen<string> { [ "a" ] }
    gs.push_t("b")
    gs.dgen_add("c")
    test_seq(gs.ts, [ "a", "b", "c" ])
    // Recursive generic fields (assignment promotes them to non-nil).
    gi.next = gi
    test_eq(gi.next.first(), 1)
    gi.next = nil
    gs.nexti = gi
    test_eq(gs.nexti.first(), 1)
    gs.nexti = nil

    // Functions with explicit generics and various type-feature interplay.
    let one = 1
    def pick<T>(x:T, y:T): return y
    test_true(pick<float>(one, one) is float)  // Explicit specializer coerces.
    test_true(pick(1.0, one) is float)         // T deduced from the first arg.
    def make_default<T>() -> T: return one
    test_eq(make_default<float>(), 1.0)
    def q2<T>() -> T, T:
        return one, one
    let q2a, q2b = q2<float>()
    test_true(q2a is float and q2b is float)
    // Type vars with is / nil / vector types / typeof.
    def is_t<T>(): return 1.0 is T
    test_true(is_t<float>())
    test_true(not is_t<int>())
    def nil_t<T>(): return nil::T
    test_true(nil_t<string>() == nil::string)
    def vec_t<T>(): return []::T
    test_seq(vec_t<string>(), []::string)
    def typeof_t<T>(): return typeof T
    // typeids can be compared against their int value.
    test_eq(typeof_t<int>(), 0)  // int is always entry 0 of the type table.
    def eq2(a, b): return a == b
    test_true(eq2(typeof string, typeof string))
    test_eq(typeof_t<string>(), typeof string)

    // Generic functions over generic classes, without naming a specialization.
    def rewrap<V>(g:DGen<V>) -> DGen<V>:
        return DGen<V> { [ g.ts[0] ] }
    test_eq(rewrap(gi).first(), 1)
    test_eq(rewrap(gs).first(), "a")
    def maybe_wrap<V>(g:DGen<V>?) -> int:
        return if g: g.ts.length else: -1
    test_eq(maybe_wrap(gi), 4)
    test_eq(maybe_wrap(nil::DGen<int>), -1)

    // Generics flowing into field default values.
    class DSet1<T>:
        dict:dictionary<T, int> = dictionary<T, int>(5)
    class DSet2<T>:
        dict = dictionary<T, int>(5)
    let ds1 = DSet1<string> {}
    let ds2 = DSet2<string> {}
    ds1.dict.set("k", 1)
    ds2.dict.set("k", 1)
    test_eq(ds1.dict.get("k", 0), ds2.dict.get("k", 0))

    // Overload selection with explicit generic instantiation.
    def build<T>(size:int): return 1
    def build<T>(xs:[T]): return 2
    test_eq(build<int>([ 1 ]), 2)
    test_eq(build<int>(3), 1)

    // Subclassing generics: pass a type var on, or fix it.
    class DPair<T, U> : DGen<T>
        u:U
    let dp = DPair<int, string> { [ 5 ], "u" }
    dp.push_t(6)
    test_seq(dp.ts, [ 5, 6 ])
    test_eq(dp.u, "u")
    class DGenFixed : DGen<float>
        tag = 1
    let df = DGenFixed { [ 1.5 ] }
    df.push_t(2.5)
    test_seq(df.ts, [ 1.5, 2.5 ])

    // Specialized methods: same code type checks separately per T.
    def sum_or_concat<T>(g:DGen<T>):
        return g.ts.fold(g.ts[0]): _a + _b
    test_eq(sum_or_concat(gi), 1 + 1 + 2 + 3 + 4)
    test_eq(sum_or_concat(gs), "aabc")
    // FIXME: passing a *subclass* of a generic class (DPair<int, string>,
    // which extends DGen<int>) to a function generic over DGen<T> fails with
    // "cannot implicitly bind type variable `T`", while a plain DGen<int>
    // binds fine:
    //test_eq(sum_or_concat(dp), 11)

generics_tests()
