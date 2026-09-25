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

// Arithmetic, comparison, bitwise and string operators, and the conversions between types.
// Shared state and emission helpers come from CodeGenBase in codegen_base.h.

namespace lobster {

struct CodeGenOps : virtual CodeGenBase {
    CodeGenOps() {}

    // The declared type of the field slot i of a struct belongs to (int for the type slot and
    // for fields packed into part of a slot), which for a struct in an abstract struct family
    // is more specific than the kind SlotTypeOf gives the slot, and decides what comparing
    // the slot means, see GenStructCompare.
    static TypeRef DeclaredSlotType(const UDT &udt, int i) {
        auto sfield = FindSlot(udt, i);
        return sfield->bits ? type_int : sfield->type;
    }

    // Calling a helper for something this small costs more than the work itself, and pushes
    // both operands and the result thru memory where the compiler could otherwise keep them in
    // registers, so emit the operator directly instead, see BinExpr. Takes two operands off
    // the stack and leaves the result.
    void GenScalarBinOp(bool isfloat, MathOp op, bool divisor_safe = false) {
        TrackUseDef(2, 1);
        auto k = ScalarKind(isfloat);
        WriteExpr(Slot(2, BinKind(isfloat, op)),
                  BinExpr(isfloat, op, Slot(2, k), Slot(1, k), divisor_safe));
    }

    // Comparing two structs is a compare per slot, joined by && (or || for a !=) into one
    // expression like a scalar compare is, so it can stay pending and a condition can jump on
    // it directly. A slot compares the way the == of its declared field type does: a string
    // (or nilable string) by contents, everything else (numbers, the type slot of a struct in
    // an abstract struct family, and other references, which compare by identity) with ==.
    void GenStructCompare(bool eq, TypeRef type) {
        auto len = ValWidth(type);
        auto prec = eq ? 11 : 12;
        Expr acc;
        for (int j = 0; j < len; j++) {
            auto lp = Slot(len * 2 - j, type, j);
            auto rp = Slot(len - j, type, j);
            auto ftype = DeclaredSlotType(*type->udt, j);
            Expr e;
            if (ftype->t == V_STRING || (ftype->t == V_NIL && ftype->sub->t == V_STRING)) {
                // The slot may hold the string as another kind of reference (a field of a
                // struct in an abstract struct family, see SlotTypeOf), which is a cast.
                auto l = Operand(lp, 2), r = Operand(rp, 2);
                if (lp.k() != VK_STRING) l.text = cat("(", CType(VK_STRING), ")", l.text);
                if (rp.k() != VK_STRING) r.text = cat("(", CType(VK_STRING), ")", r.text);
                auto op = eq ? MOP_EQ : MOP_NE;
                e = Combine(1, cat(MathOpName(ftype->t == V_NIL ? "Sn" : "S", op), "(", l.text,
                                   ", ", r.text, ")"), l, r);
                // A call is not worth (and this one not safe) evaluating twice, see WriteExpr.
                e.pure = false;
            } else {
                auto l = Operand(lp, 7);
                auto r = Operand(rp, 7, true);
                e = Combine(7, cat(l.text, eq ? " == " : " != ", r.text), l, r);
            }
            if (!j) {
                acc = e;
            } else {
                Parens(acc, prec);
                Parens(e, prec, true);
                acc = Combine(prec, cat(acc.text, eq ? " && " : " || ", e.text), acc, e);
            }
        }
        // Only written after all the reads, since the result lands in the first slot of the left
        // hand side.
        WriteExpr(Slot(len * 2, VK_INT), acc);
    }

    // The type specialized helpers below come one per MathOp, in that order, so the name of each
    // is the prefix for the types it works on followed by the name of the operator. The C++
    // backend spells out the namespace, since two nil constants give it no argument to find
    // the helper thru.
    string MathOpName(string_view prefix, MathOp op) {
        static const char *ops[] = { "Add", "Sub", "Mul", "Div", "Mod",
                                     "Lt", "Gt", "Le", "Ge", "Eq", "Ne" };
        return cat(cpp ? "lobster::" : "", "Rt", prefix, ops[op]);
    }

