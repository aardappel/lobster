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

// What is known about variables and field paths as checking proceeds: the types flow typing
// promoted them to and the writes that undo that, every path that may name the same location,
// borrows of them and the writes that conflict with those, and the free variables functions use.
// This layer depends only on TypeCheckBase; every topic other than constructors and query
// builds on it.

namespace lobster {

struct TypeCheckLocations : virtual TypeCheckBase {
    vector<FlowItem> flowstack;
    // What FlowItem::demoted_by points to.
    vector<unique_ptr<Demotion>> demotions;
    // The function whose writes ReplayAssigns is checking at the current call site.
    SubFunction *replaying = nullptr;
    set<pair<SubFunction *, SubFunction *>> freevar_check_preempt;

    // A vector since a small_vector cannot hold elements that are small_vectors themselves.
    typedef vector<LValContext> Paths;

    struct FlowBranch {
        vector<FlowItem> promoted;     // What it established, still live at its end.
        vector<FlowDemotion> demoted;  // Promotions from further out that it dropped.
    };

    TypeCheckLocations() {}

    // Flow typing: what variables and fields are known to be at this point, how conditions,
    // branches and loops change that, and the writes that undo it.

    void CheckFlowTypeIdOrDot(const Node &n, TypeRef type) {
        FlowItem fi(n, type);
        if (fi.IsValid()) flowstack.push_back(fi);
    }

    void AssignFlowPromote(Node &left, TypeRef right) {
        if (left.exptype->t == V_NIL && right->t != V_NIL) {
            CheckFlowTypeIdOrDot(left, right);
        }
    }

    void AssignFlowPromoteFI(FlowItem &fi) {
        assert(fi.IsValid());
        if (fi.now->t != V_NIL && fi.old->t == V_NIL) {
            flowstack.push_back(fi);
        }
    }

    // FIXME: this can in theory find the wrong node, if the same function nests, and the outer
    // one was specialized to a nilable and the inner one was not.
    // This would be very rare though, and benign.
    // `write` is the node doing the write, and `call` the call whose callee's writes are being
    // replayed, if that is what this is, see Demotion.
    TypeRef AssignFlowDemote(FlowItem &left, TypeRef overwritetype, ConvertFlags coercions,
                             const Node &write, const Node *call = nullptr) {
        // Early out, numeric types are not nillable, nor do they make any sense for "is"
        auto &type = left.now;
        if (type->Numeric()) return type;
        // Promotions and the write are compared as every path that may name the location,
        // see CheckLvalBorrowed.
        auto written = ExpandAliases(left);
        // A single assignment may invalidate multiple promotions: of the location itself, and
        // of everything reached thru it, which is a different object now.
        for (auto &flow : reverse(flowstack)) {
            auto deeper = false;
            auto held = ExpandAliases(flow);
            if (!WriteHits(held, written, &deeper)) continue;
            if (deeper) {
                Demote(flow, held, left, written, write, call);
                continue;
            }
            type = flow.old;
            if (!ConvertsTo(overwritetype, flow.now, coercions)) {
                // FLow based promotion is invalidated.
                Demote(flow, held, left, written, write, call);
                // TODO: It be cool to instead overwrite with whatever type is currently being
                // assigned. That currently doesn't work, since our flow analysis is a
                // conservative approximation, so if this assignment happens conditionally it
                // wouldn't work.
            }
        }
        return type;
    }

    // A write to `left` (compared as the paths `written`) undoes the promotion `flow` (compared
    // as the paths `held`), `write` and `call` are as for AssignFlowDemote.
    void Demote(FlowItem &flow, const Paths &held, const LValContext &left, const Paths &written,
                const Node &write, const Node *call) {
        if (!flow.now->Equal(*flow.old)) {
            const LValContext *held_as = nullptr, *written_as = nullptr;
            for (auto &h : held) {
                for (auto &w : written) {
                    if (!held_as && h.IsPrefix(w)) {
                        held_as = &h;
                        written_as = &w;
                    }
                }
            }
            assert(held_as);
            // The call made by the code the promotion is in: to the first function entered
            // after the promotion, or else the one whose writes are being replayed.
            auto idx = size_t(&flow - flowstack.data());
            const SubFunction *call_sf = call ? replaying : nullptr;
            auto call_line = call ? call->line : write.line;
            for (auto &sc : scopes) {
                if (sc.flowstack_size > idx) {
                    call_sf = sc.sf;
                    call_line = sc.call_context->line;
                    break;
                }
            }
            demotions.push_back(make_unique<Demotion>(Demotion {
                write.line, left, *written_as, *held_as, flow.now, call_sf, call_line }));
            flow.demoted_by = demotions.back().get();
        }
        flow.now = flow.old;
    }

