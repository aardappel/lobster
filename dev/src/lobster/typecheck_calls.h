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

// Function lookup, overloads, specialization, static/dynamic calls, and dispatch.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckCalls : virtual TypeCheckLocations {
    // Overload-selection scratch space, cleared before entering a selected body.
    // Reuse these, otherwise cause a LOT of allocations.
    vector<Overload *> pickfrom;
    vector<Overload *> matches;

    TypeCheckCalls() {}

    // The signature an overload was declared with. Unlike Signature() this uses
    // the given types, so it is also usable before it has been typechecked.
    string DeclSignature(const Overload &ov) {
        string r = ov.sf->parent->name;
        r += "(";
        for (auto [i, arg] : enumerate(ov.sf->args)) {
            UnTypeRef t = ov.givenargs[i];
            // An arg without a declared type got an implicit generic, which is
            // not something the user wrote, so don't show it as its type.
            if (t->t == V_TYPEVAR && !ov.sf->explicit_generics) t = type_any;
            FormatArg(r, arg.sid->id->name, i, t);
        }
        r += ")";
        return r;
    }

    // For errors in or about the default value of argument `i` of `f`, which `call` left out:
    // what the call gets is a clone of the one written in the declaration, so they are
    // reported on the declaration's line, which by itself doesn't say what call it was for.
    SymbolTable::ResolveContext DefaultArgContext(const Function &f, size_t i, const Node &call) {
        return { .what = "default value of argument", .f = &f,
                 .name = f.overloads[0]->sf->args[i].sid->id->name, .line = &call.line };
    }

    // All overloads of all arity variants of a function, for errors about a
    // call that none of them matched.
    string DeclaredOverloads(Function *ff) {
        string r;
        int n = 0;
        for (auto f = ff; f; f = f->sibf) {
            for (auto ov : f->overloads) {
                if (++n > 8) return r + "\n  (more overloads not shown)";
                r += "\n  declared: " + DeclSignature(*ov) + " (" +
                     parser.lex.Location(ov->declared_at) + ")";
            }
        }
        return r;
    }

    void AmbiguousOverloadError(const List &call_args, const Function &f, const TypeRef &type0,
                                const vector<Overload *> &from) {
        string err = "multiple overloads for `" + f.name +"` match the argument types `(";
        for (size_t a = 0; a < f.nargs(); a++) {
            if (a != 0) err += ", ";
            err += TypeName(a ? call_args.children[a]->exptype : type0);
        }
        err += ")`";
        for (auto ov : from)
               err += "\n  overload: " + Signature(*ov->sf);
        Error(call_args, err);
    }

    bool FreeVarsSameAsCurrent(const SubFunction &sf, bool prespecialize) {
        for (auto &freevar : sf.freevars) {
            auto sid = freevar.sid;
            auto cur = sid->Current();
            FlowItem fi(cur, cur->type, cur->type);
            assert(fi.IsValid());
            auto curtype = UseFlow(fi);
            if (sid != cur || !freevar.spec_type->Equal(*curtype)) {
                (void)prespecialize;
                assert(prespecialize || sid == cur || (sid && cur));
                return false;
            }
            // For the sake of explicit free variables which (unlike lexical free vars) can
            // refer to multiple different variables, we must check if the variable is reachable at all.
            // FIXME: we should really be able to see this without looping if cursid is null?
            if (!IsActive(sid->sf_def)) return false;
        }
        for (auto &fvfi : sf.freevarflowfields) {
            auto curtype = UseFlow(fvfi);
            if (!ConvertsTo(curtype, fvfi.now, CF_NONE)) return false;
        }
        return true;
    }

    // See if returns produced by an existing specialization are compatible with our current
    // context of functions.
    bool CompatibleReturns(const SubFunction &ssf) {
        for (auto &re : ssf.reuse_return_events) {
            auto sf = re.first;
            if (auto isc = ActiveScopeOf(sf->parent)) {
                if (isc->sf->reqret != sf->reqret) return false;
            } else if (!checking_dead_code) {
                // Function not in context. In dead code (see TypeCheckDeadCode), which lacks
                // the caller that would provide one, the return then goes nowhere however it
                // was typechecked (see ReplayReturns), while typechecking it anew would redo
                // all it calls in a context no live call of it has.
                return false;
            }
        }
        // A return from a function none of whose calls was active returned nothing to it (see
        // Return::TypeCheck), so where one is, the return must be typechecked anew. Where none
        // is, a new specialization would come out the same, and a recursive call in it would
        // make another one, without end.
        for (auto f : ssf.reuse_inactive_returns) {
            if (ActiveScopeOf(f)) return false;
        }
        return true;
    }

    // Owning parameters release the argument's borrow before checking (or replaying) the
    // callee's writes. The reference taken here protects it from those writes, including
    // writes in lambdas called by the callee.
    void PrepareCallLifetimes(SubFunction *sf, List &call_args) {
        for (auto [i, c] : enumerate(call_args.children)) {
            if (sf->args[i].sid->CallerLifetime() == LT_KEEP && IsBorrow(c->lt))
                AdjustLifetime(c, LT_KEEP);
        }
    }

    TypeRef TypeCheckMatchingCall(SubFunction *sf, List &call_args, bool static_dispatch,
                                  bool first_dynamic, DispatchEntry *de) {
        STACK_PROFILE;
        // Here we have a SubFunction witch matching specialized types.
        sf->numcallers++;
        auto parent_sf = scopes.empty() ? nullptr : scopes.back().sf;
        for (auto &caller : sf->callers) {
            if (caller.caller == parent_sf && caller.de == de) {
                goto existing_caller;
            }
        }
        sf->callers.push_back(Caller{ parent_sf, de });
        existing_caller:
        Function &f = *sf->parent;
        BindParamAliases(sf, call_args);
        if (static_dispatch || first_dynamic) PrepareCallLifetimes(sf, call_args);
        auto reused = sf->typechecked;
        if (!f.istype) TypeCheckFunctionDef(*sf, call_args);
        // A specialization typechecked for an earlier call has its writes checked against
        // this context now, while the arguments still hold their borrows (which the adjusting
        // below gives up), since it is those a write in the callee may conflict with. Not
        // while it is still being typechecked (a recursive call): its writes are checked
        // against the context of the call that entered it.
        if (reused && !IsActive(sf)) ReplayAssigns(sf, call_args);
        // Finally check all args. We do this after checking the function
        // definition, since SubType below can cause specializations of the current function
        // to be typechecked with strongly typed function value arguments.
        if (static_dispatch || first_dynamic) {
            auto call = Is<Call>(call_args);
            auto first_default = call_args.children.size() - (call ? call->num_defaults : 0);
            for (auto [i, c] : enumerate(call_args.children)) {
                auto &arg = sf->args[i];
                // Check a dynamic dispatch only for the first case, and then skip
                // checking the first arg.
                if (static_dispatch || i) {
                    optional<SymbolTable::ResolveScope> rs;
                    if (i >= first_default) rs.emplace(st, DefaultArgContext(f, i, call_args));
                    SubType(c, arg.spec_type, ArgName(i), f.name);
                }
                // We really don't want to specialize functions on variables, so we simply
                // disallow them. This should happen only infrequently.
                if (arg.spec_type->HasValueType(V_VAR))
                    Error(call_args, "can\'t infer ", Q(ArgName(i)), " argument of call to ",
                          Q(f.name));
                // A new dynamic dispatch must retain the arguments' borrows until every
                // implementation has checked its writes, see TypeCheckCallDispatch.
                if (static_dispatch) {
                    AdjustLifetime(c, arg.sid->CallerLifetime());
                    // This has to happen even to dead args:
                    DecBorrowers(c->lt, call_args);
                }
            }
        }
        // See if this call is recursive:
        if (IsActive(sf)) {
            sf->isrecursivelycalled = true;
            if (sf->returngiventype.Null())
                ErrorAlways(call_args, "recursive function ", Q(sf->parent->name),
                            " must have explicit return type");
        }
        return sf->returntype;
    }

    // How many values a specialization of `sf` returns to a call that wants `reqret`: a
    // function that declares its return type returns no more than it declares, see
    // TypeCheckCallStatic.
    size_t SpecializationReqRet(const SubFunction &sf, size_t reqret) {
        if (sf.returngiventype.Null()) return reqret;
        return std::min(reqret, sf.returngiventype->NumValues());
    }

    // What a call that wants `reqret` values gets from `sf`, which returns `rtype`: the error
    // type for any values `sf` doesn't return, which the call got an error for.
    TypeRef CallReturnType(TypeRef rtype, const SubFunction &sf, size_t reqret) {
        if (sf.reqret >= reqret) return rtype;
        // The return type of a dynamic dispatch is a type variable until its functions have
        // been typechecked, which a recursive call happens before, even when it is going to
        // be no values at all.
        return PadValues(sf.reqret ? rtype : type_void, reqret, LT_KEEP, type_error);
    }

    bool SpecializationIsCompatible(const SubFunction &sf, size_t reqret) {
        return SpecializationReqRet(sf, reqret) == sf.reqret &&
            FreeVarsSameAsCurrent(sf, false) &&
            CompatibleReturns(sf);
    }

    void UnWrapBoth(UnTypeRef &otype, TypeRef &atype) {
        while (otype->Wrapped() && otype->t == atype->t) {
            otype = otype->Element();
            atype = atype->Element();
        }
    }

    void BindTypeVar(UnTypeRef giventype, TypeRef atype,
                     vector<GenericTypeVariable> &generics, GUDT *parent = nullptr) {
        auto otype = giventype;
        UnWrapBoth(otype, atype);
        if (otype->t == V_NIL) {
            otype = otype->Element();
        }
        if (otype->t == V_TYPEVAR) {
            for (auto &gtv : generics) {
                if (gtv.tv == otype->tv && gtv.type.Null()) {
                    gtv.type = atype;
                    break;
                }
            }
        } else if (otype->t == V_UUDT &&
                   IsUDT(atype->t) &&
                   otype->spec_udt->gudt != parent) {  // Avoid recursion!
            // The argument may be of a subtype of the given type, in which case the type
            // variables take their values from the specialization of the given type it
            // derives from.
            for (auto udt = atype->udt; udt; udt = udt->ssuperclass) {
                if (otype->spec_udt->gudt != &udt->g) continue;
                assert(otype->spec_udt->specializers.size() == udt->bound_generics.size());
                for (auto [i, s] : enumerate(otype->spec_udt->specializers)) {
                    BindTypeVar({ s }, udt->bound_generics[i], generics, otype->spec_udt->gudt);
                }
                break;
            }
        }
    }

    // `dispatch_lts` are the argument lifetimes every function of a dynamic dispatch must
    // agree on, see TypeCheckCallDispatch; null for a static call.
    TypeRef TypeCheckCallStatic(SubFunction *&sf, List &call_args, size_t reqret,
                                vector<UnTypeRef> *specializers, Overload &ov,
                                bool static_dispatch, bool first_dynamic,
                                const vector<Lifetime> *dispatch_lts, DispatchEntry *de) {
        STACK_PROFILE;
        Function &f = *sf->parent;
        if (ov.isprivate && ov.declared_at.fileidx != call_args.line.fileidx)
            ErrorAlways(call_args, "call to (partially) private function ", Q(f.name));
        sf = ov.sf;
        // A method of a generic class takes the values of that class's type variables from
        // the type of its receiver, so one that could not be typed leaves them unbound, and
        // resolving the signature below would report an error per type variable on top of
        // the one the receiver already got (SymbolTable::ResolveTypeVars reports directly,
        // so those are not filtered as consequential).
        if (sf->overload->method_of && call_args.children[0]->exptype->IsError())
            return GiveUpCall(call_args);
        // Collect generic type values.
        vector<GenericTypeVariable> generics = sf->generics;
        for (auto &gtv : generics) gtv.type = nullptr;
        if (specializers) {
            if (specializers->size() > generics.size())
                ErrorAlways(call_args, "too many specializers given");
            for (auto [i, type] : enumerate(*specializers)) {
                if (i >= generics.size()) break;
                generics[i].type = st.ResolveTypeVars(type, call_args.line);
            }
        }
        bool has_lambda_args = false;
        for (auto [i, c] : enumerate(call_args.children)) {
            BindTypeVar(sf->overload->givenargs[i], c->exptype, generics);
            if (c->exptype->t == V_FUNCTION) has_lambda_args = true;
        }
        for (auto &gtv : generics) {
            if (gtv.type.Null()) {
                Error(call_args, "cannot implicitly bind type variable ", Q(gtv.tv->name),
                                 " in call to ", Q(f.name), " (argument doesn't match?)");
                gtv.type = type_error;
            }
        }
        // A call that wants more values than the function declares gets the specialization
        // for just those, padded with the error type (see CallReturnType). That is what a
        // specialization of its own would amount to, except that no call like it would find
        // that one again, so a recursive one would make new ones forever.
        auto spec_reqret = SpecializationReqRet(*sf, reqret);
        if (spec_reqret < reqret)
            ErrorAlways(call_args, "returning ", spec_reqret, " values, caller requires ", reqret);
        // Having a lifetime per arg is mostly useful on smaller functions to not get
        // unnecessary refc overhead on the border, especially if they later get inlined.
        // But for really big functions it just risks unnecessary specializations for no gain,
        // so those get one specialization that any caller adjusts to: borrowing where the
        // function allows it, since that costs a caller passing a variable nothing, and a
        // caller passing an owned value the same as an owning parameter would.
        auto single_spec = ov.gbody_node_count > 25;
        // Check if we need to specialize: generic args, free vars and need of retval
        // must match previous calls.
        auto ArgLifetime = [&](const Node *c, const Arg &arg, size_t i) {
            if (dispatch_lts)
                return (*dispatch_lts)[i];
            // An anonymous function may later be passed as a declared function type, which
            // calls every function value the same way: with borrowed arguments, a parameter
            // that needs a reference of its own copying the argument on entry (see SubType
            // for V_FUNCTION). Every specialization of one is called that way from the start,
            // so that passing it as such a type never changes what its callers were
            // typechecked with, see SpecIdent::copy_on_entry.
            if (f.anonymous)
                return ParamOwns(arg, c->exptype) ? LT_KEEP : LT_BORROW;
            if (ParamOwns(arg, c->exptype))
                return LT_KEEP;
            // This is a very special case that tends to happen if we pass a variable to a HOF, and then inside
            // the lambda to that HOF we assign to the same var. To avoid that, check !single_assignment for the
            // variable passed in, AND see if any function values are being passed in.
            // TODO: how does this apply to borrows that are not IdentRef, like Dot?
            if (has_lambda_args && Is<IdentRef>(c) && !Is<IdentRef>(c)->sid->id->single_assignment)
                return LT_KEEP;
            if (single_spec)
                return LT_BORROW;
            // No exceptions hold, it can be whatever lifetime it wants, including borrows.
            return c->lt;
        };
        // Check if any existing specializations match.
        for (sf = ov.sf; sf; sf = sf->next) {
            if (sf->typechecked && !sf->mustspecialize) {
                // Note: we compare only lt, since calling with other borrowed sid
                // should be ok to reuse.
                for (auto [i, c] : enumerate(call_args.children)) {
                    auto &arg = sf->args[i];
                    auto arg_lt = ArgLifetime(c, arg, i);
                    auto unequal_lifetimes = IsBorrow(arg_lt) != IsBorrow(arg.sid->lt);
                    // TODO: we need this check here because arg type may rely on parent
                    // struct (or function) generic, and thus isn't covered by the checking
                    // of sf->generics below. Can this be done more elegantly?
                    auto parent_generic =
                        st.IsGeneric(sf->overload->givenargs[i]) && !c->exptype->Equal(*arg.spec_type);
                    auto incompatible = unequal_lifetimes || parent_generic;
                    if (incompatible)
                        goto fail;
                }
                for (auto [i, gtv] : enumerate(sf->generics)) {
                    if (!gtv.type->Equal(*generics[i].type)) goto fail;
                }
                if (SpecializationIsCompatible(*sf, reqret)) {
                    // This function can be reused.
                    // Make sure to add any freevars this call caused to be
                    // added to its parents also to the current parents, just in case
                    // they're different.
                    // FIXME: this code trying to re-apply effects is not great, as it doesn't work
                    // with recursive functions: at this point where we are re-using a recursive
                    // call, the original call is still half-way being typechecked, so not all
                    // returns have been processed. This can be seen in
                    // tests\errors\recursive_non_local_return.lobster
                    // Where the recursive call to frecur is reused before that function is
                    // marked as having non-local returns, resulting in missing unwinding code.
                    // For that particular problem we fix this by tracker "Caller" per function,
                    // such that when the non-local return happens it knows about all active callers,
                    // not just the current one, see Return::TypeCheck.
                    // But some of the other code below may need similar treatment.
                    LOG_DEBUG("re-using: ", Signature(*sf));
                    CheckFreeVariablesFromFunction(sf);
                    ReplayReturns(sf, call_args);
                    auto rtype = TypeCheckMatchingCall(sf, call_args, static_dispatch,
                                                       first_dynamic, de);
                    return CallReturnType(rtype, *sf, reqret);
                }
                fail:;
            }
        }
        // No match, make new specialization.
        sf = CloneFunction(ov);
        // Now specialize.
        sf->reqret = spec_reqret;
        sf->generics = generics;
        UDT *udt = nullptr;
        if (sf->overload->method_of && IsUDT(call_args.children[0]->exptype->t)) {
            udt = call_args.children[0]->exptype->udt;
        }
        if (udt) {
            st.PushSuperGenerics(udt);
            sf->method_of = udt;
        }
        st.bound_typevars_stack.push_back(sf->generics);
        for (auto [i, c] : enumerate(call_args.children)) {
            auto &arg = sf->args[i];
            arg.sid->lt = ArgLifetime(c, arg, i);
            arg.sid->copy_on_entry = f.anonymous && arg.sid->lt == LT_KEEP;
            arg.spec_type = st.ResolveTypeVars(sf->overload->givenargs[i], call_args.line);
            LOG_DEBUG("arg: ", arg.sid->id->name, ":", TypeName(arg.spec_type));
        }
        // This must be the correct freevar specialization.
        assert(!f.anonymous || sf->freevarchecked);
        assert(!sf->freevars.size());
        LOG_DEBUG("specialization: ", Signature(*sf));
        auto rtype =
            TypeCheckMatchingCall(sf, call_args, static_dispatch, first_dynamic, de);
        if (udt) {
            st.PopSuperGenerics(udt);
        }
        st.bound_typevars_stack.pop_back();
        return CallReturnType(rtype, *sf, reqret);
    }

    TypeRef TypeCheckCallDispatch(UDT &dispatch_udt, SubFunction *&csf, List &call_args,
                                  size_t reqret, vector<UnTypeRef> *specializers,
                                  int &vtable_idx) {
        // FIXME: this is to lock the subudts, since adding to them later would invalidate
        // this dispatch.. would be better to solve ordering problems differently.
        dispatch_udt.subudts_dispatched_where = csf->parent->name;
        Function &f = *csf->parent;
        // We must assume the instance may dynamically be different, so go thru vtable.
        // See if we already have a vtable entry for this type of call.
        for (auto [i, disp] : enumerate(dispatch_udt.dispatch_table)) {
            // FIXME: does this guarantee it find it in the recursive case?
            // TODO: we chould check for a superclass vtable entry also, but chances
            // two levels will be present are low.
            if (disp->sf && disp->sf->method_of == &dispatch_udt && disp->dispatch_root &&
                &f == disp->sf->parent) {
                for (auto [i, c] : enumerate(call_args.children)) {
                    auto &arg = disp->sf->args[i];
                    if (i && !ConvertsTo(c->exptype, arg.spec_type, CF_NONE))
                        goto fail;
                }
                // If this ever fails, that means new types got added during typechecking..
                // which means we'd just have to create a new vtable entry instead, or somehow
                // avoid the new type.
                assert(disp->subudts_size == dispatch_udt.subudts.size());
                // We must check that ALL functions involved are compatible, since some
                // may touch different freevars that the dispatch root doesn't have, such that
                // if they were different means we can't reuse this dispatch.
                for (auto udt : dispatch_udt.subudts) {
                    auto sf = udt->dispatch_table[i]->sf;
                    if (!SpecializationIsCompatible(*sf, reqret))
                        goto fail;
                }
                // We can reuse! As with a new call, owning arguments must be protected
                // before any implementation replays its writes.
                PrepareCallLifetimes(disp->sf, call_args);
                for (auto udt : dispatch_udt.subudts) {
                    auto sf = udt->dispatch_table[i]->sf;
                    LOG_DEBUG("re-using dyndispatch: ", Signature(*sf));
                    if (sf->typechecked) {
                        // If sf is not typechecked here, it means a function before this in
                        // the list has a recursive call.
                        CheckFreeVariablesFromFunction(sf);
                        ReplayReturns(sf, call_args);
                        // The root gets these from TypeCheckMatchingCall below.
                        if (sf != disp->sf) {
                            BindParamAliases(sf, call_args);
                            ReplayAssigns(sf, call_args);
                        }
                    }
                }
                // Type check this as if it is a static dispatch to just the root function.
                TypeCheckMatchingCall(csf = disp->sf, call_args, true, false, disp.get());
                vtable_idx = (int)i;
                if (disp->sf->reqret < reqret)
                    ErrorAlways(call_args, "returning ", disp->sf->reqret,
                                " values, caller requires ", reqret);
                return CallReturnType(disp->returntype, *disp->sf, reqret);
            }
            fail:;
        }
        if (dispatch_udt.subudts.empty()) {
            // DISPATCH_BEFORE_CLASS_DEF 2
            // There is an error for when any classes are defined after a dispatch, but in
            // this case ALL of them are defined after, which needs this specialized error
            // since we have no methods to work on below.
            ErrorAlways(call_args, "dynamic dispatch of ", Q(f.name), " on ",
                        Q(dispatch_udt.name), " before its subclasses have been declared");
            return GiveUpCall(call_args);
        }
        // Must create a new vtable entry.
        // TODO: would be good to search superclass if it has this method also.
        // Probably not super important since dispatching on the "middle" type in a
        // hierarchy will be rare.
        // Find subclasses and max vtable size.
        {
            struct Pick { Overload *ov; bool supcall; };
            vector<Pick> overload_picks;
            // First, for the set of udts part of this dispatch, find the method that would apply.
            for (auto sub : dispatch_udt.subudts) {
                Overload *best = nullptr;
                int bestdist = -1;
                for (auto ov : csf->parent->overloads) {
                    if (ov->method_of) {
                        // We want the method "closest to" this udt in the inheritance chain.
                        // We must search specializations since method_of may be a generic UDT (the
                        // methods will get specialized and typechecked right below here, and
                        // method_of pointing to a specialized udt there.
                        auto sdist = DistanceToSpecializedSuper(ov->method_of, sub);
                        if (sdist >= 0 && (!best || bestdist >= sdist)) {
                            if (bestdist == sdist)
                                ErrorAlways(call_args, "more than implementation of ",
                                            Q(f.name), " applies to ", Q(sub->name),
                                            " in dynamic dispatch");
                            best = ov;
                            bestdist = sdist;
                        }
                    }
                }
                if (!best) {
                    if (!sub->g.is_abstract) {
                        ErrorAlways(call_args, "no implementation for ",
                                    Q(cat(sub->name, ".", csf->parent->name)));
                    } else {
                        // This UDT is unused, so we're ok there not being an implementation
                        // for it.. like e.g. an abstract base class.
                    }
                }
                overload_picks.push_back({ best, bestdist != 0 });
            }
            // Add functions to all vtables.
            auto de = AddDispatchEntries(dispatch_udt, [&](size_t i) {
                return DispatchEntry{ !overload_picks[i].ov ? nullptr : overload_picks[i].ov->sf };
            });
            vtable_idx = de->vtable_idx;
            // We are now going to type check all functions in the vtable for the given
            // call_args, which normally determines the lifetimes of the function args.
            // Problem is, function may change arg lifetimes based on things like internal
            // assignment, and lifetimes must be the same for all (a call adjusts to those of
            // the root), so they are decided here for all of them: a borrow where none of
            // them assigns the parameter (and it is not a struct of references, see
            // ArgLifetime), which any caller can adjust to, and owning otherwise.
            // FIXME: if any of the overloads below contain recursive calls, it may run into
            // issues finding an existing dispatch above? would be good to guarantee..
            // The fact that in subudts the superclass comes first will help avoid problems
            // in many cases.
            vector<Lifetime> dispatch_lts(call_args.children.size(), LT_BORROW);
            for (auto [j, c] : enumerate(call_args.children)) {
                if (c->exptype->t == V_STRUCT_R) dispatch_lts[j] = LT_KEEP;
                for (auto &pick : overload_picks) {
                    if (pick.ov && !pick.ov->sf->args[j].sid->id->single_assignment)
                        dispatch_lts[j] = LT_KEEP;
                }
            }
            de->returntype = st.NewTypeVar();
            // Typecheck all the individual functions.
            SubFunction *last_sf = nullptr;
            bool any_recursive = false;
            for (auto [i, udt] : enumerate(dispatch_udt.subudts)) {
                auto sf = udt->dispatch_table[vtable_idx]->sf;
                // Missing implementation for unused UDT.
                if (!sf)
                    continue;
                if (overload_picks[i].supcall) {
                    // We're using a superclass method. Skip if that superclass is in
                    // the dispatch table since we can then simply reuse that one.
                    // This might not be the case when the superclass is above the dispatch
                    // root and the dispatch root doesn't have its own implementation.
                    // Below we set the sf the superclass picked.
                    bool found_sup_impl = false;
                    for (auto &pick : overload_picks) {
                        if (!pick.supcall && pick.ov == overload_picks[i].ov) {
                            found_sup_impl = true;
                            break;
                        }
                    }
                    if (found_sup_impl)
                        continue;
                }
                call_args.children[0]->exptype = &udt->thistype;
                // FIXME: this has the side effect of giving call_args types relative to the last
                // overload type-checked, which is strictly speaking not correct, but may not
                // matter. Could call TypeCheckCallStatic once more at the end of this loop
                // to fix that?
                // FIXME: return value?
                TypeCheckCallStatic(csf, call_args, reqret, specializers, *overload_picks[i].ov,
                                    false, !last_sf, &dispatch_lts, de);
                sf = csf;
                udt->dispatch_table[vtable_idx]->sf = sf;
                if (sf->isrecursivelycalled) any_recursive = true;
                auto u = sf->returntype;
                if (de->returntype->IsBoundVar()) {
                    // FIXME: can this still happen now that recursive cases use explicit return
                    // types? If not change into assert?
                    if (!ConvertsTo(u, de->returntype, CF_UNIFICATION))
                        Error(*sf->sbody, "dynamic dispatch for ", Q(f.name),
                                          " return value type ", Q(TypeName(sf->returntype)),
                                          " doesn\'t match other case returning ",
                                          Q(TypeName(de->returntype)));
                } else {
                    if (i) {
                        // We have to be able to take the union of all retvals without
                        // coercion, since we're not fixing up any previously typechecked
                        // functions.
                        u = Union(u, de->returntype, "function return type", "other overloads",
                                  CF_NONE, &call_args);
                        // Ensure we didn't accidentally widen the type from a scalar.
                        assert(IsRef(de->returntype->t) || !IsRef(u->t));
                    }
                    de->returntype = u;
                }
                last_sf = sf;
            }
            // Pass 2.
            last_sf = nullptr;
            for (auto [i, udt] : enumerate(dispatch_udt.subudts)) {
                auto sf = udt->dispatch_table[vtable_idx]->sf;
                if (!sf) continue;
                if (any_recursive && sf->returngiventype.Null())
                    ErrorAlways(call_args, "recursive dynamic dispatch of ",
                                Q(sf->parent->name), " must have explicit return type");
                if (overload_picks[i].supcall) {
                    // Now that everything has been typechecked, set the correct sf just
                    // in case the superclass picked a different one.
                    for (auto [j, pick] : enumerate(overload_picks)) {
                        if (!pick.supcall && pick.ov == overload_picks[i].ov) {
                            udt->dispatch_table[vtable_idx]->sf =
                                dispatch_udt.subudts[j]->dispatch_table[vtable_idx]->sf;
                            break;
                        }
                    }
                }
                if (last_sf) {
                    // We do this in pass 2 because otherwise arg types will be unresolved.
                    // FIXME: good to have this check here so it only occurs for functions
                    // participating in the dispatch, but error now appears at the call site!
                    for (auto [j, arg] : enumerate(sf->args)) {
                        if (j && !arg.spec_type->Equal(*last_sf->args[j].spec_type) &&
                            !st.IsGeneric(sf->overload->givenargs[j]))
                            Error(call_args, "argument ", j + 1, " of declaration of ", Q(f.name),
                                          ", type ", Q(TypeName(arg.spec_type)),
                                          " doesn\'t match type of previous declaration: ",
                                          Q(TypeName(last_sf->args[j].spec_type)));
                    }
                }
                last_sf = sf;
            }
            call_args.children[0]->exptype = &dispatch_udt.thistype;
            // A write found only in a later implementation can still need KeepArgAlive.
            // Release the shared call arguments once, after all implementations checked
            // them, using the lifetime convention agreed on by the entire dispatch.
            for (auto [i, c] : enumerate(call_args.children)) {
                AdjustLifetime(c, dispatch_lts[i]);
                DecBorrowers(c->lt, call_args);
            }
        }
        // TypeCheckCallStatic reported any values the call wants that the functions don't
        // declare.
        auto &disp = *dispatch_udt.dispatch_table[vtable_idx];
        return disp.sf ? CallReturnType(disp.returntype, *disp.sf, reqret) : disp.returntype;
    }

    // A call that can't be typechecked: no function gets specialized for it, and it produces
    // the error type, see ErrorNode.
    TypeRef GiveUpCall(List &call_args) {
        pickfrom.clear();
        matches.clear();
        ReleaseChildren(call_args);
        return type_error;
    }

    TypeRef TypeCheckCall(SubFunction *&csf, List &call_args, size_t reqret, int &vtable_idx,
                          vector<UnTypeRef> *specializers, bool super) override {
        STACK_PROFILE;
        Function &f = *csf->parent;
        vtable_idx = -1;
        assert(!f.istype);
        // Check if we need to do dynamic dispatch. We only do this for functions that have a
        // explicit first arg type of a class or a struct in an abstract struct family (not
        // other structs, since they can never dynamically be different from their static
        // type), and only when there is a sub-class that has a method that can be called also.
        UDT *dispatch_udt = nullptr;
        // The class or struct of the receiver, whose superclass a `super` call goes to.
        UDT *receiver_udt = nullptr;
        TypeRef type0;
        if (call_args.Arity()) {
            type0 = call_args.children[0]->exptype;
            if (IsDynamicType(type0)) dispatch_udt = type0->udt;
            if (IsUDT(type0->t)) receiver_udt = type0->udt;
        }
        if (super) {
            // Static dispatch to the superclass's implementation (that there is a
            // superclass was checked by GenericCall::TypeCheck).
            if (receiver_udt && receiver_udt->ssuperclass)
                type0 = &receiver_udt->ssuperclass->thistype;
        } else if (dispatch_udt) {
            // Go thru all other overloads, and see if any of them have this one as superclass.
            for (auto ov : csf->parent->overloads) {
                if (ov->method_of &&
                    DistanceFromSpecializedSub(dispatch_udt, ov->method_of) > 0) {
                    LOG_DEBUG("dynamic dispatch: ", Signature(*ov->sf));
                    return TypeCheckCallDispatch(*dispatch_udt, csf, call_args,
                        reqret, specializers, vtable_idx);
                }
            }
            // Yay there are no sub-class implementations, we can just statically dispatch.
        }
        // Do a static dispatch.
        // if there are overloads, figure out from first arg which to pick,
        // much like dynamic dispatch. Unlike dynamic dispatch, we also include non-class types.
        // TODO: also involve the other arguments for more complex static overloads?
        // FIXME: the use of args[].type here and further downstream only works because
        // we pre-resolve these in the TypeChecker constructor, instead we should use giventypes
        // properly here, and resolve them.
        assert(pickfrom.empty());
        assert(matches.empty());
        pickfrom = f.overloads;
        for (int argidx = 0; ; argidx++) {
            if (pickfrom.size() == 1) {
                // We're done, found unique match.
                auto pick = pickfrom[0];
                pickfrom.clear();
                matches.clear();
                // A `super` call must land on a method of a strict superclass: with nothing
                // else to pick from, this may be the calling method itself.
                if (super && (!receiver_udt || !pick->method_of ||
                              DistanceToSpecializedSuper(pick->method_of, receiver_udt) <= 0)) {
                    ErrorAlways(call_args, "super must be used on a method that has a"
                                           " superclass implementation");
                    return GiveUpCall(call_args);
                }
                LOG_DEBUG("static dispatch: ", Signature(*pick->sf));
                return TypeCheckCallStatic(csf, call_args, reqret, specializers, *pick, true,
                                           false, nullptr, nullptr);
            }
            if ((int)f.nargs() == argidx) {
                // Gotten to the end and we still have multiple matches!
                if (specializers) {
                    // Last ditch effort: remove overloads that don't match the generic params.
                    pickfrom.erase(std::remove_if(pickfrom.begin(), pickfrom.end(),
                                         [&](Overload *ov) {
                        return specializers->size() != ov->sf->generics.size();
                                             }),
                                   pickfrom.end());
                    if (pickfrom.size() == 1) {
                        argidx--;
                        continue;
                    }
                }
                AmbiguousOverloadError(call_args, f, type0, pickfrom);
                return GiveUpCall(call_args);
            }
            // Now filter existing matches into a new set of matches based on current arg.
            matches.clear();
            auto type = argidx ? call_args.children[argidx]->exptype : type0;
            // First see if there is an exact match.
            for (auto ov : pickfrom) {
                if (type->Equal(*ov->givenargs[argidx])) matches.push_back(ov);
            }
            // Then see if there's a match if we'd instantiate a generic UDT arg.
            if (matches.empty() && IsUDT(type->t)) {
                for (auto ov : pickfrom) {
                    auto &arg = ov->sf->overload->givenargs[argidx];  // Want unresolved type.
                    if (arg->t == V_UUDT && arg->spec_udt->gudt == &type->udt->g) {
                        matches.push_back(ov);
                    }
                }
            }
            // Then see if there's a match by subtyping.
            if (matches.empty()) {
                for (auto ov : pickfrom) {
                    auto arg = ov->givenargs[argidx];
                    if (arg->t != V_UUDT || !IsDynamicType(type)) {
                        continue;
                    }
                    auto dist = DistanceToSpecializedSuper(arg->spec_udt->gudt, type->udt);
                    if (dist >= 0) {
                        if (matches.size() == 1) {
                            auto oarg = matches[0]->givenargs[argidx];
                            assert(oarg->t == V_UUDT);
                            auto odist =
                                DistanceToSpecializedSuper(oarg->spec_udt->gudt, type->udt);
                            if (dist < odist) {
                                matches[0] = ov;  // Overwrite with better pick.
                            } else if (odist < dist) {
                                // Keep old one.
                            } else {
                                // Keep both, and hope the next arg disambiguates.
                                matches.push_back(ov);
                            }
                        } else {
                            matches.push_back(ov);
                        }
                    }
                }
            }
            if (matches.empty()) {
                for (auto ov : pickfrom) {
                    auto arg = ov->givenargs[argidx];
                    if (!UnConvertsTo(type, arg, CF_NONE)) {
                        continue;
                    }
                    if (matches.size() == 1 && IsDynamicType(type)) {
                        auto oarg = matches[0]->givenargs[argidx];
                        // Prefer "closest" supertype.
                        auto dist = SuperDistance(arg->udt, type->udt);
                        auto odist = SuperDistance(oarg->udt, type->udt);
                        if (dist < odist) {
                            matches[0] = ov;  // Overwrite with better pick.
                        } else if (odist < dist) {
                            // Keep old one.
                        } else {
                            // Keep both, and hope the next arg disambiguates.
                            matches.push_back(ov);
                        }
                    } else {
                        matches.push_back(ov);
                    }
                }
            }
            // Then see if there's a match if we'd instantiate a fully generic arg.
            if (matches.empty()) {
                for (auto ov : pickfrom) {
                    auto arg = ov->sf->overload->givenargs[argidx];  // Want unresolved type.
                    if (arg->t == V_TYPEVAR) { matches.push_back(ov); }
                }
            }
            // If the call has specializers, we should see bind those and see if they
            // uniquely identify an overload, e.g. for a [T] arg where T is now bound, or other
            // cases where the trivial case of T above doesn't apply.
            if (matches.empty() && specializers && !specializers->empty()) {
                // FIXME: This does double the resolving work since it is also done during
                // TypeCheckCallStatic. We could cache this work, but its not needed for
                // all callers, so may not be worth it.
                vector<GenericTypeVariable> generics(specializers->size());
                for (auto ov : pickfrom) {
                    if (generics.size() != ov->sf->generics.size()) continue;
                    for (auto [i, gtv] : enumerate(generics)) {
                        gtv.tv = ov->sf->generics[i].tv;
                        gtv.type = st.ResolveTypeVars(specializers->at(i), call_args.line);
                    }
                    st.bound_typevars_stack.push_back(generics);
                    auto arg = st.ResolveTypeVars(ov->sf->overload->givenargs[argidx], call_args.line);
                    st.bound_typevars_stack.pop_back();
                    // TODO: Should we instead do ConvertsTo here?
                    if (type->Equal(*arg)) {
                        matches.push_back(ov);
                    }
                }
            }
            // Then finally try with coercion.
            if (matches.empty()) {
                for (auto ov : pickfrom) {
                    if (UnConvertsTo(type, ov->givenargs[argidx], CF_COERCIONS)) {
                        matches.push_back(ov);
                    }
                }
            }
            // Empty set: current arg can't select with any of the above methods.
            if (matches.empty()) {
                Error(call_args, "no overloads apply for ", Q(f.name), " with ", ArgName(argidx),
                      " argument type ", Q(TypeName(type)));
                return GiveUpCall(call_args);
            }
            // We still have multiple matches that apply, so let the next arg(s) decide.
            pickfrom = matches;
        }
    }

    SubFunction *PreSpecializeFunction(SubFunction *hsf) {
        // Don't pre-specialize named functions, because this is not their call-site.
        if (!hsf->parent->anonymous) return hsf;
        assert(hsf->parent->overloads.size() == 1);
        hsf = hsf->parent->overloads[0]->sf;
        auto sf = hsf;
        if (sf->freevarchecked) {
            // See if there's an existing match.
            for (; sf; sf = sf->next) if (sf->freevarchecked) {
                if (FreeVarsSameAsCurrent(*sf, true)) return sf;
            }
            sf = CloneFunction(*hsf->parent->overloads[0]);
        } else {
            // First time this function has ever been touched.
            sf->freevarchecked = true;
        }
        assert(!sf->freevars.size());
        // Output without arg types, since those are yet to be overwritten.
        LOG_DEBUG("pre-specialization: ", SignatureWithFreeVars(*sf, nullptr));
        return sf;
    }

    Node *TypeCheckDynCall(DynCall *dc, size_t reqret) {
        UpdateCurrentSid(dc->sid);
        CheckFreeVariable(*dc->sid);
        auto ftype = dc->sid->type;
        // The arguments still get typechecked when the call can't be, for the errors in them.
        auto give_up = [&]() {
            TypeCheckList(dc, LT_ANY);
            ReleaseChildren(*dc);
            return ErrorNode(*dc);
        };
        if (!ftype->IsFunction()) {
            if (!ftype->IsError())
                Error(*dc, "dynamic function call value doesn\'t have a function type ",
                      Q(TypeName(ftype)));
            return give_up();
        }
        // All dynamic calls can be statically typechecked.
        auto sf = ftype->sf;
        if (dc->Arity() < sf->parent->nargs()) {
            ErrorAlways(*dc, "function value called with too few arguments");
            return give_up();
        }
        while (dc->Arity() > sf->parent->nargs()) {
            // HOFs are allowed to supply more args than the lambda needs.
            // TODO: This is somewhat odd, since it may throw away side effects. Then
            // again, this "feature" is quite similar to specifying default arguments,
            // so it being dead code should not be that surprising?
            delete dc->children.back();
            dc->children.pop_back();
        }
        TypeCheckList(dc, LT_ANY);
        if (sf->parent->istype) {
            // Function types are always fully typed.
            // All calls thru this type must have same lifetimes, so we fix it to LT_BORROW.
            dc->exptype = TypeCheckMatchingCall(sf, *dc, true, false, nullptr);
            dc->lt = LT_KEEP;
            dc->sf = sf;
            return dc;
        } else {
            auto c = make_unique<Call>(dc->line, sf);
            c->children.append(dc->children.data(), dc->children.size());
            dc->children.clear();
            c->exptype =
                TypeCheckCallStatic(sf, *c, reqret, nullptr, *sf->parent->overloads[0], true, false, nullptr, nullptr);
            c->lt = LT_KEEP;
            c->sf = sf;
            delete dc;
            return c.release();
        }
    }

    Node *Check(GenericCall &node, size_t reqret, TypeRef /*parent_bound*/) {
        STACK_PROFILE;
        // Here we decide which of Dot / Call / NativeCall this call should be transformed into,
        // from the candidates the declchecker resolved at our lexical position.
        st.current_namespace = node.ns;
        auto nf = node.cand_native;
        auto fld = node.cand_field;
        auto ff = node.cand_function;
        // A variable holding a function value is what gets called when there is one in scope and
        // no function of the name is declared in a deeper scope: a function opens a scope of its
        // own, so one declared in the same block as the variable counts as deeper and wins. A
        // builtin wins over both.
        if (node.cand_var && !nf && (!ff || node.cand_var->id->scopelevel >= ff->scopelevel)) {
            if (node.super) ErrorAlways(node, "super must precede function call");
            // This node stays in the tree until the new one has typechecked, since an error
            // thrown from that leaves the tree to be deleted with this node in it.
            unique_ptr<DynCall> dc(new DynCall(node.line, nullptr, node.cand_var));
            dc->children = node.children;
            node.children.clear();
            auto r = TypeCheckDynCall(dc.get(), reqret);
            dc.release();
            delete &node;
            return r;
        }
        if (ff && ff->istype) {
            Error(node, "can\'t call function type ", Q(ff->name));
            TypeCheckList(&node, LT_KEEP);
            return ErrorNode(node);
        }
        // Whether the name was reported as not referring to anything callable from here, by the
        // declchecker (which leaves a call without any candidates) or below.
        auto unknown_reported = !ff && !nf && !fld && !node.cand_var && !node.cand_nonlexical;
        if (!ff && node.cand_nonlexical) {
            // The name only refers to function(s) not lexically visible here
            // ("functions as environments"): callable while their enclosing
            // function is active, innermost active scope first, which mirrors
            // how their free variables work.
            // Declared in a scope this call is lexically inside of: then lexical
            // resolution already had jurisdiction and did not find it, so it sits
            // in a block that does not contain this call (a different branch of an
            // if, say), and picking it here would make blocks that can never both
            // run share a function.
            auto encloses_this_call = [&](Overload *decl_scope) {
                for (auto ov = scopes.back().sf->overload; ov; ov = ov->sf->lexical_parent)
                    if (ov == decl_scope) return true;
                return false;
            };
            bool out_of_block = false;
            auto find_active = [&](string_view fname) -> Function * {
                auto it = st.functions_by_name.find(fname);
                if (it == st.functions_by_name.end()) return nullptr;
                for (auto &sc : reverse(scopes)) {
                    for (auto f : it->second) {
                        for (auto ov : f->overloads) {
                            if (ov->sf->lexical_parent != sc.sf->overload) continue;
                            if (encloses_this_call(ov->sf->lexical_parent)) {
                                out_of_block = true;
                                continue;
                            }
                            return f;
                        }
                    }
                }
                return nullptr;
            };
            ff = SymbolTable::LookupNS(node.name, node.ns, find_active);
            if (!ff && out_of_block) {
                Error(node, "function ", Q(node.name), " is declared in a block that does not"
                                " contain this call (a different branch of an ", Q("if"),
                                ", for instance): a local function is only visible in the block"
                                " it is declared in");
                unknown_reported = true;
            }
        }
        // We first typecheck the children, because we want to at least look at arg 1 to decide
        // what to call. But this doesn't allow an accurate parent_bound, so we only specify
        // one if it looks unambiguous.
        arglists.push_back(&node);
        for (auto [i, c] : enumerate(node.children)) {
            TypeRef parent_bound;
            // Only if the name seems to refer to 1 thing.
            if (nf && !ff && !fld) {
                if (i < nf->args.size() && !nf->overloads) {
                    auto &arg = nf->args[i];
                    parent_bound = ActualBuiltinType(arg.vttype, arg.flags, type_undefined, nf, true,
                                                        i + 1, node);
                }
            } else if (ff && !nf && !fld) {
                // For now only functions that have a single definition.
                if (!ff->sibf && ff->overloads.size() == 1) {
                    auto sf = ff->overloads[0]->sf;
                    if (i < sf->overload->givenargs.size() && sf->overload->givenargs[i]->IsConcrete()) {
                        // This function is not typechecked, so this could be a generic type, but that
                        // is ok for the current use of parent_bound.
                        parent_bound = sf->overload->givenargs[i]->Resolved();
                    }
                }
            }
            TT(c, 1, LT_ANY, parent_bound);
        }
        arglists.pop_back();
        TypeRef type;
        UDT *udt = nullptr;
        UDT *niludt = nullptr;
        if (node.children.size()) {
            type = node.children[0]->exptype;
            if (IsUDT(type->t)) udt = type->udt;
            else if(type->t == V_NIL && IsUDT(type->sub->t)) niludt = type->sub->udt;
        }
        Node *r = nullptr;
        auto sup_err = [&]() {
            if (node.super) {
                ErrorAlways(node,
                               "super must be used on a method that has a superclass implementation");
            }
        };
        // This node stands in for a call nothing applies to, see TypeChecker::ErrorNode: the
        // arguments give up what they borrow, since no call is going to take them.
        auto give_up = [&]() {
            ReleaseChildren(node);
            return ErrorNode(node);
        };
        auto likely_field_access = fld && node.fromdot && node.noparens;
        if (likely_field_access && udt && udt->g.Has(fld) >= 0) {
            unique_ptr<Dot> dot(new Dot(fld, node));
            node.children.clear();
            sup_err();
            r = dot->TypeCheck(ASTChecker(), reqret, {});
            dot.release();
        } else if (likely_field_access && niludt && niludt->g.Has(fld) >= 0) {
            // Specialized error for nil deref, since if we don't, it will try and interpret this as a function call with a nil arg.
            Error(node, "dereferencing nillable type: ", Q(TypeName(type)),
                     DemotionNote(*node.children[0]));
            return give_up();
        } else {
            // A function or builtin call. Selection is on receiver type first,
            // then arity:
            // 1. Determine which function variants (sibf chain) could complete
            //    with the given args, allowing insertion of an implicit :: self
            //    arg and default args.
            // 2. If there is a receiver (arg 0 is a class/struct), the variant
            //    with the closest matching first arg wins (a generic first arg
            //    matches any receiver, at lower priority than all concrete
            //    matches); ties and the no-receiver case go to the variant with
            //    the most args, so an insertable self arg takes priority over
            //    smaller variants.
            // 3. The builtin applies unless the selected variant matches the
            //    receiver, or is of matching arity with more args than the
            //    builtin.
            auto nargs = node.children.size();
            // The closest overload of `f` whose first arg is declared withtype
            // (::) and is a (specialized) superclass of `in_udt`: such overloads
            // can receive their first arg implicitly.
            auto best_withtype_overload = [&](Function *f, UDT *in_udt) -> SubFunction * {
                int best_superdist = INT_MAX;
                SubFunction *bsf = nullptr;
                for (auto ov : f->overloads) {
                    auto gudt0 = GetGUDTAny(ov->givenargs[0]);
                    if (gudt0 && ov->sf->args[0].sid->withtype) {
                        auto superdist = DistanceToSpecializedSuper(gudt0, in_udt);
                        if (superdist >= 0 && superdist < best_superdist) {
                            best_superdist = superdist;
                            bsf = ov->sf;
                        }
                    }
                }
                return bsf;
            };
            // The withstack entry providing an implicit :: self arg for `f`, if
            // any. We go down the withstack but skip items that don't correspond
            // to lexical order for cases where
            // withcontext1 -> withcontext2 -> lambdaincontext1,
            // or to simply not use withstack items of unrelated callers.
            auto find_self_arg = [&](Function *f) -> pair<SymbolTable::WithStackElem *, SubFunction *> {
                Overload *lex_ov = scopes.back().sf->overload;
                for (auto &wse : reverse(st.withstack)) {
                    bool in_lex_scope = false;
                    for (auto lov = lex_ov; lov; lov = lov->sf->lexical_parent) {
                        if (lov == wse.sf->overload) {
                            in_lex_scope = true;
                            break;
                        }
                    }
                    if (!in_lex_scope || !wse.id) continue;
                    auto wsf = best_withtype_overload(f, wse.udt_tc);
                    if (wsf) return { &wse, wsf };
                }
                return { nullptr, nullptr };
            };
            // Receiver match: smallest superclass distance of any overload's
            // first arg, 998 for a generic first arg (matches anything, beaten
            // by any concrete match), -1 for no match. Also gives the overload,
            // as static dispatch root.
            auto best_receiver_overload = [&](Function *f) -> pair<SubFunction *, int> {
                int udist = 999;
                SubFunction *bsf = nullptr;
                for (auto ov : f->overloads) {
                    auto ti = ov->sf->overload->givenargs[0];
                    auto dist = IsUDT(ti->t)
                        ? SuperDistance(ti->udt, udt)
                        : ti->t == V_UUDT
                            ? DistanceToSpecializedSuper(ti->spec_udt->gudt, udt)
                            : st.IsGeneric(ti) ? 998 : -1;
                    if (dist >= 0 && dist < udist) {
                        bsf = ov->sf;
                        udist = dist;
                        if (dist == 0) break;
                    }
                }
                return { bsf, bsf ? udist : -1 };
            };
            struct Variant {
                Function *f;
                bool needs_self;
                SubFunction *rsf;  // Receiver-matched overload, if any.
                int rdist;
            };
            small_vector<Variant, 4> viable;
            for (auto f = ff; f; f = f->sibf) {
                if (f->nargs() < nargs) continue;
                auto n2 = nargs;
                bool needs_self = false;
                if (n2 < f->nargs() && !node.fromdot && n2 + 1 >= f->FirstDefaultArg() &&
                    find_self_arg(f).first) {
                    n2++;
                    needs_self = true;
                }
                if (n2 < f->nargs() && n2 >= f->FirstDefaultArg()) n2 = f->nargs();
                if (n2 != f->nargs()) continue;
                auto [rsf, rdist] =
                    udt && f->nargs() ? best_receiver_overload(f) : pair<SubFunction *, int>{ nullptr, -1 };
                viable.push_back({ f, needs_self, rsf, rdist });
            }
            Variant *sel = nullptr;
            for (auto &v : viable) {
                if (!sel) sel = &v;
                else if (v.rdist >= 0 && (sel->rdist < 0 || v.rdist < sel->rdist)) sel = &v;
            }
            auto prefer_ff = sel && nf &&
                             (sel->rdist >= 0 && sel->rdist < 998
                                  ? true
                                  : sel->f->nargs() == nargs && sel->f->nargs() > nf->args.size());
            if (nf && !prefer_ff) {
                unique_ptr<NativeCall> nc(new NativeCall(nf, node.line));
                nc->children = node.children;
                node.children.clear();
                sup_err();
                r = nc->TypeCheck(ASTChecker(), reqret, {});
                nc.release();
            } else if (sel) {
                auto f = sel->f;
                SubFunction *usf = sel->rsf;
                if (sel->needs_self) {
                    auto [wse, wsf] = find_self_arg(f);
                    assert(wse);
                    usf = wsf;
                    auto self = wse->Object(node.line);
                    node.children.insert(0, self);
                    TT(node.children[0], 1, LT_ANY);
                    nargs++;
                }
                auto num_defaults = f->nargs() - nargs;
                if (nargs < f->nargs()) {
                    for (size_t i = nargs; i < f->nargs(); i++) {
                        // In the caller's context, where the type variables of the function
                        // are not bound, so one in it can't be resolved.
                        SymbolTable::ResolveScope rs(st, DefaultArgContext(*f, i, node));
                        node.children.push_back(f->default_args[i - f->FirstDefaultArg()]->Clone(true));
                        TT(node.children.back(), 1, LT_ANY);
                        nargs++;
                    }
                }
                assert(nargs == f->nargs());
                if (!usf || !usf->overload->method_of || usf->overload->method_of->gsuperclass->t == V_UNDEFINED)
                    sup_err();
                unique_ptr<Call> fc(new Call(node, usf && usf->parent == f ? usf : f->overloads[0]->sf));
                fc->children = node.children;
                fc->num_defaults = num_defaults;
                node.children.clear();
                r = fc->TypeCheck(ASTChecker(), reqret, {});
                fc.release();
            } else if (ff) {
                string err = cat("no version of function ", Q(node.name), " takes ", nargs,
                                 nargs == 1 ? " argument" : " arguments");
                if (udt) {
                    err += cat(" with ", Q(TypeName(type)), " as first argument");
                } else if (!node.fromdot) {
                    // A call without a receiver inside a method can have a `this`
                    // inserted as first arg, so name the types that was tried with.
                    string selftypes;
                    Overload *lex_ov = scopes.back().sf->overload;
                    for (auto &wse : reverse(st.withstack)) {
                        if (!wse.id || !wse.udt_tc) continue;
                        for (auto lov = lex_ov; lov; lov = lov->sf->lexical_parent) {
                            if (lov != wse.sf->overload) continue;
                            auto n = Q(wse.udt_tc->name);
                            if (selftypes.find(n) == string::npos)
                                append(selftypes, selftypes.empty() ? "" : " or ", n);
                            break;
                        }
                    }
                    if (!selftypes.empty())
                        err += cat(", or ", nargs + 1, " with ", selftypes,
                                   " as implicit first argument");
                }
                err += DeclaredOverloads(ff);
                ErrorAlways(node, err);
                return give_up();
            } else {
                if (fld && node.fromdot && node.noparens) {
                    Error(node, "type ", Q(TypeName(type)), " does not have field ", Q(fld->name),
                             DemotionNote(*node.children[0], fld));
                } else if (checking_dead_code && node.cand_nonlexical) {
                    // An env-function call: only valid with an active caller.
                    ReleaseChildren(node);
                    return SkipDeadCode(node);
                } else if (!unknown_reported) {
                    ErrorAlways(node, "unknown field/function reference ", Q(node.name));
                }
                return give_up();
            }
        }
        node.children.clear();
        delete &node;
        return r;
    }

    Node *Check(Call &node, size_t reqret, TypeRef /*parent_bound*/) {
        STACK_PROFILE;
        // The arguments were typechecked by the GenericCall this came from, and the root call has
        // none.
        assert(node.children.empty() || node.children[0]->exptype->t != V_UNDEFINED);
        node.sf = PreSpecializeFunction(node.sf);
        node.exptype = TypeCheckCall(node.sf, node, reqret, node.vtable_idx, &node.specializers, node.super);
        node.failed = node.exptype->IsError();
        node.lt = LT_KEEP;
        return &node;
    }

    Node *Check(FunRef &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.sf = PreSpecializeFunction(node.sf);
        if (node.sf->parent->istype) {
            for (auto [i, arg] : enumerate(node.sf->args)) {
                arg.spec_type = st.ResolveTypeVars(node.sf->overload->givenargs[i], node.line);
            }
            node.sf->returntype = st.ResolveTypeVars(node.sf->returngiventype, node.line);
        }
        node.exptype = &node.sf->thistype;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(DynCall &node, size_t reqret, TypeRef /*parent_bound*/) {
        return TypeCheckDynCall(&node, reqret);
    }
};

bool Call::Terminal(TypeCheckBase &tc) const {
    // Have to be conservative for recursive calls since we're not done typechecking it.
    if (failed ||
        sf->isrecursivelycalled ||
        sf->method_of ||
        sf->parent->istype) return false;
    if (!sf->num_returns) return true;  // The minimum 1 return is apparently returning out of it.
    if (sf->num_returns == 1) {
        auto ret = AssertIs<Return>(sf->sbody->children.back());
        if (ret->sf == sf) {
            return ret->child->Terminal(tc);
        } else {
            // This is a "return from", which means the real return is elsewhere, and thus
            // this call may not be terminal.
        }
    }
    // TODO: could also check num_returns > 1, but then have to scan all children.
    return false;
}

}  // namespace lobster
