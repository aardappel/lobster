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

#ifndef LOBSTER_NATREG
#define LOBSTER_NATREG

#include "lobster/vmdata.h"
#include "lobster/type.h"

// The BUILTIN macros below give builtins C linkage while they take and return C++ types, which
// is fine for our purposes, so these warnings are not useful.
#ifdef __clang__
    #pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#elif defined(_MSC_VER)
    #pragma warning(disable: 4190)
#endif

namespace lobster {

enum NArgFlags {
    NF_NONE               = 0,
    NF_SUBARG1            = 1 << 0,
    NF_SUBARG2            = 1 << 1,
    NF_SUBARG3            = 1 << 2,
    NF_ANYVAR             = 1 << 3,
    NF_CONVERTANYTOSTRING = 1 << 4,
    NF_PUSHVALUEWIDTH     = 1 << 5,
    NF_BOOL               = 1 << 6,
    NF_UNION              = 1 << 7,
    NF_CONST              = 1 << 8,
};
DEFINE_BITWISE_OPERATORS_FOR_ENUM(NArgFlags)

struct Ident;
struct SpecIdent;
struct NativeFun;

struct Narg {
    TypeRef vttype = type_undefined;
    RTType rttype = RTT_NIL;
    NArgFlags flags = NF_NONE;
    string_view name;
    char default_val = 0;
    Lifetime lt = LT_UNDEF;
    bool optional = false;

    void Set(const char *&tid, Lifetime def, Named *nf) {
        char t = *tid++;
        flags = NF_NONE;
        lt = def;
        switch (t) {
            case 'A': vttype = type_any; break;
            case 'I': vttype = type_int; break;
            case 'B': vttype = type_int; flags = flags | NF_BOOL; break;
            case 'F': vttype = type_float; break;
            case 'S': vttype = type_string; break;
            case 'L': vttype = type_function_null_void; break;  // NOTE: only used by call_function_value(), and hash(), in gui.lobster
            case 'R': vttype = type_resource; break;
            case 'T': vttype = type_typeid; break;
            default: nf->Error("illegal type code");
        }
        while (*tid && !isupper(*tid)) {
            switch (*tid++) {
                case 0: break;
                case '1': flags = flags | NF_SUBARG1; break;
                case '2': flags = flags | NF_SUBARG2; break;
                case '3': flags = flags | NF_SUBARG3; break;
                case 'u': flags = flags | NF_UNION; break;
                case '*': flags = flags | NF_ANYVAR; break;
                case 'c': flags = flags | NF_CONST; break;
                case 's': flags = flags | NF_CONVERTANYTOSTRING; break;
                case 'w': flags = flags | NF_PUSHVALUEWIDTH; break;
                case 'k': lt = LT_KEEP; break;
                case 'b': lt = LT_BORROW; break;
                case ']': {
                    auto wrapped = WrapKnown(vttype, V_VECTOR);
                    if (wrapped.Null()) nf->Error("unknown vector type");
                    vttype = wrapped;
                    break;
                }
                case '}':
                    vttype = WrapKnown(vttype, V_STRUCT_NUM);
                    if (vttype.Null()) nf->Error("unknown numeric struct type");
                    break;
                case '?':
                    optional = true;
                    if (IsRef(vttype->t)) {
                        auto wrapped = WrapKnown(vttype, V_NIL);
                        if (wrapped.Null()) nf->Error("unknown nillable type");
                        vttype = wrapped;
                    }
                    break;
                case ':':
                    if (vttype->t == V_RESOURCE) {
                        auto nstart = tid;
                        while (islower(*tid)) tid++;
                        auto rname = string_view(nstart, tid - nstart);
                        auto rt = LookupResourceType(rname);
                        if (!rt) nf->Error("unknown resource type " + rname);
                        vttype = &rt->thistype;
                    } else {
                        if (*tid < '/' || *tid > '9') nf->Error("int out of range");
                        char val = *tid++ - '0';
                        if (vttype->ElementIfNil()->Numeric())
                            default_val = val;
                        else if (vttype->t == V_STRUCT_NUM)
                            vttype = FixedNumStruct(vttype->ns->t, val);
                        else
                            nf->Error(cat("illegal type: ", vttype->t));
                    }
                    break;
                default:
                    nf->Error("illegal type modifier");
            }
        }
        if (vttype->t == V_RESOURCE && !vttype->rt)
            nf->Error("all uses of type R must have :name specifier");
        rttype = VT2RT(vttype->t);
    }
};

// The two kinds of builtin, see the BUILTIN macros below: one taking a fixed number of
// arguments by value, and the V kind that works on the stack it is given.
typedef void  (*builtinfV)(StackPtr &sp, VM &vm);
typedef Value (*builtinf0)(StackPtr &sp, VM &vm);
typedef Value (*builtinf1)(StackPtr &sp, VM &vm, Value);
typedef Value (*builtinf2)(StackPtr &sp, VM &vm, Value, Value);
typedef Value (*builtinf3)(StackPtr &sp, VM &vm, Value, Value, Value);
typedef Value (*builtinf4)(StackPtr &sp, VM &vm, Value, Value, Value, Value);
typedef Value (*builtinf5)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value);
typedef Value (*builtinf6)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value, Value);
typedef Value (*builtinf7)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value, Value, Value);

