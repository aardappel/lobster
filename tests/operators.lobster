import testing
import quaternion

// A class with operloaded operators can modify its internals with assignment
// operators.

class OC:
    a:int

    def operator+(o:OC): return OC { a + o.a }
    def operator-(o:OC): return OC { a - o.a }
    def operator*(o:OC): return OC { a * o.a }
    def operator/(o:OC): return OC { a / o.a }
    def operator%(o:OC): return OC { a % o.a }
    def operator&(o:OC): return OC { a & o.a }
    def operator|(o:OC): return OC { a | o.a }
    def operator^(o:OC): return OC { a ^ o.a }
    def operator<<(o:int): return OC { a << o.a }
    def operator>>(o:int): return OC { a >> o.a }

    def operator+=(o:OC): a += o.a
    def operator-=(o:OC): a -= o.a
    def operator*=(o:OC): a *= o.a
    def operator/=(o:OC): a /= o.a
    def operator%=(o:OC): a %= o.a
    def operator&=(o:OC): a &= o.a
    def operator|=(o:OC): a |= o.a
    def operator^=(o:OC): a ^= o.a
    def operator<<=(o:int): a <<= o.a
    def operator>>=(o:int): a >>= o.a

    def operator==(o:OC): return a == o.a
    def operator!=(o:OC): return a != o.a
    def operator<(o:OC): return a < o.a
    def operator>(o:OC): return a > o.a
    def operator<=(o:OC): return a <= o.a
    def operator>=(o:OC): return a >= o.a

    def operator~(): return OC { ~a }
    def operator-(): return OC { -a }
    def operator++(): return a++  // FIXME: post vs pre inc?
    def operator--(): return a--

    def operator=(o:OC): a = o.a

    def operator[](i:int): return if i == 0: a else: 0

// unlike the class above, this for now doesn't implement the assignment ops.
// But it's a LOT more efficient when returning new instances.

struct OS:
    a:int

    def operator+(o:OS): return OS { a + o.a }
    def operator-(o:OS): return OS { a - o.a }
    def operator*(o:OS): return OS { a * o.a }
    def operator/(o:OS): return OS { a / o.a }
    def operator%(o:OS): return OS { a % o.a }
    def operator&(o:OS): return OS { a & o.a }
    def operator|(o:OS): return OS { a | o.a }
    def operator^(o:OS): return OS { a ^ o.a }
    def operator<<(o:int): return OS { a << o.a }
    def operator>>(o:int): return OS { a >> o.a }

    def operator==(o:OS): return a == o.a
    def operator!=(o:OS): return a != o.a
    def operator<(o:OS): return a < o.a
    def operator>(o:OS): return a > o.a
    def operator<=(o:OS): return a <= o.a
    def operator>=(o:OS): return a >= o.a

    def operator~(): return OS { ~a }
    def operator-(): return OS { -a }

    def operator[](i:int): return if i == 0: a else: 0


run_test("operators"):
    // First test class instances, which have richer functionality.
    let c1 = OC { 1 }
    let c2 = OC { 2 }
    var c3 = OC { 3 }
    var c4 = OC { 4 }
    assert (c1 + c2 - c1) * c2 / c1 == c4
    c3 += c1
    assert c3 == c4  // structural equality, not pointer
    c3--
    c4 = c3  // overwrites, doesn't replace pointer!
    assert c4 == c3
    c4++
    assert (- - c4) == c4
    assert (c4 != c4) == false
    assert c4[0] == 4

    // Same with structs.
    let s1 = OS { 1 }
    let s2 = OS { 2 }
    let s3 = OS { 3 }
    let s4 = OS { 4 }
    assert (s1 + s2 - s1) * s2 / s1 == s4

    // Quaternions!
    assert -quat { 1.0, 1.0, 1.0, 1.0 } == quat { -1.0, -1.0, -1.0, 1.0 }
    assert quat { 1.0, 1.0, 1.0, 1.0 } * quat { 2.0, 2.0, 2.0, 1.0 } == quat{ 3.0, 3.0, 3.0, -5.0 }
    assert quat { 1.0, 1.0, 1.0, 1.0 } * 2.0 == quat { 2.0, 2.0, 2.0, 2.0 }