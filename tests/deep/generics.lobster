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
    def as_t(v):              // T as an internal type (per specialization).
        let x:T = v
        return x

class DGenInt = DGen<int>     // Named specialization.

class DGenFlt = DGen<float>

class DGenSub : DGenFlt       // Subclass of a named specialization.
    def push_u<U>(u:U):       // Overrides a method with its own generic.
        ts.push(u + 1.0)

// Subclassing generics: pass a type var on, or fix it.
class DPair<T, U> : DGen<T>
    u:U

class DGenFixed : DGen<float>
    tag = 1

// Non-inline "method" on the generic.
def dgen_add<T>(g::DGen<T>, t:T):
    ts.push(t)

def generics_tests():
    suite("generics")

    // Named / unnamed specializations behave the same. (The DPair<int, string>
    // specialization is created first: it subclasses DGen<int>, so it must
    // exist before any dynamically dispatched call on DGenInt.)
    let dp = DPair<int, string> { [ 5 ], "u" }
    let gi = DGenInt { [ 1 ] }
    gi.push_t(2)
    gi.push_u(3)
    gi.dgen_add(4)
    test_seq(gi.ts, [ 1, 2, 3, 4 ])
    test_eq(gi.first(), 1)
    // T used as the type of an internal variable: bool coerces to int here.
    test_eq(gi.as_t(true), 1)
    test_true(gi.as_t(true) is int)
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

    // Type arguments may be struct types, or generic classes themselves.
    let gvec = DGen<int2> { [ int2 { 1, 2 } ] }
    gvec.push_t(int2 { 3, 4 })
    test_eq(gvec.first() + int2_1, int2 { 2, 3 })
    test_eq(gvec.ts[1].y, 4)
    let gnest = DGen<DGen<int>> { [ gi ] }
    test_eq(gnest.first().first(), 1)
    test_seq(gnest.ts[0].ts, gi.ts)

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

    // Static overloads across generic classes, scalars, and a generic
    // catch-all, all resolved statically.
    class DWrap<T>:
        x:T
    def pick_ov<T>(d:DWrap<T>): return d.x
    def pick_ov(i:int): return i
    def pick_ov(s:string): return s
    def pick_ov<T>(t:T): return t + t
    test_eq(pick_ov(DWrap<int> { 1 }) + pick_ov(2), 3)
    test_eq(pick_ov(DWrap<string> { "ha" }) + pick_ov("llo"), "hallo")
    test_eq(pick_ov(1.5), 3.0)  // Falls through to the generic doubling.

    // Nested generic type arguments, incl. >> parsing.
    let nested_dict = dictionary<string, dictionary<string, int>>(7)
    test_true(nested_dict.get("foo") == nil)
    let inner_dict = dictionary<string, int>(7)
    inner_dict.set("k", 1)
    nested_dict.set("foo", inner_dict)
    test_eq((assert nested_dict.get("foo")).get("k", 0), 1)

    // Subclassing generics (types declared at top level): pass a type var on,
    // or fix it.
    dp.push_t(6)
    test_seq(dp.ts, [ 5, 6 ])
    test_eq(dp.u, "u")
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

    // A subclass of a named specialization can override a method that has its
    // own generic (push_u<U>) and still dispatch dynamically. (This runs last:
    // the dispatch requires all DGen<float> subclasses to be declared above.)
    let dsub:DGenFlt = DGenSub { [ 1.5 ] }
    dsub.push_u(3.0)
    test_eq(dsub.ts.top(), 4.0)
    // FIXME: overriding a method that uses the *superclass* type variable
    // (push_t(t:T)) in such a subclass does not work: T is not in scope in
    // the subclass, so
    //     class DGenSub : DGenInt
    //         def push_t(t:T):
    //             ts.push(t + 1)
    // fails to compile with "unknown type: T".
    // (Also commented out in tests/typetest.lobster.)

    // T {} where T resolves to a non-struct/class type stands for the default
    // value of that type, so generic code can construct a value of any type.
    def default_of<T>() -> T:
        return T {}
    test_eq(default_of<int>(), 0)
    test_eq(default_of<float>(), 0.0)
    test_eq(default_of<string>(), "")
    test_eq(default_of<bool>(), false)
    test_true(default_of<string?>() == nil)
    let dvec = default_of<[float]>()
    test_eq(dvec.length, 0)
    dvec.push(1.5)
    test_eq(dvec[0], 1.5)
    // For a class it is the normal constructor with all fields defaulted.
    class DefCon:
        x = 7
    test_eq(default_of<DefCon>().x, 7)
    // dictionary.get_or_create relies on this for non-class value types.
    let dgc = dictionary<string, int>(7)
    test_eq(dgc.get_or_create("k"), 0)
    dgc.add("k", 5)
    test_eq(dgc.get_or_create("k"), 5)
    let dgs = dictionary<int, string>(7)
    test_eq(dgs.get_or_create(1), "")
    let dgv = dictionary<int, [int]>(7)
    dgv.get_or_create(1).push(9)
    test_eq(dgv.get_or_create(1).length, 1)

    // A T {} constructor takes field tags and fills in defaults, like a
    // constructor of a named type does.
    class Tagged:
        a:int
        b = 7
        c = "x"
    def make_tagged<T>() -> T: return T { b: 2, a: 1 }
    let tg = make_tagged<Tagged>()
    test_eq(tg.a, 1)
    test_eq(tg.b, 2)
    test_eq(tg.c, "x")
    def make_defaulted<T>() -> T: return T { a: 3 }
    let td = make_defaulted<Tagged>()
    test_eq(td.a, 3)
    test_eq(td.b, 7)
    // Untagged initializers still fill the fields without a default, in order.
    class Pair2<A, B>:
        pa:A
        pb:B
    def make_pair<T>() -> T: return T { 1, "s" }
    let tp = make_pair<Pair2<int, string>>()
    test_eq(tp.pa, 1)
    test_eq(tp.pb, "s")
    def make_pair_tagged<T>() -> T: return T { pb: "t", pa: 2 }
    let tpt = make_pair_tagged<Pair2<int, string>>()
    test_eq(tpt.pa, 2)
    test_eq(tpt.pb, "t")

generics_tests()
