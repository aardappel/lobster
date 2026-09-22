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

// Function bodies, dead code, return statements, and return replay.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckFunctions : virtual TypeCheckLocations {
    TypeCheckFunctions() {}

    // Typecheck top level named functions that were never reached, purely to
    // report the basic errors they may contain. Everything that got
    // typechecked only because of this is reverted to dead afterwards, so
    // the optimizer and codegen treat it like any other unreached code.
    // Only functions whose args are all concretely typed can be checked out
    // of context, and code valid only with an active caller (uses of
    // "functions as environments", free vars of a lexically enclosing
    // scope) skips the rest of the containing function, see SkipDeadCode.
    void TypeCheckDeadCode() {
        auto num_sfs = st.subfunctiontable.size();
        struct SfState { int numcallers; size_t callers; bool typechecked; };
        vector<SfState> sf_states;
        for (auto sf : st.subfunctiontable) {
            sf_states.push_back({ sf->numcallers, sf->callers.size(), sf->typechecked });
        }
        auto num_udts = st.udttable.size();
        vector<size_t> dt_sizes;
        for (auto udt : st.udttable) dt_sizes.push_back(udt->dispatch_table.size());
        auto num_sids = st.specidents.size();
        vector<bool> sid_typed;
        sid_typed.reserve(num_sids);
        for (auto sid : st.specidents) sid_typed.push_back(!sid->type.Null());
        checking_dead_code = true;
        // The top level scope is active whenever any of these functions could run, so it is
        // one here too: globals must resolve like they do during the rest of typechecking,
        // rather than count as free variables of a scope that would have to be active, which
        // would abandon the check at the first use of one (see IdentRef::TypeCheck).
        Scope top_level_scope;
        top_level_scope.sf = st.toplevel;
        top_level_scope.call_context = parser.root;
        top_level_scope.resolve_context_size = st.resolve_context.size();
        top_level_scope.flowstack_size = flowstack.size();
        scopes.push_back(top_level_scope);
        for (size_t fi = 0; fi < st.functiontable.size(); fi++) {
            auto f = st.functiontable[fi];
            // Top level named functions only (scopelevel 2, 1 is file scope).
            if (f->anonymous || f->istype || f->scopelevel != 2) continue;
            for (auto ov : f->overloads) {
                if (!ov->gbody) continue;
                auto checked = false;
                for (auto sf = ov->sf; sf; sf = sf->next) {
                    if (sf->typechecked) checked = true;
                }
                if (checked || !ov->sf->generics.empty()) continue;
                auto annotated = true;
                for (auto &ga : ov->givenargs) {
                    if (st.IsGeneric(ga)) {
                        annotated = false;
                        break;
                    }
                }
                if (!annotated) continue;
                auto sf = CloneFunction(*ov);
                sf->reqret = sf->returngiventype.Null()
                    ? 0
                    : st.ResolveTypeVars(sf->returngiventype, ov->declared_at)->NumValues();
                for (auto [i, arg] : enumerate(sf->args)) {
                    arg.spec_type = st.ResolveTypeVars(ov->givenargs[i], ov->declared_at);
                    arg.sid->lt = LT_KEEP;
                }
                // NOTE: cursids overwritten by this are not reverted; that only affects
                // (the quality of errors in) further dead code checks.
                // The body stands in for the call there is none of: it stays in place while
                // its statements get typechecked, which may replace (and delete) them.
                TypeCheckFunctionDef(*sf, *sf->sbody);
                dead_code_skipped = false;
            }
        }
        scopes.pop_back();
        checking_dead_code = false;
        // Revert to dead: functions that were already typechecked keep that,
        // everything else (including specializations of live functions
        // created by dead code, whose emission nothing needs) does not count.
        for (auto [i, state] : enumerate(sf_states)) {
            auto sf = st.subfunctiontable[i];
            sf->typechecked = state.typechecked;
            sf->numcallers = state.numcallers;
            sf->callers.resize(state.callers);
        }
        for (size_t i = num_sfs; i < st.subfunctiontable.size(); i++) {
            st.subfunctiontable[i]->typechecked = false;
        }
        // Same for dispatch tables created by dead code, whose entries would
        // otherwise refer to functions that don't get emitted.
        for (size_t i = 0; i < num_udts; i++) {
            auto &dt = st.udttable[i]->dispatch_table;
            if (dt.size() > dt_sizes[i]) dt.resize(dt_sizes[i]);
        }
        for (size_t i = num_udts; i < st.udttable.size(); i++) {
            st.udttable[i]->dispatch_table.clear();
        }
        // Same for the types of the variables in it: codegen gives every sid that has one
        // a slot and a type table entry, so a variable of a function that does not get
        // emitted must be left without, as it is before it is typechecked. Its type can
        // also be the error type (see SkipDeadCode), which nothing downstream represents.
        for (size_t i = 0; i < num_sids; i++) {
            if (!sid_typed[i]) st.specidents[i]->type = nullptr;
        }
        for (size_t i = num_sids; i < st.specidents.size(); i++) {
            st.specidents[i]->type = nullptr;
        }
    }

    void RetVal(TypeRef type, SubFunction *sf, const Node &err) {
        for (auto &isc : reverse(scopes)) {
            if (isc.sf->parent == sf->parent) break;
            // isc.sf is a function in the call chain between the return statement and the
            // function it is returning from. Since we're affecting the return type of the
            // function we're returning from, if it gets specialized but a function along the
            // call chain (isc.sf) does not, we must ensure that return type affects the second
            // specialization.
            // We do this by tracking return types, and replaying them when a function gets
            // reused.
            // A simple test case is in return_from unit test, and recursive_exception is also
            // affected.
            // Check if return event already exists, which may happen for multiple similar return
            // statement of when called from ReplayReturns in a similar call context.
            for (auto &rre : isc.sf->reuse_return_events) {
                if (rre.first == sf && rre.second->Equal(*type)) goto found;
            }
            isc.sf->reuse_return_events.push_back({ sf, type });
            found:;
        }
        sf->num_returns++;
        if (sf != scopes.back().sf) sf->num_returns_non_local++;
        if (sf->returngiventype.Null()) {
            if (sf->reqret) {
                // We can safely generalize the type if needed, though not with coercions.
                sf->returntype = Union(type, sf->returntype, "return expression",
                                        "function return type", CF_NONE, &err);
            } else {
                // The caller doesn't want return values.
                sf->returntype = type_void;
            }
        }
    }

    // A return from `f` while none of its calls is active (see Return::TypeCheck) is in every
    // function being typechecked, which can then be reused only where that is still the case,
    // see CompatibleReturns.
    void RecordInactiveReturn(const Function &f) {
        for (auto &isc : scopes) {
            auto &irs = isc.sf->reuse_inactive_returns;
            if (std::find(irs.begin(), irs.end(), &f) == irs.end()) irs.push_back(&f);
        }
    }

    void CheckExplicitFreeVarSid(ExplicitFreeVar *fvd, SpecIdent *sid, TypeRef type, bool checkfields, Node &context) {
        if (sid->id->name == fvd->name) {
            fvd->spec.sid = sid;
            if (!fvd->spec.bound.Null()) {
                if (!ConvertsTo(fvd->spec.sid->type, fvd->spec.bound, CF_NONE))
                    Error(context, "explicit free variable ", Q(fvd->name),
                        " has type ", TypeName(fvd->spec.sid->type));
            }
        } else if (checkfields && IsUDT(type->t)) {
            auto udt = type->udt;
            for (auto [i, field] : enumerate(udt->g.fields)) {
                if (field.id->name == fvd->name) {
                    fvd->spec.sid = sid;
                    fvd->spec.field = field.id;
                    if (!fvd->spec.bound.Null()) {
                        if (!ConvertsTo(udt->sfields[i].type, fvd->spec.bound, CF_NONE))
                            Error(context, "explicit free variable ", Q(fvd->name),
                                " has type ", TypeName(fvd->spec.sid->type));
                    }
                    return;
                }
            }
        }
    }

    void TypeCheckFunctionDef(SubFunction &sf, const Node &call_context) override {
        if (sf.typechecked) return;
        STACK_PROFILE;
        // Look up explicit free variables. expensive?
        vector<ExplicitFreeVarSpec> freevardeclsbackup;
        for (auto fvd : sf.overload->freevardecls) {
            freevardeclsbackup.push_back(fvd->spec);
            if (!fvd->giventype.Null()) {
                fvd->spec.bound = st.ResolveTypeVars(fvd->giventype, sf.sbody->children[0]->line);
            }
            // First check any preferred ones.
            for (auto sid : reverse(preferfreestack)) {
                CheckExplicitFreeVarSid(fvd, sid, sid->type, true, *sf.sbody->children[0]);
                if (fvd->spec.sid) break;
            }
            // If not, fall back to a full scan of scopes.
            // FIXME: this scope scanning does not respect if a variable is declared after when a call is made,
            // or if its inside control structures etc!
            for (auto &sc : reverse(scopes)) {
                auto &ssf = *sc.sf;
                auto lookup = [&](vector<Arg> &args) {
                    for (auto &arg : args) {
                        CheckExplicitFreeVarSid(fvd, arg.sid, arg.spec_type, arg.sid->withtype, *sf.sbody->children[0]);
                        if (fvd->spec.sid) return;
                    }
                };
                lookup(ssf.locals);
                if (fvd->spec.sid) break;
                lookup(ssf.args);
                if (fvd->spec.sid) break;
            }
            if (!fvd->spec.sid) {
                // Requires an active caller providing it, which dead code has none of.
                if (checking_dead_code) {
                    dead_code_skipped = true;
                } else {
                    ErrorAlways(*sf.sbody->children[0], "explicit free variable ",
                                Q(fvd->name), " not found in context");
                }
                // The body can't be checked in this context without it (every use of it would
                // be an error of its own), so it isn't: the callers get the error type.
                for (auto [i, backup] : enumerate(freevardeclsbackup)) {
                    sf.overload->freevardecls[i]->spec = backup;
                }
                sf.typechecked = true;
                sf.returntype = ErrorReturnType(sf.reqret);
                return;
            }
        }
        LOG_DEBUG("function start: ", SignatureWithFreeVars(sf, nullptr));
        Scope scope;
        scope.sf = &sf;
        scope.call_context = &call_context;
        scope.resolve_context_size = st.resolve_context.size();
        scope.flowstack_size = flowstack.size();
        scopes.push_back(scope);
        auto pfvss = preferfreestack.size();
        auto dss = definestack.size();
        st.BlockScopeStart();
        sf.typechecked = true;
        for (auto &arg : sf.args) {
            StorageType(arg.spec_type, call_context);
            if (arg.sid->withtype) st.AddWithStructTT(arg.spec_type, arg.sid->id, &sf);
        }
        for (auto &fv : sf.freevars) UpdateCurrentSid(fv.sid);
        auto backup_vars = [&](vector<Arg> &in, vector<Arg> &backup) {
            for (auto [i, arg] : enumerate(in)) {
                // Need to not overwrite nested/recursive calls. e.g. map(): map(): ..
                backup[i].sid = arg.sid->Current();
                arg.sid->type = arg.spec_type;
                RevertCurrentSid(arg.sid);
            }
        };
        auto backup_args = sf.args; backup_vars(sf.args, backup_args);
        auto backup_locals = sf.locals; backup_vars(sf.locals, backup_locals);
        auto enter_scope = [&](const Arg &var) {
            IncBorrowers(var.sid->lt, call_context);
        };
        for (auto &arg : sf.args) enter_scope(arg);
        for (auto &local : sf.locals) enter_scope(local);
        if (sf.reqret) {
            if (sf.returngiventype.Null()) {
                sf.returntype = st.NewTypeVar();
            } else {
                sf.returntype = st.ResolveTypeVars(sf.returngiventype, call_context.line);
                assert(sf.returntype->t != V_NIL || sf.returntype->sub->t != V_INT);
                auto len = sf.returntype->NumValues();
                if (len > sf.reqret) {
                    if (sf.reqret == 1) {
                        sf.returntype = sf.returntype->Get(0);
                    } else {
                        assert(sf.returntype->t == V_TUPLE);
                        auto nt = st.NewTuple(sf.reqret);
                        nt->tup->assign(sf.returntype->tup->begin(),
                                        sf.returntype->tup->begin() + sf.reqret);
                        sf.returntype = nt;
                    }
                } else if (len < sf.reqret) {
                    ErrorAlways(call_context, "returning ", len, " values, caller requires ",
                                sf.reqret);
                    // The body returns what it declares; the call gets padded, see
                    // TypeCheckCallStatic.
                    sf.reqret = len;
                }
            }
        } else {
            sf.returntype = type_void;
        }
        auto start_borrowed_vars = borrowstack.size();
        auto start_promoted_vars = flowstack.size();
        sf.sbody->TypeCheck(ASTChecker(), 0, {});
        CleanUpFlow(start_promoted_vars);
        // Every body ends in a return (the parser adds one), so this is a function whose
        // returns are all non-local, i.e. that never returns normally.
        if (!sf.num_returns) {
            if (!sf.returngiventype.Null() && sf.returngiventype->t != V_VOID) {
                ErrorAlways(*sf.sbody->children.back(), "missing return statement");
                // The declared type stands, such that callers see what was promised.
            } else {
                sf.returntype = type_void;
            }
        }
        // Let variables go out of scope in reverse order of declaration.
        auto exit_scope = [&](const Arg &var) {
            if (var.sid->speculative) ReleaseSpeculative(var.sid);
            else DecBorrowers(var.sid->lt, call_context);
        };
        for (auto &local : reverse(sf.locals)) {
            exit_scope(local);
        }
        for (auto &arg : sf.args) {
            exit_scope(arg);  // No order.
        }
        while (borrowstack.size() > start_borrowed_vars) {
            auto &b = borrowstack.back();
            if (b.refc) {
                ErrorAlways(*sf.sbody->children.back(),
                            "variable ", Q(b.Name()), " still has ", b.refc, " borrowers");
            }
            borrowstack.pop_back();
        }
        while (dss < definestack.size()) {
            auto def = definestack.back();
            definestack.pop_back();
            // For now, don't warn when declaring multiple vars and they are mixed const,
            // since there is no way to use var for one and let for the other.
            // FIXME: this runs for every specialization, but for now Warn filters this anyway.
            bool warn_all = true;
            for (auto p : def->tsids) {
                auto id = p.sid->id;
                if (!id->single_assignment || id->constant || id->struct_field_assign ||
                    id->loop_var)
                    warn_all = false;
            }
            if (warn_all) {
                for (auto p : def->tsids) {
                    parser.WarnAt(def, "use ", Q("let"), " to declare ", Q(p.sid->id->name));
                }
            }
        }
        for (auto &back : backup_args) {
            RevertCurrentSid(back.sid);
        }
        for (auto &back : backup_locals) {
            RevertCurrentSid(back.sid);
        }
        if (sf.returntype->HasValueType(V_VAR)) {
            // If this function return something with a variable in it, then it likely will get
            // bound by the caller. If the function then gets reused without specialization, it will
            // get the wrong return type, so we force specialization for subsequent calls of this
            // function. FIXME: check in which cases this is typically true, since its expensive
            // if done without reason.
            sf.mustspecialize = true;
        }
        if (sf.parent->is_constructor_of &&
            (!IsUDT(sf.returntype->t) || &sf.returntype->udt->g != sf.parent->is_constructor_of) &&
            (sf.returntype->t != V_VOID || !sf.returngiventype.Null())  //  Return value discarded.
            ) {
            Error(*sf.sbody->children.back(), "constructor must return value of its own type");
        }
        st.BlockScopeCleanup();
        preferfreestack.resize(pfvss);
        for (auto member : scopes.back().scoped_fields) {
            auto f = member->field();
            f->in_scope = false;
        }
        scopes.pop_back();
        LOG_DEBUG("function end ", Signature(sf), " returns ",
                             TypeName(sf.returntype));
        for (auto [i, fvd] : enumerate(sf.overload->freevardecls)) {
            if (!fvd->spec.used)
                Warn(*sf.sbody->children.back(), "unused explicit free variable ", Q(fvd->name));
            fvd->spec = freevardeclsbackup[i];
        }
    }

    void ReplayReturns(const SubFunction *sf, const Node &call_context) override {
        // Apply effects of return statements for functions being reused, see
        // RetVal above.
        for (auto [isf, type] : sf->reuse_return_events) {
            for (auto &isc : reverse(scopes)) {
                if (isc.sf->parent == isf->parent) {
                    // NOTE: will have to re-apply lifetimes as well if we change
                    // from default of LT_KEEP.
                    RetVal(type, isc.sf, call_context);
                    // RetVal takes a return for a local one when the function it returns from
                    // is the current scope, which here it is only because the function that
                    // holds the return is being reused rather than typechecked. The return is
                    // in that function, not in this one, and the inliner has to know that, see
                    // Call::Optimize.
                    if (isc.sf == scopes.back().sf) isc.sf->num_returns_non_local++;
                    // This should in theory not cause an error, since the previous
                    // specialization was also ok with this set of return types.
                    // It could happen though if this specialization has an
                    // additional return statement that was optimized
                    // out in the previous one.
                    SubTypeT(type, isc.sf->returntype, call_context, "",
                        "reused return value");
                    goto destination_found;
                }
            }
            // Dead code has no caller that would provide one, so there the return goes nowhere,
            // as one typechecked in it does, see Return::TypeCheck.
            if (checking_dead_code) {
                RecordInactiveReturn(*isf->parent);
                continue;
            }
            // This error should hopefully be rare, but still possible if this call is in
            // a very different context.
            ErrorAlways(call_context, "return out of call to ", Q(sf->parent->name),
                        " can\'t find destination ", Q(isf->parent->name));
            destination_found:;
        }
        for (auto [isf, type] : sf->reuse_return_events) {
            auto start_sf = scopes.back().sf;
            auto nretslots = ValWidthMulti(isf->returntype, isf->returntype->NumValues());
            if (!RecursiveCheckReturns(start_sf, nretslots, isf, call_context))
                ErrorAlways(call_context, "return from ", Q(isf->parent->name),
                            " called out of context");
        }
        for (auto f : sf->reuse_inactive_returns) RecordInactiveReturn(*f);
    }

    // This more complex iteration is needed for recursion, see Check(Return &) below
    // and TypeCheckCallStatic
    bool RecursiveCheckReturns(SubFunction *sf, int nretslots, const SubFunction *dest_sf,
                               const Node &context) {
        if (sf->parent == dest_sf->parent) {
            // Reached destination for this particular trace.
            return true;
        }
        // Special case for returning out of top level, which is always allowed.
        if (dest_sf != st.toplevel && sf->isdynamicfunctionvalue) {
            // This is because the function has been typechecked against one context, but
            // can be called again in a different context that does not have the same
            // callers.
            Error(context, "cannot return through dynamic function value (",
                    "return statement tries to return from ", Q(dest_sf->parent->name), ")");
        }
        if (sf->returned_thru_to_max >= nretslots) {
            // We already have something returning thru here that is at least as big, check if
            // its the same function because then we're done.
            // This is not only a huge early-out optimization, it also prevents recursive calls
            // from stack overflowing this function :)
            for (auto idx : sf->returned_thru_function_ids) {
                if (idx == dest_sf->parent->idx) {
                    return true;
                }
            }
        }
        sf->returned_thru_to_max = std::max(sf->returned_thru_to_max, nretslots);
        sf->returned_thru_function_ids.push_back(dest_sf->parent->idx);
        // Now we step into the callers. This will typically only have 1 element in it in the
        // non-recursive case, and 2 for a normal active recursive call.
        for (auto &caller : sf->callers) {
            if (!caller.caller) {
                return false;  // Arrived at root call.
            }
            if (caller.de) {
                caller.de->returned_thru_to_max =
                    std::max(caller.de->returned_thru_to_max, nretslots);
                for (auto udt : caller.de->dispatch_root->subudts) {
                    // If any SubFunction in the dispatch generates an unwind check, all of them
                    // must return assuming one.
                    auto dsf = udt->dispatch_table[caller.de->vtable_idx]->sf;
                    if (!dsf) continue;
                    dsf->returned_thru_to_max = std::max(dsf->returned_thru_to_max, nretslots);
                }
            }
            if (!RecursiveCheckReturns(caller.caller, nretslots, dest_sf, context))
                return false;
        }
        return true;
    }

    Node *Check(Return &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = type_void;
        node.lt = LT_ANY;
        // Ensure what we're returning from is going to be on the stack at runtime.
        // First find correct specialization for sf.
        auto active = false;
        for (auto isc : reverse(scopes)) {
            if (isc.sf->parent == node.sf->parent) {
                node.sf = isc.sf;
                active = true;
                break;
            }
        }
        // TODO: LT_KEEP here is to keep it simple for now, since ideally we want to also allow
        // LT_BORROW, but then we have to prove that we don't outlive the owner.
        // Additionally, we have to do this for reused specializations on new SpecIdents.
        auto reqret = node.make_void ? 0 : node.sf->reqret;
        TT(node.child, reqret, LT_KEEP);
        DecBorrowers(node.child->lt, node);
        // A return from the program has at most one value: TT dropped whatever the
        // expression produced beyond what the program returns.
        assert(node.sf != st.toplevel || node.child->exptype->NumValues() <= 1);
        auto never_returns = node.child->Terminal(*this);
        if (never_returns && node.make_void && node.sf->num_returns) {
            // A return with other returns inside of it that always bypass this return,
            // so should not contribute to return types.
            if (node.child->exptype->t == V_VAR) UnifyVar(type_void, node.child->exptype, V_UNDEFINED);
            assert(node.child->exptype->t == V_VOID);
            // Call this for correct counting of number of returns, with existing type, should
            // have no effect on the type.
            RetVal(node.sf->returntype, node.sf, node);
            return &node;
        }
        if (never_returns && node.sf->reqret && node.sf->parent->anonymous) {
            // A return to the immediately enclosing anonymous function that needs to return a value
            // but is bypassed.
            RetVal(node.child->exptype, node.sf, node);  // If it's a variable, bind it.
            return &node;
        }
        if (node.make_void && node.sf->num_returns && node.sf->returntype->t != V_VOID) {
            // Specialized error to not have the code below complain that there is a non-existing
            // return statement returning void.
            // FIXME: need V_VOID check because num_returns can be 1 in functions with exp-less "return"??
            ErrorAlways(node, "control reaches end of non-void function");
            return &node;
        }
        if (!active) {
            // A non-local return from a function none of whose calls is active: an error (see
            // below), except in dead code, which is typechecked without the callers that would
            // provide one, see TypeCheckDeadCode. node.sf is still the one the parser gave it, a
            // specialization unrelated to this return (possibly of live code), so the value
            // isn't returned to anything.
            RecordInactiveReturn(*node.sf->parent);
        } else if (!Is<DefaultVal>(node.child)) {
            auto scchild = (Node *)SkipCoercions(node.child);
            if (auto mrs = Is<MultipleReturn>(scchild)) {
                RetVal(mrs->exptype, node.sf, node);
                for (auto [i, mr] : enumerate(mrs->children)) {
                    if (i < node.sf->reqret)
                        SubType(mr, node.sf->returntype->Get(i), ArgName(i), node);
                }
            } else {
                RetVal(node.child->exptype, node.sf, node);
                SubType(node.child, node.sf->returntype, "", node);
            }
        } else {
            RetVal(type_void, node.sf, node);
            SubType(node.child, node.sf->returntype, "", node);
        }
        // Now we can check what we're returning past as well.
        // Do this last, since we want RetVal to have been called on sf.
        // Previously, we would simply iterate over reverse(tc.scopes) to find all functions
        // we are returning past, but that does not work with recursive functions.
        // So now we have RecursiveCheckReturns which checks ALL active return paths from here,
        // which in the case of a recursive function includes both the original entry point and
        // the recursion point, both of which may have callers that need to be marked.
        // Typically, in a non-recursive call situation, this recursive iteration will be equivalent
        // to a simple reverse(tc.scopes), because this path will be the first in the call-graph.
        // See example in tests\errors\recursive_non_local_return.lobster
        // See also reuse code in TypeCheckCallStatic
        auto start_sf = scopes.back().sf;
        auto nretslots = ValWidthMulti(node.sf->returntype, node.sf->returntype->NumValues());
        if (!RecursiveCheckReturns(start_sf, nretslots, node.sf, node))
            ErrorAlways(node, "return from ", Q(node.sf->parent->name), " called out of context");
        return &node;
    }

    Node *Check(MultipleReturn &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TypeCheckList(&node, LT_ANY);
        node.exptype = st.NewTuple(node.children.size());
        for (auto [i, mrc] : enumerate(node.children))
            node.exptype->Set(i, mrc->exptype.get(), mrc->lt);
        node.lt = LT_MULTIPLE;
        return &node;
    }
};

bool Return::Terminal(TypeCheckBase &) const {
    return true;
}

}  // namespace lobster
