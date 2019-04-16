import testing
import std

// Misc tests related to type checking.

run_test("types"):

    do():
        def h(i): return i()
        def g(f):
            // Here, the def passed to h appears the same each time, but it has a freevar f that is different.
            return h(): f()
        assert 1 == (g(): 1)
        assert "a" == (g(): "a")

    def compile_time_if(x):
        return if x is int or x is float:
            // The division is a type error if x is a string.
            // But it compiles anyway since this if is compile-time constant,
            // and the type checker ignores this branch.
            // Would be an error in a C++ template, and most other statically typed languages,
            // because they don't cull code without the optimizer.
            1 / x
        else:
            // Also note the return type is just the else branch in the string case,
            // it doesn't need to make a type union between the branches anymore.
            x
    assert compile_time_if(1) is int
    assert compile_time_if("") is string

    assert typeof int == 0  // Always the first one added to the typetable.

    class testa { a:int }
    class testb : testa { b:int }

    // return from should generalize both specializations
    def return_from(a):
        return_from2()
        return testb { 0, 0 }
    def return_from2(): return testa { 0 } from return_from
    assert return_from(1) is testa
    assert return_from("") is testa
