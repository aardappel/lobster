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
#include "lobster/stdafx.h"

#include "lobster/natreg.h"
#include "lobster/compiler.h"

#include "lobster/lex.h"
#include "lobster/idents.h"
#include "lobster/node.h"

#include "lobster/parser.h"
#include "lobster/declcheck.h"
#include "lobster/typecheck.h"
#include "lobster/constval.h"
#include "lobster/optimizer.h"
#include "lobster/tonative.h"
#include "lobster/codegen.h"

namespace lobster {

SlabAlloc *g_current_slaballoc = nullptr;

const Type g_type_int(V_INT);
const Type g_type_float(V_FLOAT);
const Type g_type_string(V_STRING);
const Type g_type_any(V_ANY);
const Type g_type_vector_any(V_VECTOR, &g_type_any);
const Type g_type_vector_int(V_VECTOR, &g_type_int);
const Type g_type_vector_float(V_VECTOR, &g_type_float);
const Type g_type_function_null_void(V_FUNCTION);  // no args, void return.
const Type g_type_resource(V_RESOURCE);
const Type g_type_vector_resource(V_VECTOR, &g_type_resource);
const Type g_type_typeid(V_TYPEID, &g_type_any);
const Type g_type_void(V_VOID);
const Type g_type_undefined(V_UNDEFINED);

TypeRef type_int = &g_type_int;
TypeRef type_float = &g_type_float;
TypeRef type_string = &g_type_string;
TypeRef type_any = &g_type_any;
TypeRef type_vector_int = &g_type_vector_int;
TypeRef type_vector_float = &g_type_vector_float;
TypeRef type_function_null_void = &g_type_function_null_void;
TypeRef type_resource = &g_type_resource;
TypeRef type_vector_resource = &g_type_vector_resource;
TypeRef type_typeid = &g_type_typeid;
TypeRef type_void = &g_type_void;
TypeRef type_undefined = &g_type_undefined;

const Type g_type_vector_string(V_VECTOR, &g_type_string);
const Type g_type_vector_vector_int(V_VECTOR, &g_type_vector_int);
const Type g_type_vector_vector_float(V_VECTOR, &g_type_vector_float);
const Type g_type_vector_vector_vector_float(V_VECTOR, &g_type_vector_vector_float);

NumStruct g_ns_int_unknown{ V_INT, -1 };
NumStruct g_ns_float_unknown{ V_FLOAT, -1 };

NumStruct g_ns_int[] = { { V_INT, 1 }, { V_INT, 2 }, { V_INT, 3 }, { V_INT, 4 } };
NumStruct g_ns_float[] = { { V_FLOAT, 1 }, { V_FLOAT, 2 }, { V_FLOAT, 3 }, { V_FLOAT, 4 } };

const Type g_type_ns_int_unknown(&g_ns_int_unknown);
const Type g_type_ns_float_unknown(&g_ns_float_unknown);

const Type g_type_ns_int[] = { &g_ns_int[0], &g_ns_int[1], &g_ns_int[2], &g_ns_int[3] };
const Type g_type_ns_float[] = { &g_ns_float[0], &g_ns_float[1], &g_ns_float[2], &g_ns_float[3] };

const Type g_type_vector_ns_int[] = {
    { V_VECTOR, &g_type_ns_int_unknown },
    { V_VECTOR, &g_type_ns_int_unknown },
    { V_VECTOR, &g_type_ns_int[0] },
    { V_VECTOR, &g_type_ns_int[1] },
    { V_VECTOR, &g_type_ns_int[2] },
    { V_VECTOR, &g_type_ns_int[3] }
};
const Type g_type_vector_ns_float[] = {
    { V_VECTOR, &g_type_ns_float_unknown },
    { V_VECTOR, &g_type_ns_float_unknown },
    { V_VECTOR, &g_type_ns_float[0] },
    { V_VECTOR, &g_type_ns_float[1] },
    { V_VECTOR, &g_type_ns_float[2] },
    { V_VECTOR, &g_type_ns_float[3] }
};
const Type g_type_vector_vector_ns_int[] = {
    { V_VECTOR, &g_type_vector_ns_int[0] },
    { V_VECTOR, &g_type_vector_ns_int[1] },
    { V_VECTOR, &g_type_vector_ns_int[2] },
    { V_VECTOR, &g_type_vector_ns_int[3] },
    { V_VECTOR, &g_type_vector_ns_int[4] },
    { V_VECTOR, &g_type_vector_ns_int[5] }
};
const Type g_type_vector_vector_ns_float[] = {
    { V_VECTOR, &g_type_vector_ns_float[0] },
    { V_VECTOR, &g_type_vector_ns_float[1] },
    { V_VECTOR, &g_type_vector_ns_float[2] },
    { V_VECTOR, &g_type_vector_ns_float[3] },
    { V_VECTOR, &g_type_vector_ns_float[4] },
    { V_VECTOR, &g_type_vector_ns_float[5] }
};

ResourceType *g_resource_type_list = nullptr;

TypeRef WrapKnown(UnTypeRef elem, ValueType with) {
    if (with == V_VECTOR) {
        switch (elem->t) {
            case V_ANY:      return &g_type_vector_any;
            case V_INT:      return elem->e ? nullptr : type_vector_int;
            case V_FLOAT:    return type_vector_float;
            case V_STRING:   return &g_type_vector_string;
            case V_RESOURCE: return &elem->rt->thistypevec;
            case V_VECTOR:   switch (elem->sub->t) {
                case V_INT:    return elem->sub->e ? nullptr : &g_type_vector_vector_int;
                case V_FLOAT:  return &g_type_vector_vector_float;
                case V_VECTOR: switch (elem->sub->sub->t) {
                    case V_FLOAT: return &g_type_vector_vector_vector_float;
                    default:      return nullptr;
                }
                case V_STRUCT_NUM: {
                    auto ns = elem->sub->ns;
                    assert((ns->flen >= 1 && ns->flen <= 4) || ns->flen == -1);
                    return &(ns->t == V_INT ? g_type_vector_vector_ns_int
                                            : g_type_vector_vector_ns_float)[ns->flen + 1];
                }
                default:
                    return nullptr;
            }
            case V_STRUCT_NUM: {
                auto ns = elem->ns;
                assert((ns->flen >= 1 && ns->flen <= 4) || ns->flen == -1);
                return &(ns->t == V_INT ? g_type_vector_ns_int
                                        : g_type_vector_ns_float)[ns->flen + 1];
            }
            default:
                return nullptr;
        }
    } else if (with == V_NIL) {
        switch (elem->t) {
            case V_ANY:       { static const Type t(V_NIL, &g_type_any); return &t; }
            case V_INT:       { static const Type t(V_NIL, &g_type_int); return elem->e ? nullptr : &t; }
            case V_FLOAT:     { static const Type t(V_NIL, &g_type_float); return &t; }
            case V_STRING:    { static const Type t(V_NIL, &g_type_string); return &t; }
            case V_RESOURCE:  { return &elem->rt->thistypenil; }
            case V_VECTOR: switch (elem->sub->t) {
                case V_INT:    { static const Type t(V_NIL, &g_type_vector_int); return elem->sub->e ? nullptr : &t; }
                case V_FLOAT:  { static const Type t(V_NIL, &g_type_vector_float); return &t; }
                case V_STRING: { static const Type t(V_NIL, &g_type_vector_string); return &t; }
                default: return nullptr;
            }
            default: return nullptr;
        }
    } else if (with == V_STRUCT_NUM) {
        switch (elem->t) {
            case V_INT:      return &g_type_ns_int_unknown;
            case V_FLOAT:    return &g_type_ns_float_unknown;
            default: return nullptr;
        }
    } else {
        return nullptr;
    }
}

TypeRef FixedNumStruct(ValueType num, int flen) {
    assert(flen >= 1 && flen <= 4);
    return &(num == V_INT ? g_type_ns_int : g_type_ns_float)[flen - 1];
}

void RegisterBuiltin(NativeRegistry &nfr, const char *ns, const char *name,
                     const BuiltinGroup &group) {
    LOG_DEBUG("subsystem: ", name);
    nfr.NativeSubSystemStart(ns, name);
    nfr.RegisterGroup(group);
}

void PrepQuery(Query &query, vector<pair<string, string>> &filenames) {
    for (auto [i, fn] : enumerate(filenames)) {
        if (fn.first == query.file) {
            query.qloc.fileidx = (int)i;
            break;
        }
    }
    if (query.qloc.fileidx < 0) {
        THROW_OR_ABORT("query file not part of compilation: " + query.file);
    }
    query.qloc.line = parse_int<int>(query.line);
    query.filenames = &filenames;
}

void Compile(NativeRegistry &nfr, string_view fn, string_view stringsource,
             const CompileOptions &opts, string &metadata_buffer, string &c_codegen,
             string *parsedump, string *pakfile) {
    #ifdef NDEBUG
        SlabAlloc slaballoc;
        if (g_current_slaballoc) THROW_OR_ABORT("nested slab allocator use");
        g_current_slaballoc = &slaballoc;
        struct SlabReset {
            ~SlabReset() {
                g_current_slaballoc = nullptr;
            }
        } slabreset;
    #endif
    vector<pair<string, string>> filenames;
    Lex lex(fn, filenames, nfr.namespaces, stringsource, opts.max_errors);
    SymbolTable st(lex);
    Parser parser(nfr, lex, st);
    parser.Parse();
    DeclChecker dc(st, nfr);
    dc.Check();
    if (opts.query) PrepQuery(*opts.query, filenames);
    TypeChecker tc(parser, st, opts);
    if (opts.query) {
        // The typechecker did not come across the location.
        if (!tc.ProcessQuery()) THROW_OR_ABORT("query_unknown_ident: " + opts.query->iden);
    }
    if (lex.num_errors) THROW_OR_ABORT("errors encountered, aborting");
    tc.Stats(filenames);
    // Optimizer is not optional, must always run, since TypeChecker and CodeGen
    // rely on it culling const if-thens and other things.
    Optimizer opt(st, tc, opts.runtime_checks);
    if (parsedump) *parsedump = parser.DumpAll(true);
    auto src_hash = lex.HashAll();
    CodeGen cg(parser, st, opts, src_hash, c_codegen);
    st.Serialize(cg.type_table, cg.sids, cg.stringtable, metadata_buffer, filenames, cg.ser_ids, src_hash);
    if (pakfile) {
        auto err = BuildPakFile(*pakfile, metadata_buffer, parser.pakfiles, src_hash,
                                opts.code_pak ? c_codegen : string());
        if (!err.empty()) THROW_OR_ABORT(err);
    }
}

pair<string, iint> RunJIT(NativeRegistry &nfr, string_view fn, string_view metadata_buffer,
                          const string &c_codegen, vector<string> &&program_args,
                          const CompileOptions &copts, const RunOptions &ropts, string &error) {
    #if VM_JIT_MODE
        const char *export_names[] = { "compiled_entry_point", "vtables", "object_decs",
                                       nullptr };
        assert(!nfr.jit_imports.empty());
        auto &jit_options = copts.jit_options;
        auto start_time = SecondsSinceStart();
        pair<string, iint> ret;
        auto ok = RunC(
            c_codegen.c_str(), ropts.object_name, error, nfr.jit_imports.data(), export_names,
            jit_options,
            [&](void **exports) -> bool {
                LOG_INFO("time to ", jit_options.mir ? "mir" : "tcc",
                         " (seconds): ", SecondsSinceStart() - start_time);
                if (ropts.compile_only) return true;
                // Verify the bytecode.
                flatbuffers::Verifier verifier((uint8_t *)metadata_buffer.data(), metadata_buffer.size());
                auto ok = metadata::VerifyMetadataFileBuffer(verifier);
                if (!ok) THROW_OR_ABORT("metadata file failed to verify");
                auto bcf = metadata::GetMetadataFile(metadata_buffer.data());
                if (bcf->metadata_version() != LOBSTER_METADATA_FORMAT_VERSION)
                    THROW_OR_ABORT("metadata is from a different version of Lobster");
                vector<type_elem_t> type_table;
                for (flatbuffers::uoffset_t i = 0; i < bcf->typetable()->size(); i++) {
                    type_table.push_back((type_elem_t)bcf->typetable()->Get(i));
                }
                vector<string_view> stringtable;
                for (flatbuffers::uoffset_t i = 0; i < bcf->stringtable()->size(); i++) {
                    stringtable.push_back(bcf->stringtable()->Get(i)->string_view());
                }
                vector<string_view> file_names;
                for (flatbuffers::uoffset_t i = 0; i < bcf->filenames()->size(); i++) {
                    file_names.push_back(bcf->filenames()->Get(i)->string_view());
                }
                vector<string_view> function_names;
                for (flatbuffers::uoffset_t i = 0; i < bcf->functions()->size(); i++) {
                    function_names.push_back(bcf->functions()->Get(i)->name()->string_view());
                }
                vector<VMUDT> udts;
                vector<VMField> fields;
                for (flatbuffers::uoffset_t i = 0; i < bcf->udts()->size(); i++) {
                    auto udt = bcf->udts()->Get(i);
                    for (flatbuffers::uoffset_t j = 0; j < udt->fields()->size(); j++) {
                        auto field = udt->fields()->Get(j);
                        fields.push_back(VMField{ field->name()->string_view(), field->offset() });
                    }
                }
                size_t off = 0;
                for (flatbuffers::uoffset_t i = 0; i < bcf->udts()->size(); i++) {
                    auto udt = bcf->udts()->Get(i);
                    auto fspan = span(fields.data() + off, fields.data() + off + udt->fields()->size());
                    udts.push_back(VMUDT{ udt->name()->string_view(), udt->idx(), udt->size(),
                                          udt->super_idx(), udt->typeidx(), fspan });
                    off += udt->fields()->size();
                }
                vector<VMSpecIdent> specidents;
                for (flatbuffers::uoffset_t i = 0; i < bcf->specidents()->size(); i++) {
                    auto sid = bcf->specidents()->Get(i);
                    auto id = bcf->idents()->Get(sid->ididx());
                    specidents.push_back(VMSpecIdent {
                        id->name()->string_view(), sid->idx(), sid->typeidx(),
                        sid->used_as_freevar(), id->readonly(), id->global() });
                }
                vector<VMEnum> enums;
                vector<VMEnumVal> enumvals;
                for (flatbuffers::uoffset_t i = 0; i < bcf->enums()->size(); i++) {
                    auto e = bcf->enums()->Get(i);
                    for (flatbuffers::uoffset_t j = 0; j < e->vals()->size(); j++) {
                        auto ev = e->vals()->Get(j);
                        enumvals.push_back(VMEnumVal{ ev->name()->string_view(), ev->val() });
                    }
                }
                off = 0;
                for (flatbuffers::uoffset_t i = 0; i < bcf->enums()->size(); i++) {
                    auto e = bcf->enums()->Get(i);
                    auto fspan = span(enumvals.data() + off,
                                           enumvals.data() + off + e->vals()->size());
                    enums.push_back(VMEnum{ e->name()->string_view(), fspan, e->flags() });
                    off += e->vals()->size();
                }
                vector<int> ser_ids;
                for (flatbuffers::uoffset_t i = 0; i < bcf->ser_ids()->size(); i++) {
                    ser_ids.push_back(bcf->ser_ids()->Get(i));
                }
                vector<int> subfunctions_to_function;
                for (flatbuffers::uoffset_t i = 0; i < bcf->subfunctions_to_function()->size();
                     i++) {
                    subfunctions_to_function.push_back(bcf->subfunctions_to_function()->Get(i));
                }               
                VMMetaData vmmeta = {
                    bcf->metadata_version(),
                    span(type_table),
                    span(stringtable),
                    span(file_names),
                    span(function_names),
                    span(udts),
                    span(specidents),
                    span(enums),
                    span(ser_ids),
                    bcf->build_info()->string_view(),
                    bcf->src_hash(),
                    span(subfunctions_to_function),
                };
                auto vmargs = VMArgs {
                    nfr, string(fn), &vmmeta,
                    std::move(program_args),
                    (fun_base_t *)exports[1], (object_dec_t *)exports[2],
                    (fun_base_t)exports[0], ropts.dump_leaks,
                    copts.runtime_checks, ropts.stack_trace_python_ordering, jit_options
                };
                lobster::VMAllocator vma(std::move(vmargs));
                vma.vm->EvalProgram();
                ret = vma.vm->evalret;
                return true;
            });
        if (!ok || !error.empty()) {
            // So we can see what the problem is..
            FILE *f = fopen((MainDir() + "compiled_lobster_jit_debug.c").c_str(), "w");
            if (f) {
                fputs(c_codegen.c_str(), f);
                fclose(f);
            }
            error = (jit_options.mir ? "MIR JIT error: " : "libtcc JIT error: ") + string(fn) +
                    ":\n" + error;
        }
        return ret;
    #else
        (void)nfr;
        (void)fn;
        (void)metadata_buffer;
        (void)c_codegen;
        (void)program_args;
        (void)copts;
        (void)ropts;
        error = "cannot JIT code: JIT backend not enabled";
        return { "", 0 };
    #endif
}

LString *CompileRun(VM &parent_vm, LString **result, Value source, bool stringiscode,
                 vector<string> &&args) {
    string_view fn = stringiscode ? "string" : source.sval()->strv();  // fixme: datadir + sanitize?
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        CompileOptions opts;
        opts.return_value = true;
        // FIXME: let the caller decide on the runtime checks?
        opts.jit_options = parent_vm.vma.jit_options;
        string metadata_buffer;
        string c_codegen;
        Compile(parent_vm.vma.nfr, fn, stringiscode ? source.sval()->strv() : string_view(),
                opts, metadata_buffer, c_codegen);
        string error;
        auto ret = RunJIT(parent_vm.vma.nfr, fn, metadata_buffer, c_codegen, std::move(args), opts,
                          RunOptions(), error);
        if (!error.empty()) THROW_OR_ABORT(error);
        *result = parent_vm.NewString(ret.first);
        return nullptr;
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        *result = parent_vm.NewString("nil");
        return parent_vm.NewString(s);
    }
    #endif
}

