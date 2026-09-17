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

// Branches, loops, switches, and assertions.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenFlow : virtual CodeGenBase {
    CodeGenFlow() {}

    struct BlockStack {
        size_t &tstack_size;
        size_t start;
        size_t max;
        BlockStack(size_t &s) : tstack_size(s), start(s), max(s) {}
        void Start() { tstack_size = start; }
        void End() { max = std::max(max, tstack_size); }
        void Exit(CodeGenBase &cg) {
            assert(max >= tstack_size);
            // Values from something that does not return.
            while (tstack_size < max) cg.PushTemp();
        }
    };

    // Jump on the value on top of the stack testing false, or on it testing true for the
    // "no fail" version. Small enough to be worth not calling for. defslots is 1 when the value
    // stays on the stack for whoever follows. The jump goes to `lab`, or to a new label.
    // The value the jump keeps as the result can be of another kind than the result, when the
    // typechecker decided only its truth matters, see TypeCheckAndOr, in which case the jump
    // converts it: a false scalar is the nil, and either scalar the other.
    int EmitJumpCond(bool onfail, int defslots, VKind k, VKind resk, int lab = -1) {
        TrackUseDef(1, defslots);
        if (lab < 0) lab = Label();
        auto v = Slot(1, k);
        // Read as the operand of the ! it may get, and otherwise as the whole condition, which
        // needs no parentheses of its own. When the value stays on the stack it is written to
        // its slot first, since that write is what its expression may read, and the test then
        // reads the slot; when it is consumed here the expression is used as is, since the
        // flush only drops it.
        if (defslots) Flush();
        auto e = Operand(v, onfail ? 2 : 15);
        auto test = TruthOperand(v, onfail ? 2 : 15).text;
        if (!defslots) Flush();
        if (k == resk || !defslots) {
            append(cb, "    if (", onfail ? "!" : "", test, ") goto block", lab, ";\n");
            return lab;
        }
        // The test stays the whole condition, while the conversion below takes the value as
        // the operand of a cast.
        auto cond = Parens(e, 2).text;
        string conv;
        switch (resk) {
            case VK_FLOAT: conv = cat("(double)", cond); break;
            case VK_INT: conv = k == VK_FLOAT ? cat("(long long)", cond)
                                              : cat("(long long)(", cond, " != 0)"); break;
            default:
                // Between two kinds of reference only a false one gets kept, but say what it is.
                conv = IsRefKind(k) ? cat("(", CType(resk), ")", cond) : "0";
                break;
        }
        append(cb, "    if (", onfail ? "!" : "", test, ") { ", WriteText(Slot(1, resk), conv),
               " goto block", lab, "; }\n");
        return lab;
    }
    int EmitJumpFail(int defslots, VKind k, VKind resk) {
        return EmitJumpCond(true, defslots, k, resk);
    }
    int EmitJumpNoFail(int defslots, VKind k, VKind resk) {
        return EmitJumpCond(false, defslots, k, resk);
    }
    int EmitJumpFail(int defslots, VKind k) { return EmitJumpCond(true, defslots, k, k); }
    int EmitJumpNoFail(int defslots, VKind k) { return EmitJumpCond(false, defslots, k, k); }

    // A condition that is only jumped on, as that of an if or a while: the logical operators on
    // numeric operands become the jumps directly, rather than a boolean that goes thru a slot
    // to be tested again. Any other condition, including a logical operator whose value is a
    // reference, is generated as a value and tested as a whole.
    void GenCondJump(const Node *n, bool onfail, int lab) {
        auto numeric = [](const Node *c) { return c->exptype->Numeric(); };
        if (auto a = Is<And>(n); a && numeric(a) && numeric(a->left) && numeric(a->right)) {
            if (onfail) {
                GenCondJump(a->left, true, lab);
                GenCondJump(a->right, true, lab);
            } else {
                auto skip = Label();
                GenCondJump(a->left, true, skip);
                GenCondJump(a->right, false, lab);
                EmitLabelDef(skip);
            }
            return;
        }
        if (auto o = Is<Or>(n); o && numeric(o) && numeric(o->left) && numeric(o->right)) {
            if (onfail) {
                auto skip = Label();
                GenCondJump(o->left, false, skip);
                GenCondJump(o->right, true, lab);
                EmitLabelDef(skip);
            } else {
                GenCondJump(o->left, false, lab);
                GenCondJump(o->right, false, lab);
            }
            return;
        }
        if (auto nt = Is<Not>(n); nt && numeric(nt->child)) {
            GenCondJump(nt->child, !onfail, lab);
            return;
        }
        Gen(n, 1);
        TakeTemp(1, false);
        auto k = KindOf(n->exptype);
        EmitJumpCond(onfail, 0, k, k, lab);
    }

    void Generate(const And &node, size_t retval) {
        Gen(node.left, 1);
        TakeTemp(1, false);
        auto lab = EmitJumpFail(!!retval, KindOf(node.left->exptype),
                                KindOf(node.exptype));
        if (retval) GenPopSlot();
        Gen(node.right, retval);
        if (retval) TakeTemp(1, false);
        EmitLabelDef(lab);
    }

    void Generate(const Or &node, size_t retval) {
        Gen(node.left, 1);
        TakeTemp(1, false);
        auto lab = EmitJumpNoFail(!!retval, KindOf(node.left->exptype),
                                  KindOf(node.exptype));
        if (retval) GenPopSlot();
        Gen(node.right, retval);
        if (retval) TakeTemp(1, false);
        EmitLabelDef(lab);
    }

    void Generate(const IfThen &node, size_t retval) {
        auto lab = Label();
        GenCondJump(node.condition, true, lab);
        assert(!retval); (void)retval;
        Gen(node.truepart, 0);
        EmitLabelDef(lab);
    }

    void Generate(const IfElse &node, size_t retval) {
        auto lab = Label();
        GenCondJump(node.condition, true, lab);
        BlockStack bs(tstack_size);
        bs.Start();
        Gen(node.truepart, retval);
        bs.End();
        if (retval) TakeTemp(retval, true);
        auto lab2 = EmitJump();
        EmitLabelDef(lab);
        bs.Start();
        Gen(node.falsepart, retval);
        bs.End();
        if (retval) TakeTemp(retval, true);
        EmitLabelDef(lab2);
        bs.Exit(*this);
    }

    void ApplyBreaks(size_t level) {
        while (breaks.size() > level) {
            EmitLabelDef(breaks.back());
            breaks.pop_back();
        }
    }

    void Generate(const While &node, size_t retval) {
        auto loopback = EmitLabelDefBackwards();
        loops.push_back({ &node, temptypestack.size(), loopback });
        auto jumpout = Label();
        GenCondJump(node.condition, true, jumpout);
        auto break_level = breaks.size();
        Gen(node.wbody, 0);
        loops.pop_back();
        EmitJumpBack(loopback);
        EmitLabelDef(jumpout);
        ApplyBreaks(break_level);
        assert(!retval);
        (void)retval;
    }

    // The C expression for how many times a loop over this value runs, which for a vector or a
    // string comes out of the object itself, see the mirrors of those in Prologue.
    string LenOf(ValueType itertype, const Place &v) {
        switch (itertype) {
            case V_INT:
                return Read(v);
            case V_VECTOR:
            case V_STRING:
                return cat(Read(v), "->len");
            default:
                assert(false);
                return {};
        }
    }

    // The loop condition is an increment and a compare, small enough to be worth not calling for
    // the same reasons as GenScalarBinOp.
    int GenForCond(ValueType itertype) {
        // Reads the counter and the object being iterated, and leaves both for the body.
        TrackUseDef(2, 2);
        auto lab = Label();
        auto len = LenOf(itertype, Slot(1, itertype == V_INT ? VK_INT
                                         : itertype == V_STRING ? VK_STRING : VK_VECTOR));
        auto i = Slot(2, VK_INT);
        Write(cb, i, cat(Read(i), " + 1"));
        append(cb, "    if (!(", Read(i), " < ", len, ")) goto block", lab, ";\n");
        return lab;
    }

    // Both of these copy the loop counter to the top of the stack.
    void GenForCounter(int useslots, int defslots) {
        TrackUseDef(useslots, defslots);
        CopyValue(cb, Slot(0, VK_INT), Slot(2, VK_INT));
    }

    // The element the loop is on, at the counter below the object being iterated. The loop
    // condition already established the counter is in range, so this needs no check.
    // The bitmask says which slots of the element hold a reference the loop now owns.
    void GenForElem(bool isstring, int defslots, int bitmask, TypeRef elemtype) {
        TrackUseDef(2, defslots);
        // Everything but the counter and the object being iterated is the element.
        auto width = defslots - 2;
        auto idx = Read(Slot(2, VK_INT));
        if (isstring) {
            auto str = Read(Slot(1, VK_STRING));
            auto data = cpp ? cat("((unsigned char *)", str, "->data())")
                            : cat("STRING_DATA(", str, ")");
            Write(cb, Slot(0, VK_INT), cat("(long long)", data, "[", idx, "]"));
            return;
        }
        auto elems = cat(Read(Slot(1, VK_VECTOR)), cpp ? "->ElemSlots()" : "->elems");
        // A struct element is the same load per slot it occupies.
        for (int i = 0; i < width; i++) {
            CopyValue(cb, Slot(-i, elemtype, i), Elem(elems, elemtype, cat(idx), i));
        }
        rc_tag = "forelem";
        for (int i = 0; i < width; i++) {
            if ((1 << i) & bitmask) GenIncRef(cb, Slot(-i, elemtype, i));
        }
        rc_tag.clear();
    }

    void Generate(const For &node, size_t retval) {
        EmitPushInt(-1);  // i
        temptypestack.push_back({ type_int, LT_ANY });
        Gen(node.iter, 1);
        auto startloop = EmitLabelDefBackwards();
        loops.push_back({ &node, temptypestack.size(), startloop });
        auto break_level = breaks.size();
        auto tstack_level = tstack_size;
        int exitloop = -1;
        switch (node.iter->exptype->t) {
            case V_INT:      exitloop = GenForCond(V_INT); break;
            case V_STRING:   exitloop = GenForCond(V_STRING); break;
            case V_VECTOR:   exitloop = GenForCond(V_VECTOR); break;
            default:         assert(false);
        }
        Gen(node.fbody, 0);
        EmitJumpBack(startloop);
        EmitLabelDef(exitloop);
        loops.pop_back();
        TakeTemp(2, false);
        assert(tstack_level == tstack_size); (void)tstack_level;
        PopTemp();
        PopTemp();
        ApplyBreaks(break_level);
        assert(!retval);
        (void)retval;
    }

    void Generate(const ForLoopElem &node, size_t /*retval*/) {
        auto typelt = temptypestack.back();
        switch (typelt.type->t) {
            case V_INT:
                GenForCounter(2, 3);
                break;
            case V_STRING:
                GenForElem(true, 3, 0, type_int);
                break;
            case V_VECTOR: {
                auto sub = typelt.type->sub;
                // A single slot element is a reference the loop owns when its type says so, a
                // struct one has a whole bitmask of them. Unless the loop variable borrows the
                // element, see SpecIdent::speculative.
                auto bitmask = !IsRefNil(sub->t)  ? 0
                             : IsStruct(sub->t)   ? BitMaskForRefStruct(sub)
                             : node.sid && IsBorrow(node.sid->lt) ? 0
                                                  : 1;
                GenForElem(false, ValWidth(sub) + 2, bitmask, sub);
                break;
            }
            default:
                assert(false);
        }
    }

    void Generate(const ForLoopCounter & /*node*/, size_t /*retval*/) {
        GenForCounter(1, 2);
    }

    void Generate(const Break & /*node*/, size_t retval) {
        assert(!retval);
        (void)retval;
        assert(!rettypes.size());
        assert(!loops.empty());
        // The loop's own slots have to be the top of the stack for the pops below to name them.
        // Temps underneath are fine: an inlined block can sit in an expression that has temps live
        // across it, and those are still there at the break target, same as on the fall-out path.
        assert(temptypestack.size() == loops.back().temp_level);
        int lab = -1;
        if (Is<For>(loops.back().node)) {
            // The loop's own two slots come off here, but the code after the break still expects
            // them to be there, so put them back once the jump is emitted.
            GenPop(temptypestack[temptypestack.size() - 1]);
            GenPop(temptypestack[temptypestack.size() - 2]);
            lab = EmitJump();
            PushTemp();
            PushTemp();
        } else {
            lab = EmitJump();
        }
        breaks.push_back(lab);
    }

    void Generate(const Continue & /*node*/, size_t retval) {
        assert(!retval);
        (void)retval;
        assert(!rettypes.size());
        assert(!loops.empty());
        // The jump back lands on code generated for this stack depth, so nothing may be left
        // on top of the loop's own slots here either.
        assert(temptypestack.size() == loops.back().temp_level);
        EmitJumpBack(loops.back().continue_label);
    }

    void Generate(const Switch &node, size_t retval) {
        Gen(node.value, 1);
        TakeTemp(1, true);
        // See if we do a type dispatch (always a jump table).
        if (IsDynamicType(node.value->exptype)) {
            GenerateTypeDispatch(node, retval);
            return;
        }
        assert(ValWidth(node.value->exptype) == 1);
        // See if we should do an integer jump table version.
        if (GenerateJumpTable(node, retval))
            return;
        // Do slow default implementation for sparse integers, expressions and strings.
        auto valtlt = TypeLT{ *node.value, 0 };
        vector<int> nextcase, thiscase, exitswitch;
        bool have_default = false;
        PopTemp();
        BlockStack bs(tstack_size);
        for (auto n : node.cases->children) {
            bs.Start();
            PushTemp();
            EmitLabelDefs(nextcase);
            temptypestack.push_back(valtlt);
            auto cas = AssertIs<Case>(n);
            if (cas->pattern->children.empty()) have_default = true;
            for (auto c : cas->pattern->children) {
                auto is_last = c == cas->pattern->children.back();
                GenDup(valtlt);
                int loc = -1;
                auto switchtype = node.value->exptype;
                if (auto r = Is<Range>(c)) {
                    Gen(r->start, 1);
                    GenMathOp(switchtype, c->exptype, switchtype, MOP_GE);
                    loc = EmitJumpFail(0, VK_INT);
                    if (is_last) nextcase.push_back(loc);
                    GenDup(valtlt);
                    Gen(r->end, 1);
                    GenMathOp(switchtype, c->exptype, switchtype, MOP_LE);
                } else {
                    // FIXME: if this is a string, will alloc a temp string object just for the
                    // sake of comparison. Better to create special purpose opcode to compare with
                    // const string.
                    Gen(c, 1);
                    GenMathOp(switchtype, c->exptype, switchtype, MOP_EQ);
                }
                if (is_last) {
                    auto lab = EmitJumpFail(0, VK_INT);
                    nextcase.push_back(lab);
                } else {
                    auto lab = EmitJumpNoFail(0, VK_INT);
                    thiscase.push_back(lab);
                }
                if (Is<Range>(c)) {
                    if (!is_last) EmitLabelDef(loc);
                }
            }
            EmitLabelDefs(thiscase);
            GenPop(valtlt);
            TakeTemp(1, false);
            Generate(*cas, retval);
            bs.End();
            if (n != node.cases->children.back() || !have_default) {
                auto lab = EmitJump();
                exitswitch.push_back(lab);
            }
        }
        EmitLabelDefs(nextcase);
        if (!have_default) {
            bs.Start();
            PushTemp();
            GenPop(valtlt);
            bs.End();
        }
        EmitLabelDefs(exitswitch);
        bs.Exit(*this);
    }

    static pair<IntConstant *, IntConstant *> get_range(Node *c) {
        auto start = c;
        auto end = c;
        if (auto r = Is<Range>(c)) {
            start = r->start;
            end = r->end;
        }
        return { Is<IntConstant>(start), Is<IntConstant>(end) };
    }

    bool GenerateJumpTable(const Switch &node, size_t retval) {
        if (node.value->exptype->t != V_INT)
            return false;
        int64_t mini = INT64_MAX / 2, maxi = INT64_MIN / 2;
        int64_t num = 0;
        // We clamp ranges to this, to cause too big ranges to make load factor sparse.
        const int64_t max_range_weight = 10;
        for (auto n : node.cases->children) {
            auto cas = AssertIs<Case>(n);
            for (auto c : cas->pattern->children) {
                auto [istart, iend] = get_range(c);
                if (!istart || !iend || istart->integer > iend->integer)
                    return false;
                num += std::min(max_range_weight, iend->integer - istart->integer + 1);
                mini = std::min(mini, istart->integer);
                maxi = std::max(maxi, iend->integer);
            }
        }
        // Decide if jump table is economic.
        const int64_t min_vals = 3;     // Minimum to do jump table.
        // TODO: This should be slightly non-linear? More values means you really want the
        // jump table, typically.
        const int64_t min_load_factor = 5;
        int64_t range = maxi - mini + 1;
        if (num < min_vals ||
            range / num > min_load_factor ||
            mini < INT32_MIN ||
            maxi >= INT32_MAX)
            return false;
        // Emit jump table version.
        TrackUseDef(1, 0);
        GenerateJumpTableMain(node, retval, (int)range, (int)mini, (int)maxi);
        return true;
    }

    void GenerateJumpTableMain(const Switch &node, size_t retval, int range, int mini, int maxi,
                               const vector<vector<int>> *case_values = nullptr) {
        auto deflab = Label();
        vector<int> ilab(range + 1, deflab);
        // Figure out labels first, so we can generate code for it all at once.
        vector<int> labels;
        for (auto [i, n] : enumerate(node.cases->children)) {
            auto cas = AssertIs<Case>(n);
            auto lab = Label();
            labels.push_back(lab);
            for (auto c : cas->pattern->children) {
                if (case_values) {
                    ilab[i] = lab;
                } else {
                    auto [istart, iend] = get_range(c);
                    assert(istart && iend);
                    for (auto i = istart->integer; i <= iend->integer; i++) {
                        ilab[i - mini] = lab;
                    }
                }
            }
        }
        // The cases are jump targets, so what is on the stack has to be in its slots.
        string on;
        if (case_values) {
            // The case labels are the type indices themselves, so the value to switch on is the one
            // the object carries, or the type slot of a struct in an abstract struct family, which
            // is the first of its slots, see GenerateTypeDispatch.
            on = IsStruct(node.value->exptype->t)
                ? FamilyIndexText(Slot(ValWidth(node.value->exptype), VK_INT),
                                  *node.value->exptype->udt->family_root)
                : TypeIdOf(Read(Slot(1, VK_OBJECT)));
        } else if (node.vtable_idx >= 0) {
            on = cat("GetTypeSwitchID(vm, ", Read(Slot(1, VK_OBJECT)), ", ", node.vtable_idx,
                     ")");
        } else {
            on = Read(Slot(1, VK_INT));
        }
        Flush();
        if (cpp) {
            append(cb, "    switch (", on, ") {\n");
        } else {
            append(cb, "    { ", node.vtable_idx >= 0 ? "int" : "long long", " top = ", on,
                   "; switch (top) {\n");
        }
        vector<int> exitswitch;
        BlockStack bs(tstack_size);
        for (auto [i, n] : enumerate(node.cases->children)) {
            bs.Start();
            auto cas = AssertIs<Case>(n);
            auto lab = labels[i];
            auto is_last = n == node.cases->children.back();
            // Don't actually emit labels because C code turns these into "case"
            if (cas->pattern->children.empty()) {
                lab = deflab;
            }
            TrackUseDef(0, 0);
            Flush();
            append(cb, "    ");
            if (case_values) {
                for (auto v : (*case_values)[i]) append(cb, "case ", v, ":");
                // A type dispatch has a case for every type the value can have, so the last one
                // takes any other value as well, which is also where a default case sits.
                // Without a default the C compiler has to assume the switch can be skipped, and
                // then sees the slots the cases write to as possibly uninitialized after it.
                assert(lab != deflab || is_last);
                if (is_last) append(cb, "default:");
            } else {
                auto t = ilab.data();
                for (auto v = mini; v <= maxi; v++) {
                    if (*t++ == lab) append(cb, "case ", v, ":");
                }
                if (*t++ == lab) append(cb, "default:");
            }
            // A case body may stay pending until after the switch, so labels need a statement.
            cb += ";\n";
            Generate(*cas, retval);
            bs.End();
            if (!is_last) {
                auto lab = EmitJump();
                exitswitch.push_back(lab);
            }
        }
        TrackUseDef(0, 0);
        if (cpp) cb += "    } // switch\n";
        else cb += "    }} // switch\n";
        EmitLabelDefs(exitswitch);
        bs.Exit(*this);
    }

    void GenerateTypeDispatch(const Switch &node, size_t retval) {
        auto dispatch_udt = node.value->exptype->udt;
        auto de = dispatch_udt->dispatch_table[node.vtable_idx].get();
        assert(de->dispatch_root && !de->sf && de->returntype.Null() &&
               de->subudts_size == dispatch_udt->subudts.size());
        (void)de;
        // The value is borrowed, so its slots are just given up.
        TrackUseDef(ValWidth(node.value->exptype), 0);
        int range = (int)node.cases->children.size();
        // Which case each type the value can have belongs to is known right here, and so is the
        // type index each of those types carries, so the switch tests that index directly. Going
        // thru GetTypeSwitchID instead would be two dependent loads at runtime (the type table for
        // the vtable offset, then the vtable for the case) to arrive at the same constant.
        vector<vector<int>> case_values(range);
        for (auto sub : dispatch_udt->subudts) {
            auto sde = sub->dispatch_table[node.vtable_idx].get();
            if (!sde || sde->case_index < 0) continue;
            assert(sde->case_index < range);
            // A struct in an abstract struct family carries its family index instead, which
            // makes for a dense switch.
            case_values[sde->case_index].push_back(IsStruct(node.value->exptype->t)
                                                       ? sub->FamilyIndex()
                                                       : (int)GetTypeTableOffset(&sub->thistype));
        }
        GenerateJumpTableMain(node, retval, range, 0, range - 1, &case_values);
    }

    void Generate(const Case &node, size_t retval) {
        if (node.cbody->Arity()) {
            Gen(node.cbody, retval);
            if (retval) TakeTemp(1, true);
        } else {
            // An empty default case signals runtime error for enums. An `out_of_range` case takes
            // the exact same slot, but has a user block to run instead, so it is never empty.
            assert(node.pattern->children.empty() && !node.out_of_range);
            // FIXME: would be great to ensure the offending value is still on the stack for
            // this instruction to have access to.
            TrackUseDef(0, 0);
            append(cb, "    RtEnumRangeErr(vm);\n");
        }
    }

    // All that is left of an assert in the common case is the test; the reporting is a call,
    // given the text of the condition as one of the string constants, see EmitConstantStrings.
    void EmitAssert(int defslots, int line, int fileidx, int text, VKind k) {
        TrackUseDef(1, defslots);
        auto test = TruthOperand(Slot(1, k), 2).text;
        append(cb, "    if (!", test, ") ",
               cpp ? "vm.AssertFailed(" : "AssertFailed(vm, ",
               line, ", ", fileidx, ", (LString *)&str", text, ");\n");
    }

    void Generate(const Assert &node, size_t retval) {
        if (retval || runtime_checks >= RUNTIME_ASSERT) {
            Gen(node.child, 1);
            TakeTemp(1, false);
            if (runtime_checks >= RUNTIME_ASSERT) {
                EmitAssert(!!retval, node.child->line.line, node.child->line.fileidx,
                           ConstantString(node.text), KindOf(node.child->exptype));
            }
        } else {
            Gen(node.child, 0);
        }
    }
};

}  // namespace lobster
