class A:
    def foo():
        return 1
class B:
    def foo(x = 1):
        return 1 + x

assert A {}.foo() == 1
assert B {}.foo() == 2
