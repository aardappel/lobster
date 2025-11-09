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
    // If the exp type is compile-time equal, this is compile-time true,
    // except for class types that have sub-classes, since we don't know if
    // the runtime type would be equal.
    if ((child->exptype->Equal(*resolvedtype) &&
         (resolvedtype->t != V_CLASS || !resolvedtype->udt->g.has_subclasses)) ||
        resolvedtype->t == V_ANY) {
        val = VTValue(true);
        return V_INT;
    }
    if (!tc->ConvertsTo(resolvedtype, child->exptype, CF_UNIFICATION)) {
        val = VTValue(false);
        return V_INT;
    }
    // This means it is always a reference type, since int/float/function don't convert
    // into anything without coercion.
    assert(IsRefNil(child->exptype->t));
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
    val = VTValue(val.i);
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

ValueType ShiftLeft::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l << r; });
}

ValueType ShiftRight::ConstVal(TypeChecker *tc, VTValue &val) const {
    return BinOpConst<BINOP_INTONLY>(tc, val, this, [](auto l, auto r) { return l >> r; });
}


// Never const.

ValueType PreIncr::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType PreDecr::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Coercion::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType TypeAnnotation::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType List::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Unary::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType BinOp::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Assign::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType PlusEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType MinusEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType MultiplyEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType DivideEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ModEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType AndEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType OrEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType XorEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ShiftLeftEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ShiftRightEq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType DefaultVal::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType TypeOf::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Seq::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Indexing::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType PostIncr::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType PostDecr::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ToString::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Block::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType IfThen::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType IfElse::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType While::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType For::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ForLoopElem::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ForLoopCounter::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Switch::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Case::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Range::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Break::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Continue::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType IdentRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType FreeVarRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType StringConstant::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType EnumRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType GUDTRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType UDTRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType FunRef::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType GenericCall::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType VectorConstructor::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ObjectConstructor::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType AutoConstructor::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Call::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType DynCall::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType NativeCall::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Return::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType MultipleReturn::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType AssignList::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Define::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Member::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Static::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType Dot::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ToLifetime::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

ValueType ToStructSuper::ConstVal(TypeChecker *, VTValue &) const {
    return V_VOID;
}

}  // namespace lobster
