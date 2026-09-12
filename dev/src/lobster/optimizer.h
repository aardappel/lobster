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

struct Optimizer {
    SymbolTable &st;
    TypeChecker &tc;
    size_t total_changes = 0;
    vector<SubFunction *> sfstack;
    bool functions_removed = false;
    int runtime_checks;
    size_t always_inline = 32;
    size_t never_inline = 256;

    // A function this one is about to absorb reads whatever it read from an enclosing scope
    // from inside this one now, so those reads no longer happen in a body of their own, see
    // SpecIdent::freevar_reads.
    static void ForgetFreeVars(const SubFunction &sf) {
        for (auto &fv : sf.freevars) {
            assert(fv.sid->freevar_reads > 0);
            fv.sid->freevar_reads--;
        }
    }

    Optimizer(SymbolTable &_st, TypeChecker &_tc, int runtime_checks)
        : st(_st), tc(_tc), runtime_checks(runtime_checks) {
        for (auto f : st.functiontable)
            for (auto ov : f->overloads)
                for (auto sf = ov->sf; sf; sf = sf->next)
                    for (auto &fv : sf->freevars) fv.sid->freevar_reads++;
        if (runtime_checks >= RUNTIME_DEBUG) {
            // User wants to see useful stack-traces, only inline the tiniest of functions.
            always_inline = 4;
            never_inline = 8;
        }
        // We don't optimize parser.root, it only contains a single call.
        for (auto f : st.functiontable) {
            again:
            for (auto ov : f->overloads) {
                // The head of this chain is the newest specialization, which may be one that
                // was cloned at a call site and then never typechecked, so this cannot stop at
                // an untypechecked one: the ones behind it are what the codegen emits.
                for (auto sf = ov->sf; sf; sf = sf->next) {
                    functions_removed = false;
                    OptimizeFunction(*sf);
                    if (functions_removed) goto again;
                }
            }
        }
        LOG_INFO("optimizer: ", total_changes, " optimizations");
    }

    void OptimizeFunction(SubFunction &sf) {
        if (sf.optimized) return;
        sf.optimized = true;
        if (!sf.sbody) return;
        if (!sf.typechecked) {
            delete sf.sbody;
            sf.sbody = nullptr;
            sf.node_count = 0;
            return;
        }
        sfstack.push_back(&sf);
        auto nb = sf.sbody->Optimize(*this);
        sf.node_count = sf.sbody->Count();
        assert(nb == sf.sbody);
        (void)nb;
        sfstack.pop_back();
    }

    void Changed() { total_changes++; }

    // One of the bindings inlining made for the arguments of a call: which of the block's
    // children it is, the variable it binds, what to put in its place where that is a constant,
    // and whether anything is left that names it.
    struct ArgBinding {
        size_t idx;
        SpecIdent *sid;
        Node *con;
        bool named;
    };
    typedef small_vector<ArgBinding, 8> ArgBindings;

    // Substitutes the constants and collects what still names each variable, in one pass. Only
    // the body may be substituted into: an initializer runs before the binding it feeds, so a
    // read of the variable there is of whatever it held before.
    static void SubstAndName(Node **slot, ArgBindings &bs, bool subst) {
        auto n = *slot;
        if (auto ir = Is<IdentRef>(n)) {
            for (auto &b : bs) {
                if (b.sid != ir->sid) continue;
                if (subst && b.con) {
                    auto c = b.con->Clone(false);
                    c->line = ir->line;
                    delete n;
                    *slot = c;
                } else {
                    b.named = true;
                }
                break;
            }
            return;
        }
        // A dynamic call, a member and a static each hold a variable of their own with no
        // IdentRef for it in the tree, so they name one too.
        const SpecIdent *sid = nullptr;
        if (auto dc = Is<DynCall>(n)) sid = dc->sid;
        else if (auto mem = Is<Member>(n)) sid = mem->this_sid;
        else if (auto st = Is<Static>(n)) sid = st->sid;
        if (sid) for (auto &b : bs) if (b.sid == sid) { b.named = true; break; }
        auto ch = n->Children();
        for (size_t i = 0; i < n->Arity(); i++) SubstAndName(&ch[i], bs, subst);
    }

