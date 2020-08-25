// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lobster/stdafx.h"

#include "lobster/compiler.h"
#include "lobster/disasm.h"
#include "lobster/tonative.h"

#if LOBSTER_ENGINE
    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.
    #include "lobster/sdlincludes.h"
    #include "lobster/sdlinterface.h"
#endif

#ifndef GIT_COMMIT_INFOSTR
#define GIT_COMMIT_INFOSTR __DATE__ "|unknown"
#endif

using namespace lobster;

void unit_test_all() {
    // We don't really have unit tests, but let's collect some that always
    // run in debug mode:
    #ifdef NDEBUG
        return;
    #endif
    unit_test_tools();
    unit_test_unicode();
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        #ifdef _MSC_VER
            InitUnhandledExceptionFilter(argc, argv);
        #endif
    #endif
    LOG_INFO("Lobster running...");
    bool wait = false;
    bool from_bundle =
    #ifdef __IOS__
        true;
    #else
        false;
    #endif
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        bool parsedump = false;
        bool disasm = false;
        bool dump_builtins = false;
        bool dump_names = false;
        bool compile_only = false;
        int runtime_checks = RUNTIME_ASSERT;
        const char *default_lpak = "default.lpak";
        const char *lpak = nullptr;
        const char *fn = nullptr;
        vector<string> program_args;
        auto trace = TraceMode::OFF;
        auto jit_mode = true;
        string helptext = "\nUsage:\n"
            "lobster [ OPTIONS ] [ FILE ] [ -- ARGS ]\n"
            "Compile & run FILE, or omit FILE to load default.lpak\n"
            "--pak                  Compile to pakfile, don't run.\n"
            "--cpp                  Compile to C++ code, don't run (see implementation.md!).\n"
            "--parsedump            Also dump parse tree.\n"
            "--disasm               Also dump bytecode disassembly.\n"
            "--verbose              Output additional informational text.\n"
            "--debug                Output compiler internal logging.\n"
            "--silent               Only output errors.\n"
            "--runtime-shipping     Compile with asserts off.\n"
            "--runtime-asserts      Compile with asserts on (default).\n"
            "--runtime-verbose      Compile with asserts on + additional debug.\n"
            "--noconsole            Close console window (Windows).\n"
            "--gen-builtins-html    Write builtin commands help file.\n"
            "--gen-builtins-names   Write builtin commands - just names.\n"
            #if LOBSTER_ENGINE
            "--non-interactive-test Quit after running 1 frame.\n"
            #endif
            "--trace                Log bytecode instructions (SLOW).\n"
            "--trace-tail           Show last 50 bytecode instructions on error.\n"
            "--wait                 Wait for input before exiting.\n";
            int arg = 1;
        for (; arg < argc; arg++) {
            if (argv[arg][0] == '-') {
                string a = argv[arg];
                if      (a == "--wait") { wait = true; }
                else if (a == "--pak") { lpak = default_lpak; }
                else if (a == "--cpp") { jit_mode = false; }
                else if (a == "--parsedump") { parsedump = true; }
                else if (a == "--disasm") { disasm = true; }
                else if (a == "--verbose") { min_output_level = OUTPUT_INFO; }
                else if (a == "--debug") { min_output_level = OUTPUT_DEBUG; }
                else if (a == "--silent") { min_output_level = OUTPUT_ERROR; }
                else if (a == "--runtime-shipping") { runtime_checks = RUNTIME_NO_ASSERT; }
                else if (a == "--runtime-asserts") { runtime_checks = RUNTIME_ASSERT; }
                else if (a == "--runtime-verbose") { runtime_checks = RUNTIME_ASSERT_PLUS; }
                else if (a == "--noconsole") { SetConsole(false); }
                else if (a == "--gen-builtins-html") { dump_builtins = true; }
                else if (a == "--gen-builtins-names") { dump_names = true; }
                else if (a == "--compile-only") { compile_only = true; }
                #if LOBSTER_ENGINE
                else if (a == "--non-interactive-test") { SDLTestMode(); }
                #endif
                else if (a == "--trace") { trace = TraceMode::ON; }
                else if (a == "--trace-tail") { trace = TraceMode::TAIL; }
                else if (a == "--") { arg++; break; }
                // process identifier supplied by OS X
                else if (a.substr(0, 5) == "-psn_") { from_bundle = true; }
                else THROW_OR_ABORT("unknown command line argument: " + (argv[arg] + helptext));
            } else {
                if (fn) THROW_OR_ABORT("more than one file specified" + helptext);
                fn = argv[arg];
            }
        }
        for (; arg < argc; arg++) { program_args.push_back(argv[arg]); }

        unit_test_all();

        #ifdef __IOS__
            //fn = "totslike.lobster";  // FIXME: temp solution
        #endif

        NativeRegistry nfr;
        RegisterCoreLanguageBuiltins(nfr);
        auto loader = EnginePreInit(nfr);

        if (!InitPlatform(GetMainDirFromExePath(argv[0]), fn ? fn : default_lpak, from_bundle,
                          loader))
            THROW_OR_ABORT("cannot find location to read/write data on this platform!");

        LOG_INFO("lobster version " GIT_COMMIT_INFOSTR);

        if (fn) fn = StripDirPart(fn);

        string bytecode_buffer;
        if (!fn) {
            if (!LoadPakDir(default_lpak))
                THROW_OR_ABORT("Lobster programming language compiler/runtime (version "
                               GIT_COMMIT_INFOSTR ")\nno arguments given - cannot load "
                               + (default_lpak + helptext));
            // This will now come from the pakfile.
            if (!LoadByteCode(bytecode_buffer))
                THROW_OR_ABORT("Cannot load bytecode from pakfile!");
        } else {
            LOG_INFO("compiling...");
            string dump;
            string pakfile;
            auto start_time = SecondsSinceStart();
            dump.clear();
            pakfile.clear();
            bytecode_buffer.clear();
            Compile(nfr, StripDirPart(fn), {}, bytecode_buffer,
                    parsedump ? &dump : nullptr, lpak ? &pakfile : nullptr, dump_builtins,
                    dump_names, false, runtime_checks);
            LOG_INFO("time to compile (seconds): ", SecondsSinceStart() - start_time);
            if (parsedump) {
                WriteFile("parsedump.txt", false, dump);
            }
            if (lpak) {
                WriteFile(lpak, true, pakfile);
                return 0;
            }
        }
        if (disasm) {
            string sd;
            DisAsm(nfr, sd, bytecode_buffer);
            WriteFile("disasm.txt", false, sd);
        }
        if (jit_mode) {
            RunTCC(nfr, bytecode_buffer, fn ? fn : "", std::move(program_args), trace, compile_only);
        } else {
            string sd;
            auto err = ToCPP(nfr, sd, bytecode_buffer, true);
            if (!err.empty()) THROW_OR_ABORT(err);
            // FIXME: make less hard-coded.
            auto out = "dev/compiled_lobster/src/compiled_lobster.cpp";
            FILE *f = fopen((MainDir() + out).c_str(), "w");
            if (f) {
                fputs(sd.c_str(), f);
                fclose(f);
            } else {
                THROW_OR_ABORT(cat("cannot write: ", out));
            }
        }
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        LOG_ERROR(s);
        #if LOBSTER_ENGINE
            if (from_bundle) SDLMessageBox("Lobster", s.c_str());
        #endif
        if (wait) {
            LOG_PROGRAM("press <ENTER> to continue:\n");
            getchar();
        }
        #ifdef _WIN32
            _CrtSetDbgFlag(0);  // Don't bother with memory leaks when there was an error.
        #endif
        return 1;
    }
    #endif
    return 0;
}

