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

// Whether the profilers wrap each call to a builtin in the generated code, see vmops.h.
#if LOBSTER_FRAME_PROFILER_BUILTINS || LOBSTER_FRAME_PROFILER_GLOBAL
    #define LOBSTER_NATIVE_PROFILE 1
#else
    #define LOBSTER_NATIVE_PROFILE 0
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

// The number of arguments a builtin takes according to its argument type string: one per type
// letter, see Narg::Set.
constexpr int BuiltinNumArgs(const char *typeids) {
    int n = 0;
    for (; *typeids; typeids++) if (*typeids >= 'A' && *typeids <= 'Z') n++;
    return n;
}

// What C++ type an argument of a builtin becomes, from its type letter and what wraps it.
// A '}' argument is a numeric struct of the width the ':' after it gives, which becomes a
// vector of that many ints or floats. A 'w' one may be passed a struct of any width, so it
// stays a pointer to the stack slots its values are in. A ']' makes any of them a vector,
// which is a reference. An 'A' is any value at all, which is a reference wherever the
// typechecker says it has to be one, see BuiltinRetKindOf. An 'L' is a function value.
enum BuiltinArgKind {
    BAK_VALUE,     // A
    BAK_REF,       // A the typechecker requires a reference of
    BAK_FUNCTION,  // L
    BAK_INT,       // I, B, T
    BAK_FLOAT,     // F
    BAK_STRING,    // S
    BAK_VECTOR,    // ]
    BAK_RESOURCE,  // R
    BAK_IVEC,      // I}
    BAK_FVEC,      // F}
    BAK_VALUEVEC   // w
};

constexpr BuiltinArgKind BuiltinArgKindOf(const char *typeids, int arg) {
    auto n = -1;
    char base = 0;
    auto wrap = 0;  // 1 = vector, 2 = numeric struct, 3 = any width of struct.
    auto resource = false;
    for (;; typeids++) {
        auto c = *typeids;
        if (!c || (c >= 'A' && c <= 'Z')) {
            if (n == arg) {
                if (wrap == 3) return BAK_VALUEVEC;
                if (wrap == 2) return base == 'F' ? BAK_FVEC : BAK_IVEC;
                if (wrap == 1) return BAK_VECTOR;
                switch (base) {
                    case 'I': case 'B': case 'T': return BAK_INT;
                    case 'F': return BAK_FLOAT;
                    case 'S': return BAK_STRING;
                    case 'R': return BAK_RESOURCE;
                    case 'L': return BAK_FUNCTION;
                    default: return BAK_VALUE;
                }
            }
            if (!c) return BAK_VALUE;
            n++;
            base = c;
            wrap = 0;
            resource = c == 'R';
        } else if (resource && c == ':') {
            // The name of the resource type follows, which is all lowercase, see Narg::Set.
            while (typeids[1] >= 'a' && typeids[1] <= 'z') typeids++;
        } else if (c == '}') {
            wrap = 2;
        } else if (c == 'w') {
            wrap = 3;
        } else if (c == ']') {
            wrap = 1;
        }
    }
}

// The width of a numeric struct argument, from the ':' that follows its '}'.
constexpr int BuiltinArgWidthOf(const char *typeids, int arg) {
    auto n = -1;
    auto w = 0;
    auto isstruct = false;
    auto resource = false;
    for (;; typeids++) {
        auto c = *typeids;
        if (!c || (c >= 'A' && c <= 'Z')) {
            if (n == arg) return w;
            if (!c) return 0;
            n++;
            w = 0;
            isstruct = false;
            resource = c == 'R';
        } else if (resource && c == ':') {
            while (typeids[1] >= 'a' && typeids[1] <= 'z') typeids++;
        } else if (c == '}') {
            isstruct = true;
        } else if (c == ':' && isstruct) {
            w = typeids[1] - '0';
        }
    }
}

