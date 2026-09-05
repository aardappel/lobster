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

// The builtins the generated code writes out itself rather than calling, one emitter each plus
// the EmitCodegenBuiltin that picks between them, see BuiltinCodegen in natreg.h. This is
// included into the body of CodeGen in codegen.h, since all of it are members of it that lean
// on the emission helpers there.
//
// Each emitter reads the arguments out of the slots below regso and leaves the return values in
// the same ones, just as a call would. Two things they may not do: read a slot more than once
// without going thru Reused, since a slot may hold a whole expression rather than a value, and
// write a slot from inside a branch, since a write flushes whatever expressions are pending and
// those then only happen on one path.

    // The vector one of the builtins below works on, in a local, since the slot it comes in is
    // also where the value the builtin leaves behind goes. `cmt` names the builtin.
    string EmitVectorLocal(int base, string_view cmt) {
        cb += "    {";
        comment(cmt);
        append(cb, "    LVector *_v = ", Read(SlotVar(base, RTT_VECTOR)), ";\n");
        return cpp ? "_v->ElemSlots()" : "_v->elems";
    }

    // The element at `idx` of that vector, in the slots starting at `base`. A struct takes more
    // than one load, at the width the vector holds its elements at, so its start goes in a
    // local first.
    void EmitVectorElem(const Types &elem, int base, string_view idx, string_view elems,
                        TypeRef elemtype) {
        for (int i = 0; i < (int)elem.size(); i++) {
            CopyValue(cb, SlotVar(base + i, elem[i]), Elem(elems, elemtype, idx, i));
        }
    }

    // A push of one element onto a vector, which the code writes out rather than calling
    // push(), see EmitCodegenBuiltin. The vector stays in the slot it came in, which is where
    // the value push returns belongs anyway.
    void EmitVectorPush(const Types &args, string_view cmt, TypeRef elemtype) {
        auto width = (int)args.size() - 1;
        auto base = regso - (int)args.size();
        auto elems = EmitVectorLocal(base, cmt);
        append(cb, "    if (_v->len == _v->maxl) ", cpp ? "lobster::" : "",
               "RtVectorGrow(vm, _v);\n");
        for (int i = 0; i < width; i++) {
            CopyValue(cb, Elem(elems, elemtype, "_v->len", i), SlotVar(base + 1 + i, args[1 + i]));
        }
        cb += "    _v->len++;\n    }\n";
    }

    // insert(), which makes room for the element at the index it goes at and then writes it
    // there. The vector stays in the slot it came in, which is what it returns.
    void EmitVectorInsert(NativeFun *nf, const Types &args, TypeRef elemtype) {
        auto width = (int)args.size() - 2;
        auto base = regso - (int)args.size();
        auto elems = EmitVectorLocal(base, nf->name);
        append(cb, "    long long _i = ", Read(SlotVar(base + 1, RTT_INT)), ";\n");
        append(cb, "    ", cpp ? "lobster::" : "", "RtVectorInsert(vm, _v, ", nf->idx,
               ", _i);\n");
        for (int i = 0; i < width; i++) {
            CopyValue(cb, Elem(elems, elemtype, "_i", i), SlotVar(base + 2 + i, args[2 + i]));
        }
        cb += "    }\n";
    }

    // The last element of a vector, which pop() also takes out of it where top() leaves it.
    void EmitVectorPop(NativeFun *nf, const Types &rets, bool take, TypeRef elemtype) {
        auto base = regso - 1;
        auto elems = EmitVectorLocal(base, nf->name);
        append(cb, "    if (!_v->len) ", cpp ? "lobster::" : "", "RtVectorEmptyErr(vm, ",
               nf->idx, ");\n");
        if (take) cb += "    _v->len--;\n";
        EmitVectorElem(rets, base, take ? "_v->len" : "_v->len - 1", elems, elemtype);
        cb += "    }\n";
    }

    // The element of a vector at an index, which the ones behind it then shift down over.
    void EmitVectorRemove(NativeFun *nf, const Types &rets, TypeRef elemtype) {
        auto base = regso - 2;
        auto uint = cpp ? "uint64_t" : "unsigned long long";
        auto elems = EmitVectorLocal(base, nf->name);
        append(cb, "    long long _i = ", Read(SlotVar(base + 1, RTT_INT)), ";\n");
        append(cb, "    if ((", uint, ")_i >= (", uint, ")_v->len) ", cpp ? "lobster::" : "",
               "RtVectorIdxErr(vm, ", nf->idx, ", _i, _v->len);\n");
        EmitVectorElem(rets, base, "_i", elems, elemtype);
        append(cb, "    ", cpp ? "lobster::" : "", "RtVectorErase(_v, _i);\n    }\n");
    }

    // vector_capacity(), which only has anything to do when the vector holds less than it is
    // asked for. It stays in the slot it came in, which is what it returns.
    void EmitVectorCapacity(const Types &args) {
        MaterializeArgs(args);
        auto base = regso - 2;
        auto v = Read(SlotVar(base, args[0]));
        auto n = Read(SlotVar(base + 1, args[1]));
        append(cb, "    if (", v, "->maxl < ", n, ") ", cpp ? "lobster::" : "",
               "RtVectorResize(vm, ", v, ", ", n, ");");
        comment("vector_capacity");
    }

    // Below here are the builtins that are arithmetic on a number or on a numeric struct, which
    // takes one slot per element. The width one of them works at is thus how many slots its
    // arguments take over how many arguments it has, and a scalar overload is the same code at
    // width 1.

    // A slot as an operand, which is the expression it has pending where it has one.
    Expr Term(const Place &p) { return Operand(p, 15); }

    // A constant, and the zero of a kind, which several of these compare against.
    static Expr Lit(string s) { return { std::move(s), {}, true, 0 }; }
    static Expr Zero(VKind k) { return Lit(k == VK_FLOAT ? "0.0" : "0"); }

    // An operator between two expressions, each in parentheses where it binds looser than the
    // operator, see Parens.
    static Expr Op(int prec, string_view op, Expr a, Expr b) {
        Parens(a, prec);
        Parens(b, prec, true);
        return Combine(prec, cat(a.text, " ", op, " ", b.text), a, b);
    }

    // The conditional operator, which most of these are made of: C has no min, max or abs of
    // its own that does not go thru libc, which the JIT links none of.
    static Expr Cond(Expr c, const Expr &a, const Expr &b) {
        Parens(c, 12);
        Expr e = { cat(c.text, " ? ", a.text, " : ", b.text), c.vars,
                   c.pure && a.pure && b.pure, 13 };
        e.vars.insert(e.vars.end(), a.vars.begin(), a.vars.end());
        e.vars.insert(e.vars.end(), b.vars.begin(), b.vars.end());
        return e;
    }

    // A conversion to the C type of a kind.
    Expr Cast(VKind k, Expr e) {
        Parens(e, 2);
        return { cat("(", CType(k), ")", e.text), e.vars, e.pure, 2 };
    }

    // A call to a runtime helper. Never pure, whatever it calls: an expression that may be
    // deferred is one that may also end up written twice, which for a call means calling it
    // twice, see WriteExpr.
    Expr RtCall(string_view name, Expr a) {
        return { cat(cpp ? "lobster::" : "", name, "(", a.text, ")"), a.vars, false, 1 };
    }

    // The elements of a value combined left to right, the way the geom.h versions fold them.
    static void Fold(Expr &acc, int j, int prec, string_view op, Expr e) {
        acc = j ? Op(prec, op, std::move(acc), std::move(e)) : std::move(e);
    }

    // A slot the expression built from it names more than once, which one holding a pending
    // expression cannot be: that is written to its variable first, so every read is of the
    // variable.
    Place Reused(int slot, VKind k) {
        auto p = SlotVar(slot, k);
        if (HasPending(p.slot)) Materialize(pending[p.slot]);
        return p;
    }

    // The same for every argument at once, which the emitters that open a block of their own
    // do before they open it, since a pending expression is written where it is met.
    void MaterializeArgs(const Types &args) {
        auto base = regso - (int)args.size();
        for (auto [j, a] : enumerate(args)) Reused(base + (int)j, Kind(a));
    }

    // Whether a slot holds the constant zero, which the bias of an in_range() that was left out
    // is: the typechecker fills that in as a struct of zeroes.
    bool IsZero(int slot) {
        if (!HasPending(slot)) return false;
        auto &e = pending[slot].expr;
        return e == "0" || e == "0.0";
    }

    // The absolute value of what a slot holds. Subtracting from zero rather than negating,
    // and taking the zero itself down that path, is what makes this fabs() exactly: a -0.0
    // comes back as a 0.0 the way clearing the sign bit gives, where negating would leave it.
    Expr Abs(const Place &v) {
        return Cond(Op(6, "<=", Term(v), Zero(v.k())), Op(4, "-", Zero(v.k()), Term(v)),
                    Term(v));
    }

    // abs().
    void EmitAbs(const Types &args, const Types &rets) {
        auto base = regso - (int)args.size();
        for (int j = 0; j < (int)args.size(); j++) {
            auto v = Reused(base + j, Kind(args[j]));
            WriteExpr(SlotVar(base + j, rets[j]), Abs(v));
        }
    }

    // sign(): the two comparisons signum() is, which give an int whatever they compared.
    void EmitSign(const Types &args, const Types &rets) {
        auto base = regso - (int)args.size();
        for (int j = 0; j < (int)args.size(); j++) {
            auto v = Reused(base + j, Kind(args[j]));
            WriteExpr(SlotVar(base + j, rets[j]),
                      Op(4, "-", Op(6, "<", Zero(v.k()), Term(v)),
                                 Op(6, "<", Term(v), Zero(v.k()))));
        }
    }

    // min() and max() of two values, comparing the way std::min and std::max do, which decides
    // which of two that are equal comes out.
    void EmitMinMax(const Types &args, const Types &rets, bool ismax) {
        auto width = (int)args.size() / 2;
        auto base = regso - (int)args.size();
        for (int j = 0; j < width; j++) {
            auto a = Reused(base + j, Kind(args[j]));
            auto b = Reused(base + width + j, Kind(args[width + j]));
            auto c = ismax ? Op(6, "<", Term(a), Term(b)) : Op(6, "<", Term(b), Term(a));
            WriteExpr(SlotVar(base + j, rets[j]), Cond(std::move(c), Term(b), Term(a)));
        }
    }

    // clamp(), which is a min of a max, so what the max gives goes in a local rather than being
    // computed twice.
    void EmitClamp(const Types &args, const Types &rets) {
        MaterializeArgs(args);
        auto width = (int)args.size() / 3;
        auto base = regso - (int)args.size();
        auto k = Kind(args[0]);
        auto m = Var("_c", k);
        append(cb, "    { ", CType(k), " _c;");
        comment("clamp");
        for (int j = 0; j < width; j++) {
            auto v = SlotVar(base + j, k);
            auto lo = SlotVar(base + width + j, k);
            auto hi = SlotVar(base + width * 2 + j, k);
            Write(cb, m, Cond(Op(6, "<", Term(lo), Term(v)), Term(v), Term(lo)).text);
            Write(cb, SlotVar(base + j, rets[j]),
                  Cond(Op(6, "<", Term(m), Term(hi)), Term(m), Term(hi)).text);
        }
        cb += "    }\n";
    }

    // in_range(), which is two comparisons per element. A bias that was left out is zero, and
    // then drops out of both rather than being added to the range.
    void EmitInRange(const Types &args, const Types &rets) {
        auto width = (int)args.size() / 3;
        auto base = regso - (int)args.size();
        auto k = Kind(args[0]);
        Expr e = {};
        for (int j = 0; j < width; j++) {
            auto x = Reused(base + j, k);
            auto range = SlotVar(base + width + j, k);
            auto zero = IsZero(base + width * 2 + j);
            auto bias = zero ? Zero(k) : Term(Reused(base + width * 2 + j, k));
            auto lo = bias;
            auto hi = zero ? Term(range) : Op(4, "+", std::move(bias), Term(range));
            Fold(e, j, 11, "&&", Op(11, "&&", Op(6, ">=", Term(x), std::move(lo)),
                                              Op(6, "<", Term(x), std::move(hi))));
        }
        WriteExpr(SlotVar(base, rets[0]), std::move(e));
    }

    // dot(), the sum of the products of the elements of two values. It starts from the zero
    // the C++ one starts from, which is what decides the sign of a zero the products add up
    // to; the ones below all start from their first element, where that makes no difference.
    void EmitDot(const Types &args, const Types &rets) {
        auto width = (int)args.size() / 2;
        auto base = regso - (int)args.size();
        auto e = Zero(Kind(rets[0]));
        for (int j = 0; j < width; j++) {
            e = Op(4, "+", std::move(e),
                   Op(3, "*", Term(SlotVar(base + j, Kind(args[j]))),
                              Term(SlotVar(base + width + j, Kind(args[width + j])))));
        }
        WriteExpr(SlotVar(base, rets[0]), std::move(e));
    }

    // The sum of the squares of the elements, which is what magnitude() and normalize() are
    // built on as well. Every element is read twice, so none of them may be pending.
    Expr SquaredLength(const Types &args, int base, int width) {
        Expr e = {};
        for (int j = 0; j < width; j++) {
            auto v = Reused(base + j, Kind(args[j]));
            Fold(e, j, 4, "+", Op(3, "*", Term(v), Term(v)));
        }
        return e;
    }

    // magnitude() and magnitude_squared(). The square root is a helper rather than the C one,
    // since the JIT links no libc; the C++ backend inlines it back into an instruction.
    void EmitMagnitude(const Types &args, const Types &rets, bool root) {
        auto width = (int)args.size();
        auto base = regso - width;
        auto e = SquaredLength(args, base, width);
        WriteExpr(SlotVar(base, rets[0]), root ? RtCall("RtSqrt", std::move(e)) : std::move(e));
    }

    // normalize(), which divides every element by the length, or is all zeroes where there is
    // none. The length goes in a local, since every element divides by it.
    void EmitNormalize(const Types &args, const Types &rets) {
        auto width = (int)args.size();
        auto base = regso - width;
        auto len = RtCall("RtSqrt", SquaredLength(args, base, width)).text;
        append(cb, "    { ", CType(VK_FLOAT), " _l = ", len, ";");
        comment("normalize");
        auto l = Var("_l", VK_FLOAT);
        for (int j = 0; j < width; j++) {
            Write(cb, SlotVar(base + j, rets[j]),
                  Cond(Op(7, "==", Term(l), Zero(VK_FLOAT)), Zero(VK_FLOAT),
                       Op(3, "/", Term(SlotVar(base + j, Kind(args[j]))), Term(l))).text);
        }
        cb += "    }\n";
    }

    // manhattan(), the sum of the absolute values of the elements, and volume(), the product of
    // them.
    void EmitManhattan(const Types &args, const Types &rets) {
        auto width = (int)args.size();
        auto base = regso - width;
        Expr e = {};
        for (int j = 0; j < width; j++) {
            auto v = Reused(base + j, Kind(args[j]));
            Fold(e, j, 4, "+", Abs(v));
        }
        WriteExpr(SlotVar(base, rets[0]), std::move(e));
    }

    void EmitVolume(const Types &args, const Types &rets) {
        auto width = (int)args.size();
        auto base = regso - width;
        Expr e = {};
        for (int j = 0; j < width; j++) {
            Fold(e, j, 3, "*", Term(SlotVar(base + j, Kind(args[j]))));
        }
        WriteExpr(SlotVar(base, rets[0]), std::move(e));
    }

    // The smallest or largest element of a value, folded in a local since every step compares
    // against what the ones before it gave.
    void EmitMinMaxOf(const Types &args, const Types &rets, bool ismax) {
        MaterializeArgs(args);
        auto width = (int)args.size();
        auto base = regso - width;
        auto k = Kind(args[0]);
        auto m = Var("_m", k);
        append(cb, "    { ", CType(k), " _m = ", Read(SlotVar(base, k)), ";");
        comment(ismax ? "max" : "min");
        for (int j = 1; j < width; j++) {
            auto v = SlotVar(base + j, k);
            auto c = ismax ? Op(6, "<", Term(v), Term(m)) : Op(6, "<", Term(m), Term(v));
            Write(cb, m, Cond(std::move(c), Term(m), Term(v)).text);
        }
        Write(cb, SlotVar(base, rets[0]), Read(m));
        cb += "    }\n";
    }

    // cross(), whose three results each read elements of both operands, so all of them are
    // computed before any goes into the slots the first operand came in.
    void EmitCross(const Types &args, const Types &rets) {
        MaterializeArgs(args);
        auto base = regso - 6;
        // The element of each operand the result at that index is the product of.
        static const int lhs[] = { 1, 2, 0 }, rhs[] = { 2, 0, 1 };
        cb += "    {";
        comment("cross");
        for (int j = 0; j < 3; j++) {
            auto a = [&](int i) { return Term(SlotVar(base + i, Kind(args[i]))); };
            auto b = [&](int i) { return Term(SlotVar(base + 3 + i, Kind(args[3 + i]))); };
            append(cb, "    ", CType(VK_FLOAT), " _c", j, " = ",
                   Op(4, "-", Op(3, "*", a(lhs[j]), b(rhs[j])),
                              Op(3, "*", a(rhs[j]), b(lhs[j]))).text, ";\n");
        }
        for (int j = 0; j < 3; j++) Write(cb, SlotVar(base + j, rets[j]), cat("_c", j));
        cb += "    }\n";
    }

    // int() and float(), which are the C conversions between the two.
    void EmitConvert(const Types &args, const Types &rets) {
        auto base = regso - (int)args.size();
        for (int j = 0; j < (int)args.size(); j++) {
            WriteExpr(SlotVar(base + j, rets[j]),
                      Cast(Kind(rets[j]), Term(SlotVar(base + j, Kind(args[j])))));
        }
    }

    // floor() and ceiling(), which are the truncation C does plus the step it is off by for a
    // value on the wrong side of zero. The truncated value lands in the slot the result goes
    // in, which is a variable of its own from the one the value came in.
    void EmitFloorCeiling(const Types &args, const Types &rets, bool up) {
        auto base = regso - (int)args.size();
        for (int j = 0; j < (int)args.size(); j++) {
            auto v = Reused(base + j, Kind(args[j]));
            auto d = SlotVar(base + j, rets[j]);
            Write(cb, d, Cast(Kind(rets[j]), Term(v)).text, "");
            comment(up ? "ceiling" : "floor");
            Write(cb, d, Op(4, up ? "+" : "-", Term(d),
                            Op(6, up ? ">" : "<", Term(v), Term(d))).text);
        }
    }

    // round(), which is the truncation of the value shifted half a step away from zero.
    void EmitRound(const Types &args, const Types &rets) {
        auto base = regso - (int)args.size();
        for (int j = 0; j < (int)args.size(); j++) {
            auto v = Reused(base + j, Kind(args[j]));
            auto half = Op(4, "-", Cast(VK_FLOAT, Op(6, ">=", Term(v), Zero(VK_INT))),
                                   Lit("0.5"));
            WriteExpr(SlotVar(base + j, rets[j]),
                      Cast(Kind(rets[j]), Op(4, "+", Term(v), std::move(half))));
        }
    }

    // sqrt(), which the JIT gains nothing by writing out, since it calls the same helper the
    // builtin would have; the C++ backend turns it into the instruction it is.
    void EmitSqrt(const Types &args, const Types &rets) {
        auto base = regso - 1;
        WriteExpr(SlotVar(base, rets[0]), RtCall("RtSqrt", Term(SlotVar(base, Kind(args[0])))));
    }

    // div(), which divides two ints as floats.
    void EmitDiv(const Types &args, const Types &rets) {
        auto base = regso - 2;
        WriteExpr(SlotVar(base, rets[0]),
                  Op(3, "/", Cast(VK_FLOAT, Term(SlotVar(base, Kind(args[0])))),
                             Cast(VK_FLOAT, Term(SlotVar(base + 1, Kind(args[1]))))));
    }

    // length(): the field of the string or vector, which is not an expression that can wait,
    // since a push may change it. The int overload is the identity function, so it leaves the
    // value in the slot it is already in.
    void EmitLength(const Types &args, const Types &rets) {
        auto base = regso - 1;
        if (Kind(args[0]) == VK_INT) return;
        Write(cb, SlotVar(base, rets[0]), cat(Read(SlotVar(base, args[0])), "->len"), "");
        comment("length");
    }

    // type_id(): the type the header of a reference carries, which the two backends name
    // differently since the C one has a mirror of that header of its own.
    void EmitTypeId(const Types &args, const Types &rets) {
        auto base = regso - 1;
        auto ref = ReadAs(SlotVar(base, args[0]), VK_REF);
        Write(cb, SlotVar(base, rets[0]),
              cat("((", CType(VK_REF), ")", ref, ")->", cpp ? "tti" : "typeinfo"), "");
        comment("type_id");
    }

    // vm_compiled_mode(): which of the two backends this is, which is decided right here.
    void EmitCompiledMode(const Types &rets) {
        Write(cb, SlotVar(regso, rets[0]), cpp ? "1" : "0", "");
        comment("vm_compiled_mode");
    }

    // The builtins the code does not call but writes out itself, because they are leaned on
    // often enough for the call to be worth avoiding, see BuiltinCodegen. Each reads its
    // arguments from the slots below regso and leaves its return values in the same ones, just
    // as a call would. Returns whether this was one of them.
    bool EmitCodegenBuiltin(NativeFun *nf, const Types &args, const Types &rets,
                            TypeRef elemtype) {
        // No default, so that a kind added without a case here is a compile error.
        switch (nf->codegen) {
            case BCG_NONE:
                return false;
            case BCG_GL_FRAME:
                // Called by a symbol of its own, which the engine defines outside the registry.
                Write(cb, Slot(0, VK_INT), "GLFrame(vm)", "");
                comment(nf->name);
                return true;
            case BCG_PUSH:
                EmitVectorPush(args, nf->name, elemtype);
                return true;
            case BCG_INSERT:
                EmitVectorInsert(nf, args, elemtype);
                return true;
            case BCG_POP:
                EmitVectorPop(nf, rets, true, elemtype);
                return true;
            case BCG_TOP:
                EmitVectorPop(nf, rets, false, elemtype);
                return true;
            case BCG_REMOVE:
                EmitVectorRemove(nf, rets, elemtype);
                return true;
            case BCG_VECTOR_CAPACITY:
                EmitVectorCapacity(args);
                return true;
            case BCG_ABS:
                EmitAbs(args, rets);
                return true;
            case BCG_SIGN:
                EmitSign(args, rets);
                return true;
            case BCG_MIN:
                EmitMinMax(args, rets, false);
                return true;
            case BCG_MAX:
                EmitMinMax(args, rets, true);
                return true;
            case BCG_CLAMP:
                EmitClamp(args, rets);
                return true;
            case BCG_IN_RANGE:
                EmitInRange(args, rets);
                return true;
            case BCG_DOT:
                EmitDot(args, rets);
                return true;
            case BCG_MAGNITUDE:
                EmitMagnitude(args, rets, true);
                return true;
            case BCG_MAGNITUDE_SQUARED:
                EmitMagnitude(args, rets, false);
                return true;
            case BCG_NORMALIZE:
                EmitNormalize(args, rets);
                return true;
            case BCG_MANHATTAN:
                EmitManhattan(args, rets);
                return true;
            case BCG_VOLUME:
                EmitVolume(args, rets);
                return true;
            case BCG_MIN_OF:
                EmitMinMaxOf(args, rets, false);
                return true;
            case BCG_MAX_OF:
                EmitMinMaxOf(args, rets, true);
                return true;
            case BCG_CROSS:
                EmitCross(args, rets);
                return true;
            case BCG_CONVERT:
                EmitConvert(args, rets);
                return true;
            case BCG_FLOOR:
                EmitFloorCeiling(args, rets, false);
                return true;
            case BCG_CEILING:
                EmitFloorCeiling(args, rets, true);
                return true;
            case BCG_ROUND:
                EmitRound(args, rets);
                return true;
            case BCG_SQRT:
                EmitSqrt(args, rets);
                return true;
            case BCG_DIV:
                EmitDiv(args, rets);
                return true;
            case BCG_LENGTH:
                EmitLength(args, rets);
                return true;
            case BCG_TYPE_ID:
                EmitTypeId(args, rets);
                return true;
            case BCG_VM_COMPILED_MODE:
                EmitCompiledMode(rets);
                return true;
            case BCG_PASS:
                // Does nothing, which is best done by emitting nothing at all.
                return true;
        }
        assert(false);
        return false;
    }