    TypeRef UseFlow(const FlowItem &left, size_t max_flowstack_size) {
        // Early out, same as above, except for enums, which an `out_of_range` case decays to int.
        if (left.now->Numeric() && !left.now->IsEnum()) return left.now;
        for (size_t i = max_flowstack_size; i > 0; i--) {
            auto &flow = flowstack[i - 1];
            if (flow.sid == left.sid &&	flow.DerefsEqual(left)) {
                return flow.now;
            }
        }
        return left.old;
    }
    TypeRef UseFlow(const FlowItem &left) {
        return UseFlow(left, flowstack.size());
    }

    // For an error about `n` that a promotion of it would have avoided, had a write not undone
    // it: a line saying which write, since that may be far from the error, in another function
    // even, and be to a path that only aliases this one. Without `field`, the error is about
    // `n` being nilable, with, about its type not having that field.
    string DemotionNote(const Node &n, SharedField *field = nullptr) override {
        if (n.exptype.Null()) return {};
        LValContext lv(*SkipCoercions(&n));
        if (!lv.IsValid() || lv.HasElem()) return {};
        for (auto &flow : reverse(flowstack)) {
            if (flow.sid != lv.sid || !flow.DerefsEqual(lv)) continue;
            auto d = flow.demoted_by;
            if (!d) return {};
            auto promoted = d->promoted;
            if (field ? !IsUDT(promoted->t) || promoted->udt->g.Has(field) < 0
                      : n.exptype->t != V_NIL || promoted->t == V_NIL) return {};
            auto note = cat("\n  ", Q(flow.Name()), " is no longer known to be ",
                            flow.old->t == V_NIL && promoted->Equal(*flow.old->Element())
                                ? "non-nil"
                                : Q(TypeName(promoted)),
                            " after the assignment to ", Q(d->written.Name()), " at ",
                            parser.lex.Location(d->line));
            if (d->call_sf) {
                note += cat(", in the call to ", Q(d->call_sf->parent->name), " at ",
                            parser.lex.Location(d->call_line));
            }
            // How the write names the location, unless that is apparent from the paths.
            if (!flow.IsPrefix(d->written)) {
                auto promoted_alias = AliasNote(flow, d->promoted_as);
                auto written_alias = AliasNote(d->written, d->written_as);
                if (written_alias == promoted_alias) written_alias.clear();
                if (!promoted_alias.empty() || !written_alias.empty()) {
                    note += cat(" (", promoted_alias,
                                promoted_alias.empty() || written_alias.empty() ? "" : ", ",
                                written_alias, ")");
                }
            }
            return note;
        }
        return {};
    }

    // `as` is `path` after following aliases (see ExpandAliases), which keeps the fields of
    // `path` at its end: the alias this followed, if any.
    string AliasNote(const LValContext &path, const LValContext &as) {
        LValContext alias(as.sid);
        for (size_t i = 0; i + path.derefs.size() < as.derefs.size(); i++) {
            alias.derefs.push_back(as.derefs[i]);
        }
        auto name = LValContext(path.sid).Name();
        auto alias_name = alias.Name();
        if (name == alias_name) return {};
        return cat(Q(name), " may be an alias of ", Q(alias_name));
    }

    void CleanUpFlow(size_t start) {
        while (flowstack.size() > start) flowstack.pop_back();
    }

    void CheckFlowTypeChangesSub(bool iftrue, const Node *condition) {
        condition = SkipCoercions(condition);
        auto type = condition->exptype;
        if (auto c = Is<IsType>(condition)) {
            // A `?` on a type that cannot be nilable makes the test a nil check (see
            // IsType::ConstVal), which promotes nothing.
            if (iftrue && !c->accepts_nil) CheckFlowTypeIdOrDot(*c->child, c->resolvedtype);
        } else if (auto c = Is<Not>(condition)) {
            CheckFlowTypeChangesSub(!iftrue, c->child);
        } else if (auto eq = Is<Equal>(condition)) {
            if (Is<Nil>(eq->right)) CheckFlowTypeChangesSub(!iftrue, eq->left);
        } else if (auto neq = Is<NotEqual>(condition)) {
            if (Is<Nil>(neq->right)) CheckFlowTypeChangesSub(iftrue, neq->left);
        } else {
            if (iftrue && type->t == V_NIL) CheckFlowTypeIdOrDot(*condition, type->Element());
        }
    }