    void GenMathOp(TypeRef ltype, TypeRef rtype, TypeRef ptype, MathOp op,
                   bool divisor_safe = false) override {
        TakeTemp(2, true);
        // Have to check right and left because comparison ops generate ints for node
        // overall.
        if (rtype->t == V_INT && ltype->t == V_INT) {
            GenScalarBinOp(false, op, divisor_safe);
        } else if (rtype->t == V_FLOAT && ltype->t == V_FLOAT) {
            GenScalarBinOp(true, op);
        } else if (rtype->t == V_STRING && ltype->t == V_STRING) {
            // Only comparisons get here, concatenation has its own path. Nillable version
            // handled below.
            assert(op >= MOP_LT);
            TrackUseDef(2, 1);
            Write(cb, Slot(2, VK_INT), cat(MathOpName("S", op), "(", Read(Slot(2, VK_STRING)), ", ",
                                          Read(Slot(1, VK_STRING)), ")"));
        } else if ((rtype->t == V_FUNCTION && ltype->t == V_FUNCTION)) {
            assert(op == MOP_EQ || op == MOP_NE);
            TrackUseDef(2, 1);
            Write(cb, Slot(2, VK_INT), cat(Read(Slot(2, VK_FUN)), op == MOP_EQ ? " == " : " != ",
                                          Read(Slot(1, VK_FUN))));
        } else if ((rtype->t == V_TYPEID && ltype->t == V_TYPEID)) {
            assert(op == MOP_EQ || op == MOP_NE);
            GenScalarBinOp(false, op);
        } else if (op >= MOP_EQ) {
            if (IsStruct(ltype->t)) {
                // Comparing two structs is one compare per slot, so this never becomes a
                // call at all.
                TrackUseDef(ValWidth(ltype) * 2, 1);
                GenStructCompare(op == MOP_EQ, ltype);
            } else {
                assert(IsRefNil(ltype->t) && IsRefNil(rtype->t));
                TrackUseDef(2, 1);
                if ((ltype->t == V_NIL && ltype->sub->t == V_STRING) ||
                    (rtype->t == V_NIL && rtype->sub->t == V_STRING)) {
                    Write(cb, Slot(2, VK_INT), cat(MathOpName("Sn", op), "(", Read(Slot(2, VK_STRING)),
                                                  ", ", Read(Slot(1, VK_STRING)), ")"));
                } else {
                    // References compare by identity, as whatever kind the left one is.
                    auto l = Slot(2, ltype);
                    Write(cb, Slot(2, VK_INT), cat(Read(l), op == MOP_EQ ? " == " : " != ",
                                                  ReadAs(Slot(1, rtype), l.k())));
                }
            }
        } else {
            bool leftisvec = ltype->t == V_STRUCT_S;
            // If this is a comparison op, be sure to use the child type.
            TypeRef vectype = op >= MOP_LT ? (leftisvec ? ltype : rtype) : ptype;
            assert(vectype->t == V_STRUCT_S);
            auto sub = vectype->udt->sametype;
            bool withscalar = IsScalar(rtype->t) || IsScalar(ltype->t);
            auto outw = ValWidth(ptype);
            auto inw = withscalar ? outw + 1 : outw * 2;
            auto width = ValWidth(vectype);
            assert(sub->t == V_INT || sub->t == V_FLOAT);
            auto isfloat = sub->t == V_FLOAT;
            auto k = ScalarKind(isfloat);
            auto rk = BinKind(isfloat, op);
            TrackUseDef(inw, outw);
            // The same operator once per slot of the struct, with the results landing where
            // the left operand was.
            if (!withscalar) {
                for (int j = 0; j < width; j++) {
                    WriteExpr(Slot(width * 2 - j, rk),
                              BinExpr(isfloat, op, Slot(width * 2 - j, k), Slot(width - j, k)));
                }
            } else if (leftisvec) {
                // The one struct shape where the divisor is the scalar the proof is about; in
                // the other two it is a component of a struct, which is never a constant.
                for (int j = 0; j < width; j++) {
                    WriteExpr(Slot(width + 1 - j, rk),
                              BinExpr(isfloat, op, Slot(width + 1 - j, k), Slot(1, k),
                                      divisor_safe));
                }
            } else {
                // The scalar sits below the struct, in the slot the first result lands in, so
                // it is read into a local first, which the results are computed from where they
                // are since that local does not outlive the block.
                append(cb, "    { ", CType(k), " _s = ", Read(Slot(width + 1, k)), ";\n");
                auto scalar = Var("_s", k);
                for (int j = 0; j < width; j++) {
                    auto e = BinExpr(isfloat, op, scalar, Slot(width - j, k));
                    Write(cb, Slot(width + 1 - j, rk), e.text);
                }
                cb += "    }\n";
            }
        }
    }

