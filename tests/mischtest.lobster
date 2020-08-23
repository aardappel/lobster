import testing
import std
import exception
import vec

// Misc small tests that should only be run once.

run_test("misch"):
    if not vm_compiled_mode():
        let compres1, comperr1 = compile_run_code("return 1 + 2", [])
        if comperr1:
            print comperr1
        assert compres1 == "3"

        let compres2, comperr2 = compile_run_file(pakfile "plugintest.lobster", [])
        if(comperr2):
            print(comperr2)
        assert(compres2 == "plugintest return value")