    void CheckFlowTypeChangesAndOr(bool iftrue, const BinOp *condition) {
        // AND only works for then, and OR only for else.
        if (iftrue == (Is<And>(condition) != nullptr)) {
            // This allows for a chain of and's without allowing mixed operators.
            auto cleft = SkipCoercions(condition->left);
            if (typeid(*cleft) == typeid(*condition)) {
                CheckFlowTypeChanges(iftrue, condition->left);
            } else {
                CheckFlowTypeChangesSub(iftrue, condition->left);
            }
            CheckFlowTypeChangesSub(iftrue, condition->right);
        }
    }

    size_t CheckFlowTypeChanges(bool iftrue, const Node *condition) {
        auto start = flowstack.size();
        condition = SkipCoercions(condition);
        if (auto c = Is<Or>(condition)) {
            CheckFlowTypeChangesAndOr(iftrue, c);
        } else if (auto c = Is<And>(condition)) {
            CheckFlowTypeChangesAndOr(iftrue, c);
        } else {
            CheckFlowTypeChangesSub(iftrue, condition);
        }
        return start;
    }

    void BackupFlow(size_t upto, vector<TypeRef> &dest) {
        for (size_t i = 0; i < upto; i++) dest.push_back(flowstack[i].now);
    }

    // The type changes a branch established that still hold at the end of it,
    // i.e. what code following it on that path may rely on.
    void CollectFlowPromotions(size_t start, vector<FlowItem> &dest) {
        for (auto i = start; i < flowstack.size(); i++) {
            auto &fi = flowstack[i];
            if (!fi.now->Equal(*fi.old)) dest.push_back(fi);
        }
    }

    // Promotions from outside the branch that it invalidated (by assigning to
    // the variable), which are restored so a sibling branch, which the
    // assignment can't have run before, still sees them.
    void CollectFlowDemotions(const vector<TypeRef> &backup, vector<FlowDemotion> &dest) {
        for (auto [i, was] : enumerate(backup)) {
            auto &fi = flowstack[i];
            if (!fi.now->Equal(*was)) {
                dest.push_back({ i, fi.demoted_by });
                fi.now = was;
                fi.demoted_by = nullptr;
            }
        }
    }

    // Reduce `a` to what `b` establishes as well, widened to their common type:
    // code after a branching statement may have arrived over either path.
    void MergeFlowPromotions(vector<FlowItem> &a, const vector<FlowItem> &b) {
        size_t keep = 0;
        for (auto &fa : a) {
            for (auto &fb : b) {
                if (fb.sid != fa.sid || !fb.DerefsEqual(fa)) continue;
                if (!fa.now->Equal(*fb.now)) {
                    auto u = Union(fa.now, fb.now, "", "", CF_NONE, nullptr);
                    // No common type, or one that is nillable again: no promotion.
                    if (u->t == V_UNDEFINED || u->Equal(*fa.old)) break;
                    fa.now = u;
                }
                if (keep != size_t(&fa - &a[0])) a[keep] = fa;
                keep++;
                break;
            }
        }
        a.erase(a.begin() + keep, a.end());
    }

    // Everything a branch that can fall through to the code after it left
    // behind. Demotions apply if any such branch made them, promotions only if
    // they survived merging with every one of them.
    void ApplyFlow(const FlowBranch &fb) {
        for (auto d : fb.demoted) ApplyDemotion(d);
        for (auto &fi : fb.promoted) flowstack.push_back(fi);
    }

    void ApplyDemotion(FlowDemotion d) {
        auto &fi = flowstack[d.first];
        // Several branches may have undone it, any of their writes explains that.
        if (!fi.demoted_by) fi.demoted_by = d.second;
        fi.now = fi.old;
    }

    void EnterLoop() {
        auto &sc = scopes.back();
        if (sc.loop_flow.empty()) sc.loop_events_start = sc.sf->reuse_assign_events.size();
        sc.loop_flow.emplace_back();
        BackupFlow(flowstack.size(), sc.loop_flow.back().entry);
    }

    // A break or continue doesn't fall through its enclosing branches, but its
    // writes still reach the loop exit (directly or over a later iteration).
    // Save those demotions at the actual jump, before a branch restores the
    // incoming types for its siblings. Only the innermost loop is its target.
    void RecordLoopFlowExit() {
        auto &loop = scopes.back().loop_flow.back();
        for (auto [i, was] : enumerate(loop.entry)) {
            auto &fi = flowstack[i];
            if (!fi.now->Equal(*was)) loop.demoted.push_back({ i, fi.demoted_by });
        }
    }