// The number of arguments a builtin takes according to its argument type string: one per type
// letter, see Narg::Set.
constexpr int BuiltinNumArgs(const char *typeids) {
    int n = 0;
    for (; *typeids; typeids++) if (*typeids >= 'A' && *typeids <= 'Z') n++;
    return n;
}

// The function type of a builtin taking N arguments, or of the V kind for -1. The BUILTIN
// macros declare a builtin as this ahead of its definition, which checks the parameter list
// of the definition against the argument types given, and makes its address available.
template<int N> struct BuiltinSig;
template<> struct BuiltinSig<-1> { typedef std::remove_pointer_t<builtinfV> type; };
template<> struct BuiltinSig<0> { typedef std::remove_pointer_t<builtinf0> type; };
template<> struct BuiltinSig<1> { typedef std::remove_pointer_t<builtinf1> type; };
template<> struct BuiltinSig<2> { typedef std::remove_pointer_t<builtinf2> type; };
template<> struct BuiltinSig<3> { typedef std::remove_pointer_t<builtinf3> type; };
template<> struct BuiltinSig<4> { typedef std::remove_pointer_t<builtinf4> type; };
template<> struct BuiltinSig<5> { typedef std::remove_pointer_t<builtinf5> type; };
template<> struct BuiltinSig<6> { typedef std::remove_pointer_t<builtinf6> type; };
template<> struct BuiltinSig<7> { typedef std::remove_pointer_t<builtinf7> type; };

struct BuiltinDef;

// The builtins of one subsystem in definition order, filled in by the BuiltinDef constructors
// during static initialization. Has no constructor of its own such that it is zero-initialized
// before any of those run, regardless of definition order.
struct BuiltinGroup {
    BuiltinDef *first = nullptr;
    BuiltinDef *last = nullptr;
};

// The metadata of a builtin as defined by the BUILTIN macros, each of which is a global that
// links itself into its group. RegisterBuiltin() turns these into NativeFuns.
struct BuiltinDef {
    const char *symbol;  // C linkage name of the function, e.g. "builtin_gl_frame".
    const char *name;    // Lobster name, without namespace.
    const char *ids;
    const char *typeids;
    const char *rets;
    const char *help;
    bool vararg;         // Of the V kind, see builtinfV.
    // Only for the JIT to link the generated code against, which calls the function by its
    // symbol, see CodeGen::EmitNativeCall and NativeRegistry::jit_imports.
    const void *address;
    BuiltinDef *next = nullptr;

    BuiltinDef(BuiltinGroup &group, const char *symbol, const char *name, const char *ids,
               const char *typeids, const char *rets, const char *help, bool vararg,
               const void *address)
        : symbol(symbol), name(name), ids(ids), typeids(typeids), rets(rets), help(help),
          vararg(vararg), address(address) {
        if (group.last) group.last->next = this;
        else group.first = this;
        group.last = this;
    }
};

