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

// C/C++ prologue, constant string objects, and metadata emitted after the functions.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenPrologue : virtual CodeGenBase {
    vector<type_elem_t> ser_ids;
    vector<int> vtables;  // -1 = uninit, -2 and lower is case idx, positive is code offset.
    // The string constants the code carries as objects, see EmitConstantStrings, and the index
    // of the one for each distinct string, so a literal that occurs more than once has one.
    vector<string_view> constant_strings;
    map<string_view, int> constant_string_index;

    CodeGenPrologue() {}

    // The string constants, as objects laid out the way the C++ LString is (see the mirror in
    // Prologue), so that a literal is the address of one, see EmitPushStr. Each is emitted with
    // the reference the generated code holds on it, which it never gives up, so they live as
    // long as the program; see VM::StringConstantDropped for what happens if the program drops
    // it anyway.
    void EmitConstantStrings(string &sd) {
        for (auto [i, s] : enumerate(constant_strings)) {
            append(sd, "static struct { int typeinfo; int refc; long long len; unsigned char data[",
                   s.size() + 1, "]; } str", i, " = { ", (int)TYPE_ELEM_STRING_CONST, ", 1, ",
                   s.size(), ", ");
            // A C++ compiler may cap the length of a literal (MSVC at 16K), so a long one is
            // written out as its bytes.
            if (s.size() <= 2048) {
                EscapeAndQuote(s, sd, true);
            } else {
                sd += "{ ";
                for (auto c : s) append(sd, (int)(uint8_t)c, ", ");
                sd += "0 }";
            }
            sd += " };\n";
        }
    }

    // The index of the object for a string constant, see EmitConstantStrings.
    int ConstantString(string_view s) override {
        auto it = constant_string_index.find(s);
        if (it != constant_string_index.end()) return it->second;
        auto idx = (int)constant_strings.size();
        constant_strings.push_back(s);
        constant_string_index[s] = idx;
        return idx;
    }

    // A string constant is borrowed from the object the code carries for it, so pushing one is
    // taking its address.
    void EmitPushStr(int idx) {
        string q;
        EscapeAndQuote(constant_strings[idx].substr(0, 50), q, true);
        TrackUseDef(0, 1);
        CopyValue(cb, SlotVar(regso, RTT_STRING), Direct(cat("(LString *)&str", idx), RTT_STRING),
                  "");
        comment(q);
    }

    void Generate(const StringConstant &node, size_t retval) {
        if (!retval) return;
        EmitPushStr(ConstantString(node.str));
    }

    void Prologue(string &sd) {
        if (cpp) {
            sd +=
                "#include \"lobster/stdafx.h\"\n"
                "#include \"lobster/vmdata.h\"\n"
                "#include \"lobster/vmops.h\"\n"
                "#include \"lobster/compiler.h\"\n"
                "\n"
                "typedef lobster::Value Value;\n"
                "typedef lobster::StackPtr StackPtr;\n"
                "typedef lobster::VM &VMRef;\n"
                "typedef lobster::fun_base_t fun_base_t;\n"
                "typedef lobster::object_dec_t object_dec_t;\n"
                "typedef lobster::type_elem_t type_elem_t;\n"
                "typedef lobster::RefObj RefObj;\n"
                "typedef lobster::LObject LObject;\n"
                "typedef lobster::LVector LVector;\n"
                "typedef lobster::LString LString;\n"
                "typedef lobster::LResource LResource;\n"
                "typedef lobster::VMString VMString;\n"
                "typedef lobster::VMField VMField;\n"
                "typedef lobster::VMUDT VMUDT;\n"
                "typedef lobster::VMSpecIdent VMSpecIdent;\n"
                "typedef lobster::VMEnumVal VMEnumVal;\n"
                "typedef lobster::VMEnum VMEnum;\n"
                "typedef lobster::VMMetaData VMMetaData;\n"
                "\n"
                // A program is free to assign a variable it never reads, or to compare a
                // variable with itself.
                // Older clang versions don't know all of the warnings below.
                "#if defined(__clang__)\n"
                "    #pragma clang diagnostic ignored \"-Wunknown-warning-option\"\n"
                "#endif\n"
                "#if defined(__clang__) || defined(__GNUC__)\n"
                "    #pragma GCC diagnostic ignored \"-Wunused-but-set-variable\"\n"
                "    #pragma GCC diagnostic ignored \"-Wunused-variable\"\n"
                "    #pragma GCC diagnostic ignored \"-Wtautological-compare\"\n"
                "#endif\n"
                "\n"
                "#if LOBSTER_ENGINE\n"
                "    // FIXME: This makes SDL not modular, but without it it will miss the SDLMain indirection.\n"
                "    #include \"lobster/sdlincludes.h\"\n"
                "    #include \"lobster/sdlinterface.h\"\n"
                "    extern \"C\" iint GLFrame(VMRef vm);\n"
                "#endif\n"
                "\n"
                ;
        } else {
            sd +=
                // This needs to correspond to the C++ RefObj, enforced in Entry().
                "typedef struct {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "} RefObj;\n"
                // This needs to correspond to the C++ Value, enforced in Entry().
                "typedef struct LString LString;\n"
                "typedef struct LVector LVector;\n"
                "typedef struct LObject LObject;\n"
                "typedef struct {\n"
                "    union {\n"
                "        long long ival;\n"
                "        double fval;\n"
                "        RefObj *ref;\n"
                "        LString *sval;\n"
                "        LVector *vval;\n"
                "        LObject *oval;\n"
                "    };\n"
                "} Value;\n"
                // These need to correspond to the C++ LVector, LString and LObject, enforced in
                // Entry(). We mirror them so that reading a length, an element or a field is a
                // load rather than a call, and they carry the header inline rather than as a
                // RefObj so a reference count is at the same name whatever the type. Only the
                // fields up to the last one we read have to be right, but keeping the whole type
                // here means the size check covers the tail as well.
                "struct LVector {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "    long long len;\n"
                "    long long maxl;\n"
                "    long long width;\n"
                "    Value *elems;\n"
                "};\n"
                "struct LString {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "    long long len;\n"
                "};\n"
                "struct LObject {\n"
                "    int typeinfo;\n"
                "    int refc;\n"
                "};\n"
                // The characters of a string follow directly behind its header. Not a trailing
                // array member, since the C compilers we feed this to do not agree on what
                // those do to the size of the struct. The fields of an object follow behind
                // its own header the same way, which is what the struct emitted per object
                // type says, see UDTName.
                "#define STRING_DATA(S) ((unsigned char *)((S) + 1))\n";
            // This needs to correspond to the C++ VMBase, enforced in Entry().
            append(sd,
                "typedef struct {\n"
                "    int last_line;\n"
                "    int last_fileidx;\n"
                "    int ret_unwind_to;\n"
                "    Value *fvars_ptr;\n"
                "    Value ret_buf[", MAX_RETURN_SLOTS, "];\n"
                "} VMBase;\n");
            // The numeric struct a builtin takes an argument of that type as, which mirrors the
            // C++ vec of the same element type and width, so it is passed the same way, along
            // with the helper that makes one, since C has no constructors.
            // The fields are named rather than an array, since a C compiler may pass a struct
            // holding an array differently from one holding the same fields by name, and the
            // C++ side is the latter, see the vec union in geom.h.
            for (auto isint : { true, false }) {
                for (auto w = 1; w <= 4; w++) {
                    auto elem = isint ? "long long" : "double";
                    auto name = cat(isint ? "ivec" : "fvec", w);
                    append(sd, "typedef struct { ", elem);
                    for (auto i = 0; i < w; i++) append(sd, i ? ", " : " ", VecField(i));
                    append(sd, "; } ", name, ";\nstatic ", name, " mk", name, "(");
                    for (auto i = 0; i < w; i++) append(sd, i ? ", " : "", elem, " a", i);
                    append(sd, ") { ", name, " r;");
                    for (auto i = 0; i < w; i++) append(sd, " r.", VecField(i), " = a", i, ";");
                    sd += " return r; }\n";
                }
            }
            sd +=
                "typedef Value *StackPtr;\n"
                "typedef VMBase *VMRef;\n"
                "typedef void (*fun_base_t)(VMRef);\n"
                "typedef void (*object_dec_t)(VMRef, LObject *);\n"
                // An offset into the type table, which is what the helpers take one as.
                "typedef int type_elem_t;\n"
                // Explicit mirrors of the metadata in vmdata.h. No standard library types cross
                // the JIT boundary, and the initializers are the same in C and C++.
                "#define VMSpan(T) struct { const T *elems; "
                + string(sizeof(size_t) == 8 ? "unsigned long long" : "unsigned int") + " len; }\n"
                "typedef const char *VMString;\n"
                "typedef struct { VMString name; long long val; } VMEnumVal;\n"
                "typedef struct { VMString name; VMSpan(VMEnumVal) vals; unsigned char flags; } VMEnum;\n"
                "typedef struct {\n"
                "    VMString name; int idx; int typeidx;\n"
                "    unsigned char used_as_freevar, readonly, global;\n"
                "} VMSpecIdent;\n"
                "typedef struct { VMString name; int offset, bitoff, bits; } VMField;\n"
                "typedef struct {\n"
                "    VMString name; int idx, size, super_idx, typeidx; VMSpan(VMField) fields;\n"
                "} VMUDT;\n"
                "typedef struct {\n"
                "    int code_version;\n"
                "    VMSpan(type_elem_t) type_table;\n"
                "    VMSpan(VMString) file_names, function_names;\n"
                "    VMSpan(VMUDT) udts;\n"
                "    VMSpan(VMSpecIdent) specidents;\n"
                "    VMSpan(VMEnum) enums;\n"
                "    VMSpan(int) ser_ids;\n"
                "    VMString build_info;\n"
                "    unsigned long long src_hash;\n"
                "    VMSpan(int) subfunctions_to_function;\n"
                "} VMMetaData;\n"
                "#undef VMSpan\n"
                "struct ___tracy_source_location_data {\n"
                "    const char *name;\n"
                "    const char *function;\n"
                "    const char *file;\n"
                "    unsigned int line;\n"
                "    unsigned int color;\n"
                "};\n"
                "struct ___tracy_c_zone_context {\n"
                "    unsigned int id;\n"
                "    int active;\n"
                "};\n"
                "\n"
                ;

            // Check the C compiler's layouts before the host reads any exported metadata.
            int layout_index = 0;
            for (auto [name, size] : {
                     pair{ "VMField", sizeof(VMField) },
                     pair{ "VMUDT", sizeof(VMUDT) },
                     pair{ "VMSpecIdent", sizeof(VMSpecIdent) },
                     pair{ "VMEnumVal", sizeof(VMEnumVal) },
                     pair{ "VMEnum", sizeof(VMEnum) },
                     pair{ "VMMetaData", sizeof(VMMetaData) } }) {
                append(sd, "typedef char _metadata_layout", layout_index++, "[sizeof(", name,
                       ") == ", size, " ? 1 : -1];\n");
            }

            // A value of a type only known at runtime, which is all a handful of builtins take
            // and one returns, made from what the slot it comes from holds. A helper rather than
            // a compound literal, which libtcc compiles into a call to memset. One per name
            // KindName hands out, the reference one taking the type every reference has.
            for (auto k : { VK_INT, VK_FLOAT, VK_FUN, VK_REF }) {
                append(sd, "static Value mkval", KindName(k), "(", CType(k), " a) { Value v; v.",
                       Member(k), " = ", k == VK_FUN ? "(long long)a" : "a", "; return v; }\n");
            }
            // A float field stored in 32 bits of a slot (see SField::bits) sits there as the
            // bits of the 32-bit float, so reading it is widening those, and writing it is
            // making them: the C++ PackFloat32 / UnpackFloat32.
            sd += "static double mkvalF32(long long bits) { union { unsigned int u; float f; } x;"
                  " x.u = (unsigned int)bits; return (double)x.f; }\n"
                  "static long long mkvalBits32(double d) { union { unsigned int u; float f; } x;"
                  " x.f = (float)d; return (long long)x.u; }\n";
            // Every runtime helper the generated code can call. These mirror the Rt functions in
            // vmops.h, which is what the JIT links them to, see vm_ops_jit_table. Where the
            // C++ side takes any reference, it is void here, since the generated code holds
            // them as the type they are.
            sd +=
                #if LOBSTER_NATIVE_PROFILE
                "struct ___tracy_c_zone_context RtNativeProfileStart(VMRef, int);\n"
                "void RtNativeProfileEnd(struct ___tracy_c_zone_context);\n"
                #endif
                "LVector *RtNewVec(VMRef, type_elem_t, int);\n"
                "LObject *RtNewObject(VMRef, type_elem_t, int);\n"
                "void RtVectorGrow(VMRef, LVector *);\n"
                "void RtVectorResize(VMRef, LVector *, long long);\n"
                "void RtVectorEmptyErr(VMRef, int);\n"
                "void RtVectorIdxErr(VMRef, int, long long, long long);\n"
                "void RtVectorErase(LVector *, long long);\n"
                "void RtVectorInsert(VMRef, LVector *, int, long long);\n"
                "void RtExitInt(VMRef, long long, type_elem_t);\n"
                "void RtExitFloat(VMRef, double, type_elem_t);\n"
                "void RtExitFun(VMRef, fun_base_t, type_elem_t);\n"
                "void RtExitRef(VMRef, RefObj *, type_elem_t);\n"
                "void RtExitVoid(VMRef);\n"
                "void RtAbort(VMRef);\n"
                "long long RtIDiv(VMRef, long long, long long);\n"
                "long long RtIMod(VMRef, long long, long long);\n"
                "double RtFMod(double, double);\n"
                "double RtFDiv(double, double);\n"
                "double RtSqrt(double);\n"
                "LString *RtSAdd(VMRef, LString *, LString *);\n"
                "long long RtSLt(LString *, LString *);\n"
                "long long RtSGt(LString *, LString *);\n"
                "long long RtSLe(LString *, LString *);\n"
                "long long RtSGe(LString *, LString *);\n"
                "long long RtSEq(LString *, LString *);\n"
                "long long RtSNe(LString *, LString *);\n"
                "long long RtSnEq(LString *, LString *);\n"
                "long long RtSnNe(LString *, LString *);\n"
                "LString *RtStrConcatN(VMRef, LString **, int);\n"
                "LString *RtIntToString(VMRef, long long, type_elem_t);\n"
                "LString *RtFloatToString(VMRef, double, type_elem_t);\n"
                "LString *RtFunToString(VMRef, fun_base_t, type_elem_t);\n"
                "LString *RtRefToString(VMRef, RefObj *, type_elem_t);\n"
                "LString *RtStructToString(VMRef, Value *, type_elem_t);\n"
                "long long RtIsSubType(VMRef, LObject *, int, int, int);\n"
                "fun_base_t RtDynDispatch(VMRef, LObject *, int);\n"
                "fun_base_t RtDynDispatchStruct(VMRef, long long);\n"
                "void RtEnumRangeErr(VMRef);\n"
                "Value *RtLvalIndexClass(VMRef, LObject *, long long, int);\n"
                "void RtLvSAdd(VMRef, Value *, LString *);\n"
                "LString *RtSAppend(VMRef, LString *, LString *);\n"
                "LString *RtSAppendInt(VMRef, LString *, long long, type_elem_t);\n"
                "LString *RtSAppendFloat(VMRef, LString *, double, type_elem_t);\n"
                "LString *RtSAppendFun(VMRef, LString *, fun_base_t, type_elem_t);\n"
                "LString *RtSAppendRef(VMRef, LString *, RefObj *, type_elem_t);\n"
                "void RtLvSAddInt(VMRef, Value *, long long, type_elem_t);\n"
                "void RtLvSAddFloat(VMRef, Value *, double, type_elem_t);\n"
                "void RtLvSAddFun(VMRef, Value *, fun_base_t, type_elem_t);\n"
                "void RtLvSAddRef(VMRef, Value *, RefObj *, type_elem_t);\n"
                "LString *RtSAppendStruct(VMRef, LString *, Value *, type_elem_t);\n"
                "LString *RtSAppendSubstring(VMRef, LString *, LString *, long long, long long);\n"
                "LString *RtSAppendNumber(VMRef, LString *, long long, long long, long long);\n"
                "void RtLvSAddStruct(VMRef, Value *, Value *, type_elem_t);\n"
                "void RtLvSAddSubstring(VMRef, Value *, LString *, long long, long long);\n"
                "void RtLvSAddNumber(VMRef, Value *, long long, long long, long long);\n"
                "int RtStaticSetThisFrame(VMRef, int);\n"
                "int RtMemberSetThisFrame(VMRef, LObject *, int);\n"
                ;

            sd += "extern void Entry(int, int, int, int, int, int);\n"
                  "extern void IDXErr(VMRef, long long, long long, void *);\n"
                  "extern void IDXErrS(VMRef, long long, long long);\n"
                  "extern long long GLFrame(VMRef);\n"
                  "extern void BackupVar(VMRef, int);\n"
                  "extern void DecOwned(VMRef, int);\n"
                  "extern void RcStat(VMRef, int);\n"
                  "extern void DecDelete(VMRef, void *);\n"
                  "extern void DecDeleteVec(VMRef, LVector *);\n"
                  "extern void DecDeleteObj(VMRef, LObject *);\n"
                  "extern void DecDeleteStr(VMRef, LString *);\n"
                  "extern void AssertFailed(VMRef, int, int, LString *);\n"
                  "extern void RestoreBackup(VMRef, int);\n"
                  "extern int GetTypeSwitchID(VMRef, LObject *, int);\n"
                  "extern void PushFunId(VMRef, const int *, StackPtr);\n"
                  "extern void PopFunId(VMRef);\n"
                  #if LOBSTER_FRAME_PROFILER
                  "extern struct ___tracy_c_zone_context StartProfile(struct ___tracy_source_location_data *);\n"
                  "extern void EndProfile(struct ___tracy_c_zone_context);\n"
                  #endif
                  "\n";
        }

        if (runtime_checks >= RUNTIME_STACK_TRACE) {
            append(sd, "extern const int funinfo_table[];\n\n");
        }
    }

    void Epilogue(string &sd, string_view custom_pre_init_name, uint64_t src_hash) {
        if (cpp) sd += "\nstatic";
        // c2mir turns a file scope declaration that has both `extern` and an initializer into a
        // mere import, dropping the definition, so for MIR we rely on the default external
        // linkage of a file scope object instead.
        else if (mir) sd += "\n";
        else sd += "\nextern";
        sd += " const fun_base_t vtables[] = {\n";
        for (auto id : vtables) {
            sd += "    ";
            if (id >= 0) {
                append(sd, "(fun_base_t)", FunName(id));
            } else if (id <= -2) {
                append(sd, "(fun_base_t)", -id - 2);  // Bit of a hack, would be nice to separate.
            } else {
                sd += "0";
            }
            sd += ",\n";
        }
        sd += "    0\n};\n\n";  // Make sure table is never empty.

        // The string constants by index, for the VM, see VM::CheckStringConstants. Terminated by
        // a null, which also keeps it from ever being empty.
        if (cpp) sd += "static";
        else if (!mir) sd += "extern";
        sd += " LString *const_strings[] = {\n";
        for (size_t i = 0; i < constant_strings.size(); i++) {
            append(sd, "    (LString *)&str", i, ",\n");
        }
        sd += "    0\n};\n\n";

        if (runtime_checks >= RUNTIME_STACK_TRACE) {
            append(sd, "const int funinfo_table[] = {\n    ");
            for (auto [i, d] : enumerate(funstarttables)) {
                if (i && (i & 15) == 0) append(sd, "\n    ");
                append(sd, d, ", ");
            }
            append(sd, "    0\n};\n\n");
        }

        // Output the same metadata initializers in C and C++. Empty arrays get a dummy
        // element, since C requires one, but the views below keep their actual lengths.
        auto gen_string = [&](string_view s) {
            EscapeAndQuote(s, sd, true);
        };
        sd += "static const int type_table[] = {";
        for (auto [i, x] : enumerate(type_table)) {
            if ((i & 0xF) == 0) sd += "\n ";
            append(sd, " ", x, ",");
        }
        sd += "\n};\n\n";
        sd += "static const VMString file_names[] = {\n";
        for (auto [s, _] : parser.lex.filenames) {
            sd += "    ";
            gen_string(s);
            sd += ",\n";
        }
        if (parser.lex.filenames.empty()) sd += "    0\n";
        sd += "};\n\n";
        sd += "static const VMString function_names[] = {\n";
        for (auto f : st.functiontable) {
            sd += "    ";
            gen_string(f->name);
            sd += ",\n";
        }
        if (st.functiontable.empty()) sd += "    0\n";
        sd += "};\n\n";
        auto fieldsname = [](UDT *udt) {
            auto n = cat(udt->name, "_fields", udt->idx);
            std::replace(n.begin(), n.end(), '.', '_');
            return n;
        };
        for (auto udt : st.udttable) {
            if (udt->sfields.empty()) continue;
            append(sd, "static const VMField ", fieldsname(udt), "[] = {\n");
            for (auto [i, sfield] : enumerate(udt->sfields)) {
                sd += "    { ";
                gen_string(udt->g.fields[i].id->name);
                append(sd, ", ", sfield.slot, ", ", sfield.bitoff, ", ", sfield.bits, " },\n");
            }
            sd += "};\n\n";
        }
        sd += "static const VMUDT udts[] = {\n";
        for (auto udt : st.udttable) {
            sd += "    { ";
            gen_string(udt->name);
            auto fspan = cat("{ ", udt->sfields.empty() ? "0" : fieldsname(udt),
                             ", ", udt->sfields.size(), " }");
            append(sd, ", ", udt->idx, ", ", udt->numslots, ", ",
                       (udt->ssuperclass ? udt->ssuperclass->idx : -1), ", ", udt_type_offsets[udt->idx], ", ",
                       fspan, " },\n");
        }
        if (st.udttable.empty()) sd += "    { 0, 0, 0, 0, 0, { 0, 0 } }\n";
        sd += "};\n\n";
        sd += "static const VMSpecIdent specidents[] = {\n";
        for (auto &sid : sids) {
            auto id = st.identtable[sid.ididx];
            sd += "    { ";
            gen_string(id->name);
            append(sd, ", ", sid.idx, ", ", sid.typeidx, ", ", sid.used_as_freevar, ", ",
                   id->constant, ", ", id->scopelevel == 1, " },\n");
        }
        if (sids.empty()) sd += "    { 0, 0, 0, 0, 0, 0 }\n";
        sd += "};\n\n";
        auto enumvalsname = [](Enum *e) {
            auto n = cat(e->name, "_vals", e->idx);
            std::replace(n.begin(), n.end(), '.', '_');
            return n;
        };
        for (auto e : st.enumtable) {
            if (e->vals.empty()) continue;
            append(sd, "static const VMEnumVal ", enumvalsname(e), "[] = {\n");
            for (auto [i, ev] : enumerate(e->vals)) {
                sd += "    { ";
                gen_string(ev->name);
                append(sd, ", ", IntLiteral(ev->val), " },\n");
            }
            sd += "};\n\n";
        }
        sd += "static const VMEnum enums[] = {\n";
        for (auto e : st.enumtable) {
            sd += "    { ";
            gen_string(e->name);
            auto fspan = cat("{ ", e->vals.empty() ? "0" : enumvalsname(e),
                             ", ", e->vals.size(), " }");
            append(sd, ", ", fspan, ", ", e->flags, " },\n");
        }
        if (st.enumtable.empty()) sd += "    { 0, { 0, 0 }, 0 }\n";
        sd += "};\n\n";
        sd += "static const int ser_ids[] = {";
        for (auto [i, x] : enumerate(ser_ids)) {
            if ((i & 0xF) == 0) sd += "\n ";
            append(sd, " ", x, ",");
        }
        if (ser_ids.empty()) sd += " 0";
        sd += "\n};\n\n";
        sd += "static const int subfunctions_to_function[] = {";
        for (auto [i, sf] : enumerate(st.subfunctiontable)) {
            if ((i & 0xF) == 0) sd += "\n ";
            append(sd, " ", sf->parent->idx, ",");
        }
        if (st.subfunctiontable.empty()) sd += " 0";
        sd += "\n};\n\n";
        string build_info;
        auto time = std::time(nullptr);
        if (time) {
            auto tm = std::localtime(&time);
            if (tm) {
                auto ts = std::asctime(tm);
                build_info = string(ts, 24);
            }
        }
        if (cpp) sd += "static ";
        else if (!mir) sd += "extern ";
        sd += "const VMMetaData vmmeta = {\n";
        append(sd, "    ", LOBSTER_CODE_FORMAT_VERSION, ",\n");
        append(sd, "    { (const type_elem_t *)type_table, ", type_table.size(), " },\n");
        append(sd, "    { file_names, ", parser.lex.filenames.size(), " },\n");
        append(sd, "    { function_names, ", st.functiontable.size(), " },\n");
        append(sd, "    { udts, ", st.udttable.size(), " },\n");
        append(sd, "    { specidents, ", sids.size(), " },\n");
        append(sd, "    { enums, ", st.enumtable.size(), " },\n");
        append(sd, "    { ser_ids, ", ser_ids.size(), " },\n    ");
        gen_string(build_info);
        sd += ",\n    ";
        to_string_hex(sd, src_hash);
        sd += "ULL,\n";
        append(sd, "    { subfunctions_to_function, ", st.subfunctiontable.size(), " },\n");
        sd += "};\n\n";
        if (cpp) sd += "extern \"C\" ";
        sd += "void compiled_entry_point(VMRef vm) {\n";
        if (cpp) {
            append(sd, "    if (vm.vma.nfr.HashAll() != ", parser.natreg.HashAll(),
                   "ULL) vm.BuiltinError(\"code compiled with mismatching builtin function library\");\n");
        } else {
            // The offsets are what the reads we emit inline depend on; the sizes catch a field
            // being added or widened past the last one we read.
            sd += "    Entry(sizeof(Value), sizeof(VMBase), sizeof(RefObj), sizeof(LVector),\n"
                  "          (int)(long long)&((LVector *)0)->elems, sizeof(LString));\n";
        }
        append(sd, "    ", FunName(CODEGEN_SPECIAL_FUNCTION_ID_ENTRY), "(vm);\n}\n\n");
        if (cpp) {
            sd += "int main(int argc, char *argv[]) {\n";
            sd += "    // This is hard-coded to call compiled_entry_point()\n";
            if (custom_pre_init_name != "nullptr") append(sd, "    void ", custom_pre_init_name, "(lobster::NativeRegistry &);\n");
            sd += "    return RunCompiledCodeMain(argc, argv, ";
            append(sd, "&vmmeta, vtables, object_decs, const_strings, ", custom_pre_init_name,
                   ", \"\");\n}\n");
        }
    }
};

}  // namespace lobster
