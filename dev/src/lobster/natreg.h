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
                    if (vttype->n == NL_REF) {
                        auto wrapped = WrapKnownNil(vttype);
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
                        if (vttype->Numeric())
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

typedef void  (*builtinfV)(StackPtr &sp, VM &vm);
typedef Value (*builtinf0)(StackPtr &sp, VM &vm);
typedef Value (*builtinf1)(StackPtr &sp, VM &vm, Value);
typedef Value (*builtinf2)(StackPtr &sp, VM &vm, Value, Value);
typedef Value (*builtinf3)(StackPtr &sp, VM &vm, Value, Value, Value);
typedef Value (*builtinf4)(StackPtr &sp, VM &vm, Value, Value, Value, Value);
typedef Value (*builtinf5)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value);
typedef Value (*builtinf6)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value, Value);
typedef Value (*builtinf7)(StackPtr &sp, VM &vm, Value, Value, Value, Value, Value, Value, Value);

struct BuiltinPtr {
    union  {
        builtinfV fV;
        builtinf0 f0;
        builtinf1 f1;
        builtinf2 f2;
        builtinf3 f3;
        builtinf4 f4;
        builtinf5 f5;
        builtinf6 f6;
        builtinf7 f7;
    };
    int fnargs;

    BuiltinPtr()      : f0(nullptr), fnargs(0) {}
    BuiltinPtr(builtinfV f) : fV(f), fnargs(-1) {}
    BuiltinPtr(builtinf0 f) : f0(f), fnargs(0) {}
    BuiltinPtr(builtinf1 f) : f1(f), fnargs(1) {}
    BuiltinPtr(builtinf2 f) : f2(f), fnargs(2) {}
    BuiltinPtr(builtinf3 f) : f3(f), fnargs(3) {}
    BuiltinPtr(builtinf4 f) : f4(f), fnargs(4) {}
    BuiltinPtr(builtinf5 f) : f5(f), fnargs(5) {}
    BuiltinPtr(builtinf6 f) : f6(f), fnargs(6) {}
    BuiltinPtr(builtinf7 f) : f7(f), fnargs(7) {}
};

struct NativeFun : Named {
    BuiltinPtr fun;

    vector<Narg> args, retvals;

    const char *help;

    int subsystemid = -1;

    NativeFun *overloads = nullptr, *first = this;

    int TypeLen(const char *s) {
        int i = 0;
        while (*s) if(isupper(*s++)) i++;
        return i;
    };

    NativeFun(const char *ns, const char *nsname, BuiltinPtr f, const char *ids,
              const char *typeids,
              const char *rets, const char *help)
        : Named(*ns ? cat(ns, ".", nsname) : nsname, 0),
          fun(f),
          args(TypeLen(typeids)),
          retvals(TypeLen(rets)),
          help(help) {
        if ((int)args.size() != f.fnargs && f.fnargs >= 0) Error("mismatching argument count");
        auto StructArgsVararg = [&](const Narg &arg) {
            if (arg.vttype->t == V_STRUCT_NUM && f.fnargs >= 0)
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
    vector<string> subsystems;
    vector<string_view> namespaces;
    const char *cur_ns = nullptr;
    #if LOBSTER_FRAME_PROFILER_BUILTINS
        vector<tracy::SourceLocationData> pre_allocated_function_locations;
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
        #if LOBSTER_FRAME_PROFILER_BUILTINS
            for (size_t i = 0; i < nfuns.size(); i++) {
                auto f = nfuns[i];
                pre_allocated_function_locations.push_back(
                    tracy::SourceLocationData { f->name.c_str(), f->name.c_str(), "", 0, 0x880088 });
            }
        #endif
    }

    #define REGISTER(N) \
    void operator()(const char *nsname, const char *ids, const char *typeids, \
                    const char *rets, const char *help, builtinf##N f) { \
        Reg(new NativeFun(cur_ns, nsname, BuiltinPtr(f), ids, typeids, rets, help)); \
    }
    REGISTER(V)
    REGISTER(0)
    REGISTER(1)
    REGISTER(2)
    REGISTER(3)
    REGISTER(4)
    REGISTER(5)
    REGISTER(6)
    REGISTER(7)
    #undef REGISTER

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