    void LeaveLoop() {
        auto &sc = scopes.back();
        auto &loop = sc.loop_flow.back();
        // The loop may not run, so none of its new promotions survive it.
        CleanUpFlow(loop.entry.size());
        for (auto d : loop.demoted) ApplyDemotion(d);
        sc.loop_flow.pop_back();
    }

    // Writes, and every path that may name what they write.

    // `overwritetype` is what an assignment stores, see AssignEvent.
    void CheckLval(Node *n, TypeRef overwritetype = {}) {
        // This can happen due to late specialization of GenericCall.
        if (Is<Call>(n) || Is<NativeCall>(n))
            ErrorAlways(*n, "function-call cannot be an l-value");
        if (auto idx = Is<Indexing>(n); idx && idx->object->exptype->t == V_STRING)
            Error(*n, "cannot use string element as lvalue (strings are immutable)");
        if (auto idr = Is<IdentRef>(n)) {
            // This has been done before in the parser, but that missed FreeVarRef's etc.
            // FIXME: what if this is the only assignement, and other checks against
            // single_assignment make the wrong decision?
            idr->sid->id->single_assignment = false;
        } else {
            auto fn = n;
            for (;;) {
                if (auto dot = Is<Dot>(fn)) fn = dot->child;
                else if (auto idx = Is<Indexing>(fn)) fn = idx->object;
                else break;
            }
            if (auto idr = Is<IdentRef>(fn)) {
                if (IsStruct(idr->exptype->t)) {
                    idr->sid->id->StructAssign(parser.lex,&fn->line);
                }
            }
        }
        Borrow lv(*n);
        if (!lv.IsValid()) return;  // FIXME: force these to LT_KEEP?
        if (IsRefNil(n->exptype->t)) RecordWrite(n, lv, overwritetype);
        CheckLvalBorrowed(n, lv);
    }

    // A builtin that can drop elements of the vector `vec` names is a write to its elements,
    // see elem_field.
    void CheckElementWrite(Node *vec, Node *call) {
        LValContext lv(*vec);
        if (!lv.IsValid()) return;
        lv.derefs.push_back(&elem_field);
        RecordWrite(call, lv, {});
        Borrow b(lv);
        CheckLvalBorrowed(call, b);
    }

    // If any of the functions this write sits in is reused, we need to be able to replay
    // checking the errors in CheckLvalBorrowed, since the contents of the borrowstack may be
    // different.
    // The location written may be reached thru parameters that alias what their callers
    // passed, in which case the functions further out know it under the path the call passed:
    // each function on the stack up to the one that holds the variable records it as the path
    // it can see.
    // Likewise for the paths an owning variable was assigned from (see
    // SpecIdent::owning_aliases) when the variable is one the function cannot see: a caller
    // records the write thru a callee's local as one to what the local was assigned from, as
    // the caller sees that, since at a replay the local's aliases would resolve thru whatever
    // the last call bound its parameters to.
    void RecordWrite(Node *n, const LValContext &lv, TypeRef overwritetype) {
        // Don't go further than where the outermost variable really written is defined.
        size_t stop = scopes.size() - 1;
        for (auto &root : ExpandAliases(lv)) {
            size_t i = 0;
            for (auto [j, sc] : enumerate(scopes)) if (sc.sf == root.sid->sf_def) i = j;
            stop = std::min(stop, i);
        }
        for (size_t i = scopes.size(); i-- > stop; ) {
            auto sf = scopes[i].sf;
            Paths todo, seen;
            small_vector<const SpecIdent *, 4> expanded;
            todo.push_back(lv);
            while (!todo.empty()) {
                auto ev = todo.back();
                todo.pop_back();
                while (ev.sid->alias_sid && !LexicallyVisible(ev.sid, sf) && ev.Step()) {}
                if (!LexicallyVisible(ev.sid, sf) && !ev.derefs.empty() &&
                    !ev.sid->owning_aliases.empty() &&
                    find(expanded.begin(), expanded.end(), ev.sid) == expanded.end()) {
                    expanded.push_back(ev.sid);
                    for (auto &a : ev.sid->owning_aliases) todo.push_back(AliasedPath(a, ev));
                    continue;
                }
                auto dup = false;
                for (auto &s : seen) if (s.sid == ev.sid && s.DerefsEqual(ev)) dup = true;
                if (dup) continue;
                seen.push_back(ev);
                // we could uniqueify this vector, but that would entails comparing `n`
                // structurally (construct a Borrow for each?), which would probably be
                // slower than the redundant calls to CheckLvalBorrowed this causes later?
                // Especially since this uniqueifying cost is paid always, even when there
                // are no actual repeated assigns in a scope, which is not that common.
                sf->reuse_assign_events.push_back({ n, ev, overwritetype });
            }
        }
    }

