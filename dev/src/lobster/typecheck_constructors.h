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

// Type values, literals, constructors, field defaults, and UDT completion.
// This topic depends only on TypeCheckBase.

namespace lobster {

struct TypeCheckConstructors : virtual TypeCheckBase {
    // UDTs whose field default values are being type checked, to detect recursion.
    vector<UDT *> udts_in_progress;

    TypeCheckConstructors() {}

    // For errors in or about the default value of field `i` of `udt`, which `constructor` left
    // out: what the constructor gets is a clone of the one written in the declaration, so
    // they are reported on the declaration's line, which by itself doesn't say what
    // constructor it was for.
    SymbolTable::ResolveContext DefaultFieldContext(const UDT &udt, size_t i,
                                                    const Node &constructor) {
        return { .what = "default value of field", .udt = &udt,
                 .name = udt.g.fields[i].id->name, .line = &constructor.line };
    }

    // Resolve which fields of `g` the (tagged) initializers of `ac` belong
    // to, fill in defaults, check for missing fields, and return the
    // equivalent ObjectConstructor (consuming `ac`). When report_errors is
    // off, returns null on any failure instead, leaving `ac` unchanged.
    ObjectConstructor *ResolveAutoConstructor(AutoConstructor *ac, GUDT *g, UnTypeRef ctype,
                                              bool report_errors) {
        node_small_vector exps(g->fields.size(), nullptr);
        for (auto [i, c] : enumerate(ac->children)) {
            auto tag = ac->tags[i];
            if (tag) {
                auto field = g->Has(tag);
                if (field < 0) {
                    if (report_errors)
                        Error(*ac, "type ", Q(g->name), " does not have field ", Q(tag->name));
                    return nullptr;
                }
                if (exps[field]) {
                    if (report_errors)
                        Error(*ac, "field ", Q(tag->name), " initialized twice");
                    return nullptr;
                }
                exps[field] = c;
            } else {
                // An initializer without a tag. Find first field without a
                // default thats not set yet.
                for (size_t j = 0; j < exps.size(); j++) {
                    if (!exps[j] && !g->fields[j].gdefaultval) {
                        exps[j] = c;
                        goto done;
                    }
                }
                if (report_errors) Error(*ac, "too many initializers for ", Q(g->name));
                return nullptr;
                done:;
            }
        }
        for (auto [i, e] : enumerate(exps)) {
            if (e) continue;
            if (!g->fields[i].gdefaultval) {
                if (report_errors)
                    Error(*ac, "field ", Q(g->fields[i].id->name), " not initialized");
                return nullptr;
            }
            // If this type's own defaults are being type checked right now,
            // cloning them in here could recursively construct it and never
            // terminate.
            for (auto u : udts_in_progress) {
                if (&u->g == g) {
                    if (report_errors)
                        Error(*ac, "default value of field ", Q(g->fields[i].id->name),
                                   " recursively constructs ", Q(g->name));
                    return nullptr;
                }
            }
        }
        // Now that resolution can't fail anymore, transfer the children (but
        // not ownership of `ac` itself: it may still sit in the tree, and if
        // typechecking the result throws, it must be the one valid node
        // there).
        auto constructor = new ObjectConstructor(ac->line, ctype);
        for (auto [i, e] : enumerate(exps)) {
            if (e) constructor->Add(e);
            else constructor->AddDefault(g->fields[i].gdefaultval->Clone(true));
        }
        ac->children.clear();
        return constructor;
    }