    // Whether this tree binds any of the variables, which is what a second copy of the same
    // function inlined inside one of the arguments does, see Call::Optimize.
    static bool BindsAny(Node *n, const vector<Arg> &args) {
        if (auto def = Is<Define>(n))
            for (auto &p : def->tsids)
                for (auto &arg : args) if (arg.sid == p.sid) return true;
        auto ch = n->Children();
        for (size_t i = 0; i < n->Arity(); i++) if (BindsAny(ch[i], args)) return true;
        return false;
    }

    // Turns each return to sf in this tree into a jump to the end of the InlineBlock its body
    // became, see Call::Optimize.
    static void ReturnsToJumps(Node **slot, SubFunction *sf, size_t &count) {
        auto n = *slot;
        if (auto ret = Is<Return>(n)) {
            if (ret->sf == sf) {
                auto ir = new InlineReturn(ret->line, ret->child, sf, ret->make_void);
                ir->exptype = ret->exptype;
                ir->lt = ret->lt;
                ret->child = nullptr;
                delete ret;
                *slot = n = ir;
                count++;
            }
        }
        auto ch = n->Children();
        for (size_t i = 0; i < n->Arity(); i++) ReturnsToJumps(&ch[i], sf, count);
    }

    Node *Typed(TypeRef type, Lifetime lt, Node *n) {
        n->exptype = type;
        n->lt = lt;
        return n;
    }
};

Node *Node::Optimize(Optimizer &opt) {
    assert(exptype->t != V_UNDEFINED);
    for (size_t i = 0; i < Arity(); i++) {
        Children()[i] = Children()[i]->Optimize(opt);
    }
    VTValue cval;
    auto t = ConstVal(&opt.tc, cval);
    if (t == V_VOID) return this;
    // The constant has to be of the type the node says it produces, since that type is what
    // decides which stack slot codegen puts it in, and the code that reads it goes by the same
    // type. An `and` or an `or` that short circuits to a false scalar out of an expression whose
    // type is a reference is the case where the two disagree: what that produces is the nil of
    // the reference type, which is what the jump the unfolded code emits writes, see
    // EmitJumpCond. Anything else that disagrees is not ours to fold.
    if (t != exptype->t && t != V_NIL) {
        auto falsy = (t == V_INT || t == V_FLOAT) && !cval.i;
        if (!falsy || !IsRefNilNoStruct(exptype->t)) return this;
        t = V_NIL;
    }
    Node *r;
    switch (t) {
        case V_INT:   r = new IntConstant(line, cval.i); break;
        case V_FLOAT: r = new FloatConstant(line, cval.f); break;
        case V_NIL:   r = new Nil(line, { exptype }); break;
        default:      assert(false); return this;
    }
    r = opt.Typed(exptype, LT_ANY, r);
    delete this;
    opt.Changed();
    return r->Optimize(opt);
}

Node *Block::Optimize(Optimizer &opt) {
    // Not Node::Optimize: a function body is a Block as well and SubFunction::sbody has to
    // go on pointing at it, so a block never folds itself away. What it is worth is still
    // visible to whatever contains it, see Block::ConstVal.
    for (size_t i = 0; i < Arity(); i++) {
        Children()[i] = Children()[i]->Optimize(opt);
    }
    return this;
}

Node *Nil::Optimize(Optimizer &) {
    return this;
}

Node *IntConstant::Optimize(Optimizer &) {
    return this;
}

Node *FloatConstant::Optimize(Optimizer &) {
    return this;
}

// A divisor of neither zero nor minus one, which are the only two values the check in
// DivCheck can fire on. The children have already been folded by Node::Optimize, so a constant
// divisor is an IntConstant by now.
static bool SafeDivisor(const Node *n) {
    auto ic = Is<IntConstant>(n);
    return ic && ic->integer != 0 && ic->integer != -1;
}

Node *Divide::Optimize(Optimizer &opt) {
    auto r = Node::Optimize(opt);
    // Node::Optimize deletes this when it folds the whole expression away.
    if (r == this) divisor_safe = SafeDivisor(right);
    return r;
}

Node *Mod::Optimize(Optimizer &opt) {
    auto r = Node::Optimize(opt);
    if (r == this) divisor_safe = SafeDivisor(right);
    return r;
}

