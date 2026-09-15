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

// Arithmetic, comparison, bitwise, and modifying operators.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckOps : virtual TypeCheckLocations {
    TypeCheckOps() {}

    bool MathCheckVector(TypeRef &type, Node *&left, Node *&right, bool flipped = false) {
        TypeRef ltype = left->exptype;
        TypeRef rtype = right->exptype;
        // Special purpose check for vector * scalar etc.
        if (ltype->t == V_STRUCT_S && rtype->Numeric()) {
            auto etype = ltype->udt->sametype;
            if (etype->Numeric()) {
                if (etype->t == V_INT) {
                    // Don't implicitly convert int vectors to float.
                    if (rtype->t == V_FLOAT) return false;
                } else {
                    if (rtype->t == V_INT) SubType(right, type_float, "right", *right);
                }
                type = &ltype->udt->thistype;
                return true;
            }
        }
        if (!flipped) { return MathCheckVector(type, right, left, true); }
        return false;
    }

    const char *MathCheck(TypeRef &type, BinOp &n, bool &unionchecked,
                          bool typechangeallowed) {
        if (type->Numeric())
            return nullptr;
        if (IsStruct(type->t) && type->udt->sametype->Numeric())
            return nullptr;
        if (MathCheckVector(type, n.left, n.right)) {
            unionchecked = true;
            return nullptr;
        }
        if (!Is<Plus>(&n) && !Is<PlusEq>(&n))
            return "numeric/struct";
        // Special purpose checking for + on strings.
        auto ltype = n.left->exptype;
        auto rtype = n.right->exptype;
        if (ltype->t == V_STRING) {
            if (rtype->t != V_STRING) {
                // Anything can be added to a string on the right (because of +=).
                MakeString(n.right, LT_BORROW);
                // Make sure the overal type is string.
                type = type_string;
                unionchecked = true;
            }
            return nullptr;
        } else if (rtype->t == V_STRING && ltype->t != V_STRING && typechangeallowed) {
            // Only if not in a +=
            MakeString(n.left, LT_BORROW);
            type = type_string;
            unionchecked = true;
            return nullptr;
        } else {
            return "numeric/string/struct";
        }
    }

    void MathError(TypeRef &type, BinOp &n, bool &unionchecked, bool typechangeallowed) {
        auto err = MathCheck(type, n, unionchecked, typechangeallowed);
        if (err) {
            // Blame the side that can't take part in this operation at all, if there is one.
            if (MathCheck(n.left->exptype, n, unionchecked, typechangeallowed)) {
                RequiresError(err, n.left->exptype, n, "left", "", n.left);
            } else if (MathCheck(n.right->exptype, n, unionchecked, typechangeallowed)) {
                RequiresError(err, n.right->exptype, n, "right", "", n.right);
            } else {
                Error(n, "can\'t use ", Q(NiceName(n)), " on ", Q(TypeName(n.left->exptype)),
                         " and ", Q(TypeName(n.right->exptype)));
            }
            type = type_error;
        }
    }

    // The type the operands of an arithmetic or comparison operator are brought to. Enum
    // operands take part as plain ints, whatever enums they are, and the result is one.
    TypeRef MathOperandType(BinOp &n) {
        auto ltype = n.left->exptype;
        auto rtype = n.right->exptype;
        if (ltype->t == V_INT && rtype->t == V_INT) return type_int;
        return Union(ltype, rtype, "lhs", "rhs", CF_COERCIONS, nullptr);
    }

    Node *TypeCheckMathOp(BinOp &n) {
        if (auto nn = OperatorOverload(n)) return nn;
        TT(n.right, 1, LT_BORROW);
        n.exptype = MathOperandType(n);
        bool unionchecked = false;
        MathError(n.exptype, n, unionchecked, true);
        if (!unionchecked) SubTypeLR(n.exptype, n);
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_KEEP;
        return &n;
    }

    Node *TypeCheckMathOpEqBit(BinOp &n) {
        auto nn = TypeCheckBitOp(n);
        if (&n == nn) CheckLval(n.left);
        return nn;
    }

    Node *TypeCheckMathOpEq(BinOp &n) {
        if (auto nn = OperatorOverload(n)) return nn;
        if (auto idr = Is<IdentRef>(n.left)) FlipSpeculative(idr->sid);
        DecBorrowers(n.left->lt, n);
        TT(n.right, 1, LT_BORROW);
        CheckLval(n.left);
        n.exptype = n.left->exptype;
        if (!MathCheckVector(n.exptype, n.left, n.right)) {
            bool unionchecked = false;
            MathError(n.exptype, n, unionchecked, false);
            if (!unionchecked) SubType(n.right, n.exptype, "right", n);
        }
        // This really does: "left = left op right" the result of op is LT_KEEP, which is
        // implicit, so the left var must be LT_KEEP as well. This is ensured elsewhere because
        // all !single_assignment vars are LT_KEEP.
        assert(!Is<IdentRef>(n.left) || LifetimeType(Is<IdentRef>(n.left)->sid->lt) != LT_BORROW);
        DecBorrowers(n.right->lt, n);
        n.lt = PushBorrow(n.left);
        return &n;
    }

    void StructCompResult(BinOp &n, TypeRef u) {
        auto nfields = u->udt->sfields.size();
        n.exptype = st.GetVectorType(V_INT, 0, (int)nfields);
        if (n.exptype.Null()) {
            n.exptype = type_error;
            Error(n, "no suitable struct of int type of size ", nfields, " known");
        }
    }

    Node *TypeCheckComp(BinOp &n) {
        if (auto nn = OperatorOverload(n)) return nn;
        TT(n.right, 1, LT_BORROW);
        n.exptype = &st.default_bool_type->thistype;
        auto u = MathOperandType(n);
        if (!u->Numeric() && u->t != V_STRING) {
            if (Is<Equal>(&n) || Is<NotEqual>(&n)) {
                // Comparison with one result, but still by value for structs.
                if (u->t != V_VECTOR && !IsUDT(u->t) && u->t != V_NIL &&
                    u->t != V_FUNCTION && u->t != V_RESOURCE && u->t != V_TYPEID) {
                    RequiresError(TypeName(n.left->exptype), n.right->exptype, n,
                                  "right-hand side");
                    u = type_error;
                }
            } else {
                // Comparison vector op: vector inputs, vector out.
                if (u->t == V_STRUCT_S && u->udt->sametype->Numeric()) {
                    StructCompResult(n, u);
                } else if (MathCheckVector(n.exptype, n.left, n.right)) {
                    StructCompResult(n, n.exptype);
                    // Don't do SubTypeLR since type already verified and `u` not
                    // appropriate anyway.
                    goto out;
                } else {
                    Error(n, Q(n.Name()), " doesn\'t work on ", Q(TypeName(n.left->exptype)),
                             " and ", Q(TypeName(n.right->exptype)));
                    u = type_error;
                }
            }
        }
        SubTypeLR(u, n);
        out:
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_KEEP;
        return &n;
    }

    Node *TypeCheckBitOp(BinOp &n) {
        if (auto nn = OperatorOverload(n)) return nn;
        TT(n.right, 1, LT_BORROW);
        auto u = Union(n.left->exptype, n.right->exptype, "lhs", "rhs", CF_COERCIONS, nullptr);
        // Enum operands take part as ints; only `& | ^` on two values of the same enum_flags
        // type keep that type.
        auto is_shift = Is<ShiftLeft>(&n) || Is<ShiftRight>(&n) || Is<ShiftLeftEq>(&n) ||
                        Is<ShiftRightEq>(&n);
        if (u->t != V_INT || (u->e && (!u->e->flags || is_shift))) u = type_int;
        SubTypeLR(u, n);
        n.exptype = u;
        DecBorrowers(n.left->lt, n);
        DecBorrowers(n.right->lt, n);
        n.lt = LT_ANY;
        return &n;
    }

    Node *TypeCheckPlusPlus(Unary &n) {
        if (auto nn = OperatorOverload(n)) return nn;
        CheckLval(n.child);
        n.exptype = n.child->exptype;
        if (!n.exptype->Numeric())
            RequiresError("numeric", n.exptype, n);
        n.lt = n.child->lt;
        return &n;
    }

    Node *Check(Plus &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOp(node);
    }

    Node *Check(Minus &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOp(node);
    }

    Node *Check(Multiply &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOp(node);
    }

    Node *Check(Divide &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOp(node);
    }

    Node *Check(Mod &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOp(node);
    }

    Node *Check(PlusEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEq(node);
    }

    Node *Check(MultiplyEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEq(node);
    }

    Node *Check(MinusEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEq(node);
    }

    Node *Check(DivideEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEq(node);
    }

    Node *Check(ModEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEq(node);
    }

    Node *Check(AndEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEqBit(node);
    }

    Node *Check(OrEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEqBit(node);
    }

    Node *Check(XorEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEqBit(node);
    }

    Node *Check(ShiftLeftEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEqBit(node);
    }

    Node *Check(ShiftRightEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckMathOpEqBit(node);
    }

    Node *Check(NotEqual &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(Equal &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(GreaterThanEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(LessThanEq &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(GreaterThan &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(LessThan &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckComp(node);
    }

    Node *Check(Not &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (auto nn = OperatorOverload(node)) return nn;
        DecBorrowers(node.child->lt, node);
        NoStruct(*node.child, "not");
        node.exptype = &st.default_bool_type->thistype;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(BitAnd &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckBitOp(node);
    }

    Node *Check(BitOr &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckBitOp(node);
    }

    Node *Check(Xor &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckBitOp(node);
    }

    Node *Check(ShiftLeft &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckBitOp(node);
    }

    Node *Check(ShiftRight &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckBitOp(node);
    }

    Node *Check(Negate &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (auto nn = OperatorOverload(node)) return nn;
        SubType(node.child, type_int, "negated value", node);
        DecBorrowers(node.child->lt, node);
        // Only an enum_flags value keeps its type, any other enum becomes a plain int.
        auto ctype = node.child->exptype;
        node.exptype = ctype->t == V_INT && ctype->e && !ctype->e->flags ? type_int : ctype;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(PostDecr &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckPlusPlus(node);
    }

    Node *Check(PostIncr &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckPlusPlus(node);
    }

    Node *Check(PreDecr &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckPlusPlus(node);
    }

    Node *Check(PreIncr &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        return TypeCheckPlusPlus(node);
    }

    Node *Check(UnaryMinus &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (auto nn = OperatorOverload(node)) return nn;
        node.exptype = node.child->exptype;
        if (!node.exptype->Numeric() &&
            (node.exptype->t != V_STRUCT_S || !node.exptype->udt->sametype->Numeric()))
            RequiresError("numeric / numeric struct", node.exptype, node);
        // An enum negates as a plain int.
        if (node.exptype->t == V_INT && node.exptype->e) node.exptype = type_int;
        DecBorrowers(node.child->lt, node);
        node.lt = LT_KEEP;
        return &node;
    }
};

TypeRef UnaryMinus::SimpleType(SymbolTable &st) {
    auto type = child->SimpleType(st);
    // An enum negates as a plain int, see UnaryMinus::TypeCheck.
    if (!type.Null() && type->t == V_INT && type->e) return type_int;
    return type;
}

TypeRef BinOp::SimpleType(SymbolTable &st) {
    auto ltype = left->SimpleType(st);
    if (ltype.Null()) return nullptr;
    // A string on the left of + makes the right side get converted to
    // string whatever it is, so the type doesn't depend on it.
    if (Is<Plus>(this) && ltype->t == V_STRING) return type_string;
    auto rtype = right->SimpleType(st);
    if (rtype.Null()) return nullptr;
    // Only plain scalar/string operands: anything else can involve operator
    // overloads, vector math, enum unions, etc.
    auto num = [](TypeRef t) { return t->t == V_FLOAT || (t->t == V_INT && !t->e); };
    if (Is<Plus>(this) || Is<Minus>(this) || Is<Multiply>(this) || Is<Divide>(this) ||
        Is<Mod>(this)) {
        if (Is<Plus>(this) && rtype->t == V_STRING) return type_string;
        if (!num(ltype) || !num(rtype)) return nullptr;
        return ltype->t == V_FLOAT || rtype->t == V_FLOAT ? type_float : type_int;
    }
    if (Is<BitAnd>(this) || Is<BitOr>(this) || Is<Xor>(this) || Is<ShiftLeft>(this) ||
        Is<ShiftRight>(this)) {
        return ltype->t == V_INT && !ltype->e && rtype->t == V_INT && !rtype->e ? type_int
                                                                   : TypeRef(nullptr);
    }
    // Qualified, since unqualified Equal is the Node member function here.
    if (Is<lobster::Equal>(this) || Is<NotEqual>(this) || Is<LessThan>(this) ||
        Is<GreaterThan>(this) || Is<LessThanEq>(this) || Is<GreaterThanEq>(this)) {
        if (!st.default_bool_type) return nullptr;
        if ((num(ltype) && num(rtype)) || (ltype->t == V_STRING && rtype->t == V_STRING))
            return &st.default_bool_type->thistype;
    }
    return nullptr;
}

}  // namespace lobster