    // A default value that (transitively, thru the defaults of whatever it
    // constructs) constructs its own type again would expand forever when
    // cloned in at construction sites, so must be an error regardless of how
    // the field got its type. Conservative: assumes all defaults of a
    // constructed type apply, even for fields given explicitly.
    // Returns whether it does, in which case it was reported.
    bool CheckRecursiveDefault(GUDT *origin, string_view fname, Node *n, set<GUDT *> &visited,
                               const Node &errn) {
        GUDT *g = nullptr;
        if (auto ac = Is<AutoConstructor>(n)) {
            if (!ac->giventype.Null()) g = GetGUDTAny(ac->giventype);
        } else if (auto oc = Is<ObjectConstructor>(n)) {
            g = GetGUDTAny(oc->giventype);
        }
        if (g) {
            if (g == origin) {
                Error(errn, "default value of field ", Q(fname), " recursively constructs ",
                            Q(origin->name));
                return true;
            }
            if (visited.insert(g).second) {
                for (auto &f : g->fields) {
                    if (f.gdefaultval &&
                        CheckRecursiveDefault(origin, fname, f.gdefaultval.get(), visited, errn))
                        return true;
                }
            }
        }
        for (size_t i = 0; i < n->Arity(); i++) {
            if (CheckRecursiveDefault(origin, fname, n->Children()[i], visited, errn))
                return true;
        }
        return false;
    }