Node *IdentRef::Optimize(Optimizer &) {
    if (!sid->constprop) return this;
    auto con = sid->constprop->Clone(false);
    con->line = line;
    delete this;
    return con;
}

Node *Call::Optimize(Optimizer &opt) {
    Node::Optimize(opt);
    assert(sf->numcallers > 0);
    auto parent = opt.sfstack.back();
    // FIXME: Reduce these requirements where possible.
    bool is_inlinable =
        !sf->isrecursivelycalled &&
        // Every return has to be in the body itself, where a jump to the end of the block can
        // take its place, see InlineReturn. One in a function value the body calls, see e.g.
        // exception_handler, would return to a frame that is gone.
        // FIXME: if the function that caused this to be !=0 gets inlined, this needs to be
        // decremented so it can be inlined after all.
        sf->num_returns_non_local == 0 &&
        // The last statement is its own return, whose value the block falls thru with. What
        // else it can be is a return-from, which is another function's, see above.
        AssertIs<Return>(sf->sbody->children.back())->sf == sf &&
        vtable_idx < 0 &&
        // A terminal void function can be used where a value is expected. Removing its
        // final Return would expose a void expression to the value-producing caller.
        // Several values are fine: the block produces them the way the call did, since
        // Block::Generate asks its last child for as many as the caller wants, and a
        // MultipleReturn gives just those, see Return::Generate.
        sf->returntype->NumValues() == exptype->NumValues() &&
        // Because we inline so aggressively, it is possible to generate huuge functions,
        // which may cause a problem for our stack, or that of e.g. V8 in Wasm.
        parent->locals.size() < 1024;
    // Attempt to optimize function we're calling first, that way if it shrinks (or grows) it's
    // more or less likely to be inlined.
    if (is_inlinable) opt.OptimizeFunction(*sf);
    // Check if we should inline this call.
    if (!is_inlinable ||
        // Inline small functions even if called multiple times.
        (sf->numcallers > 1 && sf->node_count >= opt.always_inline) ||
        // Don't inline really gigantic functions, this helps with not flattening the call-graph too
        // much for stack traces, profiling and such, and may also make them easier to reg-alloc etc.
        (sf->numcallers <= 1 && sf->node_count >= opt.never_inline) ||
        // Don't inline functions that are being profiled.
        (LOBSTER_FRAME_PROFILER && sf->attributes.find("profile") != sf->attributes.end())) {
        return this;
    }
    // A copy shares its variables with every other copy of the same function in the same
    // parent, see AddToLocals, which is fine while the copies run one after the other. Inlining
    // this one would put a second copy inside one of its own arguments, so the bindings that
    // copy makes would overwrite these before the body here has read them. Leave it a call.
    for (auto c : children) if (Optimizer::BindsAny(c, sf->args)) return this;
    auto AddToLocals = [&](const vector<Arg> &av) {
        for (auto &arg : av) {
            // We have to check if the sid already exists, since inlining the same function
            // multiple times in the same parent can cause this. This variable is shared
            // between the copies in the parent, second use overwrites the first etc.
            // We generally have to keep using this sid rather than creating a new one, since
            // this function may call others that may refer to this sid, etc.
            for (auto &loc : parent->locals) if (loc.sid == arg.sid) goto already;
            parent->locals.push_back(arg);
            arg.sid->sf_def = parent;
            already:;
        }
    };
    AddToLocals(sf->args);
    AddToLocals(sf->locals);
    int ai = 0;
    auto list = new InlineBlock(line, sf);
    auto nargs = children.size();
    for (auto c : children) {
        auto &arg = sf->args[ai];
        // NOTE: this introduces locals which potentially borrow, which the typechecker so far
        // never introduces.
        // We can't just force these to LT_KEEP, since the sids maybe shared with non-inlined
        // instances (e.g. in dynamic dispatch).
        // Borrowing could be problematic if 2 copies of the same function get inlined, since
        // that creates an overwite of a borrowed variable, but the codegen ensures the overwrite
        // does not decref.
        auto def = new Define(line, c);
        def->tsids.push_back({ arg.sid, { arg.spec_type } });
        list->Add(opt.Typed(type_void, LT_ANY, def));
        ai++;
    }
    // TODO: triple-check this similar in semantics to what happens in CloneFunction() in the
    // typechecker.
    if (sf->numcallers == 1) {
        list->children.append(sf->sbody->children.data(), sf->sbody->children.size());
        sf->sbody->children.clear();
        delete sf->sbody;
        sf->sbody = nullptr;
        sf->node_count = 0;
        opt.functions_removed = sf->parent->RemoveSubFunction(sf);
        // Its body is part of the caller now, so whatever it read from an enclosing scope is no
        // longer read from a body of its own.
        opt.ForgetFreeVars(*sf);
        assert(opt.functions_removed);
    } else {
        for (auto c : sf->sbody->children) {
            auto nc = c->Clone(false);
            list->children.push_back(nc);
            nc->Iterate([](Node *i) {
                if (auto call = Is<Call>(i)) call->sf->numcallers++;
                if (auto dcall = Is<DynCall>(i)) dcall->sf->numcallers++;
            });
        }
    }
    sf->numcallers--;
    // The last statement is the function's own return, checked above. Its value is what the
    // block falls thru with, so only its child stays. The exception is the implicit return of a
    // function that does return values, which wraps a statement that never completes, see
    // Return::TypeCheck: that stays a return, so the statement is generated as one, and the
    // returns inside it are the only ways out.
    auto ret = AssertIs<Return>(list->children.back());
    assert(ret->sf == sf);
    assert(sf->num_returns_non_local == 0);
    auto replaced = !ret->make_void || !exptype->NumValues();
    if (replaced) {
        list->children.back() = ret->child;
        ret->child = nullptr;
        delete ret;
    }
    // Every other return to this function becomes a jump to the end of the block. They are all
    // in the body: one in a function value the body calls was excluded above, so a Return to
    // this function can only be left behind by a body that was itself inlined into it, which is
    // that same case.
    size_t jumps = 0;
    for (size_t i = nargs; i < list->children.size(); i++)
        Optimizer::ReturnsToJumps(&list->children[i], sf, jumps);
    assert(jumps + replaced <= sf->num_returns);
    // The bindings just made for the arguments, of which the first `nargs` children of the
    // block are the whole set: a constant argument is substituted into the body and its binding
    // goes, and so does one nothing names any more, which is what a function value left behind
    // by a lambda that was inlined into the body looks like. This works on the nodes of this
    // copy rather than thru sid->constprop, because AddToLocals shares one sid between every
    // copy of the same function inlined into the same parent, and the function the copy came
    // from goes on using it too. Each binds one variable; the ones it can say anything about are
    // those assigned once and not read from a body of its own.
    Optimizer::ArgBindings bs;
    for (size_t i = 0; i < nargs; i++) {
        auto def = AssertIs<Define>(list->children[i]);
        assert(def->tsids.size() == 1);
        auto sid = def->tsids[0].sid;
        if (!sid->id->single_assignment || sid->freevar_reads) continue;
        bs.push_back({ i, sid, def->child->IsConstProp(sid->type) ? def->child : nullptr, false });
    }
    if (!bs.empty()) {
        // Only the body is substituted into: an initializer runs before the binding it feeds, so
        // a read of the variable there is of whatever it held before.
        for (size_t i = 0; i < list->children.size(); i++)
            Optimizer::SubstAndName(&list->children[i], bs, i >= nargs);
        // An initializer that can do something of its own stays, and its binding with it: what
        // it produces is owned, and a bare statement would drop it.
        auto removed = false;
        for (auto &b : bs) {
            if (b.con) opt.Changed();
            auto def = AssertIs<Define>(list->children[b.idx]);
            if (b.named || def->child->SideEffectRec() || def->child->MayTrapRec()) continue;
            delete def;
            list->children[b.idx] = nullptr;
            removed = true;
            opt.Changed();
        }
        if (removed) {
            for (size_t i = list->children.size(); i-- > 0; )
                if (!list->children[i]) list->children.erase(i);
        }
    }
    auto r = opt.Typed(exptype, LT_KEEP, list);
    children.clear();
    r = r->Optimize(opt);
    delete this;  // Do this after, since Optimize may touch this same call.
    opt.Changed();
    return r;
}

}  // namespace lobster