    void ReplayAssigns(SubFunction *sf, const Node &call) {
        auto outer = replaying;
        replaying = sf;
        for (auto &ev : sf->reuse_assign_events) {
            Borrow lv(ev.lv);
            CheckLvalBorrowed(ev.n, lv);
            // The write also stands for any promotion this context has of what it overwrites.
            if (!ev.overwritetype.Null() && !ev.lv.HasElem()) {
                FlowItem fi(ev.lv, ev.n->exptype);
                AssignFlowDemote(fi, ev.overwritetype, CF_COERCIONS, *ev.n, &call);
            }
        }
        replaying = outer;
    }

    // `sid`, which owns what it holds, was assigned from `src` (a for loop element for
    // `fle`), see SpecIdent::owning_aliases.
    void RecordOwningAlias(SpecIdent &sid, const Node *src, const ForLoopElem *fle) {
        if (!SpecBorrowable(sid.type)) return;
        LValContext path(*SkipCoercions(src));
        if (fle) {
            path = LValContext(*fle->iter);
            path.derefs.push_back(&elem_field);
        }
        if (!path.IsValid() || (path.sid == &sid && path.derefs.empty())) return;
        for (auto &a : sid.owning_aliases) {
            auto ap = AliasedPath(a, LValContext(a.sid));  // The alias as a path.
            if (ap.sid == path.sid && ap.DerefsEqual(path)) return;
        }
        LOG_DEBUG("owning alias: ", sid.id->name, " of ", path.Name());
        sid.owning_aliases.push_back({ path.sid, path.derefs });
    }

    // The path `a` (a variable and fields) followed by the fields of `p`.
    static LValContext AliasedPath(const SpecIdent::AliasPath &a, const LValContext &p) {
        LValContext q(a.sid);
        for (auto f : a.derefs) q.derefs.push_back(f);
        for (auto f : p.derefs) q.derefs.push_back(f);
        return q;
    }

    // Every path that may name the location `p` names: `p` as the variable that really holds
    // it (see LValContext::Canonicalize), and when that variable owns what it holds and was
    // assigned from other paths, those with the same fields (recursively: a variable's aliases
    // are followed once, so `y = y.next` gives `y.next.f` for `y.f` and stops). A write to any
    // of these is a write to `p`, and a borrow or promotion of any of them is one of `p`.
    void ExpandAliases(LValContext p, Paths &out, small_vector<const SpecIdent *, 4> &followed) {
        p.Canonicalize();
        for (auto &o : out) if (o.sid == p.sid && o.DerefsEqual(p)) return;
        out.push_back(p);
        // Only its fields are shared: the variable itself is a location of its own.
        if (p.derefs.empty()) return;
        for (auto s : followed) if (s == p.sid) return;
        followed.push_back(p.sid);
        for (auto &a : p.sid->owning_aliases) ExpandAliases(AliasedPath(a, p), out, followed);
    }
    Paths ExpandAliases(const LValContext &p) {
        Paths out;
        small_vector<const SpecIdent *, 4> followed;
        ExpandAliases(p, out, followed);
        return out;
    }

    // Whether a write to what `written` names (any of its paths) replaces what `held` names
    // (any of its paths), or something it was reached thru (`deeper`: a write to a prefix),
    // which then is a different object.
    bool WriteHits(const Paths &held, const Paths &written, bool *deeper = nullptr) {
        auto hit = false;
        for (auto &h : held) {
            for (auto &w : written) {
                if (!h.IsPrefix(w)) continue;
                hit = true;
                if (deeper && h.derefs.size() != w.derefs.size()) *deeper = true;
            }
        }
        return hit;
    }

    // Whether code in `sf` can name `sid`: a variable of its own, or of a function it is
    // lexically inside of.
    bool LexicallyVisible(const SpecIdent *sid, SubFunction *sf) {
        if (sid->sf_def == sf || sid->sf_def == st.toplevel) return true;
        for (auto ov = sf->overload; ov; ov = ov->sf->lexical_parent) {
            if (ov == sid->sf_def->overload) return true;
        }
        return false;
    }

    // Borrows of variables and fields, and the writes that conflict with them.

