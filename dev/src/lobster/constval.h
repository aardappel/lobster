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

enum { BINOP_DIVMOD = 1, BINOP_CMP = 2, BINOP_INTONLY = 4 };

template<int FL, typename F> ValueType BinOpConst(TypeChecker *tc, VTValue &val, const BinOp *b, F f) {
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
        val = f(li, ri);
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

ValueType Nil::ConstVal(TypeChecker *, VTValue &val) const {
    val = VTValue();
    return V_NIL;
}

ValueType IntConstant::ConstVal(TypeChecker *, VTValue &val) const {
    val = VTValue(integer);
    return V_INT;
}

ValueType FloatConstant::ConstVal(TypeChecker *, VTValue &val) const {
    val = VTValue(flt);
    return V_FLOAT;
}

ValueType And::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto l = left->ConstVal(tc, val);
    if (l == V_VOID) return V_VOID;
    return !val.i ? l : right->ConstVal(tc, val);
}

ValueType Or::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto l = left->ConstVal(tc, val);
    if (l == V_VOID) return V_VOID;
    return val.i ? l : right->ConstVal(tc, val);
}

ValueType Not::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    val = VTValue(!val.i);
    return V_INT;
}

ValueType UnaryMinus::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    switch (t) {
        case V_INT: val.i = -val.i; return V_INT;
        case V_FLOAT: val.f = -val.f; return V_FLOAT;
        default: return V_VOID;
    }
}

ValueType Negate::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t != V_INT) return V_VOID;
    val.i = ~val.i;
    return V_INT;
}

ValueType IsType::ConstVal(TypeChecker *tc, VTValue &val) const {
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
    if (is_static_super && (resolvedtype->t == V_NIL || ctype->t != V_NIL)) {
        val = VTValue(true);
        return V_INT;
    }
    // Structs have no runtime type, so their static relation decides.
    if (IsStruct(ce->t) || IsStruct(te->t)) {
        val = VTValue(false);
        return V_INT;
    }
    // If no runtime type could ever match the tested type, this is
    // compile-time false. Tested against the non-nil element types, since
    // whether a nil value matches is determined by the tested type alone.
    if (!tc->ConvertsTo(te, ce, CF_UNIFICATION)) {
        val = VTValue(false);
        return V_INT;
    }
    // This means it is always a reference type, since int/float/function don't convert
    // into anything without coercion.
    assert(IsRefNil(ctype->t));
    return V_VOID;
}

ValueType Assert::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    return val.i ? t : V_VOID;
}


ValueType ToFloat::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    assert(t == V_INT);
    val = VTValue((double)val.i);
    return V_FLOAT;
}

ValueType ToInt::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    assert(t == V_FLOAT);
    val = VTValue((iint)val.f);
    return V_INT;
}

ValueType ToBool::ConstVal(TypeChecker *tc, VTValue &val) const {
    auto t = child->ConstVal(tc, val);
    if (t == V_VOID) return t;
    val = VTValue(!!val.i);
    return V_INT;
}

ValueType EnumCoercion::ConstVal(TypeChecker *tc, VTValue &val) const {
    return child->ConstVal(tc, val);
}

ValueType Plus::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<0>(tc, val, this, [](auto l, auto r) { return l + r; });
}

ValueType Minus::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<0>(tc, val, this, [](auto l, auto r) { return l - r; });
}

ValueType Multiply::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<0>(tc, val, this, [](auto l, auto r) { return l * r; });
}

ValueType Divide::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_DIVMOD>(tc, val, this, [](auto l, auto r) { return l / r; });
}

ValueType Mod::ConstVal(TypeChecker *tc, VTValue &val) const {
    // This is also defined for floats, but since that needs fmod, we for now
    // simply don't constant fold it.
    return BinOpConst<BINOP_INTONLY|BINOP_DIVMOD>(tc, val, this, [](auto l, auto r) { return l % r; });
}

ValueType Equal::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l == r; });
}

ValueType NotEqual::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l != r; });
}

ValueType LessThan::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l < r; });
}

ValueType GreaterThan::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l > r; });
}

ValueType LessThanEq::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l <= r; });
}

ValueType GreaterThanEq::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_CMP>(tc, val, this, [](auto l, auto r) { return l >= r; });
}

ValueType BitAnd::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l & r; });
}

ValueType BitOr::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l | r; });
}

ValueType Xor::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l ^ r; });
}

// NOTE: these must mask the shift count exactly like the VM does, see MaskedShiftLeft,
// otherwise a constant folded shift and the same shift computed at runtime would differ.
ValueType ShiftLeft::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) {
        return MaskedShiftLeft(l, r);
    });
}

ValueType ShiftRight::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) {
        return MaskedShiftRight(l, r);
    });
}

}  // namespace lobster