    // Complete a UDT: typecheck field defaults (which may derive still
    // unknown field types), register late specializations in the inheritance
    // forest, and compute sizes. Idempotent, and called on demand by every
    // consumer that needs any of that, so the order in which code gets
    // typechecked cannot lead to observing partial state.
    void EnsureUDTChecked(UDT &udt, const Node &errn) override {
        if (udt.state == UDTState::CHECKED) return;
        // Already being checked further up the stack (recursive types):
        // leave completion to that invocation. A consumer that needs the
        // defaults of an in-progress UDT errors via udts_in_progress.
        for (auto u : udts_in_progress) if (u == &udt) return;
        assert(udt.state == UDTState::FIELDS_RESOLVED);
        // A superclass must be complete first: field defaults get checked
        // against superclass field types below, and dispatch code relies on
        // superclasses being complete.
        if (udt.ssuperclass) EnsureUDTChecked(*udt.ssuperclass, errn);
        udts_in_progress.push_back(&udt);
        // The stored defaultval gets its constant values extracted by codegen
        // for the VM (de)serializers, which needs the resolved constructor
        // shape, not the parsed one.
        auto clone_default = [&](Node *gdefaultval) {
            auto dv = gdefaultval->Clone(true);
            if (auto ac = Is<AutoConstructor>(dv)) {
                if (!ac->giventype.Null()) {
                    if (auto g = GetGUDTAny(ac->giventype)) {
                        auto oc = ResolveAutoConstructor(ac, g, ac->giventype, false);
                        if (oc) {
                            delete ac;
                            dv = oc;
                        }
                    }
                }
            }
            return dv;
        };
        // Give a type for fields that don't have one specified.
        for (auto [i, sfield] : enumerate(udt.sfields)) {
            auto &f = udt.g.fields[i];
            if (!f.gdefaultval) {
                continue;
            }
            set<GUDT *> visited;
            if (CheckRecursiveDefault(&udt.g, f.id->name, f.gdefaultval.get(), visited, errn)) {
                // Every construction site would otherwise clone the recursion in.
                auto line = f.gdefaultval->line;
                // Replace only this field's default; inherited declarations can still
                // hold the original expression and recover independently.
                f.gdefaultval.reset(new ErrorValue(line));
            }
            // The default is a clone of one written in the declaration, so a type in it
            // that fails to resolve gets reported on the declaration's line, which by
            // itself doesn't say which specialization it was being resolved for.
            SymbolTable::ResolveScope rs(
                st, { .what = "default value of field", .udt = &udt, .name = f.id->name });
            sfield.defaultval = clone_default(f.gdefaultval.get());
            if (!sfield.type.Null()) {
                // Type was specified explicitly or CFType succeeded, we are done.
                sfield.defaultval->exptype = sfield.type;
                continue;
            }
            // With this specialization's bindings in scope, so SimpleType can
            // resolve type variables in e.g. []::T defaults.
            st.bound_typevars_stack.push_back(udt.GetBoundGenerics());
            st.PushSuperGenerics(udt.ssuperclass);
            auto simple_type = sfield.defaultval->SimpleType(st);
            st.PopSuperGenerics(udt.ssuperclass);
            st.bound_typevars_stack.pop_back();
            if (simple_type.Null()) {
                // Track how often field types can only be derived by fully
                // typechecking the initializer (with --debug), since that is
                // the one part of UDT resolution that can pull in arbitrary
                // code, and a candidate for requiring explicit types instead.
                LOG_DEBUG("field type from full typecheck: ", udt.name, ".", f.id->name);
                // TODO: put in this error once we're confident about SimpleType being conservatively correct.
                //Error(errn, "non-trivial default value for ", Q(udt.g.fields[i].id->name),
                //      " requires explicit field type");
                // FIXME: would be good to not call TT here generically but instead rely
                // on CFType entirely, just in case TT has a side effect on type
                // checking, especially function calls, whose "return from" may fail here.
                // Sadly that is not easy given the amount of type-checking code this
                // already relies on.
                st.PushSuperGenerics(&udt);
                TT(sfield.defaultval, 1, LT_ANY);
                st.PopSuperGenerics(&udt);
                DecBorrowers(sfield.defaultval->lt, errn);
                // FIXME: because the above may do things like insert coercions etc in exp,
                // we have to undo that here.
                auto n = clone_default(f.gdefaultval.get());
                n->exptype = sfield.defaultval->exptype;  // FIXME: even safer if this was not set.
                delete sfield.defaultval;
                sfield.defaultval = n;
            } else {
                LOG_DEBUG("field type via SimpleType: ", udt.name, ".", f.id->name);
                sfield.defaultval->exptype = simple_type;
            }
            sfield.defaultval->lt = LT_UNDEF;
            sfield.type = sfield.defaultval->exptype;
            //  Here we force a check against types of this field in superclasses.
            //  This is necessary because each of these defaultvals have been typechecked
            //  independently, possibly containing V_VAR instances (for e.g. []) that if
            //  we don't unify them they could get bound to different types by code,
            //  causing incompatible fields.
            //  We skip generic supers because they can legitimately allow different types
            //  for fields.
            // TODO: it is possible this is not necessary anymore because we now typecheck
            // a defaultval per specialization?
            for (auto sudt = udt.ssuperclass; sudt; sudt = sudt->ssuperclass) {
                if (i >= sudt->sfields.size()) break;
                if (!ConvertsTo(sfield.defaultval->exptype, sudt->sfields[i].type,
                                ConvertFlags(CF_EXACTTYPE | CF_UNIFICATION)))
                    Error(errn, "Field ", Q(f.id->name), " has type ",
                            Q(TypeName(sfield.defaultval->exptype)),
                            " which is incompatible with the superclass type of ",
                            Q(TypeName(sudt->sfields[i].type)));
            }
        }
        // The declchecker registered all specializations known after parsing
        // in the inheritance forest; this can only be a specialization that
        // got created during typechecking (of a generic type).
        if (!udt.in_forest) {
            // Dead code never runs, so no object of a class that only it creates can reach
            // a dispatch. Which of the two it gets to first is also just the order
            // TypeCheckDeadCode goes in: a function that dispatches may have been checked
            // by itself before the dead function that creates the class and then calls it.
            if (!checking_dead_code) {
                for (auto u = &udt; u; u = u->ssuperclass) {
                    if (!u->subudts_dispatched_where.empty()) {
                        // A dispatch on a superclass has already been typechecked, so
                        // its vtables can no longer be extended with this new
                        // specialization. Without this error, the VM could run into
                        // empty vtable entries.
                        ErrorAlways(errn, "class ", Q(udt.name),
                                    " already used in dynamic dispatch of ",
                                    Q(u->subudts_dispatched_where), " on ", Q(u->name),
                                    " before it has been declared");
                    }
                }
            }
            // Also after the error: a switch or dispatch on this class itself needs it
            // among its own subudts.
            st.RegisterSubUDT(&udt);
        }
        // Inline struct fields contribute their slots to our size, so they
        // must be complete first.
        for (auto &sfield : udt.sfields) {
            if (!sfield.type.Null() && IsStruct(sfield.type->t))
                EnsureUDTChecked(*sfield.type->udt, errn);
        }
        // The layout of an abstract struct family is decided over all its members (see
        // SymbolTable::LayoutFamily), which needs their field types, so complete them first.
        // A member whose own defaults are being typechecked further up the stack (which
        // constructs this one) can still have fields with types to be inferred from them,
        // and can't be laid out without.
        if (udt.family_root) {
            for (auto m : st.udttable) {
                if (m->family_root != udt.family_root) continue;
                if (m != &udt) EnsureUDTChecked(*m, errn);
                for (auto [i, sfield] : enumerate(m->sfields)) {
                    if (!sfield.type.Null()) continue;
                    ErrorAlways(errn, "field ", Q(m->g.fields[i].id->name), " of ", Q(m->name),
                                      " must be given an explicit type, since ", Q(m->name),
                                      " is part of abstract struct ", Q(udt.family_root->name));
                }
            }
        }
        if (!udt.ComputeSizes(st)) {
            ErrorAlways(errn, cat("struct ", Q(udt.name), " cannot be self-referential"));
        }
        udts_in_progress.pop_back();
        udt.state = UDTState::CHECKED;
    }

