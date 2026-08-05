import testing

// Tests for compile_run_c_code(), which JITs C code with the built-in C compiler,
// and thus only works when the VM is in JIT mode itself.

run_test("c_code"):
    guard not vm_compiled_mode()

    // Allocate an output buffer with realloc() and return it thru output_buf():
    // reverse the input string.
    let rev, rev_err = compile_run_c_code("""
        int main(void) {
            unsigned long long len = input_len();
            unsigned char *in = input_buf();
            unsigned char *out = realloc(0, len);
            for (unsigned long long i = 0; i < len; i++) out[i] = in[len - 1 - i];
            output_buf(out, len);
            return 0;
        }
        """, "hello, world")
    assert not rev_err
    assert rev == "dlrow ,olleh"

    // A vector of ints as input buffer: sum the elements, return sum in string.
    let sum, sum_err = compile_run_c_code("""
        int main(void) {
            int *in = input_buf();
            unsigned long long n = input_len() / sizeof(int);
            int sum = 0;
            for (unsigned long long i = 0; i < n; i++) sum += in[i];
            char *out = realloc(0, sizeof(int));
            memcpy(out, &sum, sizeof(int));
            output_buf(out, sizeof(int));
            return 0;
        }
        """, vector_to_buffer([ 1, 2, 3, 4, 100 ]))
    assert not sum_err
    assert sum
    assert sum.read_int32_le(0) == 110

    // Transform the input buffer in-place instead of producing output:
    // returns nil, but the buffer has been modified.
    let squares = vector_to_buffer([ 1, 2, 3, 4 ])
    let ip_res, ip_err = compile_run_c_code("""
        int main(void) {
            int *in = input_buf();
            unsigned long long n = input_len() / sizeof(int);
            for (unsigned long long i = 0; i < n; i++) in[i] *= in[i];
            return 0;
        }
        """, squares)
    assert not ip_err
    assert not ip_res
    assert squares == vector_to_buffer([ 1, 4, 9, 16 ])

    // Same, but on a string (must be created at runtime, since the C code may not
    // modify string constants, which are shared!).
    let ips = concat_string([ "Hello, ", "World!" ], "")
    let ips_res, ips_err = compile_run_c_code("""
        int main(void) {
            unsigned char *in = input_buf();
            unsigned long long len = input_len();
            for (unsigned long long i = 0; i < len; i++)
                if (in[i] >= 'a' && in[i] <= 'z') in[i] -= 'a' - 'A';
            return 0;
        }
        """, ips)
    assert not ips_err
    assert not ips_res
    assert ips == "HELLO, WORLD!"

    // Code that doesn't compile gives an error and no result.
    let bad_res, bad_err = compile_run_c_code("this is not C code", "")
    assert not bad_res
    assert bad_err

    // As does code that lacks a main() function.
    let nm_res, nm_err = compile_run_c_code("int not_main(void) \{ return 0; \}", "")
    assert not nm_res
    assert nm_err
