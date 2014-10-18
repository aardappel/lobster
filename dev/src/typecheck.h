
struct TypeChecker
{
    Lex &lex;
    Parser &parser;
    SymbolTable &st;
    vector<SubFunction *> scopes;

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st)
    {
        parser.root->exptype = TypeCheck(*parser.root);
    }

    void TypeError(const char *required, const Type &got, const Node &n, const char *context = nullptr)
    {
        TypeError(string("\"") +
                  (context ? context : TName(n.type)) + 
                  "\" requires type: " + 
                  required + 
                  ", got: " + 
                  st.TypeName(got).c_str(),
                  n);
    }

    void TypeError(string err, const Node &n)
    {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            auto sf = *it;
            err += "\n  in function: " + sf->parent->name + "(";
            for (int i = 0; i < sf->parent->nargs; i++)
            {
                if (i) err += ", ";
                auto &arg = sf->args[i];
                err += arg.id;
                if (arg.type.t != V_ANY)
                {
                    err += ":";
                    err += st.TypeName(arg.type);
                }
            }
            err += ")";
        }
        parser.Error(err, &n);
    }

    bool ConvertsTo(const Type &type, const Type &sub)
    {
        switch (sub.t)
        {
            case V_FLOAT: return type.t == V_INT;
            case V_STRING: return true;
        }
        return false;
    }

    Type Union(const Node *a, const Node *b, bool coercions)
    {
         return Union(a->exptype, b->exptype, coercions);
    }
    Type Union(const Type &at, const Type &bt, bool coercions)
    {
        if (at.t == bt.t)
        {
            if (at == bt) return at;
            if (at.t == V_VECTOR) return Type(V_VECTOR);
        }
        else
        {
            if (coercions)
            {
                if (ConvertsTo(at, bt)) return bt;
                if (ConvertsTo(bt, at)) return at;
            }
            if ((at.t == V_VECTOR && bt.t == V_STRUCT) ||
                (bt.t == V_VECTOR && at.t == V_STRUCT)) return Type(V_VECTOR);
        }
        return Type(V_ANY);
    }

    bool ExactType(const Node &a, const Type &sub)
    {
        const Type &type = a.exptype;
        return type == sub;
    }

    void SubType(const Type &sub, Node &n) { SubType(n.left(), sub, n); SubType(n.right(), sub, n); }

    void SubType(Node *&a, const Type &sub, const Node &context) { SubType(a, sub, TName(context.type)); }
    void SubType(Node *&a, const Type &sub, const char *context)
    {
        const Type &type = a->exptype;
        if (type == sub) return;
        switch (sub.t)
        {
            case V_ANY: return;
            case V_NILABLE: if (type.t == V_NIL || type == sub.Element()) return; break;  // FIXME: more flexible
            case V_VECTOR:
                if (sub.idx < 0)
                {
                    if (type.t == V_VECTOR || type.t == V_STRUCT) return;
                }
                break;
            case V_FLOAT: if (type.t == V_INT) { a = new Node(lex, T_I2F, a); return; } break;
            case V_STRING: a = new Node(lex, T_A2S, a); return;
        }
        TypeError(st.TypeName(sub).c_str(), type, *a, context);
    }

    Node *RetVal(Node *a)
    {
        if (scopes.empty()) return a;
        auto scope = scopes.back();
        if (scope->returntype.t == V_UNDEFINED) scope->returntype = a ? a->exptype : Type();
        else if (a) SubType(a, scope->returntype, "return value");
        else if (scope->returntype.t != V_ANY)
            parser.Error(scope->parent->name + " must return a value");
        return a;
    }

    void TypeCheck(SubFunction &sf)
    {
        if (!sf.typechecked)
        {
            scopes.push_back(&sf);

            sf.typechecked = true;
            int i = 0;
            for (Node *params = sf.body->parameters(); params; params = params->tail())
            {
                auto &arg = sf.args[i++];
                // FIXME: these idents are shared between clones. That will work for now, 
                // but will become an issue when we want to store values non-uniformly.
                params->head()->ident()->type = arg.type;
            }
            sf.body->exptype = TypeCheck(*sf.body);
            Node *last = nullptr;
            for (auto topl = sf.body->body(); topl; topl = topl->tail()) last = topl;
            assert(last);
            last->head() = RetVal(last->head());

            scopes.pop_back();
        }
    }

    void MathCheck(const Type &type, const Node &n)
    {
        if (n.type == T_MOD || n.type == T_MODEQ)
        {
            if (type.t != V_INT)
                TypeError("int", type, n);
        }
        else if (n.type == T_PLUS || n.type == T_PLUSEQ)
        {
            if (!type.Numeric() && type.t != V_VECTOR && type.t != V_STRUCT && type.t != V_STRING)
                TypeError("numeric/string/vector/struct", type, n);
        }
        else
        {
            if (!type.Numeric())
                TypeError("numeric", type, n);
        }
    }

    Type TypeCheck(Node &n)
    {
        switch (n.type)
        {
            case T_FUNDEF:
                return Type();
        }

        if (n.HasChildren())
        {
            if (n.a()) n.a()->exptype = TypeCheck(*n.a());
            if (n.b()) n.b()->exptype = TypeCheck(*n.b());
        }

        switch (n.type)
        {
            case T_INT:   return Type(V_INT);
            case T_FLOAT: return Type(V_FLOAT);
            case T_STR:   return Type(V_STRING);
            case T_NIL:   return Type(V_NIL);

            case T_DIV:
            case T_MULT:
            case T_MINUS:
            case T_PLUS:
            case T_MOD:
            {
                auto type = Union(n.left(), n.right(), true);
                MathCheck(type, n);
                SubType(type, n);
                return type;
            }

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:
            {
                auto type = n.left()->exptype;
                MathCheck(type, n);
                SubType(n.right(), type, n);
                return type;
            }

            case T_NEQ: 
            case T_EQ: 
            case T_GTEQ: 
            case T_LTEQ: 
            case T_GT:  
            case T_LT:
            {
                auto type = Union(n.left(), n.right(), true);
                if (!type.Numeric() && type.t != V_STRING)
                {
                    if (n.type == T_EQ || n.type == T_NEQ)
                    {
                        if (type.t != V_VECTOR && type.t != V_STRUCT)
                            TypeError("numeric/string/vector/struct", type, n);
                    }
                    else
                    {
                        TypeError("numeric/string", type, n);
                    }
                }
                SubType(type, n);
                return Type(V_INT);
            }

            case T_AND:
            case T_OR:
            {
                auto type = Union(n.left(), n.right(), false);
                return type;         
            }
            case T_NOT:
                return Type(V_INT);

            case T_POSTDECR:
            case T_POSTINCR:
            case T_DECR:  
            case T_INCR:
            {
                auto type = n.child()->exptype;
                if (!type.Numeric())
                    TypeError("numeric", type, n);
                return type;
            }

            case T_UMINUS:
            {
                auto type = n.child()->exptype;
                if (!type.Numeric() && type.t != V_VECTOR)
                    TypeError("numeric/vector", type, n);
                return type;
            }

            case T_IDENT:
                return n.ident()->type;

            case T_DEF:
                return n.left()->ident()->type = n.right()->exptype;

            case T_NATCALL:
            {
                auto nf = n.ncall_id()->nf();
                int i = 0;
                for (Node *list = n.ncall_args(); list; list = list->tail())
                {
                    SubType(list->head(), nf->args[i++].type, nf->name.c_str());
                }
                return nf->nretvalues ? nf->retvals[0].type : Type();  // FIXME: multiple retvals
            }

            case T_CALL:
            {
                auto &f = *n.call_function()->sf()->parent;
                if (f.multimethod)
                {
                    // Simplistic: typechecked with actual argument types.
                    // Should attempt static picking as well, if static pick succeeds, specialize.
                    // FIXME: no need to repeat this on every call.
                    for (auto sf = f.subf; sf; sf = sf->next) TypeCheck(*sf);
                    Type type = f.subf->returntype;
                    for (auto sf = f.subf->next; sf; sf = sf->next) type = Union(type, sf->returntype, false);
                    return type;
                }
                else
                {
                    int specializable = 0;
                    for (int i = 0; i < f.nargs; i++) if (f.subf->args[i].flags == AF_ANYTYPE) specializable++;
                    SubFunction *sf = f.subf;
                    if (specializable)
                    {
                        // Check if any fit.
                        for (; sf && sf->typechecked; sf = sf->next)
                        {
                            int i = 0;
                            for (Node *list = n.call_args(); list; list = list->tail())
                            {
                                auto &arg = sf->args[i++];
                                if (arg.flags == AF_ANYTYPE && !ExactType(*list->head(), arg.type)) goto fail;
                            }
                            goto match;
                            fail:;
                        }
                        // No fit. Specialize existing function, or its clone.
                        sf = f.subf;
                        if (sf->typechecked)
                        {
                            // Clone it.
                            sf = new SubFunction(&f);
                            sf->args = new Arg[f.nargs];
                            for (int i = 0; i < f.nargs; i++) sf->args[i] = f.subf->args[i];
                            sf->body = f.subf->body->Clone();
                            sf->next = f.subf;
                            f.subf = sf;
                        }
                        int i = 0;
                        for (Node *list = n.call_args(); list; list = list->tail())
                        {
                            auto &arg = sf->args[i++];
                            if (arg.flags == AF_ANYTYPE)
                            {
                                arg.type = list->head()->exptype;  // Specialized to arg.
                            }
                        }
                    }
                    match:
                    // Here we have a SubFunction witch matching specialized types.
                    // First check all the manually typed args.
                    int i = 0;
                    for (Node *list = n.call_args(); list; list = list->tail())
                    {
                        auto &arg = sf->args[i++];
                        if (arg.flags != AF_ANYTYPE) SubType(list->head(), arg.type, f.name.c_str());
                    }
                    TypeCheck(*sf);
                    n.call_function()->sf() = sf;
                    return sf->returntype;
                }
            }

            case T_RETURN:
            {
                // FIXME multiret
                n.return_value() = RetVal(n.return_value());
                return Type();
            }

            case T_TYPE:
                return *n.typenode();

            case T_IS:
                // FIXME If the typecheck fails statically, we can replace this node with false
                return Type(V_INT);

            case T_FIELD:
                return n.exptype;  // Already set by the parser.

            case T_CONSTRUCTOR:
            {
                auto &type = *n.constructor_type()->typenode();
                // FIXME: must specialize if there's any untyped fields.
                int i = 0;
                for (auto list = n.constructor_args(); list; list = list->tail())
                {
                    auto elemtype = type.t == V_STRUCT ? st.structtable[type.idx]->fields[i].type : type.Element();
                    SubType(list->head(), elemtype, n);
                    i++;
                }
                return type;
            }

            case T_DOT:
            {
                auto &type = n.left()->exptype;
                if (type.t != V_STRUCT)
                    TypeError("struct/value", type, n);
                auto struc = st.structtable[type.idx];
                auto sf = n.right()->fld();
                auto uf = struc->Has(sf);
                if (!uf) TypeError("type " + struc->name + " has no field named " + sf->name, n);
                return uf->type;
            }

            case T_INDEX:
            {
                auto vtype = n.left()->exptype;
                if (vtype.t != V_VECTOR && vtype.t != V_STRING)
                    TypeError("vector/string", vtype, n);
                auto itype = n.right()->exptype;
                switch (itype.t)
                {
                    case V_INT: vtype = vtype.t == V_VECTOR ? vtype.Element() : Type(V_INT); break;
                    case V_STRUCT:
                    {
                        auto &struc = *st.structtable[itype.idx];
                        for (auto &field : struc.fields)
                        {
                            if (field.type.t != V_INT) TypeError("int field", field.type, n);
                            if (vtype.t != V_VECTOR) TypeError("nested vector", vtype, n);
                            vtype = vtype.Element();
                        }
                        break;
                    }
                    case V_VECTOR: // FIXME only way to typecheck this correctly is if we knew the length statically
                    default: TypeError("int/struct of int", itype, n);
                }
                return vtype;
            }

            case T_SEQ:
                return n.right()->exptype;

            case T_ASSIGN:
                SubType(n.right(), n.left()->exptype, n);
                return n.left()->exptype;

            case T_CLOSURE:
                return Type(V_FUNCTION);


            case T_CO_AT:

            case T_ASSIGNLIST:

            case T_STRUCTDEF:

            case T_DYNCALL:

            case T_FUN:

            case T_NATIVE:

            case T_LIST:

            case T_MULTIRET:

            case T_COCLOSURE:

            case T_COROUTINE:







            case T_STRUCT:
                return n.exptype;

            default:
                assert(0);
                return n.exptype;
        }
    }
};
