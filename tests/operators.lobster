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
    def operator<<(o:int): return OC { a << o }
    def operator>>(o:int): return OC { a >> o }

    def operator+=(o:OC): a += o.a
    def operator-=(o:OC): a -= o.a
    def operator*=(o:OC): a *= o.a
    def operator/=(o:OC): a /= o.a
    def operator%=(o:OC): a %= o.a
    def operator&=(o:OC): a &= o.a
    def operator|=(o:OC): a |= o.a
    def operator^=(o:OC): a ^= o.a
    def operator<<=(o:int): a <<= o
    def operator>>=(o:int): a >>= o

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
    def operator<<(o:int): return OS { a << o }
    def operator>>(o:int): return OS { a >> o }

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

    // Shift counts are masked to 64 bits, same as Java/C#/JS/Wasm.
    // The vars force the runtime path, the literals the constant folded one:
    // both must agree, see MaskedShiftLeft in vmdata.h.
    // `sh` is a re-assigned var on purpose: that keeps it out of constant
    // propagation, so each assert compares the folded shift against the runtime one.
    var sh = 64
    assert 1 << 64 == 1 and 1 << sh == 1
    assert 256 >> 64 == 256 and 256 >> sh == 256
    sh = 100
    assert 1 << 100 == 1 << 36 and 1 << sh == 1 << 36
    sh = -1
    assert 1 << -1 == 1 << 63 and 1 << sh == 1 << 63
    // Left shift of a negative value, and right shift staying arithmetic.
    sh = 1
    assert -1 << 1 == -2 and -1 << sh == -2
    assert -8 >> 1 == -4 and -8 >> sh == -4
    // The assigning versions go thru their own ops.
    var sha = 1
    sha <<= 100
    assert sha == 1 << 36
    var shb = 256
    shb >>= 64
    assert shb == 256