constexpr bool BuiltinArgIsVec(const char *typeids, int arg) {
    auto k = BuiltinArgKindOf(typeids, arg);
    return k == BAK_IVEC || k == BAK_FVEC || k == BAK_VALUEVEC;
}

// The modifiers that follow the type letter of an argument, which is what says how much of a
// type an 'A' really is, see Narg::Set. Only ever asked of one, which can carry none of the
// modifiers that take an argument of their own.
constexpr string_view BuiltinArgMods(const char *typeids, int arg) {
    auto n = -1;
    const char *start = typeids;
    for (;; typeids++) {
        auto c = *typeids;
        if (!c || (c >= 'A' && c <= 'Z')) {
            if (n == arg) return string_view(start, (size_t)(typeids - start));
            if (!c) return string_view();
            n++;
            start = typeids + 1;
        }
    }
}

// What an 'A' really is, which is what lets it be something better than a Value. The
// typechecker requires a reference of an argument that carries no modifiers at all, which is
// the only place in the language where any reference type goes, and of a nilable return value,
// whose type it requires to be nillable. A return value that is the same type as an argument is
// whatever that argument is, as long as it is not one of the ones the typechecker works out a
// type of its own for: the element type of a vector, the type a typeid names, or what a
// function returns.
constexpr bool BuiltinModsPlain(string_view mods) {
    for (auto c : mods) {
        if (c == '1' || c == '2' || c == '3' || c == 'u' || c == '*' || c == 'c' || c == 's' ||
            c == 'w') return false;
    }
    return true;
}

constexpr int BuiltinModsSubArg(string_view mods) {
    for (auto c : mods) {
        if (c >= '1' && c <= '3') return c - '1';
    }
    return -1;
}

constexpr bool BuiltinModsOptional(string_view mods) {
    for (auto c : mods) if (c == '?') return true;
    return false;
}

constexpr bool BuiltinArgIsPlainAny(const char *typeids, int arg) {
    return BuiltinArgKindOf(typeids, arg) == BAK_VALUE &&
           BuiltinModsPlain(BuiltinArgMods(typeids, arg));
}

// The kind an argument reaches the builtin as.
constexpr BuiltinArgKind BuiltinParamKindOf(const char *typeids, int arg) {
    return BuiltinArgIsPlainAny(typeids, arg) ? BAK_REF : BuiltinArgKindOf(typeids, arg);
}

// The same for a return value, which also knows the arguments it may say it is the type of.
constexpr BuiltinArgKind BuiltinRetKindOf(const char *typeids, const char *rets, int ret) {
    auto k = BuiltinArgKindOf(rets, ret);
    if (k != BAK_VALUE) return k;
    auto mods = BuiltinArgMods(rets, ret);
    if (BuiltinModsPlain(mods)) return BAK_REF;
    auto sa = BuiltinModsSubArg(mods);
    if (sa < 0) return BAK_VALUE;
    if (BuiltinModsOptional(mods)) return BAK_REF;
    auto ak = BuiltinArgKindOf(typeids, sa);
    if (ak == BAK_IVEC || ak == BAK_FVEC) return ak;
    return BuiltinArgIsPlainAny(typeids, sa) ? BAK_REF : BAK_VALUE;
}

// How wide it is, which for one that is the same type as a numeric struct argument is that
// argument's width.
constexpr int BuiltinRetWidthOf(const char *typeids, const char *rets, int ret) {
    auto w = BuiltinArgWidthOf(rets, ret);
    if (w || BuiltinArgKindOf(rets, ret) != BAK_VALUE) return w;
    auto mods = BuiltinArgMods(rets, ret);
    if (BuiltinModsOptional(mods)) return 0;
    auto sa = BuiltinModsSubArg(mods);
    return sa < 0 ? 0 : BuiltinArgWidthOf(typeids, sa);
}

