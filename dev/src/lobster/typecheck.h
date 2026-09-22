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

#include "lobster/typecheck_base.h"
#include "lobster/typecheck_locations.h"
#include "lobster/typecheck_functions.h"
#include "lobster/typecheck_calls.h"
#include "lobster/typecheck_constructors.h"
#include "lobster/typecheck_flow.h"
#include "lobster/typecheck_lval.h"
#include "lobster/typecheck_ops.h"
#include "lobster/typecheck_builtin.h"
#include "lobster/typecheck_query.h"

namespace lobster {

// Each shared virtual operation has one implementation in its owning topic.
// MSVC warns about that intentional dominance through the virtual base.
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4250)
#endif

// Assemble the independent topics. TT and its result/lifetime handling live in
// the base, what is known about variables and fields in TypeCheckLocations; each
// topic keeps its node checks beside the helpers they use.
struct TypeChecker final : TypeCheckFunctions, TypeCheckCalls, TypeCheckConstructors,
                           TypeCheckFlow, TypeCheckLval, TypeCheckOps, TypeCheckBuiltin,
                           TypeCheckQuery {
    using TypeCheckFunctions::Check;
    using TypeCheckCalls::Check;
    using TypeCheckConstructors::Check;
    using TypeCheckFlow::Check;
    using TypeCheckLval::Check;
    using TypeCheckOps::Check;
    using TypeCheckBuiltin::Check;

    TypeChecker(Parser &_p, SymbolTable &_st, const CompileOptions &opts)
        : TypeCheckBase(*this, _p, _st, opts) {
        st.type_check_call_back = [&](UDT &udt) {
            EnsureUDTChecked(udt, *scopes.back().call_context);
        };
        // Type resolution runs on behalf of whatever is being typechecked, so give its
        // errors the same call stack the typechecker's own get (see ErrorAlways).
        st.error_context_call_back = [&](string &err) {
            if (full_error || parser.lex.num_errors == 0) AddStackTrace(err);
        };
        // FIXME: this is unfriendly.
        if (!st.RegisterDefaultTypes()) {
            // Nothing can be typechecked without them; Compile stops on the error.
            Error(*parser.root, "cannot find standard types (from stdtype.lobster)");
            return;
        }
        size_t retreq = opts.return_value;
        AssertIs<Call>(parser.root)->sf->reqret = retreq;
        TT(parser.root, retreq, LT_KEEP);
        CleanUpFlow(0);
        assert(borrowstack.empty());
        assert(scopes.empty());
        TypeCheckDeadCode();
        #ifndef NDEBUG
            // The error type only ever stands in for something an error was reported for,
            // since the passes after this one, which run only when there were none (see
            // Compile), can't work with it. Not in what was checked only as dead code,
            // which may have it silently, see SkipDeadCode.
            if (!st.lex.num_errors) {
                for (auto sf : st.subfunctiontable) {
                    if (!sf->sbody || !sf->typechecked) continue;
                    sf->sbody->Iterate([](Node *n) {
                        assert(n->exptype.Null() || !n->exptype->IsError());
                    });
                }
                for (auto sid : st.specidents) {
                    assert(sid->type.Null() || !sid->type->IsError());
                }
            }
        #endif
    }
};

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// The existing AST virtual methods forward to the appropriate topic's overload.
// This is the only code that needs to see all topics together.
#define TYPECHECK_NODE(T) \
    Node *T::TypeCheck(TypeChecker &tc, size_t reqret, TypeRef parent_bound) { \
        return tc.Check(*this, reqret, parent_bound); \
    }

