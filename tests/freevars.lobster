import std
import testing

run_test("freevars"):

    // Lobster has always allowed access to free variables in lexical scope:

    class Test1:
        b = 0

        def test1():
            var a = 0
            def access_freevars_in_lexical_scope(c, d):
                a = a + c  // A regular local var.
                b = b + d  // A member of freevar "this"
            access_freevars_in_lexical_scope(1, 2)
            assert a == 1
            assert b == 2

    (Test1 {}).test1()

    // But now, with a special annotation, we can do exactly the same thing, except not in
    // lexical scope!
    // This is really not that different from what happens above, because the callstack and
    // free variables it contains looks identical in this situation, all the extra declaration
    // does is signal to the parser and type checker that these variable names are expected to
    // be available in the calling context:

    def access_freevars_not_in_lexical_scope(c, d) [a, b]:
        a = a + c  // A regular local var.
        b = b + d  // A member of a freevar with :: scope!

    class Test2:
        b = 0

        def test2():
            var a = 0
            access_freevars_not_in_lexical_scope(3, 4)
            assert a == 3
            assert b == 4

    (Test2 {}).test2()

    // This would error since no variable found:
    //access_freevars_not_in_lexical_scope(3, 4)

    // If you prefer you can specify types too, which produces an error if the free variable
    // matched in this context is not of a compatible type.
    // It can also help ensure the function is not called in a context where the variable
    // referred to has not been checked for nil etc.

    def access_freevars_not_in_lexical_scope_typed(c:int, d:int) [a:int, b:int]:
        a = a + c  // A regular local var.
        b = b + d  // A member of a freevar with :: scope!

    class Test3:
        b = 0

        def test3():
            var a = 0
            access_freevars_not_in_lexical_scope_typed(3, 4)
            assert a == 3
            assert b == 4

    (Test3 {}).test3()

    do():
        let a = ""
        let b = ""
        // This would produce a type error:
        //access_freevars_not_in_lexical_scope(3, 4)
        pass()

    // Two contexts.

    class Test4:
        a = 0

    // This function will be specialized because it is accessing two different free variables.
    // The fact that the types is the same doesn't matter, because it is not actually passed
    // as parameter, it is direct access to the variable.
    def access_from_two_sources() [test4:Test4]:
        assert test4.a == 0

    do():
        let test4 = Test4 {}
        access_from_two_sources()

    do():
        let test4 = Test4 {}
        access_from_two_sources()