    // The longest constant still worth comparing a byte at a time rather than by calling the
    // helper, which does the same compare but has to load the length of both sides first.
    static const size_t max_inline_string_compare = 16;

    // A string compared against a string constant. The length of the constant is known here and
    // is nowhere in the generated code, since the table of them is filled when the VM starts,
    // so neither backend can reduce the call to the length test that already decides almost
    // every one of these. Written out as that test followed by a compare of the bytes, which
    // leaves nothing to call. Returns whether it emitted the comparison.
    bool GenStringConstCompare(const BinOp *n, MathOp op) {
        // A nilable string keeps the helper that gives nil an answer of its own.
        if (n->left->exptype->t != V_STRING || n->right->exptype->t != V_STRING) return false;
        auto lc = Is<StringConstant>(n->left);
        auto rc = Is<StringConstant>(n->right);
        // Two constants would have been folded, so exactly one of them is the case here.
        if (!lc == !rc) return false;
        auto &str = (lc ? lc : rc)->str;
        if (str.size() > max_inline_string_compare) return false;
        Gen(lc ? n->right : n->left, 1);
        TakeTemp(1, false);
        TrackUseDef(1, 1);
        // The string goes into a local first: the slot it comes in may hold a whole expression,
        // and the comparison names it once per byte. The empty constant is decided by the
        // length alone, so it gets no pointer to bytes it never reads.
        append(cb, "    { LString *_s = ", Read(Slot(1, VK_STRING)), ";");
        if (!str.empty())
            append(cb, " const unsigned char *_d = ",
                   cpp ? "(const unsigned char *)_s->data()" : "STRING_DATA(_s)", ";");
        string q;
        EscapeAndQuote(str, q, true);
        comment(q);
        auto e = cat("_s->len == ", str.size());
        for (size_t i = 0; i < str.size(); i++) {
            auto c = (uint8_t)str[i];
            // A printable byte reads as the character it is; the rest as their value.
            auto lit = c >= 0x20 && c < 0x7f && c != '\'' && c != '\\'
                ? cat("'", string(1, (char)c), "'")
                : cat((int)c);
            append(e, " && _d[", i, "] == ", lit);
        }
        if (op == MOP_NE) e = cat("!(", e, ")");
        Write(cb, Slot(1, VK_INT), e);
        cb += "    }\n";
        return true;
    }

    void GenMathOp(const BinOp *n, size_t retval, MathOp op) {
        if (retval && (op == MOP_EQ || op == MOP_NE) && GenStringConstCompare(n, op)) return;
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            // The final tree already contains folded constants. Derive this here so cloning
            // and substitution need no separate proof to preserve or invalidate. DivCheck
            // can only fail on zero or minus one.
            auto divisor = op == MOP_DIV || op == MOP_MOD ? Is<IntConstant>(n->right) : nullptr;
            auto divisor_safe = divisor && divisor->integer != 0 && divisor->integer != -1;
            GenMathOp(n->left->exptype, n->right->exptype, n->exptype, op, divisor_safe);
        }
    }

