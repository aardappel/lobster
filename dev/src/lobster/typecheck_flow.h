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

// Branches, loops, switches, assertions, and how flow promotions pass thru them.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckFlow : virtual TypeCheckLocations {
    Switch *switch_case_context = nullptr;
    // Switch cases are typechecked thru the generic TT(), so Case::TypeCheck
    // leaves theirs here for Switch::TypeCheck to pick up.
    FlowBranch case_flow;

    TypeCheckFlow() {}

    TypeRef TypeCheckBranch(bool iftrue, const Node *condition, Block *block, size_t reqret,
                            FlowBranch *fb = nullptr) {
        auto flowstart = CheckFlowTypeChanges(iftrue, condition);
        // Loops don't pass an fb: their body may run any number of times, so
        // its demotions have to hold for the code around it as well.
        vector<TypeRef> outer;
        if (fb) BackupFlow(flowstart, outer);
        block->TypeCheck(ASTChecker(), reqret, {});
        if (fb) {
            CollectFlowPromotions(flowstart, fb->promoted);
            CollectFlowDemotions(outer, fb->demoted);
        }
        CleanUpFlow(flowstart);
        return block->exptype;
    }

    void TypeCheckAndOr(BinOp &ao, bool only_true_type, bool reqret, TypeRef &promoted_type) {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.
        TypeRef tleft, tright;
        TypeCheckAndOrSub(ao.left, Is<Or>(ao), true, tleft);
        auto flowstart = CheckFlowTypeChanges(Is<And>(ao), ao.left);
        TypeCheckAndOrSub(ao.right, only_true_type, reqret, tright);
        CleanUpFlow(flowstart);
        if (only_true_type && Is<And>(ao)) {
            ao.exptype = tright;
            ao.lt = ao.right->lt;
            DecBorrowers(ao.left->lt, ao);
        } else {
            // A nil (or a variable initialized from an untyped nil) whose element type is
            // still unbound arrives here as that element type (see TypeCheckAndOrSub), which
            // the union binds to the other operand's type.
            auto lparent = ao.left->exptype->t == V_NIL && tleft->t != V_NIL ? V_NIL : V_UNDEFINED;
            auto rparent = ao.right->exptype->t == V_NIL && tright->t != V_NIL ? V_NIL : V_UNDEFINED;
            ao.exptype = Union(tleft, tright, "lhs", "rhs", CF_NONE, nullptr, lparent, rparent);
            // An enum (e.g. bool) mixed with a plain int would result in the int value,
            // which is surprising, so treat it like unrelated types below and force to
            // bool, rather than letting the enum decay to int.
            if (tleft->t == V_INT && tright->t == V_INT && tleft->e != tright->e)
                ao.exptype = type_undefined;
            if (ao.exptype->t == V_UNDEFINED) {
                // Unless that type cannot be nilable (see UnifyVar), which leaves the nil
                // without a type.
                if (lparent == V_NIL && tleft->t == V_VAR) {
                    Error(ao, "nil cannot take type ", Q(TypeName(tright)),
                              ", which cannot be nilable");
                } else if (rparent == V_NIL && tright->t == V_VAR) {
                    Error(ao, "nil cannot take type ", Q(TypeName(tleft)),
                              ", which cannot be nilable");
                }
                // Special case: unlike elsewhere, we allow merging scalar and reference types,
                // since they are just tested and thrown away. To make this work, we force all
                // values to bools.
                MakeBool(ao.left);
                MakeBool(ao.right);
                ao.exptype = &st.default_bool_type->thistype;
                ao.lt = LT_ANY;
            } else {
                ao.lt = LifetimeUnion(ao.left, ao.right, Is<And>(ao));
            }
        }
        promoted_type = ao.exptype;
    }

    void TypeCheckAndOrSub(Node *&n, bool only_true_type, bool reqret, TypeRef &promoted_type) {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.
        if (!Is<And>(n) && !Is<Or>(n)) {
            TT(n, reqret, LT_ANY);
            NoStruct(*n, "and / or");
            promoted_type = n->exptype;
            if (promoted_type->t == V_NIL && only_true_type)
                promoted_type = promoted_type->Element();
        } else {
            auto ao = dynamic_cast<BinOp *>(n);
            assert(ao);
            TypeCheckAndOr(*ao, only_true_type, reqret, promoted_type);
        }
    }

    optional<VTValue> TypeCheckCondition(Node *&condition, Node *context, const char *name) {
        TT(condition, 1, LT_BORROW);
        NoStruct(*condition, name);
        DecBorrowers(condition->lt, *context);
        if (IsRef(condition->exptype->t)) {
            Warn(*condition, "condition will always succeed with non-nil reference type ",
                Q(TypeName(condition->exptype)));
        }
        VTValue cval;
        if (condition->ConstVal(this, cval) != V_VOID) return cval;
        return {};
    }

    Lifetime LifetimeUnion(Node *&a, Node *&b, bool is_and) {
        if (a->lt == b->lt) {
            DecBorrowers(b->lt, *b);
            return a->lt;
        } else if (a->lt == LT_ANY && b->lt >= LT_BORROW) {
            // This case may apply in an if-then between a var and nil, or an and/or between
            // a var and a scalar.
            return b->lt;
        } else if (b->lt == LT_ANY && a->lt >= LT_BORROW) {
            // Same.
            return a->lt;
        } else if (is_and && a->lt >= LT_BORROW && b->lt >= LT_BORROW) {
            // var_a and var_b never results in var_a.
            DecBorrowers(a->lt, *a);
            return b->lt;
        } else {
            // If it is an and we want to borrow the lhs since it will never be used.
            // Otherwise default to LT_KEEP for everything.
            // FIXME: for cases where both sides are >= LT_BORROW (in an if-then) we'd like to
            // combine both lifetimes into one, but we currently can't represent that.
            AdjustLifetime(a, is_and ? LT_BORROW : LT_KEEP);
            if (is_and) DecBorrowers(a->lt, *a);
            AdjustLifetime(b, LT_KEEP);
            return LT_KEEP;
        }
    }

    Node *Check(Block &node, size_t reqret, TypeRef /*parent_bound*/) {
        // Function scoping has been resolved by the declchecker, so no functions
        // need to be brought into scope here.
        for (auto &c : node.children) {
            TT(c, c != node.children.back() ? 0 : reqret, LT_ANY);
        }
        node.lt = node.children.back()->lt;
        node.exptype = node.children.back()->exptype;
        return &node;
    }

    Node *Check(Seq &node, size_t reqret, TypeRef /*parent_bound*/) {
        TT(node.head, 0, LT_ANY);
        TT(node.tail, reqret, LT_ANY);
        node.exptype = node.tail->exptype;
        node.lt = node.tail->lt;
        return &node;
    }

    Node *Check(Or &node, size_t reqret, TypeRef /*parent_bound*/) {
        TypeRef dummy;
        TypeCheckAndOr(node, false, reqret, dummy);
        return &node;
    }

    Node *Check(And &node, size_t reqret, TypeRef /*parent_bound*/) {
        TypeRef dummy;
        TypeCheckAndOr(node, false, reqret, dummy);
        return &node;
    }

    Node *Check(IfThen &node, size_t, TypeRef /*parent_bound*/) {
        auto constant = TypeCheckCondition(node.condition, &node, "if");
        if (!constant || constant->i) {
            FlowBranch thenb;
            TypeCheckBranch(true, node.condition, node.truepart, 0, &thenb);
            if (constant) {
                // Always taken, so nothing merges into it.
                ApplyFlow(thenb);
            } else if (node.truepart->Terminal(*this)) {
                // This is an if ..: return, we should leave promotions for code after the if.
                // Nothing the branch did to the flow types reaches that code.
                CheckFlowTypeChanges(false, node.condition);
            } else {
                // Code after the if is reached both by falling out of the branch and
                // by skipping it, so keep only what holds either way. Skipping it
                // means the condition was false, which may promote by itself.
                auto flowstart = CheckFlowTypeChanges(false, node.condition);
                FlowBranch elseb;
                CollectFlowPromotions(flowstart, elseb.promoted);
                CleanUpFlow(flowstart);
                MergeFlowPromotions(thenb.promoted, elseb.promoted);
                ApplyFlow(thenb);
            }
            if (constant && constant->i) {
                // Replace if-then by just the branch.
                auto r = node.truepart;
                node.truepart = nullptr;
                delete &node;
                return r;
            }
        } else {
            // constant == false: this if-then is entirely redundant, replace.
            auto r = new DefaultVal(node.line);
            r->exptype = type_void;
            r->lt = LT_ANY;
            delete &node;
            return r;
        }
        // No else: this always returns void.
        node.truepart->exptype = type_void;
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(IfElse &node, size_t reqret, TypeRef /*parent_bound*/) {
        auto constant = TypeCheckCondition(node.condition, &node, "if");
        if (!constant) {
            FlowBranch thenb, elseb;
            auto tleft = TypeCheckBranch(true, node.condition, node.truepart, reqret, &thenb);
            auto tright = TypeCheckBranch(false, node.condition, node.falsepart, reqret, &elseb);
            auto tterm = node.truepart->Terminal(*this);
            auto fterm = node.falsepart->Terminal(*this);
            // Code after this is reached over whichever branches can fall out of.
            if (tterm) {
                if (!fterm) ApplyFlow(elseb);
            } else if (fterm) {
                ApplyFlow(thenb);
            } else {
                MergeFlowPromotions(thenb.promoted, elseb.promoted);
                for (auto i : elseb.demoted) thenb.demoted.push_back(i);
                ApplyFlow(thenb);
            }
            // FIXME: this is a bit of a hack. Much better if we had an actual type
            // to signify NORETURN, to be taken into account in more places.
            if (tterm) {
                node.exptype = tright;
                node.lt = node.falsepart->lt;
            } else if (fterm) {
                node.exptype = tleft;
                node.lt = node.truepart->lt;
            } else {
                node.exptype = Union(tleft, tright, "then branch", "else branch",
                                   CF_COERCIONS, &node);
                SubType(node.truepart->children.back(), node.exptype, "then branch", node);
                SubType(node.falsepart->children.back(), node.exptype, "else branch", node);
                node.lt = LifetimeUnion(node.truepart->children.back(), node.falsepart->children.back(), false);
            }
            return &node;
        } else if (constant->i) {
            // Ignore the else part, and delete it, since we don't want to TT it.
            FlowBranch thenb;
            TypeCheckBranch(true, node.condition, node.truepart, reqret, &thenb);
            ApplyFlow(thenb);
            auto r = node.truepart;
            node.truepart = nullptr;
            delete &node;
            return r;
        } else {
            // Ignore the then part, and delete it, since we don't want to TT it.
            FlowBranch elseb;
            TypeCheckBranch(false, node.condition, node.falsepart, reqret, &elseb);
            ApplyFlow(elseb);
            auto r = node.falsepart;
            node.falsepart = nullptr;
            delete &node;
            return r;
        }
    }

    Node *Check(While &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TypeCheckCondition(node.condition, &node, "while");
        EnterLoop();
        TypeCheckBranch(true, node.condition, node.wbody, 0);
        LeaveLoop();
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(For &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.iter, 1, LT_BORROW);
        auto itertype = node.iter->exptype;
        if (itertype->t == V_INT) {}
        else if (itertype->t == V_STRING)
            itertype = type_int;
        else if (itertype->t == V_VECTOR)
            itertype = itertype->Element();
        else {
            Error(node, Q("for"), " can only iterate over int / string / vector, not ",
                            Q(TypeName(itertype)), DemotionNote(*node.iter));
            itertype = type_error;
        }
        st.BlockScopeStart();
        auto def = Is<Define>(node.fbody->children[0]);
        ForLoopElem *fle = nullptr;
        if (def) {
            fle = Is<ForLoopElem>(def->child);
            if (fle) {
                fle->exptype = itertype;
                fle->iter = node.iter;
                fle->elem_borrow = LT_UNDEF;
                fle->sid = nullptr;
                if (def->tsids[0].sid->withtype)
                    st.AddWithStructTT(itertype, def->tsids[0].sid->id, scopes.back().sf);
                // The loop variable may borrow the element rather than own it (see
                // SpecIdent::speculative): the elements of the vector are borrowed for the
                // duration of the loop, and a write to them in the body turns the variable into
                // an owner. Whether the variable takes the borrow is up to its Define.
                if (node.iter->exptype->t == V_VECTOR && SpecBorrowable(itertype)) {
                    LValContext lv(*node.iter);
                    if (lv.IsValid()) {
                        lv.derefs.push_back(&elem_field);
                        fle->elem_borrow = PushBorrowPath(lv, node.iter->line);
                    }
                }
            }
        }
        EnterLoop();
        node.fbody->TypeCheck(ASTChecker(), 0, {});
        LeaveLoop();
        st.BlockScopeCleanup();
        if (fle && fle->sid && fle->sid->speculative) ReleaseSpeculative(fle->sid);
        DecBorrowers(node.iter->lt, node);
        // Currently always return V_NIL
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(ForLoopElem &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        // Already been assigned a type in For, and possibly a borrow of the elements.
        node.lt = node.elem_borrow >= 0 ? node.elem_borrow : LT_KEEP;
        return &node;
    }

    Node *Check(ForLoopCounter &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = type_int;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Break &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (scopes.back().loop_flow.empty())
            Error(node, Q("break"), " must occur inside a ", Q("while"), " or ", Q("for"));
        else
            RecordLoopFlowExit();
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Continue &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (scopes.back().loop_flow.empty())
            Error(node, Q("continue"), " must occur inside a ", Q("while"), " or ", Q("for"));
        else
            RecordLoopFlowExit();
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Switch &node, size_t reqret, TypeRef /*parent_bound*/) {
        // TODO: much like If, should only typecheck one case if the value is constant, and do
        // the corresponding work in the optimizer.
        TT(node.value, 1, LT_BORROW);
        DecBorrowers(node.value->lt, node);
        auto ptype = node.value->exptype;
        auto on_types = IsDynamicType(ptype);
        if (!ptype->Numeric() && ptype->t != V_STRING && !on_types) {
            Error(node, "switch value must be int / float / string / class / abstract struct");
            ptype = type_error;
        }
        node.exptype = nullptr;
        ssize_t default_loc = -1;
        ssize_t out_of_range_loc = -1;
        vector<bool> enum_cases;
        vector<iint> ints_seen;
        if (ptype->IsEnum()) enum_cases.resize(ptype->e->vals.size());
        if (ptype->t == V_INT) ints_seen.reserve(64);
        node.cases->exptype = type_void;
        node.cases->lt = LT_ANY;
        FlowBranch merged;
        bool any_fallthrough = false;
        for (auto [i, n] : enumerate(node.cases->children)) {
            switch_case_context = &node;
            TT(n, reqret, LT_KEEP);
            auto cflow = std::move(case_flow);
            auto cas = AssertIs<Case>(n);
            if (!cas->pattern->Arity()) (cas->out_of_range ? out_of_range_loc : default_loc) = i;
            cas->pattern->exptype = type_void;
            cas->pattern->lt = LT_ANY;
            for (auto c : cas->pattern->children) {
                if (on_types) {
                    if (!Is<UDTRef>(c)) Error(*c, "non-type value in switch on type");
                } else {
                    SubTypeT(c->exptype, ptype, *c, "", "case");
                    DecBorrowers(c->lt, *cas);
                    auto RegNum = [&](iint vi) {
                        if (ptype->IsEnum()) {
                            assert(c->exptype->IsEnum());
                            for (auto [i, ev] : enumerate(ptype->e->vals)) {
                                if (ev->val == vi) {
                                    enum_cases[i] = true;
                                    break;
                                }
                            }
                        }
                        for (auto i : ints_seen) {
                            if (i == vi) Error(*c, "integer value used twice in switch: ", vi);
                        }
                        ints_seen.push_back(vi);
                    };
                    if (auto r = Is<Range>(c)) {
                        VTValue vs, ve;
                        if (r->start->ConstVal(this, vs) == V_INT && r->end->ConstVal(this, ve) == V_INT) {
                            for (auto ei = vs.i; ei <= ve.i; ei++) {
                                RegNum(ei);
                            }
                        }
                    } else {
                        VTValue v;
                        if (c->ConstVal(this, v) == V_INT) {
                            RegNum(v.i);
                        }
                    }
                }
            }
            if (!cas->cbody->Terminal(*this)) {
                node.exptype = node.exptype.Null() ? cas->cbody->exptype
                                         : Union(node.exptype, cas->cbody->exptype, "switch type", "case type",
                                                    CF_COERCIONS, cas);
                // Only cases that can fall out of the switch decide what holds
                // after it.
                for (auto d : cflow.demoted) merged.demoted.push_back(d);
                if (!any_fallthrough) merged.promoted = std::move(cflow.promoted);
                else MergeFlowPromotions(merged.promoted, cflow.promoted);
                any_fallthrough = true;
            }
        }
        if (out_of_range_loc >= 0 && !ptype->IsEnum())
            Error(node, Q(TName(T_OUT_OF_RANGE)), " can only be used in a switch on an enum value");
        // The parser guarantees at most one of the two, and both take the same slot in codegen.
        auto empty_pattern_loc = default_loc >= 0 ? default_loc : out_of_range_loc;
        if (empty_pattern_loc >= 0) {
            // Stick the default at the end, simplifies codegen.
            auto d = node.cases->children[empty_pattern_loc];
            node.cases->children.erase(empty_pattern_loc);
            node.cases->children.push_back(d);
        }
        for (auto n : node.cases->children) {
            auto cas = AssertIs<Case>(n);
            if (!cas->cbody->Terminal(*this)) {
                assert(!node.exptype.Null());
                SubType(cas->cbody, node.exptype, "", "case block");
            }
        }
        if (node.exptype.Null()) node.exptype = type_void;  // Empty switch or all return statements.
        if (on_types) {
            auto &dispatch_udt = *ptype->udt;
            dispatch_udt.subudts_dispatched_where = "switch";
            vector<int> case_picks;
            for (auto udt : dispatch_udt.subudts) {
                int pick = -1;
                if (!udt->g.is_abstract) {
                    int best_dist = -1;
                    int default_case = -1;
                    for (auto [i, n] : enumerate(node.cases->children)) {
                        auto cas = AssertIs<Case>(n);
                        if (cas->pattern->Arity()) {
                            auto udtref = Is<UDTRef>(cas->pattern->children[0]);
                            if (!udtref) continue;  // Reported above.
                            auto sdist = SuperDistance(udtref->udt, udt);
                            if (sdist >= 0 && (pick < 0 || best_dist >= sdist)) {
                                if (best_dist == sdist)
                                    Error(*udtref, "more than one case applies to ", Q(udt->name));
                                pick = (int)i;
                                best_dist = sdist;
                            }
                        } else {
                            default_case = (int)i;
                        }
                    }
                    if (pick < 0) pick = default_case;
                    if (pick < 0) Error(node, "no case applies to ", Q(udt->name));
                }
                case_picks.push_back(pick);
            }
            // FIXME: check here if any vtable entries are equal so we don't need to store
            // a new one.
            // Add cases to all vtables.
            node.vtable_idx = AddDispatchEntries(dispatch_udt, [&](size_t i) {
                return DispatchEntry{ nullptr, case_picks[i] };
            })->vtable_idx;
        } else if (default_loc < 0) {
            if (ptype->IsEnum()) {
                for (auto [i, ev] : enumerate(ptype->e->vals)) {
                    if (!enum_cases[i])
                        Error(*node.value, "enum value ", Q(ev->name), " not tested in switch");
                }
                if (out_of_range_loc < 0) {
                    // Add a runtime error for when the value is out of range. An `out_of_range` case
                    // already occupies this slot with code of its own.
                    auto pat = new List(node.cases->line);
                    pat->exptype = type_void;
                    pat->lt = LT_ANY;
                    // Blocks always have minimum of 1 statement in them, so an empty one signals runtime error here.
                    auto blk = new Block(node.cases->line);
                    blk->exptype = type_void;
                    blk->lt = LT_ANY;
                    auto cas = new Case(node.cases->line, pat, blk);
                    cas->exptype = type_void;
                    cas->lt = LT_ANY;
                    node.cases->Add(cas);
                }
            } else {
                if (reqret && !ptype->IsError()) {
                    Error(node,
                             "non-exhaustive switch that returns a value must have a default case");
                }
            }
        }
        // A promotion made by every case only holds after the switch if some case
        // always runs; the errors above guarantee that for a class or enum value.
        // Demotions hold regardless, since any case may have run.
        if (default_loc < 0 && !on_types && !ptype->IsEnum()) merged.promoted.clear();
        ApplyFlow(merged);
        node.lt = LT_KEEP;
        return &node;
    }

    Node *Check(Case &node, size_t reqret, TypeRef /*parent_bound*/) {
        // FIXME: Since string constants are the real use case, LT_KEEP would be more
        // natural here, as this will introduce a lot of keeprefs. Alternatively make sure
        // string consts don't introduce keeprefs.
        auto sw = switch_case_context;
        switch_case_context = nullptr;
        auto flowstart = flowstack.size();
        vector<TypeRef> outer;
        BackupFlow(flowstart, outer);
        st.BlockScopeStart();
        if (node.pattern->Arity()) {
            if (auto udtref = Is<UDTRef>(node.pattern->children[0])) {
                CheckFlowTypeIdOrDot(*sw->value, &udtref->udt->thistype);
                udtref->TypeCheck(ASTChecker(), 0, {});
                if (node.withtype) {
                    // The parser resolved the fields the body names, and only allows :: on a
                    // switch value that is a variable or field path. Methods of the value the body
                    // calls unqualified get it as receiver, like in a function with a :: argument.
                    LValContext lv(*sw->value);
                    if (lv.IsValid()) {
                        st.AddWithStructTT(&udtref->udt->thistype, lv.sid->id,
                                              scopes.back().sf, lv.derefs);
                    }
                }
            } else {
                TypeCheckList(node.pattern, LT_BORROW);
            }
        } else if (node.out_of_range) {
            // This block only runs for a value that is not one of the enum's values, so reading it
            // here gives a plain int. Like the type narrowing a `case` on a class does, this only
            // applies if the value is something flow typing can name, i.e. a variable or a field.
            CheckFlowTypeIdOrDot(*sw->value, type_int);
        }
        TT(node.cbody, reqret, LT_KEEP);
        st.BlockScopeCleanup();
        // Cases are alternatives like the branches of an if, so the same applies:
        // Switch::TypeCheck combines these for the code after it.
        case_flow = FlowBranch();
        CollectFlowPromotions(flowstart, case_flow.promoted);
        CollectFlowDemotions(outer, case_flow.demoted);
        CleanUpFlow(flowstart);
        node.exptype = node.cbody->exptype;
        node.lt = LT_KEEP;
        return &node;
    }

    Node *Check(Range &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.start, 1, LT_KEEP);
        TT(node.end, 1, LT_KEEP);
        node.exptype = node.start->exptype;
        if (node.exptype->t != node.end->exptype->t || !node.exptype->Numeric())
            Error(node, "range can only be two equal numeric types");
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Assert &node, size_t reqret, TypeRef /*parent_bound*/) {
        // If the assert value is passed on, we want any lifetime, so we can minimize inc/dec.
        // If it is not used we just want to borrow it, so it is not up to us to dec it.
        TT(node.child, 1, reqret ? LT_ANY : LT_BORROW);
        node.exptype = node.child->exptype;
        node.lt = node.child->lt;
        NoStruct(*node.child, "assert");
        // Special case, add to flow:
        CheckFlowTypeChanges(true, node.child);
        if (IsRef(node.exptype->t)) {
            Warn(node, "assert will always succeed with non-nil reference type ",
                    Q(TypeName(node.exptype)));
        }
        VTValue val;
        auto t = node.child->ConstVal(this, val);
        if (t != V_VOID && val.i) {
            string sd;
            val.ToString(sd, t);
            Warn(node, "assert will always succeed with constant value: ", sd);
        }
        // Also make result non-nil, if it was.
        if (node.exptype->t == V_NIL) node.exptype = node.exptype->Element();

        return &node;
    }

    Node *Check(IsType &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.child, 1, LT_BORROW);
        DecBorrowers(node.child->lt, node);
        node.resolvedtype = st.ResolveTypeVars(node.giventype, node.line);
        node.exptype = &st.default_bool_type->thistype;
        node.lt = LT_ANY;
        // Check for constness early, to be able to lift out side effects, which
        // makes downstream if-then optimisations easier.
        VTValue cval;
        bool unknown;
        auto t = node.ConstVal(this, cval, unknown);
        if (unknown) {
            // Whatever the test decided now, later code could bind that type to something
            // that contradicts it.
            auto ctype = node.child->exptype;
            if (ctype->HasValueType(V_VAR)) {
                Error(node, Q("is"), " cannot test a value whose type is not known yet: ",
                      Q(TypeName(ctype)));
            } else {
                Error(node, Q("is"), " cannot test against a type that is not known yet: ",
                      Q(TypeName(node.resolvedtype)));
            }
        } else if (t == V_INT) {
            // Made the way the parser makes `true` and `false`, which gives it the test's `bool`
            // type.
            auto ic = new IntConstant(node.line, cval.i);
            ic->from = st.default_bool_type->Lookup(cval.i);
            auto intc = ic->TypeCheck(ASTChecker(), 1, {});
            if (node.child->SideEffectRec()) {
                // must retain side effects.
                auto seq = new Seq(node.child->line, node.child, intc);
                seq->exptype = node.exptype;
                seq->lt = LT_ANY;
                node.child = nullptr;
                delete &node;
                return seq;
            } else {
                delete &node;
                return intc;
            }
        } else {
            // ConstVal is always const for a struct that is not in an abstract struct family,
            // since only those carry a dynamic type, unless the test is a nil check of a
            // reference (see the nil_check there), or nothing is known about the value since an
            // error was reported for it.
            assert(!IsStruct(node.resolvedtype->t) || node.resolvedtype->udt->family_root ||
                   (node.accepts_nil && node.child->exptype->t == V_NIL) || node.child->exptype->IsError());
        }
        return &node;
    }
};