    Node *Check(IntConstant &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = node.from ? &node.from->e->thistype : type_int;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(FloatConstant &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = type_float;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(StringConstant &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = type_string;
        // A constant is an object of the generated code, which holds a reference to it for the
        // length of the program, so it can be borrowed: no keepvars when used in + and builtin
        // calls, at the cost of an incref when stored in a variable or a data structure.
        node.lt = LT_BORROW;
        return &node;
    }

    Node *Check(Nil &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (node.giventype.Null()) {
            node.exptype = st.Wrap(st.NewTypeVar(), V_NIL, &node.line);
        } else {
            node.exptype = st.ResolveTypeVars(node.giventype, node.line);
            if (!st.IsNillable(node.exptype->sub)) Error(node, "illegal nil type");
        }
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(TypeAnnotation &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = st.ResolveTypeVars(node.giventype, node.line);
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(VectorConstructor &node, size_t /*reqret*/, TypeRef parent_bound) {
        if (node.giventype.Null()) {
            if (parent_bound->t == V_VECTOR && parent_bound->sub->IsConcrete()) {
                node.exptype = parent_bound;
                TypeCheckList(&node, LT_KEEP, node.exptype->sub);
            } else {
                if (node.Arity()) {
                    TypeCheckList(&node, LT_KEEP);
                    // No type was specified.. first find union of all elements.
                    TypeRef u(nullptr);
                    for (auto c : node.children) {
                        u = u.Null()
                                ? c->exptype
                                : Union(u, c->exptype, "vector", "vector element", CF_COERCIONS, c);
                    }
                    node.exptype = st.Wrap(u, V_VECTOR, &node.line);
                    StorageType(node.exptype, node);
                } else {
                    // special case for empty vectors
                    node.exptype = st.Wrap(st.NewTypeVar(), V_VECTOR, &node.line);
                }
            }
        } else {
            node.exptype = st.ResolveTypeVars(node.giventype, node.line);
            if (node.exptype->t != V_VECTOR) {
                if (!node.exptype->IsError())
                    Error(node, "type does not resolve to vector: ", Q(TypeName(node.exptype)));
                node.exptype = st.Wrap(type_error, V_VECTOR, &node.line);
            }
            // These may include field initializers copied from the definition, which may include
            // type variables that are now bound.
            TypeCheckList(&node, LT_KEEP, node.exptype->sub);
        }
        for (auto [i, c] : enumerate(node.children)) {
            TypeRef elemtype = node.exptype->Element();
            SubType(c, elemtype, ArgName(i), node);
        }
        node.lt = LT_KEEP;
        return &node;
    }

    Node *Check(AutoConstructor &node, size_t reqret, TypeRef parent_bound) {
        // Resolve which fields the (tagged) initializers belong to, fill in
        // defaults, and check for missing fields, all against the completed
        // declaration of the type, then hand over to ObjectConstructor.
        GUDT *g = nullptr;
        UnTypeRef ctype = node.giventype;
        // Stands in for a constructor whose fields can't be resolved, see TypeChecker::ErrorNode;
        // the initializers still get typechecked, for the errors in them.
        auto give_up = [&]() {
            TypeCheckList(&node, LT_KEEP);
            return ErrorNode(node);
        };
        if (!ctype.Null()) {
            if (ctype->t == V_TYPEVAR) {
                // A `T {}` constructor: what it constructs is only known now that
                // the type variable is bound.
                auto rt = st.ResolveTypeVars(ctype, node.line);
                if (rt->IsError()) return give_up();
                if (!IsUDT(rt->t)) {
                    // A T that is not a class/struct has no fields to resolve
                    // against; ObjectConstructor turns `T {}` into that type's
                    // default value.
                    for (auto tag : node.tags)
                        if (tag)
                            Error(node, "type ", Q(TypeName(rt)), " does not have field ",
                                            Q(tag->name));
                    unique_ptr<ObjectConstructor> oc(new ObjectConstructor(node.line, { rt }));
                    oc->children = node.children;
                    node.children.clear();
                    auto r = oc->TypeCheck(ASTChecker(), reqret, parent_bound);
                    oc.release();
                    delete &node;
                    return r;
                }
                ctype = { rt };
            }
            g = GetGUDTAny(ctype);
            assert(g);
        } else {
            auto pb = parent_bound->ElementIfNil();
            if (!IsUDT(pb->t)) {
                if (!pb->IsError())
                    Error(node, "class/struct type for auto constructor unknown in this context");
                return give_up();
            }
            g = &pb->udt->g;
            ctype = { &pb->udt->thistype };
        }
        unique_ptr<ObjectConstructor> constructor(ResolveAutoConstructor(&node, g, ctype, true));
        if (!constructor) return give_up();
        // Our children have been transferred; if typechecking below throws, the
        // new node owns them and gets deleted, while this node (still in the
        // tree) is left childless.
        auto r = constructor->TypeCheck(ASTChecker(), reqret, parent_bound);
        constructor.release();
        delete &node;
        return r;
    }

    Node *Check(ObjectConstructor &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        UDT *udt = nullptr;
        if (node.giventype->t == V_UUDT && node.giventype->spec_udt->specializers.empty()) {
            // Special case for generic type constructor with no specializers.
            // Versions WITH specializers are instead resolved below.
            auto gudt = node.giventype->spec_udt->gudt;
            bool any_named = false;
            for (auto udti = gudt->first; udti; udti = udti->next)
                if (!udti->unnamed_specialization) { any_named = true; break; }
            if (!any_named) {
                // There is nothing for the initializers to be matched against, so that is
                // the error, not whatever checking them turns up: the ones filled in from
                // the declaration mention type variables this gives no values for, so those
                // are bound to the error type to keep errors about them from standing in
                // for this one.
                vector<GenericTypeVariable> unresolved = gudt->generics;
                for (auto &gtv : unresolved) gtv.type = type_error;
                st.bound_typevars_stack.push_back(unresolved);
                TypeCheckList(&node, LT_KEEP);
                st.bound_typevars_stack.pop_back();
                // Not about the types of the initializers, so reported even when one of
                // those could not be typed.
                ErrorAlways(node, "cannot construct ", Q(gudt->name),
                                  " without specializers: no named specialization of it to"
                                  " match (write ", Q(cat(gudt->name, "<...>{...}")),
                                  " instead)");
                return ErrorNode(node);
            }
            // The initializers filled in from the declaration may still mention its type
            // variables, whose values are only known once one of the specializations below
            // is picked, which needs the types of the initializers first.
            SymbolTable::ResolveContext no_specializers = {
                .what = "constructor", .gudt = gudt, .tail = "given no specializers",
                .line = &node.line };
            arglists.push_back(&node);
            for (auto [i, c] : enumerate(node.children)) {
                optional<SymbolTable::ResolveScope> rs;
                if (node.IsDefault(i)) rs.emplace(st, no_specializers);
                TT(c, 1, LT_KEEP);
            }
            arglists.pop_back();
            // Try and find a matching named specialization.
            if (node.Arity() != gudt->fields.size())
                ErrorAlways(node, "incorrect argument count for generic constructor");
            // Now find a match:
            int bestmatch = 0;
            for (auto udti = gudt->first; udti; udti = udti->next) {
                if (udti->unnamed_specialization) continue;
                EnsureUDTChecked(*udti, node);
                int nmatches = 0;
                for (auto [i, arg] : enumerate(node.children)) {
                    if (i >= udti->sfields.size()) break;
                    auto &sfield = udti->sfields[i];
                    if (ConvertsTo(arg->exptype, sfield.type, CF_NONE)) nmatches++;
                    else break;
                }
                if (nmatches > bestmatch) {
                    bestmatch = nmatches;
                    udt = udti;
                }
            }
            if (!udt) {
                string s;
                for (auto &arg : node.children) s += " " + TypeName(arg->exptype);
                auto err = "generic constructor matches no named explicit specialization of " +
                            Q(gudt->name) + " with types:" + s;
                for (auto udti = gudt->first; udti; udti = udti->next) {
                    err += "\n  specialization: ";
                    err += Signature(*udti);
                }
                return ErrorNode(node, err);
            }
            node.exptype = &udt->thistype;
        } else {
            node.exptype = st.ResolveTypeVars(node.giventype, node.line);
            if (node.exptype->IsError()) {
                TypeCheckList(&node, LT_KEEP);
                return ErrorNode(node);
            }
            if (!IsUDT(node.exptype->t)) {
                // A T {} where T resolves to a non-struct/class type stands for the
                // default value of that type, so generic code (like
                // dictionary.get_or_create) can construct a value of any type.
                if (!node.Arity()) {
                    Node *r = nullptr;
                    switch (node.exptype->t) {
                        case V_INT:
                            r = new IntConstant(node.line, 0);
                            break;
                        case V_FLOAT:
                            r = new FloatConstant(node.line, 0.0);
                            break;
                        case V_STRING:
                            r = new StringConstant(node.line, string());
                            break;
                        case V_VECTOR: {
                            auto vc = new VectorConstructor(node.line);
                            vc->giventype = { node.exptype };
                            r = vc;
                            break;
                        }
                        case V_NIL:
                            r = new Nil(node.line, node.exptype);
                            break;
                        default:
                            break;
                    }
                    if (r) {
                        r = r->TypeCheck(ASTChecker(), 1, {});
                        // For an enum (e.g. bool), keep the enum type on the 0 value.
                        if (node.exptype->t == V_INT) r->exptype = node.exptype;
                        delete &node;
                        return r;
                    }
                }
                Error(node, "type does not resolve to an object constructor: ", Q(TypeName(node.exptype)));
                TypeCheckList(&node, LT_KEEP);
                return ErrorNode(node);
            }
            udt = node.exptype->udt;
            // Complete the UDT on demand: its field defaults must have been
            // typechecked before they can be cloned in below, regardless of where
            // its declaration statement sits relative to this constructor.
            EnsureUDTChecked(*udt, node);
            st.PushSuperGenerics(udt);
            // Fill in default args.. already done in the parser normally, but can happen if
            // this is a T {} constructor.
            for (size_t i = node.children.size(); i < udt->sfields.size(); i++) {
                if (udt->sfields[i].defaultval) {
                    // If this type's own defaults are being type checked right now, then this
                    // default recursively constructs it, which would never terminate (and
                    // neither would cloning it in below).
                    if (std::find(udts_in_progress.begin(), udts_in_progress.end(), udt) !=
                        udts_in_progress.end()) {
                        Error(node, "default value of field ", Q(udt->g.fields[i].id->name),
                                        " recursively constructs ", Q(udt->name));
                        st.PopSuperGenerics(udt);
                        TypeCheckList(&node, LT_KEEP);
                        return ErrorNode(node);
                    }
                    node.AddDefault(udt->sfields[i].defaultval->Clone(true));
                } else {
                    Error(node, "field ", Q(udt->g.fields[i].id->name), " not initialized");
                }
            }
            // These may include field initializers copied from the definition, which may include
            // type variables that are now bound. Initializers beyond the fields (reported below)
            // and fields without one (reported above) leave the two lists different in length.
            for (auto [i, c] : enumerate(node.children)) {
                optional<SymbolTable::ResolveScope> rs;
                if (node.IsDefault(i)) rs.emplace(st, DefaultFieldContext(*udt, i, node));
                TT(c, 1, LT_KEEP, i < udt->sfields.size() ? udt->sfields[i].type : type_error);
            }
            st.PopSuperGenerics(udt);
        }
        assert(udt);
        // We have to check this here, since the parser couldn't check this yet.
        if (udt->sfields.size() < node.children.size())
            ErrorAlways(node, "too many initializers for ", Q(udt->name));
        node.exptype = &udt->thistype;
        auto in_constructor = [&]() {
            // Anonymous calls may build on behalf of their nearest named caller.
            for (auto &scope : reverse(scopes)) {
                auto f = scope.sf->parent;
                if (!f->anonymous) return f->is_constructor_of == &udt->g;
            }
            return false;
        };
        if (udt->g.has_constructor_function && !in_constructor()) {
            ErrorAlways(node, Q(udt->name),
                           " may only be constructed thru its constructor function");
        }
        for (auto [i, c] : enumerate(node.children)) {
            if (i >= udt->sfields.size()) break;
            TypeRef elemtype = node.exptype->udt->sfields[i].type;
            optional<SymbolTable::ResolveScope> rs;
            if (node.IsDefault(i)) rs.emplace(st, DefaultFieldContext(*udt, i, node));
            SubType(c, elemtype, ArgName(i), node);
        }
        node.lt = LT_KEEP;  // Or LT_ANY if this is a numeric struct?
        return &node;
    }

    Node *Check(TypeOf &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.child, 1, LT_BORROW);
        DecBorrowers(node.child->lt, node);
        auto ti = st.NewType();
        node.exptype = node.child->exptype->Wrap(ti, V_TYPEID);
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(EnumCoercion &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        TT(node.child, 1, LT_BORROW);
        SubType(node.child, type_int, "coerced value", node);
        DecBorrowers(node.child->lt, node);
        node.exptype = &node.e->thistype;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(EnumRef &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(GUDTRef &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        if (!node.predeclaration) {
            for (auto udt = node.gudt->first; udt; udt = udt->next) {
                EnsureUDTChecked(*udt, node);
            }
        }
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }

    Node *Check(UDTRef &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        // Its specializations get checked from GUDTRef::TypeCheck.
        node.exptype = type_void;
        node.lt = LT_ANY;
        return &node;
    }
};

TypeRef EnumCoercion::SimpleType(SymbolTable &) {
    return &e->thistype;
}

TypeRef Nil::SimpleType(SymbolTable &st) {
    return giventype.Null() ? TypeRef(nullptr) : SimpleResolve(giventype, st);
}

TypeRef VectorConstructor::SimpleType(SymbolTable &st) {
    if (!giventype.Null()) return SimpleResolve(giventype, st);
    // Without a given type, the element type is the union of the elements,
    // which we can only match locally when they're all the same simple type.
    if (children.empty()) return nullptr;
    TypeRef elem = nullptr;
    for (auto c : children) {
        auto ct = c->SimpleType(st);
        if (ct.Null()) return nullptr;
        if (elem.Null()) elem = ct;
        else if (!ct->Equal(*elem)) return nullptr;
    }
    return st.Wrap(elem, V_VECTOR, &line);
}

TypeRef ObjectConstructor::SimpleType(SymbolTable &) {
    // Only a directly concrete type: resolving type variables here could
    // create specializations, which isn't local anymore. The argument
    // expressions don't affect the type and get typechecked wherever this
    // constructor ends up used.
    return IsUDT(giventype->t) ? TypeRef(&giventype->udt->thistype) : TypeRef(nullptr);
}

TypeRef AutoConstructor::SimpleType(SymbolTable &) {
    // As above.
    return !giventype.Null() && IsUDT(giventype->t) ? TypeRef(&giventype->udt->thistype)
                                                    : TypeRef(nullptr);
}

}  // namespace lobster
