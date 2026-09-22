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

namespace lobster {

// BINOP_WRAP marks an integer operation that wraps (two's complement), which is computed
// unsigned, signed overflow being undefined in C++ as well.
enum { BINOP_DIVMOD = 1, BINOP_CMP = 2, BINOP_INTONLY = 4, BINOP_WRAP = 8 };

template<int FL, typename F> ValueType BinOpConst(TypeCheckBase *tc, VTValue &val, const BinOp *b, F f) {
    VTValue lv;
    VTValue rv;
    auto tl = b->left->ConstVal(tc, lv);
    auto tr = b->right->ConstVal(tc, rv);
    if (tl == V_INT && tr == V_INT) {
        auto li = lv.i;
        auto ri = rv.i;
        if constexpr ((FL & BINOP_DIVMOD) != 0) {
            if (ri <= 0 && ri >= -1 && (!ri || li == LLONG_MIN)) return V_VOID;
        }
        if constexpr ((FL & BINOP_WRAP) != 0) {
            val = VTValue((iint)f((uint64_t)li, (uint64_t)ri));
        } else {
            val = f(li, ri);
        }
        return V_INT;
    }
    if constexpr (!(FL & BINOP_INTONLY)) {
        if (tl == V_FLOAT && tr == V_FLOAT) {
            val = f(lv.f, rv.f);
            if constexpr ((FL & BINOP_CMP) != 0) return V_INT;
            else return V_FLOAT;
        }
    }
    return V_VOID;
}

// A block of one expression is that expression. Inlining leaves these behind once the
// bindings it made for the arguments are gone, and without this the block hides the value from
// everything that folds.
ValueType Block::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    if (children.size() != 1) return V_VOID;
    return children[0]->ConstVal(tc, val);
}

ValueType Nil::ConstVal(TypeCheckBase *, VTValue &val) const {
    val = VTValue();
    return V_NIL;
}

ValueType IntConstant::ConstVal(TypeCheckBase *, VTValue &val) const {
    val = VTValue(integer);
    return V_INT;
}

ValueType FloatConstant::ConstVal(TypeCheckBase *, VTValue &val) const {
    val = VTValue(flt);
    return V_FLOAT;
}

// Truth uses the payload bits, matching Value::True/False even for floats:
// positive zero is false, while negative zero's sign bit makes it true.
ValueType And::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto l = left->ConstVal(tc, val);
    if (l == V_VOID) return V_VOID;
    return !val.i ? l : right->ConstVal(tc, val);
}

ValueType Or::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto l = left->ConstVal(tc, val);
    if (l == V_VOID) return V_VOID;
    return val.i ? l : right->ConstVal(tc, val);
}

ValueType Not::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    val = VTValue(!val.i);
    return V_INT;
}

ValueType UnaryMinus::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    switch (t) {
        // Wraps (the smallest int negates to itself), so unsigned, see BINOP_WRAP.
        case V_INT: val.i = (iint)(0 - (uint64_t)val.i); return V_INT;
        case V_FLOAT: val.f = -val.f; return V_FLOAT;
        default: return V_VOID;
    }
}

ValueType Negate::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t != V_INT) return V_VOID;
    val.i = ~val.i;
    return V_INT;
}

ValueType IsType::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    bool unknown;
    return ConstVal(tc, val, unknown);
}

