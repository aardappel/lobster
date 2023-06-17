import testing
import matrix

run_test("matrix"):

    do():
        let a = mat2x2 { 1.0, 2.0, 3.0, 4.0 }
        let b = mat2x2 { 5.0, 6.0, 7.0, 8.0 }

        assert a == a
        assert a == mat2x2 { 1.0, 2.0, 3.0, 4.0 }
        assert a != b

        assert 5.0 + a == mat2x2 { 6.0, 7.0, 8.0, 9.0 }
        assert a + 5.0 == mat2x2 { 6.0, 7.0, 8.0, 9.0 }
        assert a + b == mat2x2 { 6.0, 8.0, 10.0, 12.0 }

        assert 5.0 - a == mat2x2 { 4.0, 3.0, 2.0, 1.0 }
        assert a - 5.0 == mat2x2 { -4.0, -3.0, -2.0, -1.0 }
        assert b - a == mat2x2 { 4.0, 4.0, 4.0, 4.0 }

        assert 5.0 * a == mat2x2 { 5.0, 10.0, 15.0, 20.0 }

        // Matrix specific operations

        assert a * b == mat2x2 { 19.0, 22.0, 43.0, 50.0 }
        assert b * a == mat2x2 { 23.0, 34.0, 31.0, 46.0 }

        assert a.determinant() == -2.0
        assert a.trace() == 4.0
        assert a.adjugate() == mat2x2 { 4.0, -2.0, -3.0, 1.0 }
        assert a.inverse() == mat2x2 { -2.0, 1.0, 1.5, -0.5 }

        assert a * a.inverse() == mat2x2_identity

        // Matrix and Vector Math

        let c = float2 { 2.0, 3.0 }

        assert a * c == float2 { 8.0, 18.0 }
        assert b * c == float2 { 28.0, 38.0 }