
namespace lobster
{
    
struct Optimizer
{
    Parser &parser;
    SymbolTable &st;
    TypeChecker &tc;
    bool changes_this_pass;
    size_t total_changes;
    Node *dummy_node;
    
    Optimizer(Parser &_p, SymbolTable &_st, TypeChecker &_tc, int maxpasses)
        : parser(_p), st(_st), tc(_tc), changes_this_pass(true), total_changes(0), dummy_node(nullptr)
    {
        //dummy_node = NewNode(T_EMPTY, type_any);
        int i = 0;
        maxpasses = max(1, maxpasses);  // MUST run at least 1 pass, to guarantee certain unwanted code is gone.
        for (; changes_this_pass && i < maxpasses; i++)
        {
            changes_this_pass = false;
            Optimize(parser.root);
            for (auto f : parser.st.functiontable)
            {
                if (f->subf && f->subf->typechecked)
                {
                    for (auto sf = f->subf; sf; sf = sf->next) if (sf->body) Optimize(sf->body);
                }
            }
        }
        Output(OUTPUT_INFO, "optimizer: %d passes, %d optimizations", i, total_changes);
        assert(i);  // Must run at least one pass.
    }
    
    void Changed() { changes_this_pass = true; total_changes++; }
    
    Node *NewNode(Node *context, TType t, TypeRef type, Node *a = nullptr, Node *b = nullptr)
    {
        auto n = new Node(context->line, t, a, b);
        n->exptype = type;
        return n;
    }
    
    Node *NewTernary(Node *context, TType t, TypeRef type, Node *a = nullptr, Node *b = nullptr, Node *c = nullptr)
    {
        auto n = new Node(context->line, t, a, b, c);
        n->exptype = type;
        return n;
    }

    void Optimize(Node *&n_ptr)
    {
        Node &n = *n_ptr;

        switch (n.type)
        {
            case T_LIST:
                // Flatten the Optimize recursion a bit
                for (Node *stats = &n; stats; stats = stats->b()) Optimize(stats->aref());
                return;
                
            case T_IF:  // This optimzation MUST run, since it deletes untypechecked code.
            {
                Optimize(n.if_condition());
                Value cval;
                if (tc.ConstVal(*n.if_condition(), cval))
                {
                    Changed();
                    auto branch = cval.True() ? n.if_then() : n.if_else();
                    auto other  = cval.True() ? n.if_else() : n.if_then();
                    Optimize(branch);
                    n_ptr = branch;
                    if (other->type != T_DEFAULTVAL)
                    {
                        if (other->type == T_E2N) other = other->child();
                        auto &sf = other->call_function()->sf();
                        if (!sf->typechecked)
                        {
                            // Typechecker did not typecheck this function for use in this if-then, but neither did any
                            // other instances, so it can be removed.
                            sf->parent->DeleteSubFunction(sf);
                            sf = nullptr;
                        }
                    }
                }
                else
                {
                    Optimize(n.if_then());
                    Optimize(n.if_else());
                }
                return;
            }

        }
        
        if (n.a()) Optimize(n.aref());
        if (n.b()) Optimize(n.bref());
        if (n.c()) Optimize(n.cref());
        
        switch (n.type)
        {
            case T_IS:
            {
                Value cval;
                if (tc.ConstVal(n, cval))
                {
                    Changed();
                    // FIXME: if the LHS has side-effects, then this is incorrect!
                    n_ptr = new Node(n.line, cval.ival());
                }
                break;
            }

            case T_DYNCALL:  // This optimization MUST run, to remove redundant arguments.
            {
                auto ftype = n.dcall_fval()->exptype;
                if (ftype->IsFunction())
                {
                    auto sf = ftype->sf;
                    int i = 0;
                    for (auto list = &n.dcall_args(); *list; list = &(*list)->tail())
                    {
                        if (i++ == sf->parent->nargs())
                        {
                            delete *list;
                            *list = nullptr;
                            break;
                        }
                    }
                }
                break;
            }

        }
    }
};
    
}  // namespace lobster