    Lifetime PushBorrow(Node *n) {
        // Because of the Var in the check below, we may end up with things on the borrowstack
        // that are not refs once the var binds! This should be benign, as all we're doing
        // is locking it from overwrites, so in the worst case it gives an undeserved error?
        if (!IsRefNilVar(n->exptype->t)) return LT_ANY;
        Borrow lv(*n);
        // An expression that is not a path (like f()[i]) gets a generic borrow, which
        // disables lock checks, so is unsafe.
        if (!lv.IsValid()) return LT_BORROW;
        return PushBorrowPath(lv, n->line);
    }

    Lifetime PushBorrowPath(const LValContext &lv, const Line &line) {
        for (auto &b : reverse(borrowstack)) {
            if (b.sid == lv.sid && b.DerefsEqual(lv)) {
                b.refc++;
                return (Lifetime)(&b - &borrowstack[0]);
            }
        }
        // FIXME: this path is slow, should not have to scan all of borrowstack.
        auto lt = (Lifetime)borrowstack.size();
        Borrow b(lv, line);
        borrowstack.push_back(b);
        return lt;
    }

    // A borrowed parameter is another name for what the call passed. When that is a variable or
    // a path of fields from one, writes thru the parameter (in this function or its callees)
    // are writes to that location, which the borrows and flow promotions of the callers name as
    // that variable: the binding lets the checks of those compare the two, see
    // LValContext::Step. Bound for every call, so a reused specialization gets checked (thru
    // ReplayAssigns) against what the new call passed. A call to a function still being
    // typechecked (recursion) keeps the binding of the call that entered it, since its body is
    // being checked against that.
    void BindParamAliases(SubFunction *sf, List &call_args) {
        if (IsActive(sf)) return;
        for (auto [i, c] : enumerate(call_args.children)) {
            auto sid = sf->args[i].sid;
            sid->alias_sid = nullptr;
            sid->alias_derefs.clear();
            sid->arg_slot = nullptr;
            if (c->lt < 0) continue;
            LValContext lv(*c);
            if (!lv.IsValid() || lv.sid == sid) continue;
            sid->alias_sid = lv.sid;
            sid->alias_derefs = lv.derefs;
            // An owning parameter is a location of its own (only its fields are those of what
            // was passed, see LValContext::Step), so nothing needs to keep what it was passed
            // alive for it.
            if (IsBorrow(sid->lt)) sid->arg_slot = &call_args.children[i];
        }
    }

    // What a borrowed parameter names is about to be written while the parameter, or
    // something borrowed thru it, is still in use. Rather than an error, the caller of the
    // current call keeps the value alive: its argument owns a reference for the rest of the
    // caller's scope (an inc, and a temporary given up at its end), so the parameter no
    // longer depends on the location, and stops naming it. Cheaper than making the parameter
    // own, which every call would pay for.
    void KeepArgAlive(SpecIdent *sid) {
        auto &c = *sid->arg_slot;
        LOG_DEBUG("argument for ", sid->id->name, " kept alive by the caller");
        DecBorrowers(c->lt, *c);
        MakeLifetime(c, LT_BORROW, 1, 1);
        // The count TypeCheckFunctionDef took on the caller's borrow for the parameter, which
        // exists only while the function is being typechecked.
        if (IsActive(sid->sf_def)) DecBorrowers(sid->lt, *c);
        sid->lt = LT_BORROW;
        sid->alias_sid = nullptr;
        sid->alias_derefs.clear();
        sid->arg_slot = nullptr;
    }