    void GenConcatOp(const BinOp *n, size_t retval) {
        // Exception to the code below, since we want to generate an efficient concatenation
        // of any number of strings.
        node_small_vector strs;
        strs.push_back(n->left);
        strs.push_back(n->right);
        for (;;) {
            auto c = strs[0];
            if (auto lt = Is<ToLifetime>(c)) {
                assert(lt->decref == 1 && lt->incref == 0);
                c = lt->child;
            }
            auto p = Is<Plus>(c);
            if (p && p->left->exptype->t == V_STRING && p->right->exptype->t == V_STRING) {
                strs.erase(0);
                strs.insert(0, p->right);
                strs.insert(0, p->left);
            } else {
                break;
            }
        }
        // TODO: we can even detect any ToString nodes here and generate an even more efficient
        // call that does I2S etc inline with even fewer allocations.
        for (auto s : strs) {
            Gen(s, retval);
            TakeTemp(retval, false);
        }
        if (!retval) return;
        auto nstrs = (int)strs.size();
        TrackUseDef(nstrs, 1);
        auto d = SlotVar(regso - nstrs, RTT_STRING);
        if (nstrs == 2) {
            Write(cb, d, cat("RtSAdd(vm, ", Read(Slot(2, VK_STRING)), ", ", Read(Slot(1, VK_STRING)),
                             ")"));
        } else {
            append(cb, "    {\n    LString *_cs[", nstrs, "];\n");
            for (int i = 0; i < nstrs; i++) {
                append(cb, "    _cs[", i, "] = ",
                       Read(SlotVar(regso - nstrs + i, RTT_STRING)), ";\n");
            }
            Write(cb, d, cat("RtStrConcatN(vm, _cs, ", nstrs, ")"));
            cb += "    }\n";
        }
    }

    void Generate(const Plus &node, size_t retval) {
        if (node.left->exptype->t == V_STRING && node.right->exptype->t == V_STRING) {
            GenConcatOp(&node, retval);
        } else {
            GenMathOp(&node, retval, MOP_ADD);
        }
    }

    void Generate(const Minus &node, size_t retval) {
        GenMathOp(&node, retval, MOP_SUB);
    }

    void Generate(const Multiply &node, size_t retval) {
        GenMathOp(&node, retval, MOP_MUL);
    }

    void Generate(const Divide &node, size_t retval) {
        GenMathOp(&node, retval, MOP_DIV);
    }

    void Generate(const Mod &node, size_t retval) {
        GenMathOp(&node, retval, MOP_MOD);
    }

    void Generate(const NotEqual &node, size_t retval) {
        GenMathOp(&node, retval, MOP_NE);
    }

    void Generate(const Equal &node, size_t retval) {
        GenMathOp(&node, retval, MOP_EQ);
    }

    void Generate(const GreaterThanEq &node, size_t retval) {
        GenMathOp(&node, retval, MOP_GE);
    }

    void Generate(const LessThanEq &node, size_t retval) {
        GenMathOp(&node, retval, MOP_LE);
    }

    void Generate(const GreaterThan &node, size_t retval) {
        GenMathOp(&node, retval, MOP_GT);
    }

    void Generate(const LessThan &node, size_t retval) {
        GenMathOp(&node, retval, MOP_LT);
    }

    void GenBitOp(const BinOp *n, size_t retval, BitOp op) {
        Gen(n->left, retval);
        Gen(n->right, retval);
        if (retval) {
            TakeTemp(2, false);
            TrackUseDef(2, 1);
            WriteExpr(Slot(2, VK_INT), BitExpr(op, Slot(2, VK_INT), Slot(1, VK_INT)));
        }
    }

    void Generate(const BitAnd &node, size_t retval) {
        GenBitOp(&node, retval, BIT_AND);
    }

    void Generate(const BitOr &node, size_t retval) {
        GenBitOp(&node, retval, BIT_OR);
    }

    void Generate(const Xor &node, size_t retval) {
        GenBitOp(&node, retval, BIT_XOR);
    }

    void Generate(const ShiftLeft &node, size_t retval) {
        GenBitOp(&node, retval, BIT_ASL);
    }

    void Generate(const ShiftRight &node, size_t retval) {
        GenBitOp(&node, retval, BIT_ASR);
    }

    void Generate(const UnaryMinus &node, size_t retval) {
        Gen(node.child, retval);
        if (!retval) return;
        TakeTemp(1, true);
        auto ctype = node.child->exptype;
        assert(ctype->t == V_INT || ctype->t == V_FLOAT || ctype->t == V_STRUCT_S);
        // The same negation per slot, of whatever kind each is.
        auto width = ValWidth(ctype);
        TrackUseDef(width, width);
        for (int i = 0; i < width; i++) {
            auto v = Slot(width - i, ctype, i);
            WriteExpr(v, v.k() == VK_INT ? WrappingNegate(v) : Unary("-", v));
        }
    }