bool Block::Terminal(TypeCheckBase &tc) const {
    return children.empty() || children.back()->Terminal(tc);
}

bool IfElse::Terminal(TypeCheckBase &tc) const {
    return truepart->Terminal(tc) && falsepart->Terminal(tc);
}

bool While::Terminal(TypeCheckBase &tc) const {
    // NOTE: if wbody is terminal, that does not entail the loop is, since
    // condition may be false on first iteration.
    // Instead, it is only terminal if this is an infinite loop.
    VTValue val;
    return condition->ConstVal(&tc, val) != V_VOID && val.i;
}

bool Break::Terminal(TypeCheckBase &) const {
    return true;
}

bool Continue::Terminal(TypeCheckBase &) const {
    return true;
}

bool Switch::Terminal(TypeCheckBase &tc) const {
    auto have_default = false;
    for (auto c : cases->children) {
        auto cas = AssertIs<Case>(c);
        if (cas->pattern->children.empty()) have_default = true;
        if (!cas->cbody->Terminal(tc)) return false;
    }
    if (!value->exptype.Null() &&  // Should already been typechecked but just in case.
        (IsDynamicType(value->exptype) || value->exptype->IsEnum()))  // Guaranteed exhaustive or runtime error.
        return true;
    // Other types, cannot guarantee it is terminal without a default.
    return have_default;
}

bool Assert::Terminal(TypeCheckBase &) const {
    // A function may end in "assert false" and have only its previous return statements
    // taken into account.
    auto i = Is<IntConstant>(child);
    return i && !i->integer;
}

}  // namespace lobster