    void CheckLvalBorrowed(Node *n, Borrow &lv) {
        if (lv.derefs.empty() && LifetimeType(lv.sid->lt) == LT_BORROW) {
            // A parameter that is assigned owns (see ParamOwns), whichever way the function
            // is called (a dynamic dispatch, or a function value called thru its type, copies
            // on entry), so this is only reachable when a parameter the parser saw no
            // assignment to gets assigned after all (a FreeVarRef, see CheckLval).
            ErrorAlways(*n, "cannot assign to borrowed argument ", Q(lv.sid->id->name));
        }
        // Borrows and the write are compared as every path that may name the location: a
        // function on the stack may have borrowed it thru a parameter that aliases it, the
        // write may be thru one as well, and either may go thru an owning variable that was
        // assigned the object, see ExpandAliases.
        auto written = ExpandAliases(lv);
        // FIXME: make this faster.
        for (auto &b : reverse(borrowstack)) {
            if (!b.refc) continue;          // Lval is not borowed, writing is ok.
            if (!WriteHits(ExpandAliases(b), written)) continue;  // Not overwriting this one.
            if (!b.spec_holders.empty()) {
                // Variables borrowing this speculatively own a reference instead from here on.
                auto holders = b.spec_holders;
                for (auto h : holders) FlipSpeculative(h);
                if (!b.refc) continue;
            }
            // A borrow of (something reached thru) a variable that itself borrows
            // speculatively: that variable owning a reference instead may be what makes the
            // write safe, see LValContext::Step.
            for (auto s = b.sid; s; s = s->alias_sid) {
                if (s->speculative) FlipSpeculative(const_cast<SpecIdent *>(s));
            }
            // The borrow may be the argument of an active call for a parameter bound to it,
            // or be of (something reached thru) such a parameter: the caller of the
            // outermost such call keeps the value alive instead, see KeepArgAlive.
            auto bi = (Lifetime)(&b - &borrowstack[0]);
            auto keep_args_of = [&](SubFunction *sf) {
                for (auto &arg : sf->args) {
                    if (arg.sid->arg_slot && (*arg.sid->arg_slot)->lt == bi) KeepArgAlive(arg.sid);
                }
            };
            for (auto &sc : scopes) keep_args_of(sc.sf);
            // A reused specialization is not on the stack while its writes are replayed.
            if (replaying) keep_args_of(replaying);
            {
                LValContext walk = b;
                const SpecIdent *pick = nullptr;
                for (;;) {
                    if (walk.derefs.empty() && walk.sid->arg_slot) pick = walk.sid;
                    if (!walk.Step()) break;
                }
                if (pick) KeepArgAlive(const_cast<SpecIdent *>(pick));
            }
            // Or it may be held by an earlier element of an argument list whose later
            // element is doing the write (f(v[0], v.pop())): that element takes a reference
            // of its own instead (an inc), which the call it is for adjusts like any owned
            // value it is passed. The elements still to be typechecked have no lifetime yet.
            for (auto list : arglists) {
                for (auto &c : list->children) {
                    if (c->lt != bi) continue;
                    LOG_DEBUG("argument ", NiceName(*c), " owns a reference for a later write");
                    DecBorrowers(c->lt, *c);
                    MakeLifetime(c, LT_KEEP, 1, 0);
                }
            }
            if (!b.refc) continue;
            if (!WriteHits(ExpandAliases(b), written)) continue;
            LValContext cb = b;
            cb.Canonicalize();
            LValContext clv = lv;
            clv.Canonicalize();
            auto same = cb.sid == clv.sid && cb.DerefsEqual(clv);
            Error(*n, "cannot modify ", Q(lv.Name()), " while ",
                      same ? string("it is borrowed") : cat(Q(b.Name()), " borrows it"),
                      " at ", parser.lex.Location(b.line));
        }
    }

    // Whether a variable of this type could borrow what it is initialized with, see
    // SpecIdent::speculative: a reference, but not a struct of them, which is several.
    bool SpecBorrowable(TypeRef type) {
        auto e = type->ElementIfNil();
        return IsRefNil(type->t) && !IsStruct(e->t) && e->t != V_VAR;
    }

    // `sid` borrows the location `lt` (a borrow stack entry that has a count for it already)
    // for the rest of its scope, and names the same location as `alias`.
    void HoldSpeculative(SpecIdent *sid, Lifetime lt, Define *def, const LValContext &alias) {
        assert(lt >= 0);
        sid->lt = lt;
        sid->speculative = true;
        sid->spec_define = def;
        borrowstack[lt].spec_holders.push_back(sid);
        sid->alias_sid = alias.sid;
        sid->alias_derefs = alias.derefs;
        LOG_DEBUG("speculative borrow: ", sid->id->name, " of ", borrowstack[lt].Name());
    }

    // Whether a loop of the current function that the code being typechecked is inside of
    // wrote to `path` earlier in its body. A variable defined in a loop lives across its
    // iterations (a function value made after it can be called before its definition runs
    // again), so such a write is one to what it would borrow from while it is alive.
    bool LoopWroteBefore(const LValContext &path) {
        auto &sc = scopes.back();
        if (sc.loop_flow.empty()) return false;
        auto held = ExpandAliases(path);
        auto &evs = sc.sf->reuse_assign_events;
        for (size_t i = sc.loop_events_start; i < evs.size(); i++) {
            if (WriteHits(held, ExpandAliases(evs[i].lv))) return true;
        }
        return false;
    }