#if VM_JIT_MODE

// Context for compile_run_c_code(), accessible from the C code thru the functions below.
// Thread-local such that multiple VMs on different threads can each use this builtin.
struct CCodeContext {
    const void *input = nullptr;
    size_t input_len = 0;
    void *output = nullptr;
    size_t output_len = 0;
    bool output_set = false;
};

static thread_local CCodeContext *g_c_code_ctx = nullptr;

static void *CCodeInputBuf() {
    return (void *)g_c_code_ctx->input;
}

static size_t CCodeInputLen() {
    return g_c_code_ctx->input_len;
}

static void CCodeOutputBuf(void *p, size_t len) {
    auto ctx = g_c_code_ctx;
    // If called multiple times, only the last buffer is kept.
    if (ctx->output_set && ctx->output && ctx->output != ctx->input) free(ctx->output);
    ctx->output = p;
    ctx->output_len = len;
    ctx->output_set = true;
}

#endif  // VM_JIT_MODE

LString *CompileRunCCode(VM &vm, LString **result, Value code, Value input) {
    #if VM_JIT_MODE
        auto err = [&](string msg) {
            *result = nullptr;
            return vm.NewString(msg);
        };
        CCodeContext ctx;
        ctx.input = input.sval()->data();
        ctx.input_len = (size_t)input.sval()->len;
        // These are available to the C code without needing to declare them.
        auto prelude =
            "void *input_buf(void);\n"
            "unsigned long long input_len(void);\n"
            "void output_buf(void *p, unsigned long long len);\n"
            "void *malloc(unsigned long long size);\n"
            "void *realloc(void *p, unsigned long long size);\n"
            "void free(void *p);\n"
            "void *memcpy(void *dest, const void *src, unsigned long long n);\n"
            "void *memmove(void *dest, const void *src, unsigned long long n);\n"
            "void *memset(void *s, int c, unsigned long long n);\n"
            "unsigned long long strlen(const char *s);\n"
            "#line 1 \"c_code\"\n";
        auto source = prelude + string(code.sval()->strv());
        // memmove is already added by RunC() itself.
        static const void *imports[] = {
            "input_buf", (const void *)CCodeInputBuf,
            "input_len", (const void *)CCodeInputLen,
            "output_buf", (const void *)CCodeOutputBuf,
            "malloc", (const void *)malloc,
            "realloc", (const void *)realloc,
            "free", (const void *)free,
            "memcpy", (const void *)memcpy,
            "memset", (const void *)memset,
            "strlen", (const void *)strlen,
            nullptr
        };
        const char *export_names[] = { "main", nullptr };
        string error;
        auto prev_ctx = g_c_code_ctx;
        g_c_code_ctx = &ctx;
        auto ok = RunC(source.c_str(), nullptr, error, imports, export_names, vm.vma.jit_options,
            [&](void **exports) -> bool {
                auto cmain = (int (*)())exports[0];
                if (!cmain) {
                    error = "C code must contain a main() function";
                    return false;
                }
                cmain();
                return true;
            });
        g_c_code_ctx = prev_ctx;
        auto free_output = [&]() {
            if (ctx.output_set && ctx.output && ctx.output != ctx.input) free(ctx.output);
        };
        if (!ok || !error.empty()) {
            free_output();
            return err("compile_run_c_code: " + error);
        }
        if (ctx.output_set) {
            auto sv = ctx.output ? string_view((const char *)ctx.output, ctx.output_len)
                                 : string_view();
            *result = vm.NewString(sv);
            free_output();
        } else {
            *result = nullptr;
        }
        return nullptr;
    #else
        (void)code;
        (void)input;
        *result = nullptr;
        return vm.NewString("cannot JIT code: libtcc not enabled");
    #endif
}

