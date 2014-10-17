
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

    void TypeError(const char *context, const char *required, const char *got, Node &n)
    {
        TypeError("\"" + string(context) + "\" requires type: " + required + ", got: " + got, n);
    }

    void TypeError(string err, Node &n)
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

    Type Union(const Node *a, const Node *b)
    {
        if (a->exptype == b->exptype) return a->exptype;
        if (ConvertsTo(a->exptype, b->exptype)) return b->exptype;
        if (ConvertsTo(b->exptype, a->exptype)) return a->exptype;
        return Type(V_ANY);
    }

    bool ExactType(const Node &a, const Type &sub)
    {
        const Type &type = a.exptype;
        return type == sub;
    }

    void SubType(Node *&a, const Type &sub, const char *context)
    {
        const Type &type = a->exptype;
        if (type == sub) return;
        switch (sub.t)
        {
            case V_ANY: return;
            //case V_FLOAT: if (type.t == V_INT) { a = new Node(T_I2F, a); return; } break;
        }
        TypeError(context, st.TypeName(sub).c_str(), st.TypeName(type).c_str(), *a);
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

    void TypeCheckBody(SubFunction *sf, Node *args, Function &f)
    {
        // Check all the manually typed args.
        int i = 0;
        for (Node *list = args; list; list = list->tail())
        {
            auto &arg = sf->args[i++];
            if (arg.flags != AF_ANYTYPE) SubType(list->head(), arg.type, f.name.c_str());
        }
        if (!sf->typechecked)
        {
            scopes.push_back(sf);

            sf->typechecked = true;
            i = 0;
            for (Node *params = sf->body->parameters(); params; params = params->tail())
            {
                auto &arg = sf->args[i++];
                // FIXME: these idents are shared between clones. That will work for now, 
                // but will become an issue when we want to store values non-uniformly.
                params->head()->ident()->type = arg.type;
            }
            sf->body->exptype = TypeCheck(*sf->body);
            Node *last = nullptr;
            for (auto topl = sf->body->body(); topl; topl = topl->tail()) last = topl;
            assert(last);
            last->head() = RetVal(last->head());

            scopes.pop_back();
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
                auto type = Union(n.left(), n.right());
                if (n.type == T_MOD)
                {
                    if (type.t != V_INT)
                        TypeError(TName(n.type), "int", st.TypeName(type).c_str(), n);
                }
                else if (n.type == T_PLUS)
                {
                    if (type.t != V_FLOAT && type.t != V_INT && type.t != V_VECTOR && type.t != V_STRUCT && type.t != V_STRING)
                        TypeError(TName(n.type), "numeric/string/vector", st.TypeName(type).c_str(), n);
                }
                else
                {
                    if (type.t != V_FLOAT && type.t != V_INT)
                        TypeError(TName(n.type), "numeric", st.TypeName(type).c_str(), n);
                }
                SubType(n.left(), type, TName(n.type));
                SubType(n.right(), type, TName(n.type));
                return type;
            }

            case T_NEQ: 
            case T_EQ: 
            case T_GTEQ: 
            case T_LTEQ: 
            case T_GT:  
            case T_LT:  
                return Type();

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
                    // FIXME
                    return Type();
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
                    TypeCheckBody(sf, n.call_args(), f);
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
                    SubType(list->head(), elemtype, "constructor");
                    i++;
                }
                return type;
            }

            case T_DOT:
            {
                auto &type = n.left()->exptype;
                if (type.t != V_STRUCT)
                    TypeError(".", "struct/value", st.TypeName(type).c_str(), n);
                auto struc = st.structtable[type.idx];
                auto sf = n.right()->fld();
                auto uf = struc->Has(sf);
                if (!uf) TypeError("type " + struc->name + " has no field named " + sf->name, n);
                return uf->type;
            }

            case T_CO_AT:

            case T_ASSIGNLIST:

            case T_ASSIGN: 

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:

            case T_POSTDECR: 
            case T_POSTINCR: 
            case T_DECR:  
            case T_INCR:  

            case T_UMINUS:


            case T_CLOSURE: 

            case T_STRUCTDEF:

            case T_DYNCALL:

            case T_FUN:

            case T_NATIVE:

            case T_LIST:

            case T_SEQ:

            case T_MULTIRET:

            case T_AND:

            case T_OR:

            case T_NOT:



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
