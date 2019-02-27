include testing
include std
include exception
include vec

// Misc small tests that should only be run once.

run_test("misch"):
    if not vm_compiled_mode():
        compres1, comperr1 := compile_run_code("return 1 + 2", [])
        if comperr1:
            print comperr1
        assert compres1 == "3"

        compres2, comperr2 := compile_run_file(pakfile "plugintest.lobster", [])
        if(comperr2):
            print(comperr2)
        assert(compres2 == "plugintest return value")

        // Minimal log test with ref types:
        for(3) i:
            gl_log_frame(0.01)
            v ?= []
            assert v.length == i
            v.push(string(i))