    void DropSpeculative(SpecIdent *sid) {
        auto &b = borrowstack[sid->lt];
        for (auto [i, h] : enumerate(b.spec_holders)) {
            if (h == sid) {
                b.spec_holders.erase(i);
                break;
            }
        }
        b.refc--;
        assert(b.refc >= 0);
        sid->speculative = false;
    }

    // What `sid` borrows from is about to be written, so it owns a reference instead: its
    // initializer gets an inc, and it gets a dec when it goes out of scope or is overwritten
    // like any owning variable, see CodeGen::ShouldDec.
    void FlipSpeculative(SpecIdent *sid) {
        if (!sid->speculative) return;
        LOG_DEBUG("speculative borrow of ", sid->id->name, " flipped to keep");
        DropSpeculative(sid);
        sid->lt = LT_KEEP;
        if (sid->spec_define) MakeLifetime(sid->spec_define->child, LT_KEEP, 1, 0);
    }

    // The scope of `sid` ends without anything having written to what it borrows.
    void ReleaseSpeculative(SpecIdent *sid) {
        DropSpeculative(sid);
        sid->lt = LT_BORROW;
    }

    // The free variables functions use, including what flow typing knows about them.

    void CheckFreeVariable(SpecIdent &sid) {
        // We mostly don't track variables from top level scope as freevars, since they
        // are the same for everyone, unless the type could be changed by flow typing.
        // This check typically culls >90% of free variables.
        if (sid.sf_def->parent->scopelevel == 1 && !sid.type->FlowSensitive()) {
            if (scopes.size() > 1) {
                // Set this already for codegen, since it won't show up in freevars.
                sid.used_as_freevar = true;
            }
            return;
        }
        // These will be removed by the optimizer. No need to count them as free variables either.
        if (sid.constprop)
            return;
        // If this is a free variable, record it in all parents up to the definition point.
        // FIXME: this is technically not the same as a "free variable" in the literature,
        // since HOFs get marked with freevars of their functionvalue this way.
        // This is benign, since the HOF will be specialized to the function value anyway,
        // but would be good to clean up.
        // We currently don't have an easy way to test for lexically enclosing functions.
        for (int i = (int)scopes.size() - 1; i >= 0; i--) {
            auto sf = scopes[i].sf;
            // Check if we arrived at the definition point.
            if (sid.sf_def == sf)
                break;
            FlowItem fi(&sid, sid.type, sid.type);
            assert(fi.IsValid());
            auto it = sf->IterFreeVar(sid);
            if (sf->IsFreeVar(it, sid)) {
                // If the freevar was already there, a previous call must have added it all the way
                // to the definition point, so we can stop here too.
                break;
            } else {
                auto flowtype = UseFlow(fi, scopes[i].flowstack_size);
                sf->AddFreeVar(it, sid, flowtype);
            }
        }
    }

    void CheckFreeVariablesFromFunction(SubFunction *sf) {
        if (sf->freevars.empty()) return;
        auto par = scopes.back().sf;
        // Checking these freevars is very expensive, so we check for multiple calls in the
        // same parent, which removes about 30%.
        if (freevar_check_preempt.find({ sf, par }) == freevar_check_preempt.end()) {
            for (auto &fv : sf->freevars) CheckFreeVariable(*fv.sid);
            freevar_check_preempt.insert({ sf, par });
        }
    }

    void FlowFieldAddToFreeVariables(FlowItem &fi, TypeRef unpromoted) {
        // Here we do a specialized thing: if this field was upgraded, and it is on
        // the basis of a free variable, then future checks for compatible free variables
        // need to check if the new callsite has this upgrade also.
        // FIXME: this needs to be on the basis of the flowstack at the point of each of these functions!
        for (int i = (int)scopes.size() - 1; i >= 0; i--) {
            auto sf = scopes[i].sf;
            // Check if we arrived at the definition point.
            if (fi.sid->sf_def == sf)
                break;
            auto it = sf->IterFreeVar(*fi.sid);
            if (sf->IsFreeVar(it, *fi.sid)) {
                for(auto &fvfi : sf->freevarflowfields) {
                    if (fvfi.sid == fi.sid && fvfi.DerefsEqual(fi)) {
                        // If the flow item was already there, a previous call must have added it all the way
                        // to the definition point, so we can stop here.
                        return;
                    }
                }
                auto flowtype = UseFlow(fi, scopes[i].flowstack_size);
                if (!flowtype->Equal(*unpromoted)) {
                    fi.now = flowtype;
                    sf->freevarflowfields.push_back(fi);
                } else {
                    // We've crossed the point where the field was promoted, so we can stop.
                    return;
                }
            }
        }
    }
};

}  // namespace lobster