template<BuiltinArgKind K, int W> struct BuiltinParamType;
template<int W> struct BuiltinParamType<BAK_VALUE, W>        { typedef Value type; };
template<int W> struct BuiltinParamType<BAK_REF, W>          { typedef RefObj *type; };
template<int W> struct BuiltinParamType<BAK_FUNCTION, W>     { typedef fun_base_t type; };
template<int W> struct BuiltinParamType<BAK_INT, W>          { typedef iint type; };
template<int W> struct BuiltinParamType<BAK_FLOAT, W>        { typedef double type; };
template<int W> struct BuiltinParamType<BAK_STRING, W>       { typedef LString *type; };
template<int W> struct BuiltinParamType<BAK_VECTOR, W>       { typedef LVector *type; };
template<int W> struct BuiltinParamType<BAK_RESOURCE, W>     { typedef LResource *type; };
template<int W> struct BuiltinParamType<BAK_IVEC, W>         { typedef vec<iint, W> type; };
template<int W> struct BuiltinParamType<BAK_FVEC, W>         { typedef vec<double, W> type; };

template<typename TIDS, size_t P> struct BuiltinParam {
    typedef typename BuiltinParamType<BuiltinParamKindOf(TIDS::tids, (int)P),
                                      BuiltinArgWidthOf(TIDS::tids, (int)P)>::type type;
};

// What a builtin returns. The return type string has the same shape as the argument one, so
// the kind of each return value comes out of the same parser. A builtin returns its last
// return value as the type that kind implies, so one that has none returns void, and so does
// one of the V kind, which writes all of them thru a pointer instead.
template<typename TIDS, bool PushRets> struct BuiltinRet {
    static constexpr int n = BuiltinNumArgs(TIDS::rids);
    typedef std::conditional_t<
        PushRets || n == 0, void,
        typename BuiltinParamType<BuiltinRetKindOf(TIDS::tids, TIDS::rids, n - 1),
                                  BuiltinRetWidthOf(TIDS::tids, TIDS::rids, n - 1)>::type> type;
};

// How many return values a builtin does not return but writes thru a pointer the caller passes
// ahead of its arguments: all of them for the V kind, all but the last one for the rest.
constexpr int BuiltinNumOuts(const char *rets, bool pushrets) {
    auto n = BuiltinNumArgs(rets);
    return !n ? 0 : pushrets ? n : n - 1;
}

// The type one of those pointers is to.
template<typename TIDS, size_t R> struct BuiltinOut {
    typedef typename BuiltinParamType<BuiltinRetKindOf(TIDS::tids, TIDS::rids, (int)R),
                                      BuiltinRetWidthOf(TIDS::tids, TIDS::rids, (int)R)>::type
                                          *type;
};

template<typename TIDS, bool PushRets, typename Outs, typename Params> struct BuiltinSigT;
template<typename TIDS, bool PushRets, size_t... O, size_t... P>
struct BuiltinSigT<TIDS, PushRets, std::index_sequence<O...>, std::index_sequence<P...>> {
    typedef typename BuiltinRet<TIDS, PushRets>::type ret;
    typedef ret type(VM &, typename BuiltinOut<TIDS, O>::type...,
                     typename BuiltinParam<TIDS, P>::type...);
};

// The function type the argument types of a builtin imply, where TIDS is a type that carries
// them as a `tids` member, see BUILTIN_DEF_. The BUILTIN macros declare a builtin as this ahead
// of its definition, which checks the parameter list of the definition against the argument
// types given, and makes its address available.
template<typename TIDS, bool PushRets> using BuiltinSig =
    BuiltinSigT<TIDS, PushRets,
                std::make_index_sequence<(size_t)BuiltinNumOuts(TIDS::rids, PushRets)>,
                std::make_index_sequence<BuiltinNumArgs(TIDS::tids)>>;