// Builtins are defined at file scope with the macros below. A file first declares the group its
// builtins go into and how their C symbols are formed, e.g.:
//
//     BuiltinGroup graphics_builtins;
//     #define BUILTIN_GROUP graphics_builtins
//     #define BUILTIN_SYM(name) builtin_gl_##name
//
// and then each builtin is a function definition whose head the macro generates, from the
// same name, argument names, argument types, return types and help as before, with the
// parameter list and body following it:
//
//     BUILTIN(set_shader, "shader", "S", "", "changes the current shader.")
//     (StackPtr &, VM &vm, Value shader) {
//         ...
//         return NilVal();
//     }
//
// This defines extern "C" Value builtin_gl_set_shader(StackPtr &, VM &, Value), plus a
// BuiltinDef global holding the metadata. The symbol is thus always "builtin_" followed by the
// full Lobster name with a "_" for the ".", which RegisterGroup() below verifies, and it is
// what the generated code calls the builtin by. The function is declared ahead of the
// definition with the type the argument types given imply, see BuiltinSig, so a parameter list
// that does not match them is a compile error.
// BUILTIN_V is for the vararg kind of builtin that pops its arguments and pushes its results
// itself, and returns void. The _OVERLOAD variants take a distinct symbol name and the Lobster
// name separately, for names that are defined more than once with different argument types.
// The symbol must then still start with the plain one, followed by a distinguishing suffix.
#define BUILTIN_CAT_(a, b) a##b
#define BUILTIN_CAT(a, b) BUILTIN_CAT_(a, b)
#define BUILTIN_STR_(a) #a
#define BUILTIN_STR(a) BUILTIN_STR_(a)
#define BUILTIN_DEF_(sym, name, ids, typeids, rets, help, nargs) \
    extern "C" lobster::BuiltinSig<nargs>::type sym; \
    static lobster::BuiltinDef BUILTIN_CAT(sym, _def)( \
        BUILTIN_GROUP, BUILTIN_STR(sym), name, ids, typeids, rets, help, (nargs) < 0, \
        (const void *)sym)
#define BUILTIN(name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(name), #name, ids, typeids, rets, help, \
                 lobster::BuiltinNumArgs(typeids)); \
    extern "C" lobster::Value BUILTIN_SYM(name)
#define BUILTIN_V(name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(name), #name, ids, typeids, rets, help, -1); \
    extern "C" void BUILTIN_SYM(name)
#define BUILTIN_OVERLOAD(sym, name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(sym), name, ids, typeids, rets, help, \
                 lobster::BuiltinNumArgs(typeids)); \
    extern "C" lobster::Value BUILTIN_SYM(sym)
#define BUILTIN_V_OVERLOAD(sym, name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(sym), name, ids, typeids, rets, help, -1); \
    extern "C" void BUILTIN_SYM(sym)

struct NativeFun : Named {
    vector<Narg> args, retvals;

    const char *help;

    // C linkage name of the function, which is how the generated code calls it.
    const char *symbol;

    // Of the V kind, see builtinfV.
    bool vararg;

    // See BuiltinDef::address.
    const void *address;

    int subsystemid = -1;

    NativeFun *overloads = nullptr, *first = this;

    int TypeLen(const char *s) {
        int i = 0;
        while (*s) if(isupper(*s++)) i++;
        return i;
    };

    NativeFun(const char *ns, const char *nsname, const char *ids, const char *typeids,
              const char *rets, const char *help, const char *symbol, bool vararg,
              const void *address)
        : Named(*ns ? cat(ns, ".", nsname) : nsname, 0),
          args(TypeLen(typeids)),
          retvals(TypeLen(rets)),
          help(help),
          symbol(symbol),
          vararg(vararg),
          address(address) {
        auto StructArgsVararg = [&](const Narg &arg) {
            if (arg.vttype->t == V_STRUCT_NUM && !vararg)
                Error("struct types can only be used by vararg builtins");
            (void)arg;
        };
        for (auto [i, arg] : enumerate(args)) {
            const char *idend = strchr(ids, ',');
            if (!idend) {
                // if this fails, you're not specifying enough arg names in the comma separated list
                if (i != args.size() - 1) Error("incorrect argument name specification");
                idend = ids + strlen(ids);
            }
            arg.name = string_view(ids, idend - ids);
            ids = idend + 1;
            arg.Set(typeids, LT_BORROW, this);
            StructArgsVararg(arg);
        }
        for (auto &ret : retvals) {
            ret.Set(rets, LT_KEEP, this);
            StructArgsVararg(ret);
        }
    }

    bool IsGLFrame() {
        return name == "gl.frame";
    }
};

