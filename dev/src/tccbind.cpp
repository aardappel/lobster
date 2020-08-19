#include "lobster/stdafx.h"

#include "libtcc.h"

namespace lobster {

void ErrorCallBack(void *err, const char *msg) {
    *((string *)err) += msg;
    *((string *)err) += "\n";
}

bool RunC(const char *source, const char *options, string &error) {
    auto state = tcc_new();
    tcc_set_output_type(state, TCC_OUTPUT_MEMORY);
    tcc_set_error_func(state, &error, ErrorCallBack);
    tcc_set_options(state, options);
    auto ret = tcc_compile_string(state, source);
    if (ret >= 0) {
        ret = tcc_relocate(state, TCC_RELOCATE_AUTO);
        if (ret >= 0) {
            typedef int (*const compiled_fun)(const int);
            auto foo = (compiled_fun)tcc_get_symbol(state, "foo");
            foo(32);
        }
    }
    tcc_delete(state);
    return ret >= 0;
}

}

void unit_test_libtcc() {
    string err;
    auto src = "int foo(int a) { return a + 1; }";
    auto ret = lobster::RunC(src, "-nostdlib", err);
    assert(ret);
    (void)ret;
}