    void Generate(const Negate &node, size_t retval) {
        Gen(node.child, retval);
        if (!retval) return;
        TakeTemp(1, false);
        TrackUseDef(1, 1);
        auto v = Slot(1, VK_INT);
        WriteExpr(v, Unary("~", v));
    }

    // The ones below are a move or a test on the stack and nothing else. Calling a helper for
    // one of those costs more than the work itself, and pushes its operand and result thru
    // memory where the compiler could otherwise keep them in a register.

    // Turning a reference into a bool can drop it first: what is left only gets tested against
    // nil, which does not need the value alive.
    void EmitBoolTest(string_view test, bool decref, VKind k) {
        TrackUseDef(1, 1);
        auto v = Slot(1, k);
        rc_tag = "booltest";
        if (decref) GenDecRef(cb, v);
        rc_tag.clear();
        auto e = TruthOperand(v, 7);
        e.text = cat(e.text, " ", test);
        e.prec = 7;
        WriteExpr(Slot(1, VK_INT), e);
    }

    void EmitIntToFloat() {
        TrackUseDef(1, 1);
        auto e = Operand(Slot(1, VK_INT), 2);
        e.text = "(double)" + e.text;
        e.prec = 2;
        WriteExpr(Slot(1, VK_FLOAT), e);
    }

    void Generate(const Not &node, size_t retval) {
        Gen(node.child, retval);
        if (retval) {
            TakeTemp(1, false);
            EmitBoolTest("== 0", false, KindOf(node.child->exptype));
        }
    }

    void Generate(const ToBool &node, size_t retval) {
        Gen(node.child, retval);
        if (!retval) return;
        TakeTemp(1, false);
        EmitBoolTest("!= 0", ShouldDec(TypeLT(*node.child, 0)), KindOf(node.child->exptype));
    }

    void Generate(const ToFloat &node, size_t retval) {
        Gen(node.child, retval);
        if (!retval) return;
        TakeTemp(1, false);
        EmitIntToFloat();
    }

    void Generate(const ToInt &node, size_t retval) {
        Gen(node.child, retval);
        // No actual opcode needed, this node is purely to store correct types.
        if (retval) TakeTemp(1, false);
    }

    // The operands of a helper that works on a run of values, in an array of its own that it
    // gets a pointer to, since the slots they come from are variables. Declares the array, so
    // whoever calls this opens a block for it and closes it after the call.
    void StageValues(string_view arr, const Types &ts) {
        auto first = regso - (int)ts.size();
        append(cb, "    {\n    Value ", arr, "[", ts.size(), "];\n");
        for (auto [i, t] : enumerate(ts)) {
            CopyValue(cb, Mem(cat(arr, "[", i, "]"), t), SlotVar(first + (int)i, t));
        }
    }

    // The fields of a struct are of the types they are, so the one helper that takes all of
    // them at once gets them as the Values the slots of everything else in memory hold.
    void EmitStructToString(int type_idx, const Types &args, TypeRef type) {
        TrackUseDef((int)args.size(), 1);
        StageValues("_ss", args);
        Write(cb, SlotVar(regso - (int)args.size(), RTT_STRING),
              cat("RtStructToString(vm, _ss, (type_elem_t)", type_idx, ")"), "");
        TypeComment(type);
        cb += "    }\n";
    }

    void Generate(const ToString &node, size_t retval) {
        Gen(node.child, retval);
        if (!retval) return;
        TakeTemp(1, true);
        switch (node.child->exptype->t) {
            case V_STRUCT_R:
            case V_STRUCT_S: {
                // TODO: can also roll these into A2S?
                EmitStructToString(GetTypeTableOffset(node.child->exptype),
                                   TypesOf(node.child->exptype, 1), node.child->exptype);
                break;
            }
            default: {
                auto ti = (int)GetTypeTableOffset(node.child->exptype->ElementIfNil());
                TrackUseDef(1, 1);
                auto p = Slot(1, node.child->exptype);
                Write(cb, SlotVar(regso - 1, RTT_STRING),
                      cat("Rt", KindName(p.k()), "ToString(vm, ", ReadTyped(p),
                          ", (type_elem_t)", ti, ")"));
                break;
            }
        }
    }
};

}  // namespace lobster