TYPECHECK_NODE(Block)
TYPECHECK_NODE(Or)
TYPECHECK_NODE(And)
TYPECHECK_NODE(IfThen)
TYPECHECK_NODE(IfElse)
TYPECHECK_NODE(While)
TYPECHECK_NODE(For)
TYPECHECK_NODE(ForLoopElem)
TYPECHECK_NODE(ForLoopCounter)
TYPECHECK_NODE(Break)
TYPECHECK_NODE(Continue)
TYPECHECK_NODE(Switch)
TYPECHECK_NODE(Case)
TYPECHECK_NODE(Range)
TYPECHECK_NODE(Define)
TYPECHECK_NODE(Member)
TYPECHECK_NODE(Static)
TYPECHECK_NODE(AssignList)
TYPECHECK_NODE(IntConstant)
TYPECHECK_NODE(FloatConstant)
TYPECHECK_NODE(StringConstant)
TYPECHECK_NODE(Nil)
TYPECHECK_NODE(Plus)
TYPECHECK_NODE(Minus)
TYPECHECK_NODE(Multiply)
TYPECHECK_NODE(Divide)
TYPECHECK_NODE(Mod)
TYPECHECK_NODE(PlusEq)
TYPECHECK_NODE(MultiplyEq)
TYPECHECK_NODE(MinusEq)
TYPECHECK_NODE(DivideEq)
TYPECHECK_NODE(ModEq)
TYPECHECK_NODE(AndEq)
TYPECHECK_NODE(OrEq)
TYPECHECK_NODE(XorEq)
TYPECHECK_NODE(ShiftLeftEq)
TYPECHECK_NODE(ShiftRightEq)
TYPECHECK_NODE(NotEqual)
TYPECHECK_NODE(Equal)
TYPECHECK_NODE(GreaterThanEq)
TYPECHECK_NODE(LessThanEq)
TYPECHECK_NODE(GreaterThan)
TYPECHECK_NODE(LessThan)
TYPECHECK_NODE(Not)
TYPECHECK_NODE(BitAnd)
TYPECHECK_NODE(BitOr)
TYPECHECK_NODE(Xor)
TYPECHECK_NODE(ShiftLeft)
TYPECHECK_NODE(ShiftRight)
TYPECHECK_NODE(Negate)
TYPECHECK_NODE(PostDecr)
TYPECHECK_NODE(PostIncr)
TYPECHECK_NODE(PreDecr)
TYPECHECK_NODE(PreIncr)
TYPECHECK_NODE(UnaryMinus)
TYPECHECK_NODE(IdentRef)
TYPECHECK_NODE(FreeVarRef)
TYPECHECK_NODE(Assign)
TYPECHECK_NODE(GenericCall)
TYPECHECK_NODE(Assert)
TYPECHECK_NODE(NativeCall)
TYPECHECK_NODE(Call)
TYPECHECK_NODE(FunRef)
TYPECHECK_NODE(DynCall)
TYPECHECK_NODE(Return)
TYPECHECK_NODE(TypeAnnotation)
TYPECHECK_NODE(IsType)
TYPECHECK_NODE(VectorConstructor)
TYPECHECK_NODE(AutoConstructor)
TYPECHECK_NODE(ObjectConstructor)
TYPECHECK_NODE(Dot)
TYPECHECK_NODE(Indexing)
TYPECHECK_NODE(Seq)
TYPECHECK_NODE(TypeOf)
TYPECHECK_NODE(EnumCoercion)
TYPECHECK_NODE(MultipleReturn)
TYPECHECK_NODE(EnumRef)
TYPECHECK_NODE(GUDTRef)
TYPECHECK_NODE(UDTRef)

#undef TYPECHECK_NODE

Node *List::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/, TypeRef /*parent_bound*/) {
    assert(false);  // Parents call TypeCheckList
    return this;
}

Node *Unary::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/, TypeRef /*parent_bound*/) {
    assert(false);
    return this;
}

Node *BinOp::TypeCheck(TypeChecker & /*tc*/, size_t /*reqret*/, TypeRef /*parent_bound*/) {
    assert(false);
    return this;
}

Node *ErrorValue::TypeCheck(TypeChecker &tc, size_t, TypeRef /*parent_bound*/) {
    return tc.ErrorNode(*this);
}

Node *DefaultVal::TypeCheck(TypeChecker &, size_t, TypeRef /*parent_bound*/) {
    exptype = type_void;
    lt = LT_ANY;
    return this;
}

Node *Coercion::TypeCheck(TypeChecker &tc, size_t reqret, TypeRef /*parent_bound*/) {
    assert(false);  // Should not be called, since only inserted by TT.
    tc.TT(child, reqret, LT_ANY);
    return this;
}

Node *InlineReturn::TypeCheck(TypeChecker &, size_t /*reqret*/, TypeRef /*parent_bound*/) {
    assert(false);
    return this;
}

Node *InlineBlock::TypeCheck(TypeChecker &, size_t /*reqret*/, TypeRef /*parent_bound*/) {
    assert(false);
    return this;
}

}  // namespace lobster