// The compiler knows how to call itself!
BuiltinGroup compiler_builtins;
#define BUILTIN_GROUP compiler_builtins
#define BUILTIN_SYM(name) builtin_##name

BUILTIN(compile_run_code, "code,args", "SS]", "SS?",
    "compiles and runs lobster source, sandboxed from the current program (in its own VM)."
    " the argument is a string of code. returns the return value of the program as a string,"
    " with an error string as second return value, or nil if none. using parse_data(),"
    " two program can communicate more complex data structures even if they don't have the same"
    " version of struct definitions.")
(VM &vm, LString **result, LString *filename, LVector *args) {
    return CompileRun(vm, result, filename, true, ValueToVectorOfStrings(args));
}

BUILTIN(compile_run_file, "filename,args", "SS]", "SS?",
    "same as compile_run_code(), only now you pass a filename.")
(VM &vm, LString **result, LString *filename, LVector *args) {
    return CompileRun(vm, result, filename, false, ValueToVectorOfStrings(args));
}

BUILTIN(compile_run_c_code, "code,input", "SS", "S?S?",
    "compiles and runs C source code using the built-in C compiler (available in JIT mode only)."
    " the code must contain a main() function, which will be called."
    " input must be a string, whose (mutable) contents are available"
    " to the C code thru input_buf() and input_len() (in bytes)."
    " additionally available: output_buf(ptr, len) to return a buffer allocated thru"
    " malloc/realloc as a string (ownership is transferred, do not free), and further"
    " malloc/realloc/free/memcpy/memmove/memset/strlen. all these are pre-declared."
    " returns the output as a string (or nil if output_buf() was never called), plus an error"
    " string as second return value (nil if none). note the C code can also modify the input"
    " buffer in-place as a way to return data.")
