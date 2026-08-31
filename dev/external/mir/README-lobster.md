# MIR in Lobster

This is a hand picked subset of https://github.com/vnmakarov/mir, taken at commit
`a8ab7c31cd5f9b23b77d84c60b3d83e62d9d304c`. Lobster uses it as an alternative JIT backend to
libtcc, selected at runtime with `--mir` (see `dev/src/mirbind.cpp`).

Only what is needed to compile C to machine code in memory is here: `mir.c` (the IR plus the
interpreter that `MIR_link` needs for expression data), `mir-gen.c` (the machine code generator)
and `c2mir/c2mir.c` (the C front end), along with every header and `.c` file they `#include`.
Of the five CPU targets MIR supports only x86-64 and aarch64 are kept, which covers the platforms
Lobster builds for.

Left out: the command line drivers (`c2mir-driver.c`, `mir-bin-*.c`), the ppc64/s390x/riscv64
targets, `mir2c`, `llvm2mir`, `mir-utils`, and all tests and benchmarks.

To update, copy the same file list over from a newer checkout; the build files that reference
them are `dev/lobster/language.vcxproj` and `dev/CMakeLists.txt`.
