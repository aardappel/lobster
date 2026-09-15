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

// Function definitions, blocks and returns, the signatures of functions, and calls, by name,
// thru a function value, or by dynamic dispatch.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenFunctions : virtual CodeGenBase {
    // The inlined function bodies the code being emitted sits inside of, innermost last: what
    // a return out of one needs to know, see InlineBlock::Generate and InlineReturn::Generate.
    struct InlineBlockState {
        const SubFunction *sf;
        size_t tstack_start;  // Slots in use when the block began, where its values go.
        size_t temp_level;    // Temp stack depth when it began, below which nothing is its own.
        size_t nvals;         // How many values it leaves.
        int label;            // Its end, emitted only when a return jumps there.
        bool jumped;
    };
    vector<InlineBlockState> inline_blocks;
    const SubFunction *cursf = nullptr;
    int f_function_idx = -1;
    // The struct a function with more than one return value comes back in, by its name, which
    // the kinds of those values give it, see RetStruct.
    map<string, Types> rets_used;
    // The parameter each slot of the arguments comes in as, in f_args order, and what the
    // function returns, see FunSignature.
    vector<Place> f_arg_places;
    Types f_ret_types;
    vector<int> f_args;
    vector<int> f_defs;
    vector<int> ownedvars;
    bool has_profile = false;

    CodeGenFunctions() {}

    // What a function returns: nothing, a value, or several values in a struct.
    enum RetKind { RK_VOID, RK_VALUE, RK_MULTI };
    static RetKind RetKindOf(int outw) {
        return outw == 0 ? RK_VOID : outw == 1 ? RK_VALUE : RK_MULTI;
    }

    // A letter per kind of value, which is what names the struct several of them come back in.
    static char KindCode(VKind k) {
        static const char codes[] = { 'i', 'f', 'p', 'n', 's', 'v', 'o' };
        return codes[k];
    }

    // The struct a function returns several values in, one per list of kinds, whose members are
    // those values. A struct rather than an array the caller passes a pointer to, since only
    // this way can they stay in registers, and nothing else names the type.
    string RetStruct(const Types &rets) {
        string name = "ret_";
        for (auto r : rets) name += KindCode(Kind(r));
        rets_used.insert({ name, rets });
        return name;
    }

    // The structs the functions that return several values return, see RetStruct.
    void DeclareRetStructs(string &sd) {
        for (auto &[name, rets] : rets_used) {
            sd += "typedef struct {\n";
            for (auto [i, r] : enumerate(rets)) {
                auto ct = CType(Kind(r));
                append(sd, "    ", ct, ct.back() == '*' ? "" : " ", "r", i, ";\n");
            }
            append(sd, "} ", name, ";\n");
        }
    }

    // The C signature of a function: its return value if it has one, the array several land in
    // otherwise, then an argument per slot of its arguments. With names for the definition,
    // without for a declaration.
    string FunSignature(string_view name, const Types &args, const Types &rets,
                        const vector<Place> *params) {
        auto rk = RetKindOf((int)rets.size());
        string s = cat("static ", rk == RK_VALUE ? CType(Kind(rets[0]))
                               : rk == RK_MULTI ? RetStruct(rets) : string("void"),
                       " ", name, "(VMRef");
        if (params) s += " vm";
        for (auto [i, a] : enumerate(args)) {
            auto type = CType(Kind(a));
            append(s, ", ", type);
            if (params) append(s, type.back() == '*' ? "" : " ", (*params)[i].s);
        }
        return s + ")";
    }

    // The same as the type of a pointer to one, for calling a function value or the function
    // a dynamic dispatch lands in.
    string FunPtrType(const Types &args, const Types &rets) {
        auto rk = RetKindOf((int)rets.size());
        string s = cat(rk == RK_VALUE ? CType(Kind(rets[0]))
                     : rk == RK_MULTI ? RetStruct(rets) : string("void"), " (*)(VMRef");
        for (auto &a : args) append(s, ", ", CType(Kind(a)));
        return s + ")";
    }

    // The slots the arguments of a function take, and its return values.
    static Types ArgTypes(const SubFunction &sf) {
        Types ks;
        for (auto &arg : sf.args) if (!arg.sid->constprop) AddTypes(ks, arg.sid->type);
        return ks;
    }

    static Types ReturnTypes(const SubFunction &sf) {
        return TypesOf(sf.returntype, sf.returntype->NumValues());
    }

    // A function is named after what it is called in the program, made a C identifier and
    // told apart from its other specializations and overloads by a number. The prefix keeps
    // it clear of everything else in the file, since no local starts with it, see UniqueName.
    string UniqueFunName(const SubFunction &sf, set<string> &used) {
        string base;
        for (auto c : sf.parent->name) {
            if (isalnum((uint8_t)c)) base += c;
            else if (base.empty() || base.back() != '_') base += '_';
        }
        if (!base.empty() && base.back() == '_') base.pop_back();
        if (!base.empty() && base[0] == '_') base.erase(0, 1);
        auto name = "fun_" + base;
        auto unique = name;
        for (int n = 2; used.count(unique); n++) unique = cat(name, "_", n);
        used.insert(unique);
        return unique;
    }

    void DeclareFunction(SubFunction &sf, string &sd) {
        append(sd, FunSignature(FunName(sf.idx), ArgTypes(sf), ReturnTypes(sf), nullptr), ";\n");
    }

    // A declaration of the variables of one kind, a line per 12 of them to keep it readable.
    void GenDecls(string &sd, VKind k, const vector<string> &names) {
        if (names.empty()) return;
        auto type = CType(k);
        // A pointer type needs its star on each of the variables.
        string star;
        if (type.back() == '*') {
            star = "*";
            type.resize(type.size() - 2);
        }
        for (size_t i = 0; i < names.size(); i++) {
            if (i % 12 == 0) append(sd, i ? ";\n    " : "    ", type, " ");
            else sd += ", ";
            append(sd, star, names[i]);
        }
        sd += ";\n";
    }

    // The same for places of any kind, grouped by it.
    void GenPlaceDecls(string &sd, const vector<Place> &places) {
        for (int k = 0; k < VK_COUNT; k++) {
            vector<string> names;
            for (auto &p : places) if (p.k() == k) names.push_back(p.s);
            GenDecls(sd, VKind(k), names);
        }
    }

    void EmitProfile(string_view profile_name) {
        TrackUseDef(0, 0);
        string name;
        EscapeAndQuote(profile_name, name, true);
        append(cb, "    static struct ___tracy_source_location_data tsld = { ", name, ", ", name,
               ", \"\", 0, 0x888800 }; struct ___tracy_c_zone_context ctx = ",
               cpp ? "lobster::" : "", "StartProfile(&tsld);\n");
        has_profile = true;
    }

    void DefineFunction(string &sd, bool label) {
        sd += "\n";
        auto sf_idx = f_function_idx;
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START)
            append(sd, "// ", Signature(*st.subfunctiontable[sf_idx]), "\n");
        assert(f_arg_places.size() == f_args.size());
        // The body is complete, so a slot whose write is still deferred is never read:
        // dropping those here also keeps the writes to locals below from flushing them
        // into the body, which is already past the declarations they would need.
        pending.clear();
        Types argtypes;
        for (auto &p : f_arg_places) argtypes.push_back(p.rtt);
        append(sd, FunSignature(FunName(sf_idx), argtypes, f_ret_types, &f_arg_places), " {\n");
        // NOTE: f_keeps, f_slot_kinds and f_stage_max are not known until the
        // end of codegen of the function!
        vector<Place> slots, keeps, locals;
        for (auto [i, kinds] : enumerate(f_slot_kinds)) {
            for (int k = 0; k < VK_COUNT; k++) {
                if (kinds & (1 << k)) slots.push_back(SlotVar((int)i, VKind(k)));
            }
        }
        for (int i = 0; i < (int)f_keeps.size(); i++) keeps.push_back(KeepVar(i));
        // The arguments are the parameters, so only the locals after them are declared here.
        int nargs_local = 0;
        for (auto varidx : f_args) if (!sids[varidx].used_as_freevar) nargs_local++;
        for (int i = nargs_local; i < (int)local_places.size(); i++) locals.push_back(Local(i));
        GenPlaceDecls(sd, slots);
        for (auto k : { VK_INT, VK_FLOAT }) {
            if (f_stage_max[k])
                append(sd, "    ", CType(k), " ", StageArray(k), "[", f_stage_max[k], "];\n");
        }
        if (f_uses_pctx) append(sd, "    ", cpp ? "" : "struct ", "___tracy_c_zone_context pctx;\n");
        GenPlaceDecls(sd, keeps);
        GenPlaceDecls(sd, locals);
        if (ShadowLocals() && !local_places.empty())
            append(sd, "    Value locals[", local_places.size(), "];\n");
        if (f_uses_lval) append(sd, "    void *lv = 0;\n");
        if (f_uses_lobj) append(sd, "    LObject *lo = 0;\n");
        if (f_uses_lelem) append(sd, "    void *lvec = 0;\n    long long lidx = 0;\n");
        if (f_ret_types.size() == 1) {
            append(sd, "    ", CType(Kind(f_ret_types[0])), " ret;\n");
        } else if (f_ret_types.size() > 1) {
            append(sd, "    ", RetStruct(f_ret_types), " ret;\n");
        }
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            auto &p = f_arg_places[i];
            if (sids[varidx].used_as_freevar) {
                // The argument is the global for the duration of the call, whose old value the
                // parameter holds meanwhile, to go back at the end. That old value is only
                // ever put back, so it is read past the tag, which on the first call still
                // says nil, the global not having been initialized yet.
                auto t = Var("_t", p.rtt);
                append(sd, "    { ", CType(p.k()), " _t; ", CopyValueText(t, p), " ",
                       WriteText(p, ReadNil(Global(varidx))), " ",
                       CopyValueText(Global(varidx), t), " }\n");
            } else if (ShadowLocals()) {
                CopyValue(sd, Shadow(var_to_local[varidx]), p);
            }
        }
        // A parameter that owns a copy of what the caller passed takes it here, see
        // SpecIdent::copy_on_entry. Given up on exit with the other owned variables, see
        // EmitReturn, which is where --rcstats counts it.
        if (sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            rc_suppress = true;
            for (auto &arg : st.subfunctiontable[sf_idx]->args) {
                if (!arg.sid->copy_on_entry || arg.sid->constprop) continue;
                for (int i = 0; i < ValWidth(arg.sid->type); i++) {
                    auto varidx = arg.sid->Idx() + i;
                    if (!IsRefNil(var_types[varidx]->t)) continue;
                    GenIncRef(sd, sids[varidx].used_as_freevar
                                      ? Global(varidx)
                                      : Local(var_to_local[varidx]));
                }
            }
            rc_suppress = false;
        }
        for (int i = 0; i < (int)f_defs.size(); i++) {
            // for most locals, this just saves an nil, only in recursive cases it has an
            // actual value.
            auto varidx = f_defs[i];
            if (sids[varidx].used_as_freevar) {
                append(sd, "    BackupVar(vm, ", varidx, ");\n");
            } else {
                // A reference starts out nil, since a return before its definition has run
                // still gives up what the variable holds, see EmitReturn. A number is always
                // written before it is read, so it is left alone.
                auto k = var_to_local[varidx];
                if (IsRefKind(Local(k).k())) SetNil(sd, Local(k));
                if (ShadowLocals()) SetNil(sd, Shadow(k));
            }
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && sf_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            // FIXME: can make this just and index and instead store funinfo_table ref in
            // VM. Calling this here because now locals have been fully initialized.
            append(sd, "    PushFunId(vm, funinfo_table + ", funstarttables.size(), ", ",
                   local_places.empty() ? "0" : "locals", ");\n");
            // This can be any format we want, see VM::DumpStackFrame
            funstarttables.push_back(f_function_idx);
            funstarttables.push_back((int)f_args.size());
            funstarttables.insert(funstarttables.end(), f_args.begin(), f_args.end());
            funstarttables.push_back((int)f_defs.size());
            funstarttables.insert(funstarttables.end(), f_defs.begin(), f_defs.end());
        }
        for (int i = 0; i < (int)f_keeps.size(); i++) {
            SetNil(sd, KeepVar(i));
        }
        // A return that has nothing of its own to say leaves these alone, since the caller then
        // reads what a non-local return left in the buffer instead, see GenUnwind. Written one
        // at a time rather than with an initializer, which libtcc compiles into a memset call.
        if (f_ret_types.size() > 1) {
            for (auto [i, kr] : enumerate(f_ret_types)) SetNil(sd, RetSlot((int)i, kr));
        }

        sd += cb;
        cb.clear();

        if (label) sd += "    epilogue:;\n";
        if (has_profile) {
            append(sd, "    ", cpp ? "lobster::" : "", "EndProfile(ctx);\n");
        }
        // The locals that live in a global get their old value back, see BackupVar above.
        for (int i = (int)f_defs.size() - 1; i >= 0; i--) {
            auto varidx = f_defs[i];
            if (sids[varidx].used_as_freevar) {
                append(sd, "    RestoreBackup(vm, ", varidx, ");\n");
            }
        }
        // Counted for --rcstats where the keep was made, see EmitKeep.
        rc_suppress = true;
        for (int i = 0; i < (int)f_keeps.size(); i++) {
            GenDecRef(sd, KeepVar(i));
        }
        rc_suppress = false;
        for (int i = 0; i < (int)f_args.size(); i++) {
            auto varidx = f_args[i];
            if (sids[varidx].used_as_freevar) CopyValue(sd, Global(varidx), f_arg_places[i]);
        }
        if (runtime_checks >= RUNTIME_STACK_TRACE && f_function_idx < CODEGEN_SPECIAL_FUNCTION_ID_START) {
            append(sd, "    PopFunId(vm);\n");
        }
        if (!f_ret_types.empty()) sd += "    return ret;\n";
        sd += "}\n";
        ownedvars.clear();
        f_keeps.clear();
        f_slot_kinds.clear();
        f_stage_max[VK_INT] = 0;
        f_stage_max[VK_FLOAT] = 0;
        f_uses_pctx = false;
        f_uses_lval = false;
        f_uses_lobj = false;
        f_uses_lelem = false;
        local_places.clear();
        f_names_used.clear();
        nlabel = 0;
        has_profile = false;
    }

    void GenStatDebug(const Node *c) {
        if (runtime_checks >= RUNTIME_STACK_TRACE) {
            if (cpp) {
                append(cb, "    vm.last = {", c->line.line, ", ", c->line.fileidx, "};\n");
            } else {
                append(cb, "    vm->last_line = ", c->line.line, ";\n");
                append(cb, "    vm->last_fileidx = ", c->line.fileidx, ";\n");
            }
        }
    }

    // Transfer an owned variable to the caller when the function's only return is its final
    // "return var". The ordinary typed tree retains the inc, so inlining needs no undo: only
    // an actual function exit can omit it together with this variable's scope-exit dec.
    // Decide from the final tree and unwind information, after optimization is complete.
    static const IdentRef *ReturnedOwnedVar(const SubFunction &sf) {
        if (sf.num_returns != 1 || sf.returned_thru_to_max >= 0 || !sf.reqret) return nullptr;
        auto ret = AssertIs<Return>(sf.sbody->children.back());
        if (ret->sf != &sf || ret->make_void) return nullptr;
        auto inc = Is<ToLifetime>(ret->child);
        if (!inc || inc->incref != 1 || inc->decref) return nullptr;
        auto ir = Is<IdentRef>(inc->child);
        if (!ir || ir->sid->lt != LT_KEEP || !IsRefNil(ir->sid->type->t)) return nullptr;
        // Inlined copies share these variables and can change sf_def, so membership in the
        // function's actual scope, which GenScope uses for cleanup, is what matters here.
        for (auto &arg : sf.args) if (arg.sid == ir->sid) return ir;
        for (auto &local : sf.locals) if (local.sid == ir->sid) return ir;
        return nullptr;
    }

    void GenScope(SubFunction &sf) {
        cursf = &sf;
        if (!sf.typechecked) {
            auto s = DumpNode(*sf.sbody, 0, false);
            LOG_DEBUG("untypechecked: ", sf.parent->name, " : ", s);
            assert(0);
        }

        f_function_idx = sf.idx;
        f_keeps.clear();  // Not valid until end of codegen of this function.

        auto ir = ReturnedOwnedVar(sf);

        #ifndef NDEBUG
            var_to_local.clear();
            var_to_local.resize(sids.size(), -1);
        #endif
        local_places.clear();
        f_names_used.clear();
        f_arg_places.clear();
        f_ret_types = ReturnTypes(sf);
        auto emitvars = [&](const vector<Arg> &v, vector<int> &f_ad) {
            f_ad.clear();
            for (auto &arg : v) {
                if (arg.sid->constprop) {
                    continue;
                }
                auto n = ValWidth(arg.sid->type);
                for (int i = 0; i < n; i++) {
                    auto varidx = arg.sid->Idx() + i;
                    f_ad.push_back(varidx);
                    if (ShouldDec(IsStruct(arg.sid->type->t)
                                      ? TypeLT { SlotTypeOf(*arg.sid->type->udt, i),
                                                 arg.sid->lt }
                                      : TypeLT { *arg.sid }) && (!ir || arg.sid != ir->sid)) {
                        ownedvars.push_back(arg.sid->Idx() + i);
                    }
                    auto vtype = var_types[varidx];
                    if (!sids[varidx].used_as_freevar) {
                        var_to_local[varidx] = (int)local_places.size();
                        local_places.push_back(Var(LocalName(*arg.sid, i), RtTypeOf(vtype)));
                        if (&f_ad == &f_args) f_arg_places.push_back(local_places.back());
                    } else if (&f_ad == &f_args) {
                        // Lives in a global while the function runs, so the parameter only
                        // holds the old value of that meanwhile, see DefineFunction.
                        f_arg_places.push_back(Var(LocalName(*arg.sid, i), RtTypeOf(vtype)));
                    }
                }
            }
        };
        emitvars(sf.args, f_args);
        emitvars(sf.locals, f_defs);

        auto profile = sf.attributes.find("profile");
        if (profile != sf.attributes.end() && LOBSTER_FRAME_PROFILER) {
            auto str = string(profile->second);
            if (str.empty()) {
                str = sf.parent->name;
                if (!sf.args.empty() && !sf.parent->overloads.empty()) {
                    append(str, "(", TypeName(sf.args[0].spec_type), (sf.args.size() > 1 ? ", .." : ""), ")");
                }
            }
            EmitProfile(str);
        }

        for (auto c : sf.sbody->children) {
            GenStatDebug(c);
            Gen(c, 0);
            assert(!tstack_size);
        }

        assert(temptypestack.empty());
        assert(breaks.empty());
        assert(loops.empty());
        assert(inline_blocks.empty());
        assert(!tstack_size);
        cursf = nullptr;
    }

    void Generate(const Block &node, size_t retval) {
        auto tstack_start = tstack_size;
        (void)tstack_start;
        for (auto c : node.children) {
            GenStatDebug(c);
            if (c != node.children.back()) {
                // Not the last element.
                Gen(c, 0);
                assert(tstack_start == tstack_size);
            } else {
                Gen(c, retval);
                TakeTemp(retval, true);
            }
        }
    }

    void Generate(const Seq &node, size_t retval) {
        Gen(node.head, 0);
        Gen(node.tail, retval);
        if (retval) TakeTemp(1, true);
    }

    // Slot i of the values a non-local return passes to the function it returns from.
    Place RetBufSlot(int i, RTType rtt) {
        return Mem(cpp ? cat("vm.ret_buf.v[", i, "]") : cat("vm->ret_buf[", i, "]"), rtt);
    }
    // The function's own return value, and slot i of it when it returns several, which are the
    // members of the struct it returns them in, see RetStruct.
    Place RetVar() { return Var("ret", f_ret_types[0]); }
    Place RetSlot(int i, RTType rtt) { return Var(cat("ret.r", i), rtt); }
    string_view vmref() { return string_view(cpp ? "vm." : "vm->"); }

    // The function's return values, from wherever `src` says slot i of them is.
    template<typename F> void GenReturnValues(F src) {
        if (f_ret_types.size() == 1) {
            CopyValue(cb, RetVar(), src(0));
        } else {
            for (auto [i, kr] : enumerate(f_ret_types)) {
                CopyValue(cb, RetSlot((int)i, kr), src((int)i));
            }
        }
    }

    // A return. The values come off the stack into the function's own return channel, or for
    // a non-local return into the VM's buffer, marked for the function they return from,
    // parent_idx, whose frame then sends them out thru its own channel, see GenUnwind. RET_ANY
    // passes such a return on to the caller, which has nothing to copy. `rets` says what the
    // values are, for the function they return from.
    void EmitReturn(ReturnKind kind, const Types &rets, int parent_idx, int useslots) {
        auto nretslots = (int)rets.size();
        TrackUseDef(useslots, 0);
        // FIXME: emit epilogue stuff only once at end of function.
        if (kind == RET_LOCAL) {
            if (nretslots) {
                GenReturnValues([&](int i) { return SlotVar(regso - nretslots + i, rets[i]); });
            } else if (f_ret_types.size() == 1) {
                // A return without a value out of a function whose result is not used.
                SetNil(cb, RetVar());
            }
        } else if (kind == RET_NONLOCAL) {
            if (nretslots > MAX_RETURN_SLOTS) {
                Error("too many values returned thru a non-local return",
                      node_context.back()->line);
                nretslots = MAX_RETURN_SLOTS;
            }
            for (int i = 0; i < nretslots; i++) {
                CopyValue(cb, RetBufSlot(i, rets[i]), SlotVar(regso - nretslots + i, rets[i]));
            }
            append(cb, "    ", vmref(), "ret_unwind_to = ", parent_idx, ";\n");
            // Nothing of its own to return, but the caller expects a value all the same.
            if (f_ret_types.size() == 1) SetNil(cb, RetVar());
        }
        for (auto varidx : ownedvars) {
            auto is_arg = find(f_args.begin(), f_args.end(), varidx) != f_args.end();
            rc_tag = is_arg ? "scope-exit:arg" : "scope-exit:local";
            rc_extra = IdName(varidx, var_types[varidx]);
            if (sids[varidx].used_as_freevar) {
                if (auto rs = RcStatCall(false); !rs.empty())
                    append(cb, "    if (", Read(Global(varidx)), ") ", rs, "\n");
                append(cb, "    DecOwned(vm, ", varidx, ");\n");
            } else {
                GenDecRef(cb, Local(var_to_local[varidx]));
            }
            rc_tag.clear();
            rc_extra.clear();
        }
        if (kind == RET_ANY) {
            // What the call we are passing thru from left is still on the tstack.
            for (int i = 0; i < nretslots; i++)
                PopTemp();
            for (auto &tse : reverse(temptypestack)) {
                GenPop(tse);
            }
        }
        Flush();
        append(cb, "    goto epilogue;\n");
    }

    void Generate(const MultipleReturn &node, size_t retval) {
        for (auto [i, c] : enumerate(node.children))
            Gen(c, i < retval);
        TakeTemp(retval, true);
        for (auto[i, c] : enumerate(node.children))
            if (i < retval)
                rettypes.push_back({ c->exptype, c->lt });
    }

    void Generate(const Return &node, size_t retval) {
        assert(!rettypes.size());
        small_vector<TypeLT, 8> typestackbackup = temptypestack;
        auto tstackbackup = tstack_size;
        // The code after the return, reached some other way, still has the pending slots.
        auto pendingbackup = pending;
        if (temptypestack.size()) {
            // We have temps on the stack, these can be from:
            // * an enclosing for.
            // * an (inlined) block, whose caller already had temps on the stack.
            // We can't actually remove these from the stack permanently as the parent nodes still
            // expect them to be there.
            while (!temptypestack.empty()) {
                GenPop(temptypestack.back());
                temptypestack.pop_back();
            }
        }
        Types rets;
        if (node.sf->reqret) {
            auto nretvals = node.make_void ? 0 : node.sf->returntype->NumValues();
            if (!Is<DefaultVal>(node.child)) {
                auto ir = node.sf == cursf && &node == node.sf->sbody->children.back()
                    ? ReturnedOwnedVar(*node.sf) : nullptr;
                Gen(ir ? ir : node.child, nretvals);
                TakeTemp(nretvals, true);
            } else {
                EmitPushNil(node.sf->returntype);
                assert(nretvals == 1);
            }
            rets = TypesOf(node.sf->returntype, nretvals);
        } else {
            if (!Is<DefaultVal>(node.child)) Gen(node.child, 0);
        }
        // FIXME: we could change the VM to instead work with SubFunction ids.
        // Note: this can only work as long as the type checker forces specialization
        // of the functions in between here and the function returned to.
        // Actually, doesn't work with DDCALL and RETURN_THRU.
        // FIXME: shouldn't need any type here if V_VOID, but nretvals is at least 1 ?
        auto nretslots = (int)rets.size();
        if (node.sf == cursf) {
            EmitReturn(RET_LOCAL, rets, -1, nretslots);
        } else {
            EmitReturn(RET_NONLOCAL, rets, node.sf->parent->idx, nretslots);
        }

        reset_from_small_vector(temptypestack, typestackbackup);
        tstack_size = tstackbackup;
        pending = pendingbackup;
        // We can promise to be providing whatever retvals the caller wants.
        for (size_t i = 0; i < retval; i++) {
            rettypes.push_back({ type_undefined, LT_ANY });
            PushTemp();  // FIXME: is this necessary? do more generally?
        }
    }

    void Generate(const InlineBlock &node, size_t retval) {
        // Like any block, this leaves just the values asked for, and so does every return in it.
        assert(retval <= node.exptype->NumValues());
        inline_blocks.push_back({ node.sf, tstack_size, temptypestack.size(), retval,
                                  Label(), false });
        Generate(static_cast<const Block &>(node), retval);
        auto ib = inline_blocks.back();
        inline_blocks.pop_back();
        if (!ib.jumped) return;
        // The jumps arrive with the values in their slots. The code falling thru here leaves the
        // same, unless nothing ever falls thru, in which case what a terminal statement leaves
        // stands in for them, and that can be fewer slots than they take, see Return::Generate.
        assert(temptypestack.size() == ib.temp_level);
        auto end = ib.tstack_start + ValWidthMulti(node.exptype, retval);
        assert(tstack_size <= end);
        while (tstack_size < end) PushTemp();
        EmitLabelDef(ib.label);
    }

    void Generate(const InlineReturn &node, size_t retval) {
        assert(!rettypes.size());
        // This is in the innermost inlined body: one in a body this body calls would be a
        // non-local return, which inlining leaves as a call, see Call::Optimize.
        assert(!inline_blocks.empty());
        auto ibi = inline_blocks.size() - 1;
        auto ib = inline_blocks[ibi];
        assert(ib.sf == node.sf);
        auto typestackbackup = temptypestack;
        auto tstackbackup = tstack_size;
        auto pendingbackup = pending;
        if (node.make_void) {
            // The implicit return at the end of a function that does return values wraps a
            // statement that never completes, see Return::TypeCheck. Every way out of it is a
            // jump of its own, so it is generated as the statement it is.
            assert(!Is<DefaultVal>(node.child));
            Gen(node.child, 0);
        } else {
            // What the block's own constructs hold on the temp stack goes, the way a return out
            // of a frame drops all of it: the iterator of a loop this is in, the arguments before
            // this one of a call it is an argument of. Below that is the caller's, which stays.
            assert(temptypestack.size() >= ib.temp_level);
            while (temptypestack.size() > ib.temp_level) {
                GenPop(temptypestack.back());
                temptypestack.pop_back();
            }
            // Nothing else of the block's is live, so the values land where the block leaves them.
            assert(tstack_size == ib.tstack_start);
            if (!Is<DefaultVal>(node.child)) {
                Gen(node.child, ib.nvals);
                TakeTemp(ib.nvals, true);
            } else if (ib.nvals) {
                assert(ib.nvals == 1);
                EmitPushNil(node.sf->returntype);
            }
            assert(tstack_size ==
                   ib.tstack_start + (size_t)ValWidthMulti(node.sf->returntype, ib.nvals));
            inline_blocks[ibi].jumped = true;
            EmitJumpBack(ib.label);
        }
        // What follows is not reached, but the code emitted for it has to see the stack the way it
        // was, and the values it was promised, the same as after a return.
        temptypestack = typestackbackup;
        tstack_size = tstackbackup;
        pending = pendingbackup;
        for (size_t i = 0; i < retval; i++) {
            rettypes.push_back({ type_undefined, LT_ANY });
            PushTemp();
        }
    }

    void EmitPushFun(int fidx) {
        TrackUseDef(0, 1);
        Defer(Slot(0, VK_FUN), cat("(fun_base_t)", FunName(fidx)), "");
    }

    void Generate(const FunRef &node, size_t retval) {
        if (!retval) return;
        // If no body, then the function has been optimized away, meaning this
        // function value will never be used.
        // FIXME: instead, ensure such values are removed by the optimizer.
        if (node.sf->parent->anonymous && node.sf->sbody && node.sf->typechecked) {
            EmitPushFun(node.sf->idx);
        } else {
            EmitPushFun(CODEGEN_SPECIAL_FUNCTION_ID_DUMMY);
        }
    }

    // A call to `callee`, an expression for the function, with the arguments on the stack and
    // the result landing where they were. `uses` is what the call consumes, which is the
    // arguments plus the function value above them when there is one.
    void EmitCallTo(string_view callee, const Types &args, const Types &rets, int uses,
                    string_view cmt = {}) {
        auto defs = (int)rets.size();
        TrackUseDef(uses, defs);
        auto rk = RetKindOf(defs);
        auto base = regso - uses;
        string call = cat(callee, "(vm");
        for (auto [i, a] : enumerate(args)) append(call, ", ", Read(SlotVar(base + (int)i, a)));
        call += ")";
        if (rk == RK_MULTI) {
            // Several results come back in a struct, whose members go into the slots they
            // belong in.
            append(cb, "    {");
            if (cmt.empty()) cb += "\n"; else comment(cmt);
            append(cb, "    ", RetStruct(rets), " _r = ", call, ";\n");
            for (int i = 0; i < defs; i++) {
                Write(cb, SlotVar(base + i, rets[i]), cat("_r.r", i));
            }
            cb += "    }\n";
            return;
        }
        if (rk == RK_VALUE) Write(cb, SlotVar(base, rets[0]), call, "");
        else append(cb, "    ", call, ";");
        if (cmt.empty()) cb += "\n"; else comment(cmt);
    }

    void EmitCall(const SubFunction &sf, int inw) {
        EmitCallTo(FunName(sf.idx), ArgTypes(sf), ReturnTypes(sf), inw,
                   "call: " + Signature(sf));
    }

    // The function value on top of the stack is called with what sits below it, cast to the
    // signature the type checker gave it.
    void EmitCallValue(const Types &args, const Types &rets) {
        TrackUseDef(0, 0);
        auto ptr = cat("((", FunPtrType(args, rets), ")", Read(Slot(1, VK_FUN)), ")");
        EmitCallTo(ptr, args, rets, (int)args.size() + 1);
    }

    // Same, for the function a dispatch on the dynamic type of the first argument lands in:
    // the class of an object, or the member of an abstract struct family whose index the
    // type slot of a struct (the first of its slots) holds. The vtables of a family's
    // members sit at one stride from the root's (see CodeGen::CodeGen), so the entry is
    // computed from the index right here, with one load left for the runtime.
    void EmitDynDispatch(int vtable_idx, const Types &args, const Types &rets, TypeRef self) {
        TrackUseDef(0, 0);
        string target;
        if (IsStruct(self->t)) {
            auto root = self->udt->family_root;
            target = cat("RtDynDispatchStruct(vm, ", root->vtable_start + vtable_idx, " + ",
                         FamilyIndexText(Slot((int)args.size(), VK_INT), *root), " * ",
                         root->family_vtable_stride, ")");
        } else {
            target = cat("RtDynDispatch(vm, ", ReadAs(Slot((int)args.size(), args[0]), VK_OBJECT),
                         ", ", vtable_idx, ")");
        }
        auto ptr = cat("((", FunPtrType(args, rets), ")", target, ")");
        EmitCallTo(ptr, args, rets, (int)args.size());
    }

    // After a call to a function a non-local return can come out of. If one is in flight this
    // function is done too: either it is the one being returned from, in which case what it
    // returns is in the VM's buffer and goes out thru its own return channel, or it passes the
    // return on to its caller, see EmitReturn. A call that never completes comes back this way
    // if it comes back at all, so then there is no path around it, see GenCall.
    void GenUnwind(const Types &rets, bool completes) {
        TrackUseDef(0, 0);
        Flush();
        auto lab = -1;
        if (completes) {
            lab = Label();
            append(cb, "    if (", vmref(), "ret_unwind_to < 0) goto block", lab, ";\n");
        }
        // Here we are emitting code executed only if we're unwinding, so temp modify the
        // tstack to match that.
        auto tstackbackup = tstack_size;
        // Only a Lobster function can be returned from, the entry function is never the target.
        if (cursf) {
            append(cb, "    if (", vmref(), "ret_unwind_to == ", cursf->parent->idx, ") {\n");
            append(cb, "    ", vmref(), "ret_unwind_to = -1;\n");
            GenReturnValues([&](int i) { return RetBufSlot(i, f_ret_types[i]); });
            if (f_ret_types.size() == 1) {
                // Passing it on, with nothing of its own to return.
                cb += "    } else {\n";
                SetNil(cb, RetVar());
            }
            cb += "    }\n";
        } else if (f_ret_types.size() == 1) {
            SetNil(cb, RetVar());
        }
        EmitReturn(RET_ANY, rets, -1, 0);
        if (completes) EmitLabelDef(lab);
        tstack_size = tstackbackup;
    }

    void GenCall(const Call &call, size_t retval) {
        auto &sf = *call.sf;
        auto &f = *sf.parent;
        int inw = 0;
        for (auto c : call.children) {
            Gen(c, 1);
            inw += ValWidth(c->exptype);
        }
        size_t nargs = call.children.size();
        if (f.nargs() != nargs)
            Error(cat("call to function ", Q(f.name), " needs ", f.nargs(), " arguments, ",
                      nargs, " given"),
                  node_context.back()->line);
        TakeTemp(nargs, true);
        auto args = ArgTypes(sf);
        auto rets = ReturnTypes(sf);
        if (inw != (int)args.size()) {
            Error(cat("internal error: call to ", Q(f.name), " passes ", inw,
                      " slots where it takes ", args.size()),
                  node_context.back()->line);
        }
        auto nretvals = sf.returntype->NumValues();
        // Only a call that never completes is asked for values it does not return, see
        // TypeChecker::TT. The code after it reads them from slots nothing writes (see Read), so
        // nothing in the C may reach that code either, or a C++ compiler warns about the reads.
        auto completes = nretvals >= retval;
        if (call.vtable_idx < 0) {
            EmitCall(sf, inw);
            if (sf.returned_thru_to_max >= 0 || !completes) {
                GenUnwind(rets, completes);
            }
        } else {
            EmitDynDispatch(call.vtable_idx, args, rets, call.children[0]->exptype);
            // We get the dispatch from arg 0, since sf is an arbitrary overloads and
            // doesn't necessarily point to the dispatch root (which may not even have an sf).
            auto dispatch_type = call.children[0]->exptype;
            assert(IsUDT(dispatch_type->t));
            auto de = dispatch_type->udt->dispatch_table[call.vtable_idx].get();
            assert(de->dispatch_root && !de->returntype.Null() && de->subudts_size);
            if (de->returned_thru_to_max >= 0) {
                // This works because all overloads of a DD sit under a single Function.
                GenUnwind(rets, completes);
            }
        }
        for (size_t i = 0; i < nretvals; i++) {
            if (retval) {
                rettypes.push_back({ sf, i });
            } else {
                // FIXME: better if this is impossible by making sure typechecker makes it !reqret.
                GenPop({ sf, i });
            }
        }
        for (size_t i = nretvals; i < retval; i++) {
            // This can happen in a function that ends in a non-local return (thus nretvals==0)
            // but retval>0 because it is inside an if-then-else branch.
            // FIXME: take care of this in Gen() instead? Are there other nodes for which this
            // can happen?
            PushTemp();
        }
    }

    void Generate(const Call &node, size_t retval) {
        GenCall(node, retval);
    }

    void Generate(const DynCall &node, size_t retval) {
        assert(node.sf && node.sf == node.sid->type->sf && node.sf->parent->istype);
        // The function type has no variables for its arguments, so their kinds come from what
        // is passed.
        Types args;
        for (auto c : node.children) {
            Gen(c, 1);
            AddTypes(args, c->exptype);
        }
        size_t nargs = node.children.size();
        assert(nargs == node.sf->args.size());
        GenPushVar(1, type_function_null_void, node.sid->Idx(), node.sid->used_as_freevar);
        TakeTemp(nargs, true);
        EmitCallValue(args, ReturnTypes(*node.sf));
        if (node.sf->reqret) {
            if (!retval) GenPop({ node.exptype, node.lt });
        } else {
            assert(!retval);
        }
    }
};

}  // namespace lobster
