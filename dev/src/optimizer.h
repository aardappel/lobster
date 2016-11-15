
namespace lobster {

struct Optimizer {
    Parser &parser;
    SymbolTable &st;
    TypeChecker &tc;
    bool changes_this_pass;
    size_t total_changes;
    Node *dummy_node;
    SubFunction *cursf;

    Optimizer(Parser &_p, SymbolTable &_st, TypeChecker &_tc, int maxpasses)
        : parser(_p), st(_st), tc(_tc), changes_this_pass(true), total_changes(0),
          dummy_node(nullptr), cursf(nullptr) {
        //dummy_node = NewNode(T_EMPTY, type_any);
        int i = 0;
        // MUST run at least 1 pass, to guarantee certain unwanted code is gone.
        maxpasses = max(1, maxpasses);
        for (; changes_this_pass && i < maxpasses; i++) {
            changes_this_pass = false;
            // We don't optimize parser.root, it only contains a single call.
            for (auto f : parser.st.functiontable) {
                if (f->subf && f->subf->typechecked) {
                    for (auto sf = f->subf; sf; sf = sf->next) if (sf->body) {
                        cursf = sf;
                        Optimize(sf->body, T_LIST);
                    }
                }
            }
        }
        Output(OUTPUT_INFO, "optimizer: %d passes, %d optimizations", i, total_changes);
        assert(i);  // Must run at least one pass.
    }

    void Changed() { changes_this_pass = true; total_changes++; }

    Node *Typed(TypeRef type, Node *n) {
        Changed();
        n->exptype = type;
        return n;
    }

    void Optimize(Node *&n_ptr, TType parent_type) {
        Node &n = *n_ptr;
        switch (n.type) {
            case T_LIST:
                // Flatten the Optimize recursion a bit
                for (Node *stats = &n; stats; stats = stats->b()) Optimize(stats->aref(), T_LIST);
                return;
            case T_IF: {  // This optimzation MUST run, since it deletes untypechecked code.
                Optimize(n.if_condition(), T_IF);
                Value cval;
                if (tc.ConstVal(*n.if_condition(), cval)) {
                    Changed();
                    auto branch = cval.True() ? n.if_then() : n.if_else();
                    auto other  = cval.True() ? n.if_else() : n.if_then();
                    Optimize(branch, T_IF);
                    n_ptr = branch;
                    if (other->type != T_DEFAULTVAL) {
                        if (other->type == T_E2N) other = other->child();
                        auto &sf = other->call_function()->sf();
                        if (!sf->typechecked) {
                            // Typechecker did not typecheck this function for use in this if-then,
                            // but neither did any other instances, so it can be removed.
                            sf->parent->RemoveSubFunction(sf);
                            sf = nullptr;
                        }
                    }
                } else {
                    Optimize(n.if_then(), T_IF);
                    Optimize(n.if_else(), T_IF);
                }
                return;
            }
        }
        if (n.a()) Optimize(n.aref(), n.type);
        if (n.b()) Optimize(n.bref(), n.type);
        if (n.c()) Optimize(n.cref(), n.type);
        switch (n.type) {
            case T_IS: {
                Value cval;
                if (tc.ConstVal(n, cval)) {
                    n_ptr = Typed(type_int, new Node(n.line, cval.ival()));
                    if (n.left()->HasSideEffects()) {
                        n_ptr = Typed(type_int, new Node(n.line, T_SEQ, n.left(), n_ptr));
                    }
                }
                break;
            }
            case T_DYNCALL: {  // This optimization MUST run, to remove redundant arguments.
                auto ftype = n.dcall_fval()->exptype;
                if (ftype->IsFunction()) {
                    auto sf = ftype->sf;
                    int i = 0;
                    for (auto list = &n.dcall_args(); *list; list = &(*list)->tail()) {
                        if (i++ == sf->parent->nargs()) {
                            *list = nullptr;
                            break;
                        }
                    }
                    // Now convert it to a T_CALL if possible. This also allows it to be inlined.
                    auto spec_sf = n.dcall_function()->sf();
                    (void)spec_sf;
                    assert(sf && sf == spec_sf);  // Sanity check.
                    if (!sf->parent->istype && !n.dcall_fval()->HasSideEffects()) {
                        n_ptr = Typed(n.exptype,
                                      new Node(n.line, T_CALL, n.dcall_function(), n.dcall_args()));
                    }
                }
                break;
            }
            case T_CALL: {
                auto sf = n.call_function()->sf();
                // FIXME: Reduce these requirements where possible.
                if (/*parent_type == T_FOR ||*/  // Always inline for bodies.
                    (sf->parent->anonymous &&
                     !sf->iscoroutine &&
                     !sf->dynscoperedefs.size() &&
                     sf->returntypes.size() <= 1 &&
                     parent_type != T_FOR))
                {
                    for (auto &arg : sf->locals.v) if (arg.id->logvaridx >= 0) goto skip;
                    if (sf->numcallers <= 1 || CountNodes(sf->body) < 8) {  // FIXME: configurable.
                        n_ptr = Inline(n, *sf);
                    }
                    skip:;
                }
                break;
            }
        }
    }

    Node *Inline(Node &call, SubFunction &sf) {
        // Note that sf_def in these Ident's being moved is now not correct anymore, but the
        // only use for that field is to determine if the variable is "global" after the optimizer,
        // so we let that slip.
        cursf->locals.v.insert(cursf->locals.v.end(), sf.args.v.begin(), sf.args.v.end());
        cursf->locals.v.insert(cursf->locals.v.end(), sf.locals.v.begin(), sf.locals.v.end());
        int ai = 0;
        Node *newn = nullptr;
        Node **tail = &newn;
        for (auto list = call.call_args(); list; list = list->tail()) {
            auto &arg = sf.args.v[ai];
            *tail = Typed(type_any,
                        new Node(call.line, T_LIST,
                            Typed(type_any,
                                new Node(call.line, T_DEF,
                                    Typed(arg.type,
                                        new Node(call.line, arg.id, arg.sid)),
                                    list->head(),
                                    nullptr)),
                            nullptr));
            list->head() = nullptr;
            tail = &(*tail)->bref();
            ai++;
        }
        if (sf.numcallers <= 1) {
            *tail = sf.body;
            sf.body = nullptr;
            sf.parent->RemoveSubFunction(&sf);
        } else {
            *tail = sf.body->Clone();
            sf.numcallers--;
        }
        newn = Typed(sf.returntypes[0], new Node(call.line, T_INLINED, newn));
        return newn;
    }
};

}  // namespace lobster


