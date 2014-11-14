
struct TypeChecker
{
    Lex &lex;
    Parser &parser;
    SymbolTable &st;
    struct Scope { SubFunction *sf; const Node *call_context; };
    vector<Scope> scopes;
    vector<Type> type_variables;

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st)
    {
        parser.root->exptype = TypeCheck(*parser.root);
    }

    string TypeName(const Type &type) { return st.TypeName(type, type_variables.data()); }

    template<typename T> string TypedArg(const Typed<T> &arg)
    {
        string s = arg.id->name;
        if (arg.type.t != V_ANY) s += ":" + TypeName(arg.type);
        return s;
    }

    template<typename T> string Signature(const vector<Typed<T>> &v)
    {
        string s = "(";
        int i = 0;
        for (auto &arg : v)
        {
            if (i++) s += ", ";
            s += TypedArg(arg);
        }
        return s + ")"; 
    }

    string Signature(const Struct *struc)   { return struc->name      + Signature(struc->fields); }
    string Signature(const SubFunction *sf) { return sf->parent->name + Signature(sf->args.v); }

    string SignatureWithFreeVars(const SubFunction *sf)
    {
        string s = Signature(sf) + " { ";
        for (auto &freevar : sf->freevars.v) s += TypedArg(freevar) + " ";
        s += "}";
        return s;
    }

    void TypeError(const char *required, const Type &got, const Node &n, const char *context = nullptr)
    {
        TypeError(string("\"") +
                  (context ? context : TName(n.type)) + 
                  "\" requires type: " + 
                  required + 
                  ", got: " + 
                  TypeName(got).c_str(),
                  n);
    }

    void TypeError(string err, const Node &n)
    {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            auto &scope = *it;
            err += "\n  in " + parser.lex.Location(scope.call_context->fileidx, scope.call_context->linenumber) + ": ";
            err += SignatureWithFreeVars(scope.sf);
            for (Node *list = scope.sf->body; list; list = list->tail())
            {
                for (auto dl = list->head(); dl->type == T_DEF; dl = dl->right())
                {
                    auto id = dl->left()->ident();
                    if (id->type.t != V_ANY)
                    {
                        err += ", " + id->name + ":" + TypeName(id->type);
                    }
                }
            }
        }
        parser.Error(err, &n);
    }

    Type NewTypeVar()
    {
        type_variables.push_back(Type(V_UNDEFINED));
        return Type(V_ANYVAR, (int)type_variables.size() - 1);
    }

    Type Promote(const Type &type)
    {
        if (type.t == V_ANYVAR && type_variables[type.idx].t != V_UNDEFINED)
        {
            return Promote(type_variables[type.idx]);
        }
        else if (type.t == V_VECTOR)
        {
            auto pe = Promote(type.Element());
            return pe.CanWrap() ? pe.Wrap() : type;
        }
        else
        {
            return type;
        }
    }

    Type UnifyVar(const Type &type, const Type &hasvar)
    {
        auto &var = type_variables[hasvar.idx];
        if (var.t == V_UNDEFINED) var = type;
        return var;
    }

    bool ConvertsTo(const Type &type, const Type &sub, bool coercions)
    {
        if (sub == type) return true;
        if (type.t == V_ANYVAR) return ConvertsTo(UnifyVar(sub, type), sub, coercions);
        switch (sub.t)
        {
            case V_ANY:      return true;
            case V_ANYVAR:   return ConvertsTo(type, UnifyVar(type, sub), coercions);
            case V_FLOAT:    return type.t == V_INT && coercions;
            case V_STRING:   return coercions;
            case V_FUNCTION: return sub.idx < 0 && type.t == V_FUNCTION;
            case V_NILABLE:  return type.t == V_NIL || ConvertsTo(type, sub.Element(), false);
            case V_VECTOR:   return ((type.t == V_VECTOR && ConvertsTo(type.Element(), sub.Element(), false)) ||
                                     (type.t == V_STRUCT && sub.t2 == V_ANY));
        }
        return false;
    }

    Type Union(const Node *a, const Node *b, bool coercions)
    {
         return Union(a->exptype, b->exptype, coercions);
    }
    Type Union(const Type &at, const Type &bt, bool coercions)
    {
        if (ConvertsTo(at, bt, coercions)) return Promote(bt);
        if (ConvertsTo(bt, at, coercions)) return Promote(at);
        if (at.t == V_VECTOR && bt.t == V_VECTOR) return Type(V_VECTOR);
        return Type(V_ANY);
    }

    bool ExactType(const Type &a, const Type &b)
    {
        return Promote(a) == Promote(b);
    }

    void SubType(const Type &sub, Node &n) { SubType(n.left(), sub, n); SubType(n.right(), sub, n); }

    void SubType(Node *&a, const Type &sub, const Node &context) { SubType(a, sub, TName(context.type)); }
    void SubType(Node *&a, const Type &sub, const char *context)
    {
        const Type &type = a->exptype;
        if (ConvertsTo(type, sub, false)) { a->exptype = Promote(type); return; }
        switch (sub.t)
        {
            case V_FLOAT:
                if (Promote(type).t == V_INT)
                {
                    a = new Node(lex, T_I2F, a);
                    a->exptype = Type(V_FLOAT);
                    return;
                }
                break;
            case V_STRING:
                a = new Node(lex, T_A2S, a);
                a->exptype = Type(V_STRING);
                return;
        }
        TypeError(TypeName(sub).c_str(), type, *a, context);
    }
    void SubType(Type &type, const Type &sub, const Node &n, const char *context = nullptr)
    {
        if (!ConvertsTo(type, sub, false)) TypeError(TypeName(sub).c_str(), type, n, context);
        type = Promote(type);
    }

    void RetVal(Node *&a)
    {
        if (scopes.empty()) return;
        auto scope = scopes.back();
        if (a) SubType(a, scope.sf->returntype, "return value");
        else scope.sf->returntype = Type();  // FIXME: this allows "return" followed by "return 1" ?
    }

    void TypeCheck(SubFunction &sf, const Node *call_context)
    {
        if (!sf.typechecked)
        {
            Scope scope;
            scope.sf = &sf;
            scope.call_context = call_context;
            scopes.push_back(scope);

            sf.typechecked = true;

            auto backup_args = sf.args;
            auto backup_locals = sf.locals;
            int i = 0;
            for (auto &arg : sf.args.v)
            {
                // Need to not overwrite nested/recursive calls. e.g. map(): map(): ..
                backup_args.v[i].type = arg.id->type;

                // FIXME: these idents are shared between clones. That will work for now, 
                // but will become an issue when we want to store values non-uniformly.
                arg.id->type = arg.type;
                i++;
            }
            // Same for locals
            i = 0;
            for (auto &local : sf.locals.v)
            {
                backup_locals.v[i].type = local.id->type;
                local.id->type = local.type;
                i++;
            }

            sf.returntype = NewTypeVar();
            sf.body->exptype = TypeCheck(*sf.body);

            for (auto &back : backup_args.v) back.id->type = back.type;
            for (auto &back : backup_locals.v) back.id->type = back.type;

            Node *last = nullptr;
            for (auto topl = sf.body; topl; topl = topl->tail()) last = topl;
            assert(last);
            RetVal(last->head());

            scopes.pop_back();
        }
    }

    void MathCheck(const Type &type, const Node &n, int op)
    {
        if (op == T_MOD)
        {
            if (type.t != V_INT)
                TypeError("int", type, n);
        }
        else
        {
            if (!type.Numeric() && type.t != V_VECTOR && type.t != V_STRUCT)
            {
                if (op == T_PLUS)
                {
                    if (type.t != V_STRING)
                        TypeError("numeric/string/vector/struct", type, n);
                }
                else
                {
                    TypeError("numeric/vector/struct", type, n);
                }
            }
        }
    }

    Struct *SpecializeStruct(Struct *head, const Node *args)
    {
        // This code is very similar to function specialization, but not similar enough to share.
        // If they're all typed, we bail out early:
        for (auto &field : head->fields) if (field.flags == AF_ANYTYPE) goto specialize;
        return head;

        // First collect types for all args.
        specialize:
        vector<Type> argtypes;
        for (auto list = args; list; list = list->tail())
        {
            if (list->head()->type == T_SUPER)
            {
                auto stype = list->head()->exptype;
                SubType(stype, Type(V_STRUCT, head->superclassidx), *list->head(), "super");
                auto sstruc = st.structtable[stype.idx];
                for (auto &f : sstruc->fields) argtypes.push_back(f.type);
            }
            else
            {
                argtypes.push_back(list->head()->exptype);
            }
        }
        assert(argtypes.size() == head->fields.size());

        // Now find a match:
        auto struc = head;
        for (; struc; struc = struc->next)
        {
            int i = 0;
            for (auto &type : argtypes)
            {
                auto &field = struc->fields[i++];
                if (field.flags == AF_ANYTYPE && !ExactType(type, field.type)) goto fail;
            }
            return struc;  // Found a match.
            fail:;
        }

        // No match.
        struc = head;
        if (head->typechecked)
        {
            // This one is already in use.. clone it.
            struc = head->Clone();
            struc->idx = st.structtable.size();
            st.structtable.push_back(struc);
            DebugLog(1, "cloned struct: %s", struc->name.c_str());
        }
        int i = 0;
        for (auto &type : argtypes)
        {
            auto &field = struc->fields[i++];
            if (field.flags == AF_ANYTYPE) field.type = type;  // Specialize to arg.
        }
        DebugLog(1, "specialized struct: %s", Signature(struc).c_str());
        return struc;
    }

    Type TypeCheckCall(Function &f, Node *call_args, Node *function_def_node)
    {
        if (f.multimethod)
        {
            // Simplistic: typechecked with actual argument types.
            // Should attempt static picking as well, if static pick succeeds, specialize.
            // FIXME: no need to repeat this on every call.
            for (auto sf = f.subf; sf; sf = sf->next) TypeCheck(*sf, function_def_node);
            Type type = f.subf->returntype;
            for (auto sf = f.subf->next; sf; sf = sf->next) type = Union(type, sf->returntype, false);
            return type;
        }
        else
        {
            SubFunction *sf = f.subf;
            // First see any args are untyped, this means we must specialize.
            for (auto &arg : sf->args.v) if (arg.flags == AF_ANYTYPE) goto specialize;
            // If we didn't find any such args, and we also don't have any freevars, we don't specialize.
            if (!sf->freevars.v.size()) goto match;
            specialize:
            {
                // Check if any fit.
                for (; sf && sf->typechecked; sf = sf->next)
                {
                    int i = 0;
                    for (Node *list = call_args; list; list = list->tail())
                    {
                        auto &arg = sf->args.v[i++];
                        if (arg.flags == AF_ANYTYPE && !ExactType(list->head()->exptype, arg.type)) goto fail;
                    }
                    for (auto &freevar : sf->freevars.v)
                    {
                        // FIXME: call ExactType instead?
                        if (freevar.type != freevar.id->type) goto fail;
                    }
                    goto match;
                    fail:;
                }
                // No fit. Specialize existing function, or its clone.
                sf = f.subf;
                if (sf->typechecked)
                {
                    // Clone it.
                    DebugLog(1, "cloning: %s", sf->parent->name.c_str());
                    sf = new SubFunction();
                    sf->SetParent(f, f.subf);
                    sf->CloneIds(*f.subf->next);
                    sf->body = f.subf->next->body->Clone();
                }
                int i = 0;
                for (Node *list = call_args; list; list = list->tail())
                {
                    auto &arg = sf->args.v[i++];
                    if (arg.flags == AF_ANYTYPE)
                    {
                        arg.type = list->head()->exptype;  // Specialized to arg.
                    }
                }
                for (auto &freevar : sf->freevars.v)
                {
                    freevar.type = freevar.id->type;  // Specialized to current value.
                }
                DebugLog(1, "specialization: %s", SignatureWithFreeVars(sf).c_str());
            }
            match:
            // Here we have a SubFunction witch matching specialized types.
            // First check all the manually typed args.
            int i = 0;
            for (Node *list = call_args; list; list = list->tail())
            {
                auto &arg = sf->args.v[i++];
                if (arg.flags != AF_ANYTYPE) SubType(list->head(), arg.type, f.name.c_str());
            }
            TypeCheck(*sf, function_def_node);
            function_def_node->sf() = sf;
            DebugLog(1, "function %s returns %s", Signature(sf).c_str(), TypeName(sf->returntype).c_str());
            return sf->returntype;
        }
    }

    Type TypeCheckDynCall(Node *fval, Node **args_ptr, Node *fdef = nullptr)
    {
        auto ftype = Promote(fval->exptype);
        if (ftype.t == V_FUNCTION && ftype.idx >= 0)
        {
            // We can statically typecheck this dynamic call. Happens for almost all non-escaping closures.
            auto &f = *st.functiontable[ftype.idx];
            // Check we have correct number of args:
            int i = 0;
            for (Node **list = args_ptr; *list; list = &(*list)->tail())
            {
                i++;
                if (i > f.nargs())
                {
                    // We just throw away excess args here.
                    delete *list;
                    *list = nullptr;
                    break;
                }
            }
            if (i < f.nargs())
                TypeError("function value called with too few arguments", *fval);
            return TypeCheckCall(f, *args_ptr, fdef ? fdef : fval->closure_def());
        }
        else
        {
            // We have to do this call entirely at runtime. We take any args, and return any.
            // FIXME: the body T_CLOSUREDEF that created this function value hasn't been typechecked
            // at all, meaning its contents is all T_ANY. This is not necessary esp if the function
            // had no args, or all typed args, but we have no way of telling which T_CLOSUREDEF's
            // will end up this way.
            // Btw, some values ending up here may be T_COCLOSURE.
            return Type();
        }
    }

    Type TypeCheck(Node &n)
    {
        switch (n.type)
        {
            case T_STRUCTDEF:
            case T_FUNDEF:
                return Type();

            case T_CLOSUREDEF:
                return Type(V_FUNCTION, n.closure_def()->sf()->parent->idx);

            case T_LIST:
                // Flatten the TypeCheck recursion a bit
                for (Node *stats = &n; stats; stats = stats->b())
                    stats->a()->exptype = TypeCheck(*stats->a());
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
            case T_NIL:   return NewTypeVar().Wrap(V_NILABLE);

            case T_PLUS:
            case T_MINUS:
            case T_MULT:
            case T_DIV:
            case T_MOD:
            {
                auto type = Union(n.left(), n.right(), true);
                MathCheck(type, n, n.type);
                SubType(type, n);
                return type;
            }

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:
            {
                auto type = Promote(n.left()->exptype);
                MathCheck(type, n, n.type - T_PLUSEQ + T_PLUS);
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
                        if (type.t != V_VECTOR && type.t != V_STRUCT && type.t != V_NILABLE)
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
                auto type = Promote(n.child()->exptype);
                if (!type.Numeric())
                    TypeError("numeric", type, n);
                return type;
            }

            case T_UMINUS:
            {
                auto type = Promote(n.child()->exptype);
                if (!type.Numeric() && type.t != V_VECTOR)
                    TypeError("numeric/vector", type, n);
                return type;
            }

            case T_IDENT:
                return n.ident()->type;

            case T_DEF:
            {
                auto id = n.left()->ident();
                return id->type = n.right()->exptype;
            }

            case T_NATCALL:
            {
                auto nf = n.ncall_id()->nf();
                int i = 0;
                vector<Type> argtypes;
                for (Node *list = n.ncall_args(); list; list = list->tail())
                {
                    auto &arg = nf->args.v[i];
                    auto argtype = arg.type;
                    switch (arg.flags)
                    {
                        case NF_SUBARG1:
                            if (argtypes[0].t == V_VECTOR)
                            {
                                SubType(list->head(), argtype.t == V_VECTOR
                                    ? argtypes[0]
                                    : argtypes[0].Element(),
                                    nf->name.c_str());
                            }
                            else
                            {
                                SubType(list->head(), argtypes[0], nf->name.c_str());
                            }
                            break;

                        case NF_ANYVAR:
                            if (argtype.t == V_VECTOR) argtype = NewTypeVar().Wrap();
                            else if (argtype.t == V_ANY) argtype = NewTypeVar();
                            else assert(0);
                            break;
                    }
                    SubType(list->head(), argtype, nf->name.c_str());
                    argtypes.push_back(list->head()->exptype);
                    i++;
                }
                if (!nf->retvals.v.size()) return Type();
                // FIXME: multiple retvals
                switch (nf->retvals.v[0].flags)
                {
                    case NF_SUBARG1: return argtypes[0];
                    case NF_ANYVAR: return nf->retvals.v[0].type.t == V_VECTOR ? NewTypeVar().Wrap() : NewTypeVar();
                    default: return nf->retvals.v[0].type;
                }
            }

            case T_CALL:
            {
                auto &f = *n.call_function()->sf()->parent;
                return TypeCheckCall(f, n.call_args(), n.call_function());
            }

            case T_DYNCALL:
                return TypeCheckDynCall(n.dcall_fval(),
                                        &n.dcall_info()->dcall_args(),
                                        n.dcall_info()->dcall_function());

            case T_RETURN:
            {
                // FIXME multiret
                RetVal(n.return_value());
                return Type();
            }

            case T_IF:
            {
                Node *args = nullptr;
                if (n.if_branches()->right()->type != T_NIL)
                {

                    auto tleft  = TypeCheckDynCall(n.if_branches()->left(), &args);
                    auto tright = TypeCheckDynCall(n.if_branches()->right(), &args);
                    auto type = Union(tleft, tright, false);
                    // FIXME: we would want to allow coercions here, but we can't do so without changing
                    // these closure to a T_DYNCALL or inlining them
                    SubType(tleft, type, *n.if_branches()->left());
                    SubType(tright, type, *n.if_branches()->right());
                    return type;
                }
                else
                {
                    TypeCheckDynCall(n.if_branches()->left(), &args);
                    // No else: this currently returns either the condition or the branch value.
                    return Type();
                }
                break;
            }

            case T_WHILE:
            {
                Node *args = nullptr;
                TypeCheckDynCall(n.while_condition(), &args);
                TypeCheckDynCall(n.while_body(), &args);
                // Currently always return V_UNDEFINED
                return Type();
            }

            case T_FOR:
            {
                // We create temp arg nodes just for typechecking this:
                auto args = new Node(lex, T_LIST,
                                new Node(lex, T_FORLOOPVAR),
                                new Node(lex, T_LIST,
                                    new Node(lex, T_FORLOOPVAR)));
                auto itertype = Promote(n.for_iter()->exptype);
                if (itertype.t == V_INT || itertype.t == V_STRING) itertype = Type(V_INT);
                else if (itertype.t == V_VECTOR) itertype = itertype.Element();
                else TypeError("for can only iterate over int/string/vector, not: " + TypeName(itertype), n);
                args->head()->exptype = itertype;
                args->tail()->head()->exptype = Type(V_INT);
                TypeCheckDynCall(n.for_body(), &args);
                delete args;
                // Currently always return V_UNDEFINED
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
                auto type = *n.constructor_type()->typenode();
                if (type == Type(V_VECTOR))
                {
                    // No type was specified.. first find union of all elements.
                    Type u;
                    int i = 0;
                    for (auto list = n.constructor_args(); list; list = list->tail())
                    {
                        u = i ? Union(u, list->head()->exptype, true) : list->head()->exptype;
                        i++;
                    }
                    if (!u.CanWrap()) TypeError("can\'t nest vector values this deep", n);
                    type = u.Wrap();
                    if (!i) type = NewTypeVar().Wrap();  // special case for empty vectors
                }
                if (type.t == V_STRUCT)
                {
                    type.idx = SpecializeStruct(st.structtable[type.idx], n.constructor_args())->idx;
                }
                int i = 0;
                for (auto list = n.constructor_args(); list; list = list->tail())
                {
                    Type elemtype;
                    if (list->head()->type == T_SUPER)
                    {
                        assert(type.t == V_STRUCT);  // Parser checks this.
                        elemtype = Type(V_STRUCT, st.structtable[type.idx]->superclassidx);
                    }
                    else
                    {
                        elemtype = type.t == V_STRUCT ? st.structtable[type.idx]->fields[i].type : type.Element();
                    }
                    SubType(list->head(), elemtype, n);
                    i++;
                }
                return type;
            }

            case T_DOT:
            {
                auto type = Promote(n.left()->exptype);
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
                auto vtype = Promote(n.left()->exptype);
                if (vtype.t != V_VECTOR && vtype.t != V_STRING)
                    TypeError("vector/string", vtype, n);
                auto itype = Promote(n.right()->exptype);
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

            case T_COCLOSURE:
                return Type(V_FUNCTION, -1);

            case T_COROUTINE:
                return Type(V_COROUTINE);

            case T_SUPER:
                return n.child()->exptype;



            case T_CO_AT:

            case T_ASSIGNLIST:


            case T_MULTIRET:



            case T_FUN:
            case T_NATIVE:
            case T_LIST:
            case T_BRANCHES:
            case T_DYNINFO:
            case T_STRUCT:
                return n.exptype;

            default:
                assert(0);
                return n.exptype;
        }
    }
};
