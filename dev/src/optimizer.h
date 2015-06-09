
namespace lobster
{
    
struct Optimizer
{
    Parser &parser;
    SymbolTable &st;
    bool changes_this_pass;
    Value constant;
    Node *dummy_node;
    
    Optimizer(Parser &_p, SymbolTable &_st, int maxpasses)
        : parser(_p), st(_st), changes_this_pass(false), dummy_node(nullptr)
    {
        //dummy_node = NewNode(T_EMPTY, type_any);
        
        for (int i = 0; i < maxpasses; i++)
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
            
            if (!changes_this_pass) break;
        }
    }
    
    void Changed() { changes_this_pass = true; }
    
    Node *NewNode(Node *context, TType t, TypeRef type, Node *a = nullptr, Node *b = nullptr)
    {
        auto n = new Node(context->line, t, a, b);
        n->exptype = type;
        Changed();
        return n;
    }
    
    Node *NewTernary(Node *context, TType t, TypeRef type, Node *a = nullptr, Node *b = nullptr, Node *c = nullptr)
    {
        auto n = (Node *)new Ternary(context->line, t, a, b, c);
        n->exptype = type;
        Changed();
        return n;
    }

    bool IsConst(Node &n)
    {
        switch (n.type)
        {
            case T_INT:   constant = Value(n.integer());    return true;
            case T_FLOAT: constant = Value((float)n.flt()); return true;
            case T_NIL:   constant = Value(nullptr, V_NIL); return true;
            default: return false;
        }
    }
    
    void Optimize(Node *&n_ptr)
    {
        Node &n = *n_ptr;
        
        if (n.a()) Optimize(n.aref());
        if (n.b()) Optimize(n.bref());
        if (n.c()) Optimize(n.cref());
        
        switch (n.type)
        {
            case T_IF:
                if (IsConst(*n.if_condition()))
                {
                    auto branch = constant.True() ? n.if_then() : n.if_else();
                    auto other  = constant.True() ? n.if_else() : n.if_then();
                    if (branch->type != T_DEFAULTVAL)
                    {
                        n_ptr = NewTernary(branch, T_DYNCALL, branch->sf()->returntypes[0], branch, branch, nullptr);
                    }
                    else
                    {
                        n_ptr = branch;
                    }
                    if (other) other->sf()->parent->DeleteSubFunction(other->sf());
                }
                break;
        }
    }
};
    
}  // namespace lobster