// The builtins the generated code writes out itself rather than calling, because they are
// leaned on often enough that the call, and moving their arguments thru the stack to make it,
// is worth avoiding. CodeGen::EmitCodegenBuiltin has a case for each of these. One defined
// with BUILTIN_CODEGEN has no function at all, only the metadata the language needs. The names
// start with BCG since a BS_ prefix collides with the Windows button styles.
enum BuiltinCodegen {
    BCG_NONE = 0,
    BCG_GL_FRAME,
    BCG_PUSH,
    BCG_INSERT,
    BCG_POP,
    BCG_TOP,
    BCG_REMOVE,
};

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
    bool pushrets;       // Of the V kind, see the BUILTIN macros below.
    BuiltinCodegen codegen;  // Written out by codegen rather than called, if at all.
    // Only for the JIT to link the generated code against, which calls the function by its
    // symbol, see CodeGen::EmitNativeCall and NativeRegistry::jit_imports. Null for one that
    // has no function of its own, see BUILTIN_CODEGEN.
    const void *address;
    BuiltinDef *next = nullptr;

    BuiltinDef(BuiltinGroup &group, const char *symbol, const char *name, const char *ids,
               const char *typeids, const char *rets, const char *help, bool pushrets,
               BuiltinCodegen codegen, const void *address)
        : symbol(symbol), name(name), ids(ids), typeids(typeids), rets(rets), help(help),
          pushrets(pushrets), codegen(codegen), address(address) {
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
//     (VM &vm, LString *shader) {
//         ...
//     }
//
// This defines extern "C" void builtin_gl_set_shader(VM &, LString *), plus a
// BuiltinDef global holding the metadata. The symbol is thus always "builtin_" followed by the
// full Lobster name with a "_" for the ".", which RegisterGroup() below verifies, and it is
// what the generated code calls the builtin by. The function is declared ahead of the
// definition with the type the argument and return types given imply, see BuiltinSig, so a
// parameter list or return type that does not match them is a compile error. An argument that
// lives in a run of stack slots (a numeric struct, or one that may be a struct) becomes a
// vector of its width, or a pointer to those slots when its width is not fixed.
// A builtin returns its last return value as the type its letter implies, or void when it has
// none, and pushes the ones before it onto a stack it is given a pointer to, which it only
// takes when it has any to push.
// BUILTIN_V is for the kind of builtin that leaves all of its return values on that stack
// rather than returning the last one, which is what a struct takes, and returns void.
// The stack it gets starts where its arguments are, so those slots are what it writes its
// return values into, and a builtin that takes a pointer to some of them must be done reading
// it before it pushes anything. The _OVERLOAD variants take a distinct symbol name and the
// Lobster name separately, for names that are defined more than once with different argument
// types. The symbol must then still start with the plain one, followed by a distinguishing
// suffix.
// BUILTIN_CODEGEN declares one the generated code writes out itself, see BuiltinCodegen. It
// has only the metadata the language needs, no function and thus no body, so it ends in a ';'
// like any other declaration.
#define BUILTIN_CAT_(a, b) a##b
#define BUILTIN_CAT(a, b) BUILTIN_CAT_(a, b)
#define BUILTIN_STR_(a) #a
#define BUILTIN_STR(a) BUILTIN_STR_(a)
#define BUILTIN_META_(sym, name, ids, typeids, rets, help, pushrets, codegen, address) \
    static lobster::BuiltinDef BUILTIN_CAT(sym, _def)( \
        BUILTIN_GROUP, BUILTIN_STR(sym), name, ids, typeids, rets, help, pushrets, \
        codegen, address)
#define BUILTIN_DEF_(sym, name, ids, typeids, rets, help, pushrets) \
    struct BUILTIN_CAT(sym, _tids) { static constexpr const char *tids = typeids; \
                                     static constexpr const char *rids = rets; }; \
    extern "C" lobster::BuiltinSig<BUILTIN_CAT(sym, _tids), pushrets>::type sym; \
    BUILTIN_META_(sym, name, ids, typeids, rets, help, pushrets, lobster::BCG_NONE, \
                  (const void *)sym)
#define BUILTIN_RET_(sym) lobster::BuiltinSig<BUILTIN_CAT(sym, _tids), false>::ret
#define BUILTIN(name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(name), #name, ids, typeids, rets, help, false); \
    extern "C" BUILTIN_RET_(BUILTIN_SYM(name)) BUILTIN_SYM(name)
#define BUILTIN_V(name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(name), #name, ids, typeids, rets, help, true); \
    extern "C" void BUILTIN_SYM(name)
#define BUILTIN_OVERLOAD(sym, name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(sym), name, ids, typeids, rets, help, false); \
    extern "C" BUILTIN_RET_(BUILTIN_SYM(sym)) BUILTIN_SYM(sym)
#define BUILTIN_V_OVERLOAD(sym, name, ids, typeids, rets, help) \
    BUILTIN_DEF_(BUILTIN_SYM(sym), name, ids, typeids, rets, help, true); \
    extern "C" void BUILTIN_SYM(sym)
#define BUILTIN_CODEGEN(codegen, name, ids, typeids, rets, help) \
    BUILTIN_META_(BUILTIN_SYM(name), #name, ids, typeids, rets, help, false, \
                  lobster::codegen, nullptr)

struct NativeFun : Named {
    vector<Narg> args, retvals;

    const char *help;

    // C linkage name of the function, which is how the generated code calls it.
    const char *symbol;

    // Of the V kind, which leaves its return values on the stack, see the BUILTIN macros.
    bool pushrets;

    // See BuiltinDef::codegen and BuiltinDef::address.
    BuiltinCodegen codegen;
    const void *address;

    int subsystemid = -1;

    NativeFun *overloads = nullptr, *first = this;

    int TypeLen(const char *s) {
        int i = 0;
        while (*s) if(isupper(*s++)) i++;
        return i;
    };

    NativeFun(const char *ns, const char *nsname, const char *ids, const char *typeids,
              const char *rets, const char *help, const char *symbol, bool pushrets,
              BuiltinCodegen codegen, const void *address)
        : Named(*ns ? cat(ns, ".", nsname) : nsname, 0),
          args(TypeLen(typeids)),
          retvals(TypeLen(rets)),
          help(help),
          symbol(symbol),
          pushrets(pushrets),
          codegen(codegen),
          address(address) {
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
        }
        for (auto &ret : retvals) {
            ret.Set(rets, LT_KEEP, this);
        }
        // An argument that may be a struct of any width has no one C++ type, so only a builtin
        // the generated code writes out itself can take one, see BuiltinCodegen.
        for (auto &arg : args) {
            if ((arg.flags & NF_PUSHVALUEWIDTH) && codegen == BCG_NONE)
                Error("an argument of any width needs a codegen builtin");
        }
        // A struct takes more than one slot, so one that is not the last return value has to
        // be pushed, which only the V kind has a way to do. The last one is returned as the
        // vector of its width, see BuiltinRet.
        for (size_t i = 0; i + 1 < retvals.size(); i++) {
            if (retvals[i].vttype->t == V_STRUCT_NUM && !pushrets)
                Error("a struct can only be returned before other values by a V builtin");
        }
    }

    // The kind the type of an argument or a return value belongs to, which must agree with
    // what BuiltinArgKindOf makes of the type string it came from.
    static BuiltinArgKind KindOf(const Narg &n) {
        if (n.flags & NF_PUSHVALUEWIDTH) return BAK_VALUEVEC;
        if (n.vttype->t == V_STRUCT_NUM)
            return n.vttype->ns->t == V_FLOAT ? BAK_FVEC : BAK_IVEC;
        switch (n.vttype->ElementIfNil()->t) {
            case V_VECTOR: return BAK_VECTOR;
            case V_INT:
            case V_TYPEID: return BAK_INT;
            case V_FLOAT: return BAK_FLOAT;
            case V_STRING: return BAK_STRING;
            case V_RESOURCE: return BAK_RESOURCE;
            case V_FUNCTION: return BAK_FUNCTION;
            default: return BAK_VALUE;
        }
    }

    // An 'A' that carries no modifiers, which is the one the typechecker requires a reference
    // of, see BuiltinRetKindOf.
    bool IsPlainAny(const Narg &n) const { return KindOf(n) == BAK_VALUE && !n.flags; }

    // The C++ type argument `i` reaches the builtin as, mirroring BuiltinParamKindOf.
    BuiltinArgKind ArgKind(size_t i) const {
        return IsPlainAny(args[i]) ? BAK_REF : KindOf(args[i]);
    }

    // The argument a return value says it is the same type as, if any, mirroring
    // BuiltinModsSubArg.
    const Narg *SameTypeArg(const Narg &r) const {
        auto sub = r.flags & (NF_SUBARG1 | NF_SUBARG2 | NF_SUBARG3);
        if (r.optional || !sub || sub != r.flags) return nullptr;
        auto sa = (size_t)(sub == NF_SUBARG1 ? 0 : sub == NF_SUBARG2 ? 1 : 2);
        return sa < args.size() ? &args[sa] : nullptr;
    }

    // The kind return value `i` has, mirroring BuiltinRetKindOf.
    BuiltinArgKind RetValKind(size_t i) const {
        auto &r = retvals[i];
        auto k = KindOf(r);
        if (k != BAK_VALUE) return k;
        if (!r.flags) return BAK_REF;
        if (r.optional) return BAK_REF;
        auto a = SameTypeArg(r);
        if (!a) return BAK_VALUE;
        if (a->vttype->t == V_STRUCT_NUM) return KindOf(*a);
        return IsPlainAny(*a) ? BAK_REF : BAK_VALUE;
    }

    // How many values a numeric struct return value has, which its type says, or the argument
    // it says it is the same type as does, and how many slots that takes.
    int RetValWidth(size_t i) const {
        auto &r = retvals[i];
        if (r.vttype->t == V_STRUCT_NUM) return r.vttype->ns->flen;
        if (KindOf(r) != BAK_VALUE) return 0;
        auto a = SameTypeArg(r);
        return a && a->vttype->t == V_STRUCT_NUM ? a->vttype->ns->flen : 0;
    }
    int RetValSlots(size_t i) const { return std::max(1, RetValWidth(i)); }

    // The one the builtin returns its last return value as, see BuiltinRet. Of the V kind, or
    // with nothing to return, it returns void, which has no kind of its own.
    bool ReturnsValue() const { return !pushrets && !retvals.empty(); }
    BuiltinArgKind RetKind() const { return RetValKind(retvals.size() - 1); }
    int RetWidth() const { return retvals.empty() ? 0 : RetValWidth(retvals.size() - 1); }
    int RetSlots() const { return ReturnsValue() ? RetValSlots(retvals.size() - 1) : 0; }

    // The return values it writes thru a pointer instead, mirroring BuiltinNumOuts.
    int OutValues() const {
        return retvals.empty() ? 0 : (int)retvals.size() - (pushrets ? 0 : 1);
    }

    // How many values a numeric struct argument has, which its type says.
    int ArgWidth(size_t i) const { return args[i].vttype->ns->flen; }
    bool ArgIsVec(size_t i) const {
        auto k = ArgKind(i);
        return k == BAK_IVEC || k == BAK_FVEC || k == BAK_VALUEVEC;
    }

    bool IsGLFrame() const { return codegen == BCG_GL_FRAME; }
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
                // The generated code writes out what a codegen one does, so there is nothing
                // to link it against, see BUILTIN_CODEGEN.
                if (!nf->address) continue;
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
                                    def->help, def->symbol, def->pushrets, def->codegen,
                                    def->address);
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
