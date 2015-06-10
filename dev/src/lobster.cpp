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

// lobster.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "sdlincludes.h"    // FIXME: this makes SDL not modular, but without it it will miss the SDLMain indirection

//#include <huffman.h>
#include "wentropy.h"

namespace lobster
{
    SlabAlloc *vmpool = nullptr;               // set during the lifetime of a VM object
    static SlabAlloc *parserpool = nullptr;    // set during the lifetime of a Parser object
}

#include "vmdata.h"
#include "natreg.h"

namespace lobster
{
    AutoRegister *autoreglist = nullptr;
    NativeRegistry natreg;
    VMBase *g_vm = nullptr;                    // set during the lifetime of a VM object
    const Type g_type_int          (V_INT);                  TypeRef type_int = &g_type_int;
    const Type g_type_float        (V_FLOAT);                TypeRef type_float = &g_type_float;
    const Type g_type_string       (V_STRING);               TypeRef type_string = &g_type_string;
    const Type g_type_any          (V_ANY);                  TypeRef type_any = &g_type_any;
    const Type g_type_vector_any   (V_VECTOR, &*type_any);   TypeRef type_vector_any = &g_type_vector_any;
    const Type g_type_vector_int   (V_VECTOR, &*type_int);   TypeRef type_vector_int = &g_type_vector_int;
    const Type g_type_vector_float (V_VECTOR, &*type_float); TypeRef type_vector_float = &g_type_vector_float;
    const Type g_type_function_null(V_FUNCTION);             TypeRef type_function_null = &g_type_function_null;
    const Type g_type_function_cocl(V_YIELD);                TypeRef type_function_cocl = &g_type_function_cocl;
    const Type g_type_coroutine    (V_COROUTINE);            TypeRef type_coroutine = &g_type_coroutine;
    const Type g_type_function_nil (V_NILABLE,
                                    &*type_function_null);   TypeRef type_function_nil = &g_type_function_nil;
}

#include "bytecode_generated.h"

// FlatBuffers takes care of backwards compatibility of all metadata, but not the actual bytecode.
// This needs to be bumped each time we make changes to the format.
const int LOBSTER_BYTECODE_FORMAT_VERSION = 1;

#include "ttypes.h"
#include "lex.h"
#include "idents.h"
#include "node.h"
#include "parser.h"
#include "typecheck.h"
#include "optimizer.h"
#include "codegen.h"

#include "disasm.h"
#include "vm.h"

using namespace lobster;

const char *fileheader = "\xA5\x74\xEF\x19";
const int fileheaderlen = 4;

struct CompiledProgram
{
    vector<uchar> bytecode;

    enum CompileFlags
    {
        PARSEDUMP = 1,
        DISASM = 2,
    };

    void Compile(const char *fn, char *stringsource, int flags)
    {
        SymbolTable st;

        Parser parser(fn, st, stringsource);
        parser.Parse();

        TypeChecker tc(parser, st);
        
        Optimizer opt(parser, st, 100);

        if (flags & PARSEDUMP)
        {
            auto dump = parser.DumpAll();
            FILE *f = OpenForWriting("parsedump.txt", false);
            if (f)
            {
                fprintf(f, "%s\n", dump.c_str());
                fclose(f);
            }
        }

        vector<int> code;
        vector<bytecode::LineInfo> linenumbers;
        CodeGen cg(parser, st, code, linenumbers);

        st.Serialize(code, linenumbers, bytecode);

        if (flags & DISASM)
        {
            FILE *f = OpenForWriting("disasm.txt", false);
            if (f)
            {
                string s;
                DisAsm(s, bytecode.data());
                fputs(s.c_str(), f);
                fclose(f);
            }
        }

        //parserpool->printstats();
    }

    void Save(const char *bcf)
    {
        vector<uchar> out;
        WEntropyCoder<true>(bytecode.data(), bytecode.size(), bytecode.size(), out);

        FILE *f = OpenForWriting(bcf, true);
        if (f)
        {
            fwrite(fileheader, fileheaderlen, 1, f);
            auto len = (uint)bytecode.size();
            fwrite(&len, sizeof(uint), 1, f);  // FIXME: not endianness-safe
            fwrite(out.data(), out.size(), 1, f);
            fclose(f);
        }
    }

    bool Load(const char *bcf)
    {
        size_t bclen = 0;
        uchar *bc = LoadFile(bcf, &bclen);
        if (!bc) return false;

        if (memcmp(fileheader, bc, fileheaderlen)) { free(bc); throw string("bytecode file corrupt: ") + bcf; }
        uint origlen = *(uint *)(bc + fileheaderlen);
        bytecode.clear();
        WEntropyCoder<false>(bc + fileheaderlen + sizeof(uint), bclen - fileheaderlen - sizeof(uint), origlen, bytecode);

        free(bc);

        flatbuffers::Verifier verifier(bytecode.data(), bytecode.size());
        auto ok = bytecode::VerifyBytecodeFileBuffer(verifier);
        assert(ok);
        return ok;
    }