(VM &vm, LString **result, LString *code, LString *input) {
    return CompileRunCCode(vm, result, code, input);
}

void RegisterCoreLanguageBuiltins(NativeRegistry &nfr) {
    extern BuiltinGroup core_builtins;
    extern BuiltinGroup file_builtins;
    extern BuiltinGroup flatbuffers_builtins;
    extern BuiltinGroup parsedata_builtins;
    extern BuiltinGroup matrix_builtins;
    extern BuiltinGroup noise_builtins;

    RegisterBuiltin(nfr, "", "builtin", core_builtins);
    RegisterBuiltin(nfr, "", "compiler", compiler_builtins);
    RegisterBuiltin(nfr, "", "file", file_builtins);
    RegisterBuiltin(nfr, "flatbuffers", "flatbuffers", flatbuffers_builtins);
    RegisterBuiltin(nfr, "", "parsedata", parsedata_builtins);
    RegisterBuiltin(nfr, "matrix", "matrix", matrix_builtins);
    RegisterBuiltin(nfr, "", "noise", noise_builtins);
}

#if !LOBSTER_ENGINE
FileLoader EnginePreInit(NativeRegistry &nfr) {
    nfr.DoneRegistering();
    return DefaultLoadFile;
}
#endif

extern "C" int RunCompiledCodeMain(int argc, const char *const *argv, const VMMetaData *vmmeta,
                                   const lobster::fun_base_t *vtables,
                                   const lobster::object_dec_t *object_decs,
                                   void *custom_pre_init, const char *aux_src_path) {
    #ifdef _MSC_VER
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        InitUnhandledExceptionFilter(argc, (char **)argv);
    #endif
    #ifdef USE_EXCEPTION_HANDLING
    try
    #endif
    {
        NativeRegistry nfr;
        RegisterCoreLanguageBuiltins(nfr);
        if (custom_pre_init) ((void (*)(NativeRegistry &))(custom_pre_init))(nfr);
        auto loader = EnginePreInit(nfr);
        min_output_level = OUTPUT_WARN;
        InitPlatform(GetMainDirFromExePath(argv[0]), aux_src_path, false, loader);
        auto from_lpak = true;
        uint64_t src_hash = 0;
        if (!LoadPakDir("default.lpak", src_hash)) {
            // FIXME: this is optional, we don't know if the compiled code wants to load this
            // file, so we don't error or even warn if this file can't be found.
            from_lpak = false;
        }
        auto vmargs = VMArgs {
            nfr,
            from_lpak ? string{} : StripDirPart(string_view_nt(argv[0])),
            vmmeta,
            {},
            vtables,
            object_decs,
            nullptr,
            false,
            RUNTIME_ASSERT
        };
        for (int arg = 1; arg < argc; arg++) { vmargs.program_args.push_back(argv[arg]); }
        lobster::VMAllocator vma(std::move(vmargs));
        if (from_lpak && src_hash != vma.vm->vma.meta->src_hash) {
            THROW_OR_ABORT("lpak file from different version of the source code than the compiled code");
        }
        vma.vm->EvalProgram();
        // Reflects any set_exit_code() calls, just like the JIT mode main().
        return (int)vma.vm->evalret.second;
    }
    #ifdef USE_EXCEPTION_HANDLING
    catch (string &s) {
        LOG_ERROR(s);
        return 1;
    }
    #endif
    return 0;
}

}  // namespace lobster

stack_vector_storage g_svs1;
stack_vector_storage g_svs2;
stack_vector_storage g_svs4;
stack_vector_storage g_svs8;

#if STACK_PROFILING_ON
    vector<StackProfile> stack_profiles;
#endif