struct NativeRegistry {
    vector<NativeFun *> nfuns;
    unordered_map<string_view, NativeFun *> nfunlookup;  // Key points to value!
    #if VM_JIT_MODE
        // What the generated code links against: the runtime helpers of vm_ops_jit_table, and
        // all builtins by their symbol. Pairs of name and address, ending in a pair of nulls,
        // see RunTCC. Filled in by DoneRegistering().
        vector<const void *> jit_imports;
    #endif
    vector<string> subsystems;
    vector<string_view> namespaces;
    const char *cur_ns = nullptr;
    #if LOBSTER_FRAME_PROFILER_BUILTINS
        vector<___tracy_source_location_data> pre_allocated_function_locations;
    #endif

    NativeRegistry() {
        nfuns.reserve(1024);
    }

    ~NativeRegistry() {
        for (auto f : nfuns) delete f;
    }

    void NativeSubSystemStart(const char *ns, const char *name) {
        cur_ns = ns;
        if (*ns) namespaces.push_back(ns);
        subsystems.push_back(name);
    }

    void DoneRegistering() {
        #if VM_JIT_MODE
            for (auto p = vm_ops_jit_table; *p; p += 2) {
                jit_imports.push_back(p[0]);
                jit_imports.push_back(p[1]);
            }
            for (auto nf : nfuns) {
                jit_imports.push_back(nf->symbol);
                jit_imports.push_back(nf->address);
            }
            jit_imports.push_back(nullptr);
            jit_imports.push_back(nullptr);
        #endif
        #if LOBSTER_FRAME_PROFILER_BUILTINS
            for (size_t i = 0; i < nfuns.size(); i++) {
                auto f = nfuns[i];
                pre_allocated_function_locations.push_back(
                    ___tracy_source_location_data { f->name.c_str(), f->name.c_str(), "", 0, 0x880088 });
            }
        #endif
    }

    void Reg(NativeFun *nf) {
        nf->idx = (int)nfuns.size();
        nf->subsystemid = (int)subsystems.size() - 1;
        auto existing = FindNative(nf->name);
        if (existing) {
            if (/*nf->args.v.size() != existing->args.v.size() ||
                nf->retvals.v.size() != existing->retvals.v.size() || */
                nf->subsystemid != existing->subsystemid ) {
                // Must have similar signatures.
                assert(0);
                THROW_OR_ABORT("native library name clash: " + nf->name);
            }
            nf->overloads = existing->overloads;
            existing->overloads = nf;
            nf->first = existing->first;
        } else {
            nfunlookup[nf->name /* must be in value */] = nf;
        }
        nfuns.push_back(nf);
    }

    // Registers all builtins defined with the BUILTIN macros into a group, in definition order.
    void RegisterGroup(const BuiltinGroup &group) {
        for (auto def = group.first; def; def = def->next) {
            auto nf = new NativeFun(cur_ns, def->name, def->ids, def->typeids, def->rets,
                                    def->help, def->symbol, def->vararg, def->address);
            // Catches a file whose BUILTIN_SYM doesn't match the namespace it is registered
            // under, which would make the symbol of its builtins unpredictable.
            auto expected = cat("builtin_", *cur_ns ? cat(cur_ns, "_") : string(), def->name);
            string_view symbol = def->symbol;
            if (symbol.substr(0, expected.size()) != expected ||
                (symbol.size() > expected.size() && symbol[expected.size()] != '_')) {
                nf->Error(cat("symbol ", symbol, " should start with ", expected));
            }
            Reg(nf);
        }
    }

    NativeFun *FindNative(string_view name) {
        auto it = nfunlookup.find(name);
        return it != nfunlookup.end() ? it->second : nullptr;
    }

    uint64_t HashAll() {
        uint64_t h = 0xABADCAFEDEADBEEF;
        for (auto nf : nfuns) {
            h ^= FNV1A64(nf->name);
            for (auto &a : nf->args) {
                h ^= FNV1A64(a.name);
            }
        }
        return h;
    }
};

struct Query {
    Line qloc{ -1, -1 };
    string kind;
    string file;
    string line;
    string iden;
    vector<string> args;
    vector<pair<string, string>> *filenames = nullptr;
};

}  // namespace lobster

#endif  // LOBSTER_NATREG
