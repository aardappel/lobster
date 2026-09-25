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

// Variables, definitions, assignment, and field/index access.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckLval : virtual TypeCheckLocations {
    TypeCheckLval() {}

    Node *Check(Define &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        for (auto &p : node.tsids) {
            UpdateCurrentSid(p.sid);
            // We have to set these here just in case the init exp is a function call that
            // tries use/assign this variable, type_undefined will force that to be an error.
            // TODO: could make this a specialized error, but probably not worth it because it is rare.
            p.sid->type = type_undefined;
            p.sid->lt = LT_UNDEF;
        }
        // We default to LT_KEEP here.
        // There are case where we could allow borrow, but in practise this runs into trouble easily:
        // - Variables that later get assigned (!sid->id->single_assignment) where taking ownership
        //   was really what was intended (since the lval being assigned from may go away).
        // - old := cur cases, where old is meant to hang on to the previous value as cur gets updated,
        //   which then runs into borrowing errors.
        auto parent_bound = node.tsids.size() == 1 && !node.tsids[0].giventype.Null()
            ? node.tsids[0].giventype->Resolved()
            : TypeRef{};
        // Except that a single variable that is never assigned to may borrow what it is
        // initialized with when that is a variable, field or element (or a for loop element), see
        // SpecIdent::speculative. That is decided from what the initializer turns out to be, so
        // it is typechecked without a recipient lifetime and adjusted here.
        auto may_borrow = node.tsids.size() == 1 && !Is<DefaultVal>(node.child) &&
                          node.tsids[0].sid->id->single_assignment;
        auto fle = Is<ForLoopElem>(node.child);
        if (fle && !may_borrow && fle->elem_borrow >= 0) {
            // The loop set up the borrow of its elements, which is not going to be used.
            DecBorrowers(fle->elem_borrow, node);
            fle->elem_borrow = LT_UNDEF;
        }
        TT(node.child, Is<DefaultVal>(node.child) ? 0 : node.tsids.size(), may_borrow ? LT_ANY : LT_KEEP,
              parent_bound);
        // Nothing after it can run, so the variables it would define are never defined (what a
        // call whose every return is non-local, or a `return`, leaves as its value is padding, see
        // TT).
        if (node.child->Terminal(*this))
            ErrorAlways(node, "initializer of ", Q(node.tsids[0].sid->id->name), " never completes");
        auto speculate = may_borrow && node.child->lt >= 0 && SpecBorrowable(node.child->exptype);
        // What it would borrow: only a variable, field or element, since e.g. a builtin returning
        // a borrow (top) borrows the vector, which says nothing about writes to its elements.
        LValContext hold(*node.child);
        if (speculate) {
            if (fle) {
                hold = LValContext(*fle->iter);
                hold.derefs.push_back(&elem_field);
            }
            if (!hold.IsValid() || LoopWroteBefore(hold)) speculate = false;
        }
        if (may_borrow && !speculate) {
            if (fle && node.child->lt >= 0) {
                DecBorrowers(node.child->lt, node);
                node.child->lt = LT_KEEP;
            } else {
                AdjustLifetime(node.child, LT_KEEP);
            }
        }
        for (auto [i, p] : enumerate(node.tsids)) {
            auto var = TypeLT(*node.child, i);
            if (!p.giventype.Null()) {
                var.type = st.ResolveTypeVars(p.giventype, node.line);
                if (Is<DefaultVal>(node.child)) {  // A pre-decl.
                    p.sid->id->predeclaration = true;
                } else {
                    p.sid->id->predeclaration = false;
                    // Have to subtype the initializer value, as that node may contain
                    // unbound vars (a:[int] = []) or values that that need to be coerced
                    // (a:float = 1)
                    if (node.tsids.size() == 1) {
                        SubType(node.child, var.type, "initializer", "definition");
                    } else {
                        // FIXME: no coercion when mult-return?
                        SubTypeT(node.child->exptype->Get(i), var.type, node, p.sid->id->name);
                    }
                    // In addition, the initializer may already cause the type to be promoted.
                    // a:string? = ""
                    FlowItem fi(p.sid, var.type, node.child->exptype);
                    AssignFlowPromoteFI(fi);
                }
            }
            auto sid = p.sid;
            sid->type = var.type;
            StorageType(var.type, node);
            sid->type = var.type;
            sid->lt = var.lt;
            if (sid->id->preferfree && !Is<DefaultVal>(node.child)) {
                preferfreestack.push_back(sid);
            }
            LOG_DEBUG("var: ", sid->id->name, ":", TypeName(var.type));
        }
        if (node.tsids.size() == 1) {
            auto &sid = *node.tsids[0].sid;
            if (sid.id->constant && node.child->IsConstProp(sid.type)) {
                // We will have the optimizer remove this var, and not use it as a freevar.
                sid.constprop = node.child;
            }
            if (speculate) {
                if (fle) fle->sid = &sid;
                HoldSpeculative(&sid, node.child->lt, fle ? nullptr : &node, hold);
            } else if (!Is<DefaultVal>(node.child)) {
                RecordOwningAlias(sid, node.child, fle);
            }
        }
        definestack.push_back(&node);
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Member &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.child, 1, LT_KEEP);
        auto &f = *node.field();
        f.in_scope = true;
        scopes.back().scoped_fields.push_back(&node);
        if (node.this_sid) UpdateCurrentSid(node.this_sid);
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(Static &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.child, 1, node.sid->lt);
        SubType(node.child, node.sid->type, "static initializer", node);
        // FIXME: not doing any of the flow stuff Assign / Define do, needed?
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(AssignList &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        for (auto &c : node.children) {
            if (c != node.children.back()) {
                TT(c, 1, LT_BORROW);
                DecBorrowers(c->lt, node);
                if (auto idr = Is<IdentRef>(c)) FlipSpeculative(idr->sid);
                if (!Is<IdentRef>(c) && !Is<Dot>(c)) {
                    ErrorAlways(node,
                                   "assignment list elements must be variables or class members");
                }
            } else {
                TT(c, node.children.size() - 1, LT_MULTIPLE /*unused*/, {}, & node.children);
                if (c->Terminal(*this)) ErrorAlways(node, "assigned expression never completes");
            }
        }
        auto mr = Is<MultipleReturn>((Node *)SkipCoercions(node.children.back()));
        for (size_t i = 0; i < node.children.size() - 1; i++) {
            auto left = node.children[i];
            if (!Is<IdentRef>(left) && !Is<Dot>(left)) continue;  // Reported above.
            TypeRef righttype = node.children.back()->exptype->Get(i);
            if (auto idr = Is<IdentRef>(left); idr && mr)
                RecordOwningAlias(*idr->sid, mr->children[i], nullptr);
            CheckLval(left, righttype);
            FlowItem fi(*left, left->exptype);
            assert(fi.IsValid());
            AssignFlowDemote(fi, righttype, CF_NONE, node);
            SubTypeT(righttype, left->exptype, node, "right");
            StorageType(left->exptype, *left);
            // TODO: should call tc.AssignFlowPromote(*left, vartype) here?
        }
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(IdentRef &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        UpdateCurrentSid(node.sid);
        if (!IsActive(node.sid->sf_def)) {
            // A free var of a scope that would have to be active.
            if (checking_dead_code) return SkipDeadCode(node);
            // Its function need not even have been typechecked yet (when this is in a function
            // value that a `static` initializer calls, say), so there is no type to go on.
            return ErrorNode(node, "free variable ", Q(node.sid->id->name),
                             " not in scope: it is defined in ", Q(node.sid->sf_def->parent->name),
                             " (", parser.lex.Location(node.sid->id->line),
                             "), so a function value that uses it can only be called while that"
                             " is in scope");
        }
        if (node.sid->id->predeclaration)
            Error(node, "access of ", Q(node.sid->id->name), " before being initialized");
        CheckFreeVariable(*node.sid);
        if (node.sid->type->t == V_UNDEFINED) {
            // FIXME: this is a stop-gap error and should be checked differently:
            // When we scan scopes above, we need to somehow detect if the variable has
            // already been declared at that point we are in the callgraph.
            // This could also help with our free variable checking which may have a similar
            // issue.
            return ErrorNode(node, "access of ", Q(node.sid->id->name), " before being initialized");
        }
        node.exptype = node.sid->type;
        FlowItem fi(node, node.exptype);
        assert(fi.IsValid());
        node.exptype = UseFlow(fi);
        node.lt = node.sid->constprop ? LT_ANY : PushBorrow(&node);
        return &node;
    }

    Node *Check(FreeVarRef &node, size_t reqret, TypeRef parent_bound) {
        if (!node.fvd->spec.sid) {
            // This can happen if this was used in the init exp of a "member" decl.
            // Only way to stop this from happening is to make the parser not find this explicit freevar.
            return ErrorNode(node, "explicit free variable ", Q(node.fvd->name), " not in scope");
        }
        node.fvd->spec.used = true;
        Node *nn = new IdentRef(node.line, node.fvd->spec.sid);;
        if (node.fvd->spec.field) {
            nn = new Dot(node.fvd->spec.field, node.line, nn);
        }
        nn->TypeCheck(ASTChecker(), reqret, parent_bound);
        delete &node;
        return nn;
    }

    Node *Check(Assign &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (auto nn = OperatorOverload(node)) return nn;
        if (LvalueLifetime(*node.left, false) == LT_UNDEF) {
            // A variable that is not initialized yet, or of a function not even typechecked
            // yet, which the left side reported: there is nothing to assign to.
            TT(node.right, 1, LT_ANY);
            ReleaseChildren(node);
            return ErrorNode(node);
        }
        // An assigned variable owns, which decides how the right hand side is adjusted below.
        if (auto idr = Is<IdentRef>(node.left)) FlipSpeculative(idr->sid);
        DecBorrowers(node.left->lt, node);
        TT(node.right, 1, LvalueLifetime(*node.left, false));
        if (node.right->Terminal(*this)) ErrorAlways(node, "assigned expression never completes");
        if (auto idr = Is<IdentRef>(node.left)) RecordOwningAlias(*idr->sid, node.right, nullptr);
        CheckLval(node.left, node.right->exptype);
        FlowItem fi(*node.left, node.left->exptype);
        if (fi.IsValid()) {
            node.left->exptype = AssignFlowDemote(fi, node.right->exptype, CF_COERCIONS, node);
        }
        SubType(node.right, node.left->exptype, "right", node);
        if (fi.IsValid()) AssignFlowPromote(*node.left, node.right->exptype);
        node.exptype = node.left->exptype;
        if (fi.IsValid()) node.exptype = UseFlow(fi);
        node.lt = PushBorrow(node.left);
        return &node;
    }

    Node *Check(Dot &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (node.child->exptype->t == V_UNDEFINED) {
            // A field of a :: argument, or an explicit free variable that is one, which the
            // parser and FreeVarRef make directly rather than thru a GenericCall.
            TT(node.child, 1, LT_ANY);
        }
        AdjustLifetime(node.child, LT_BORROW);
        DecBorrowers(node.child->lt, node);  // New borrow created below.
        auto stype = node.child->exptype;
        if (!IsUDT(stype->t)) {
            RequiresError("class/struct", stype, node, "object");
            return ErrorNode(node);
        }
        auto udt = stype->udt;
        auto fieldidx = udt->g.Has(node.fld);
        if (fieldidx < 0)
            return ErrorNode(node, "type ", Q(udt->name), " has no field ", Q(node.fld->name));
        auto &field = udt->g.fields[fieldidx];
        if (field.isprivate && node.line.fileidx != field.defined_in.fileidx)
            Error(node, "field ", Q(field.id->name), " is private");
        if (field.member_of) {
            // A field declared by `member` belongs to the method that declares it:
            // it is in scope there once that statement has been reached, and in
            // anything lexically nested in it, but nowhere else.
            auto visible = false;
            for (auto ov = scopes.back().sf->overload; ov; ov = ov->sf->lexical_parent) {
                if (ov == field.member_of) { visible = true; break; }
            }
            if (!visible)
                Error(node, "field ", Q(field.id->name), " was declared by a ", Q("member"),
                                " in ", Q(field.member_of->sf->parent->name), " (",
                                parser.lex.Location(field.defined_in),
                                "), so it can only be used there");
            else if (!field.in_scope)
                Error(node, "field ", Q(field.id->name), " is not in scope");
        }
        node.exptype = udt->sfields[fieldidx].type;
        if (node.exptype.Null()) {
            // The field type is still to be inferred from its default value.
            EnsureUDTChecked(*udt, node);
            node.exptype = udt->sfields[fieldidx].type;
            if (node.exptype.Null()) {
                node.exptype = type_error;
                Error(node, "type of field ", Q(field.id->name),
                                " cannot be inferred here, give it an explicit type");
            }
        }
        FlowItem fi(node, node.exptype);
        if (fi.IsValid()) {
            auto flowtype = UseFlow(fi);
            if (!flowtype->Equal(*node.exptype)) {
                FlowFieldAddToFreeVariables(fi, node.exptype);
            }
            node.exptype = flowtype;
        }
        node.lt = PushBorrow(&node);
        return &node;
    }

    Node *Check(Indexing &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (auto nn = OperatorOverload(node)) return nn;
        TT(node.index, 1, LT_BORROW);
        DecBorrowers(node.index->lt, node);
        auto vtype = node.object->exptype;
        auto itype = node.index->exptype;
        if (vtype->t != V_VECTOR &&
            vtype->t != V_STRING &&
            (!IsStruct(vtype->t) || !vtype->udt->sametype->Numeric())) {
            RequiresError("vector/string/numeric struct", vtype, node, "container", "", node.object);
            node.exptype = type_error;
        } else switch (itype->t) {
            case V_INT:
                node.exptype = vtype->t == V_VECTOR
                    ? vtype->Element()
                    : (IsUDT(vtype->t) ? vtype->udt->sametype : type_int);
                break;
            case V_STRUCT_S: {
                if (vtype->t != V_VECTOR) {
                    Error(node, "multi-dimensional indexing on non-vector");
                    node.exptype = type_error;
                    break;
                }
                auto &udt = *itype->udt;
                node.exptype = vtype;
                for (auto &field : udt.sfields) {
                    if (field.type->t != V_INT)
                        RequiresError("int field", field.type, node, "index");
                    if (node.exptype->t != V_VECTOR) {
                        RequiresError("nested vector", node.exptype, node, "container");
                        node.exptype = type_error;
                        break;
                    }
                    node.exptype = node.exptype->Element();
                }
                break;
            }
            default:
                RequiresError("int/struct of int", itype, node, "index");
                node.exptype = type_error;
        }
        // The element borrows from the vector's elements as one location (see elem_field), which
        // a write to any element, or a builtin that drops elements, is a write to. Only when
        // the vector is a variable or field path; otherwise the element just borrows whatever the
        // vector expression borrowed, or nothing specific.
        auto olt = node.object->lt;
        node.lt = PushBorrow(&node);
        if (node.lt == LT_BORROW) node.lt = olt;
        else DecBorrowers(olt, node);
        return &node;
    }
};

TypeRef IdentRef::SimpleType(SymbolTable &st) {
    auto id = sid->id;
    // Only globals: anything else may not even be in scope where this exp
    // ends up being typechecked.
    if (id->scopelevel != 1) return nullptr;
    TypeRef type = nullptr;
    if (!id->giventype.Null()) {
        // Note: for a global, any type variables in scope here can't apply
        // to its type, so anything not concrete fails to resolve.
        type = SimpleResolve(id->giventype, st);
        if (type.Null()) return nullptr;
    } else if (id->single_assignment && id->toplevel_initializer) {
        // Constant globals like float2_1: their type is that of their
        // defining expression (which can only refer to globals defined
        // before it, so this recursion terminates).
        type = id->toplevel_initializer->SimpleType(st);
        if (type.Null()) return nullptr;
    } else {
        return nullptr;
    }
    // A flow sensitive type could get promoted by TypeCheck(), so the
    // declared type would not always equal what full typechecking gives.
    if (type->FlowSensitive()) return nullptr;
    return type;
}

}  // namespace lobster