    void Run(string &evalret, const char *programname)
    {
        VM vm(programname, bytecode.data());
        vm.EvalProgram(evalret);
    }
};

Value CompileRun(Value &source, bool stringiscode)
{
    ValueRef fref(source);
    string fn = stringiscode ? "string" : source.sval()->str();  // fixme: datadir + sanitize?
    SlabAlloc *parentpool = vmpool; vmpool = nullptr;
    VMBase    *parentvm   = g_vm;   g_vm = nullptr;
    try
    {
        string ret;
        CompiledProgram cp;
        cp.Compile(fn.c_str(), stringiscode ? source.sval()->str() : nullptr, 0);
        cp.Run(ret, fn.c_str());
        assert(!vmpool && !g_vm);
        vmpool = parentpool;
        g_vm = parentvm;
        g_vm->Push(Value(g_vm->NewString(ret)));
        return Value(0, V_NIL);
    }
    catch (string &s)
    {
        vmpool = parentpool;
        g_vm = parentvm;
        g_vm->Push(Value(0, V_NIL));
        return Value(g_vm->NewString(s));
    }
}

void AddCompiler()  // it knows how to call itself!
{
    STARTDECL(compile_run_code) (Value &filename)
    {
        return CompileRun(filename, true);
    }
    ENDDECL1(compile_run_code, "code", "S", "AA",
        "compiles and runs lobster source, sandboxed from the current program (in its own VM)."
        " the argument is a string of code. returns the return value of the program as a string,"
        " with an error string as second return value, or nil if none. using parse_data(),"
        " two program can communicate more complex data structures even if they don't have the same version"
        " of struct definitions.");

    STARTDECL(compile_run_file) (Value &filename)
    {
        return CompileRun(filename, false);
    }
    ENDDECL1(compile_run_file, "filename", "S", "AA",
        "same as compile_run_code(), only now you pass a filename.");
}

AutoRegister __ac("compiler", AddCompiler);

