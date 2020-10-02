class C:
    x = 1

class A:
    def f(a:C?): return 1

class B : A
    def f(a:C?): return 2

(A {}).f(C {})
