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

// Reference increments, decrements, keep slots, object deleters, and reference-count statistics.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenRefCount : virtual CodeGenBase {
    CodeGenRefCount() {}

    // Keeps the reference at the given depth in a temporary of its type until the function
    // returns, since the reference is only borrowed by whatever consumes it.
    void EmitKeep(int stack_offset, RTType rtt) {
        // Inside a loop the slot may already hold a reference from a previous iteration.
        auto inloop = !loops.empty();
        TrackUseDef(0, 0);
        auto offset = (int)f_keeps.size();
        f_keeps.push_back(rtt);
        // The reference is given up when the function returns (or when the loop this is in
        // comes back around), which is counted here, where the site is known.
        if (auto rs = RcStatCall(false, ":keep"); !rs.empty())
            append(cb, "    if (", Read(Slot(stack_offset + 1, rtt)), ") ", rs, "\n");
        rc_suppress = true;
        if (inloop) GenDecRef(cb, KeepVar(offset));
        rc_suppress = false;
        CopyValue(cb, KeepVar(offset), Slot(stack_offset + 1, rtt));
    }

    // Only the decrement itself is worth emitting: what happens when it reaches zero is a good
    // deal more code, and stays a call.
    // A pending reference that is a constant can only be nil, which has no count.
    bool IsNilConstant(const Place &p) {
        return HasPending(p.slot) && pending[p.slot].vars.empty() && !pending[p.slot].prec;
    }

    // The deleter for the kind of reference the static type says a place holds, which skips
    // the type lookup and switch of the general one; a kind without one of its own goes thru
    // that.
    static const char *DecDeleteName(RTType rtt) {
        switch (rtt) {
            case RTT_VECTOR: return "DecDeleteVec";
            case RTT_CLASS: return "DecDeleteObj";
            case RTT_STRING: return "DecDeleteStr";
            default: return "DecDelete";
        }
    }

    static const Node *SkipCoercionsForRc(const Node *n) {
        while (auto c = dynamic_cast<const Coercion *>(n)) n = c->child;
        return n;
    }

    // A short description of what a value comes from, for the --rcstats site descriptions.
    static string RcNodeDesc(const Node *n) {
        if (auto c = Is<Call>(n)) return cat("call ", c->sf->parent->name);
        if (auto c = dynamic_cast<const Coercion *>(n)) return cat(n->Name(), "(", RcNodeDesc(c->child), ")");
        if (auto nc = Is<NativeCall>(n)) return cat("native ", nc->nf->name);
        if (auto id = Is<IdentRef>(n)) return cat("ident ", id->sid->id->name);
        if (auto d = Is<Dot>(n)) return cat(RcNodeDesc(d->child), ".", d->fld->name);
        if (auto t = Is<ToLifetime>(n)) return RcNodeDesc(t->child);
        return string(n->Name());
    }

    // The call that counts an executed inc/dec for --rcstats, with a site registered for it
    // that says what the op is for (rc_tag, rc_extra), the node being generated, and where
    // that is. Empty when not counting.
    string RcStatCall(bool inc, string_view extra = {}) override {
        if (!rcstats || rc_suppress) return {};
        auto n = node_context.back();
        auto desc = cat(inc ? "inc " : "dec ", rc_tag, extra, " [", n->Name(), " ",
                        rc_extra.empty() ? RcNodeDesc(n) : rc_extra, "] ",
                        parser.lex.Location(n->line));
        g_rcstat_sites.push_back({ desc, cat(rc_tag, extra), inc });
        g_rcstat_counts.push_back(0);
        return cat("RcStat(vm, ", g_rcstat_sites.size() - 1, "); ");
    }

    void GenDecRef(string &sd, const Place &p) override {
        if (IsNilConstant(p)) return;
        auto r = Read(p);
        auto dd = DecDeleteName(p.rtt);
        auto rs = cpp ? string() : RcStatCall(false);
        // Only a variable is free to be named more than once, so the rest go thru a local.
        if (p.var) {
            if (cpp) append(sd, "    if (", r, ") ", r, "->Dec(vm);\n");
            else if (rs.empty()) append(sd, "    if (", r, " && --", r, "->refc <= 0) ", dd, "(vm, ", r, ");\n");
            else append(sd, "    if (", r, ") { ", rs, "if (--", r, "->refc <= 0) ", dd, "(vm, ", r, "); }\n");
        } else if (cpp && !p.typed) {
            append(sd, "    ", p.s, ".LTDECRTNIL(vm);\n");
        } else if (cpp) {
            append(sd, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->Dec(vm); }\n");
        } else if (rs.empty()) {
            append(sd, "    { ", CType(p.k()), "_r = ", r, ";"
                       " if (_r && --_r->refc <= 0) ", dd, "(vm, _r); }\n");
        } else {
            append(sd, "    { ", CType(p.k()), "_r = ", r, ";"
                       " if (_r) { ", rs, "if (--_r->refc <= 0) ", dd, "(vm, _r); } }\n");
        }
    }

    void GenIncRef(string &sd, const Place &p) override {
        if (IsNilConstant(p)) return;
        auto r = Read(p);
        auto rs = cpp ? string() : RcStatCall(true);
        if (p.var) {
            if (cpp) append(sd, "    if (", r, ") ", r, "->Inc();\n");
            else if (rs.empty()) append(sd, "    if (", r, ") ", r, "->refc++;\n");
            else append(sd, "    if (", r, ") { ", rs, r, "->refc++; }\n");
        } else if (cpp && !p.typed) {
            append(sd, "    ", p.s, ".LTINCRTNIL();\n");
        } else if (cpp) {
            append(sd, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->Inc(); }\n");
        } else if (rs.empty()) {
            append(sd, "    { ", CType(p.k()), "_r = ", r, "; if (_r) _r->refc++; }\n");
        } else {
            append(sd, "    { ", CType(p.k()), "_r = ", r, "; if (_r) { ", rs, "_r->refc++; } }\n");
        }
    }

    void EmitIncRef(int off, RTType rtt) {
        TrackUseDef(0, 0);
        // FIXME: even when the static type is IsRef (i.e. no NIL or scalar), at runtime it is
        // still possible we get passed an int false value due to the way and/or are compiled?
        // See e.g. astar_result in the test.
        // Would be great to remove this case since the if-check is not needed in almost all cases.
        GenIncRef(cb, Slot(off + 1, rtt));
    }

    void EmitPopRef(RTType rtt) {
        TrackUseDef(1, 0);
        rc_tag = "pop";
        GenDecRef(cb, Slot(1, rtt));
        rc_tag.clear();
    }

    void GenPop(TypeLT typelt) override {
        if (IsStruct(typelt.type->t)) {
            if (typelt.type->t == V_STRUCT_R) {
                // TODO: alternatively call a single helper with a list or bitmask?
                // See BitMaskForRefStruct.
                for (int j = typelt.type->udt->numslots - 1; j >= 0; j--) {
                    auto stype = SlotTypeOf(*typelt.type->udt, j);
                    if (IsRefNil(stype->t)) EmitPopRef(RtTypeOf(stype));
                    else GenPopSlot();
                }
            } else {
                // A struct of scalars is just slots to give up.
                TrackUseDef(typelt.type->udt->numslots, 0);
            }
        } else {
            if (ShouldDec(typelt)) EmitPopRef(RtTypeOf(typelt.type)); else GenPopSlot();
        }
    }

    // Calls `f` with the stack offset and runtime type of every slot of a value of `type`,
    // whose first slot is at `stack_offset`, that holds a reference: the value itself, or
    // the reference fields of a struct of references.
    // TODO: alternatively call a single helper with a list or bitmask? See BitMaskForRefStruct.
    template<typename F> static void ForEachRefSlot(TypeRef type, int stack_offset, F f) {
        if (type->t != V_STRUCT_R) {
            f(stack_offset, RtTypeOf(type));
            return;
        }
        auto &udt = *type->udt;
        for (int j = 0; j < udt.numslots; j++) {
            auto stype = SlotTypeOf(udt, j);
            if (IsRefNil(stype->t)) f(stack_offset + udt.numslots - 1 - j, RtTypeOf(stype));
        }
    }

    void Generate(const ToLifetime &node, size_t retval) {
        Gen(node.child, retval);
        rc_tag = cat("tolt:", SkipCoercionsForRc(node.child)->Name());
        rc_extra = RcNodeDesc(node.child);
        int stack_offset = 0;
        for (int fi = 0; fi < (int)retval; fi++) {
            // We have to check for reftype again, since typechecker allowed V_VAR values that may
            // have become scalars by now.
            auto i = (int)(retval - fi - 1);
            assert(i < ssize(temptypestack));
            auto type = temptypestack[temptypestack.size() - 1 - fi].type;
            if (IsRefNil(node.child->exptype->Get(i)->t)) {
                if (node.incref & (1LL << i)) {
                    assert(IsRefNil(type->t));
                    ForEachRefSlot(type, stack_offset,
                                   [&](int off, RTType rtt) { EmitIncRef(off, rtt); });
                }
                if (node.decref & (1LL << i)) {
                    assert(IsRefNil(type->t));
                    ForEachRefSlot(type, stack_offset,
                                   [&](int off, RTType rtt) { EmitKeep(off, rtt); });
                }
            }
            stack_offset += ValWidth(type);
        }
        rc_tag.clear();
        rc_extra.clear();
        // We did not consume these, so we have to pass them on.
        for (size_t i = 0; i < retval; i++) {
            // Note: take LT from this node, not existing one on temptypestack,
            // which we just changed!
            rettypes.push_back(TypeLT(node, i));
            temptypestack.pop_back();
        }
    }

    // What an object of each type gives up when it is deleted: the references among its
    // fields, which the code knows exactly and which it therefore says outright rather than
    // leaving the runtime to walk the type info for every field, see LObject::DeleteSelf.
    // A type with none of them has no function and a null in the table.
    void EmitObjectDecs(string &sd) {
        vector<string> decs(st.udttable.size());
        for (auto udt : st.udttable) {
            if (udt->g.is_struct || udt->numslots <= 0) continue;
            // Declared in a function that never got typechecked, so the slots have no types to
            // ask about. Nothing can construct one either, so it needs no deleter.
            if (udt->state != UDTState::CHECKED) continue;
            string body;
            rc_tag = "udt-dec";
            rc_extra = udt->name;
            for (int i = 0; i < udt->numslots; i++) {
                auto rtt = RtTypeOf(SlotTypeOf(*udt, i));
                if (RTIsRefNil(rtt)) GenDecRef(body, Field("o", *udt, i));
            }
            rc_tag.clear();
            rc_extra.clear();
            if (body.empty()) continue;
            decs[udt->idx] = UDTName(*udt) + "_dec";
            append(sd, "\nstatic void ", decs[udt->idx], "(VMRef vm, LObject *o) {\n", body,
                   "}\n");
        }
        if (cpp) sd += "\nstatic";
        else if (mir) sd += "\n";
        else sd += "\nextern";
        sd += " const object_dec_t object_decs[] = {\n";
        for (auto &d : decs) {
            if (d.empty()) sd += "    0,\n";
            else append(sd, "    ", d, ",\n");
        }
        // Not a language the empty array is legal in, and a program may have no object at all.
        if (decs.empty()) sd += "    0,\n";
        sd += "};\n";
    }
};

}  // namespace lobster