void DumpBuiltins()
{
    FILE *f = OpenForWriting("builtin_functions_reference.html", false);
    if (f)
    {
        fprintf(f, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n");
        fprintf(f, "<html>\n<head>\n<title>lobster builtin function reference</title>\n");
        fprintf(f, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n");
        fprintf(f, "<style type=\"text/css\">table.a, tr.a, td.a {font-size: 10pt;border: 1pt solid #DDDDDD;");
        fprintf(f, " border-Collapse: collapse; max-width:1200px}</style>\n");
        fprintf(f, "</head>\n<body><center><table border=0><tr><td>\n<p>lobster builtin functions:");
        fprintf(f, "(file auto generated by compiler, do not modify)</p>\n\n");
        int cursubsystem = -1;
        bool tablestarted = false;
        for (auto nf : natreg.nfuns)
        {
            if (nf->subsystemid != cursubsystem)
            {
                if (tablestarted) fprintf(f, "</table>\n");
                tablestarted = false;
                fprintf(f, "<h3>%s</h3>\n", natreg.subsystems[nf->subsystemid].c_str());
                cursubsystem = nf->subsystemid;
            }

            if (!tablestarted)
            {
                fprintf(f, "<table class=\"a\" border=1 cellspacing=0 cellpadding=4>\n");
                tablestarted = true;
            }

            fprintf(f, "<tr class=\"a\" valign=top><td class=\"a\"><tt><b>%s</b>(", nf->name.c_str());
            int i = 0;
            for (auto &a : nf->args.v)
            {
                fprintf(f, "%s%s%s<font color=\"#666666\">%s</font>%s",
                    a.type->t == V_NILABLE ? " [" : "",
                    i ? ", " : "",
                    nf->args.GetName(i).c_str(),
                    a.type->t == V_ANY ? "" : (string(":") + 
                        TypeName(a.type->t == V_NILABLE ? a.type->Element() : a.type)).c_str(),
                    a.type->t == V_NILABLE ? "]" : ""
                );
                i++;
            }
            fprintf(f, ")");
            if (nf->retvals.v.size())
            {
                fprintf(f, " -> ");
                size_t i = 0;
                for (auto &a : nf->retvals.v)
                {
                    fprintf(f, "<font color=\"#666666\">%s</font>%s",
                                TypeName(a.type).c_str(), 
                                i++ < nf->retvals.v.size() - 1 ? ", " : "");
                }
            }
            fprintf(f, "</tt></td><td class=\"a\">%s</td></tr>\n", nf->help);
        }
        fprintf(f, "</table>\n</td></tr></table></center></body>\n</html>\n");
        fclose(f);
    }
}

void DumpNames()
{
    FILE *f = OpenForWriting("builtin_functions_names.txt", false);
    if (f)
    {
        for (auto nf : natreg.nfuns) fprintf(f, "%s ", nf->name.c_str());
    }
}

int main(int argc, char* argv[])
{
    #ifdef WIN32
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    #endif

    Output(OUTPUT_INFO, "Lobster running...");

    InitTime();

    bool wait = false;
    bool from_bundle = 
    #ifdef __IOS__
        true;
    #else
        false;
    #endif

    try
    {
        if (sizeof(void *) != sizeof(int))
        {
            assert(0);
            Output(OUTPUT_WARN, "WARNING: Lobster has been compiled in 64bit mode, which doubles memory usage, "
                                "and should only be used for testing. Make sure to use a 32bit build for release.");
        }

        vector<AutoRegister *> autoregs;
        while (autoreglist)
        {
            autoregs.push_back(autoreglist);
            autoreglist = autoreglist->next;
        }
        sort(autoregs.begin(), autoregs.end(), [](AutoRegister *a, AutoRegister *b)
        {
            return strcmp(a->name, b->name) < 0;
        });
        for (auto ar : autoregs)
        {
            Output(OUTPUT_DEBUG, "subsystem: %s", ar->name);
            natreg.NativeSubSystemStart(ar->name);
            ar->regfun();
        }

        int flags = 0;
        const char *default_bcf = "default.lbc";
        const char *bcf = nullptr;

        const char *fn = nullptr;
        for (int arg = 1; arg < argc; arg++) if (argv[arg][0] == '-')
        {
            string a = argv[arg];
            if      (a == "-w") { wait = true; }
            else if (a == "-b") { bcf = default_bcf; }
            else if (a == "--parsedump") { flags |= CompiledProgram::PARSEDUMP; }
            else if (a == "--disasm")    { flags |= CompiledProgram::DISASM; }
            else if (a == "--verbose")   { min_output_level = OUTPUT_INFO; }
            else if (a == "--debug")     { min_output_level = OUTPUT_DEBUG; }
            else if (a == "--gen-builtins-html")  { DumpBuiltins(); return 0; }
            else if (a == "--gen-builtins-names") { DumpNames();    return 0; }
            else if (a == "-c") {}  // deprecated, remove this one, not needed anymore.
            else if (a.substr(0, 5) == "-psn_") { from_bundle = true; }  // process identifier supplied by OS X
            else throw string("unknown command line argument: ") + argv[arg];
        }
        else
        {
            if (fn) throw string("more than one file specified");
            fn = argv[arg];
        }

        #ifdef __ANDROID__
            fn = "pythtree.lobster";  // FIXME: temp solution
        #endif
        #ifdef __IOS__
            //fn = "totslike.lobster";  // FIXME: temp solution
        #endif

        if (!SetupDefaultDirs(argv[0], fn, from_bundle))
            throw string("cannot find location to read/write data on this platform!");

        CompiledProgram cp;

        if (!fn)
        {
            if (!cp.Load(default_bcf))
                throw string("Lobster programming language compiler/runtime (version " __DATE__ 
                             ")\nno arguments given - cannot load ") + default_bcf;
        }
        else
        {
            Output(OUTPUT_INFO, "compiling...");

            cp.Compile(StripDirPart(fn).c_str(), nullptr, flags);

            if (bcf)
            {
                cp.Save(bcf);
                return 0;
            }
        }

        string ret;
        cp.Run(ret, fn ? StripDirPart(fn).c_str() : "");
    }
    catch (string &s)
    {
        Output(OUTPUT_ERROR, s.c_str());
        if (from_bundle) MsgBox(s.c_str());
        if (wait)
        {
            Output(OUTPUT_PROGRAM, "press <ENTER> to continue:\n");
            getchar();
        }
    }

    extern void GraphicsShutDown(); GraphicsShutDown();

    #ifdef __IOS__
        exit(0); // to forcibly shut down the wrapper main()
    #endif

    return 0;
}

/*

#include <io.h>
#include <fcntl.h>

int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    AllocConsole();

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, nullptr, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, nullptr, _IONBF, 128);
    *stdin = *hf_in;


    int    argc;
    char** argv;

    char*  arg;
    int    index;
    int    result;

    // count the arguments

    argc = 1;
    arg  = lpCmdLine;

    while (arg[0] != 0) {

        while (arg[0] != 0 && arg[0] == ' ') {
            arg++;
        }

        if (arg[0] != 0) {

            argc++;

            while (arg[0] != 0 && arg[0] != ' ') {
                arg++;
            }

        }

    }

    // tokenize the arguments

    argv = (char**)malloc(argc * sizeof(char*));

    arg = lpCmdLine;
    index = 1;

    while (arg[0] != 0) {

        while (arg[0] != 0 && arg[0] == ' ') {
            arg++;
        }

        if (arg[0] != 0) {

            argv[index] = arg;
            index++;

            while (arg[0] != 0 && arg[0] != ' ') {
                arg++;
            }

            if (arg[0] != 0) {
                arg[0] = 0;
                arg++;
            }

        }

    }

    // put the program name into argv[0]

    char filename[_MAX_PATH];

    GetModuleFileNameA(nullptr, filename, _MAX_PATH);
    argv[0] = filename;

    // call the user specified main function

    result = main(argc, argv);

    free(argv);
    return result;


}

*/