ValueType IsType::ConstVal(TypeCheckBase *tc, VTValue &val, bool &unknown) const {
    unknown = false;
    if (!tc) {
        // This may be called from the parser, where we do not support this as a constant.
        return V_VOID;
    }
    // NOTE: IsType::TypeCheck lifts out the child if it is a side effect, so
    // we can assume here it doesn't, and thus make the whole exp constant if
    // the type comparison is a constant.
    if (resolvedtype->t == V_ANY) {
        val = VTValue(true);
        return V_INT;
    }
    auto ctype = child->exptype;
    // Nothing is known about a value an error was reported for.
    if (ctype->IsError() || resolvedtype->IsError()) return V_VOID;
    auto ce = ctype->ElementIfNil();
    auto te = resolvedtype->ElementIfNil();
    // A value's runtime type is always its static type or a subtype thereof,
    // so if the tested type is a superclass of (or equal to) the static type,
    // this is compile-time true regardless of what subclasses exist, provided
    // a possibly nil value is also matched by a nilable tested type.
    auto is_static_super = ce->Equal(*te) ||
                           (((ce->t == V_CLASS && te->t == V_CLASS) ||
                             (IsStruct(ce->t) && IsStruct(te->t))) &&
                            SuperDistance(te->udt, ce->udt) >= 0);
    if (is_static_super && (AcceptsNil() || ctype->t != V_NIL)) {
        val = VTValue(true);
        return V_INT;
    }
    // Whether a value of the static type could have the tested type, compared as the non-nil
    // element types, since whether a nil value matches is determined by the tested type alone.
    // Where that depends on what a type variable not bound yet (the element type of a `[]` or
    // `nil`, say) will be, the test can't tell, see TypeChecker::Check(IsType &): binding it
    // here would make the test true by construction.
    bool binds;
    auto converts = tc->CouldConvertTo(te, ce, binds,
                                       resolvedtype->t == V_NIL ? V_NIL : V_UNDEFINED,
                                       ctype->t == V_NIL ? V_NIL : V_UNDEFINED);
    if (converts && binds) {
        unknown = true;
        return V_VOID;
    }
    // Where the tested type can match nothing but the nil a possibly nil value may be, the
    // test is a nil check at run time.
    auto nil_check = AcceptsNil() && ctype->t == V_NIL;
    // Structs have no runtime type, so their static relation decides, except in an abstract
    // struct family, where a value of a supertype may hold the tested type.
    if (IsStruct(ce->t) || IsStruct(te->t)) {
        if (IsStruct(ce->t) && IsStruct(te->t) && ce->udt->family_root &&
            ce->udt->family_root == te->udt->family_root &&
            SuperDistance(ce->udt, te->udt) >= 0) {
            return V_VOID;
        }
        if (nil_check) return V_VOID;
        val = VTValue(false);
        return V_INT;
    }
    // Scalars and function values have no runtime type either, and are of their static type
    // only, which is not the tested type here: an enum converts to `int`, but an `int` is not
    // of an enum type.
    if (IsUnBoxed(ce->t)) {
        val = VTValue(false);
        return V_INT;
    }
    // If no runtime type could ever match the tested type, this is compile-time false.
    if (!converts) {
        if (nil_check) return V_VOID;
        val = VTValue(false);
        return V_INT;
    }
    // Only a reference type is left, whose runtime type decides.
    assert(IsRefNil(ctype->t));
    return V_VOID;
}

ValueType Assert::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    return val.i ? t : V_VOID;
}


ValueType ToFloat::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    assert(t == V_INT);
    val = VTValue((double)val.i);
    return V_FLOAT;
}

ValueType ToInt::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    assert(t == V_FLOAT);
    val = VTValue((iint)val.f);
    return V_INT;
}

ValueType ToBool::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    val = VTValue(!!val.i);
    return V_INT;
}

ValueType EnumCoercion::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return child->ConstVal(tc, val);
}

ValueType Plus::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_WRAP>(tc, val, this, [](auto l, auto r) { return l + r; });
}

ValueType Minus::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_WRAP>(tc, val, this, [](auto l, auto r) { return l - r; });
}

ValueType Multiply::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_WRAP>(tc, val, this, [](auto l, auto r) { return l * r; });
}

ValueType Divide::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_DIVMOD>(tc, val, this, [](auto l, auto r) { return l / r; });
}

ValueType Mod::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    // This is also defined for floats, but since that needs fmod, we for now
    // simply don't constant fold it.
    return BinOpConst<BINOP_INTONLY|BINOP_DIVMOD>(tc, val, this, [](auto l, auto r) { return l % r; });
}

ValueType Equal::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l == r; });
}

ValueType NotEqual::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l != r; });
}

ValueType LessThan::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l < r; });
}

ValueType GreaterThan::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l > r; });
}

ValueType LessThanEq::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l <= r; });
}

ValueType GreaterThanEq::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l >= r; });
}

ValueType BitAnd::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l & r; });
}

ValueType BitOr::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l | r; });
}

ValueType Xor::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l ^ r; });
}

// NOTE: these must mask the shift count exactly like the VM does, see MaskedShiftLeft,
// otherwise a constant folded shift and the same shift computed at runtime would differ.
ValueType ShiftLeft::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) {
        return MaskedShiftLeft(l, r);
    });
}

ValueType ShiftRight::ConstVal(TypeCheckBase *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) {
        return MaskedShiftRight(l, r);
    });
}

}  // namespace lobster
