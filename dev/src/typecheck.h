
namespace lobster
{

struct TypeChecker
{
    Lex &lex;
    Parser &parser;
    SymbolTable &st;

    struct Scope { SubFunction *sf; const Node *call_context; };
    vector<Scope> scopes, named_scopes;

    vector<Type> type_variables;

    vector<Node *> flowstack;  // For now, only for changes from nilable and back, for ident and dot nodes.

    bool verbose;

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st), verbose(true)
    {
        TypeCheck(parser.root);

        assert(!scopes.size());
        assert(!named_scopes.size());
        assert(!flowstack.size());
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

    string ArgName(int i)
    {
        switch (i)
        {
            case 0: return "1st";
            case 1: return "2nd";
            case 2: return "3rd";
            default: return inttoa(i + 1) + string("th");
        }
    }

    void TypeError(const char *required, const Type &got, const Node &n, const char *argname, const char *context = nullptr)
    {
        TypeError(string("\"") +
                  (context ? context : TName(n.type)) + 
                  "\" " +
                  (argname ? string("(") + argname + " argument) " : "") +
                  "requires type: " +
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
                    //if (id->type.t != V_ANY)
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
        return Type(V_VAR, (int)type_variables.size() - 1);
    }

    Type Promote(const Type &type)
    {
        if (type.t == V_VAR && type_variables[type.idx].t != V_UNDEFINED)
        {
            return Promote(type_variables[type.idx]);
        }
        else if (type.t == V_VECTOR || type.t == V_NILABLE)
        {
            auto pe = Promote(type.Element());
            return pe.CanWrap() ? pe.Wrap(type.t) : type;
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
        if (type.t == V_VAR) return ConvertsTo(UnifyVar(sub, type), sub, coercions);
        switch (sub.t)
        {
            case V_ANY:      return true;
            case V_VAR:      return ConvertsTo(type, UnifyVar(type, sub), coercions);
            case V_FLOAT:    return type.t == V_INT && coercions;
            case V_STRING:   return coercions;
            case V_FUNCTION: return type.t == V_FUNCTION && sub.idx < 0;
            case V_NILABLE:  return type.t == V_NIL ||
                                    (type.t == V_NILABLE && ConvertsTo(type.Element(), sub.Element(), false)) ||
                                    ConvertsTo(type, sub.Element(), false);
            case V_VECTOR:   return ((type.t == V_VECTOR && ConvertsTo(type.Element(), sub.Element(), false)) ||
                                     (type.t == V_STRUCT && sub.t2 == V_ANY));
            case V_STRUCT:   return type.t == V_STRUCT && st.IsSuperTypeOrSame(sub.idx, type.idx);
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

    void SubTypeLR(const Type &sub, Node &n) { SubType(n.left(), sub, "left", n); SubType(n.right(), sub, "right", n); }

    void SubType(Node *&a, const Type &sub, const char *argname, const Node &context)
    {
        SubType(a, sub, argname, TName(context.type));
    }
    void SubType(Node *&a, const Type &sub, const char *argname, const char *context)
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
            case V_FUNCTION:
                if (type.t == V_FUNCTION && sub.idx >= 0 && type.idx >= 0)
                {
                    // See if these functions can be made compatible. Specialize and typecheck if needed.
                    auto sf = st.functiontable[type.idx]->subf;
                    auto ss = st.functiontable[sub.idx]->subf;
                    if (sf->args.v.size() != ss->args.v.size()) break;
                    int i = 0;
                    for (auto &arg : sf->args.v)
                    {
                        // Specialize to the function type, if requested.
                        if (!sf->typechecked && arg.flags == AF_ANYTYPE)
                        {
                            arg.type = ss->args.v[i].type;
                        }
                        // Note this has the args in reverse: function args are contravariant.
                        if (!ConvertsTo(ss->args.v[i].type, arg.type, false)) goto error;
                        i++;
                    }
                    TypeCheck(*sf, sf->body);
                    // Covariant again.
                    if (!ConvertsTo(sf->returntype, ss->returntype, false)) break;
                    return;
                }
                break;
        }
        error:
        TypeError(TypeName(sub).c_str(), type, *a, argname, context);
    }
    void SubType(Type &type, const Type &sub, const Node &n, const char *argname, const char *context)
    {
        if (!ConvertsTo(type, sub, false)) TypeError(TypeName(sub).c_str(), type, n, argname, context);
        type = Promote(type);
    }

    const char *MathCheck(const Type &type, int op)
    {
        if (op == T_MOD)
        {
            if (type.t != V_INT) return "int";
        }
        else
        {
            if (!type.Numeric() && type.t != V_VECTOR && type.t != V_STRUCT)
            {
                if (op == T_PLUS)
                {
                    if (type.t != V_STRING)
                        return "numeric/string/vector/struct";
                }
                else
                {
                    return "numeric/vector/struct";
                }
            }
        }
        return nullptr;
    }

    void MathError(const Type &type, const Node &n, int op)
    {
        auto err = MathCheck(type, op);
        if (err)
        {
            if (MathCheck(n.left()->exptype, op)) TypeError(err, n.left()->exptype, n, "left");
            if (MathCheck(n.right()->exptype, op)) TypeError(err, n.right()->exptype, n, "right");
            TypeError(string("can\'t use \"") +
                      TName(n.type) + 
                      "\" on " + 
                      TypeName(n.left()->exptype) + 
                      " and " + 
                      TypeName(n.right()->exptype), n);
        }
    }

    SubFunction *TopScope(vector<Scope> &scopes) { return scopes.empty() ? nullptr : scopes.back().sf; }

    void RetVal(Node *&a, SubFunction *sf)
    {
        if (!sf) return;
        if (a) SubType(a, sf->returntype, nullptr, "return value");
        else sf->returntype = Type();  // FIXME: this allows "return" followed by "return 1" ?
    }

    void TypeCheck(SubFunction &sf, const Node *call_context)
    {
        if (!sf.typechecked)
        {
            Scope scope;
            scope.sf = &sf;
            scope.call_context = call_context;
            scopes.push_back(scope);
            if (!sf.parent->anonymous) named_scopes.push_back(scope);

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
                //local.id->type = local.type;  // not needed, will be set inside body
                i++;
            }

            sf.returntype = NewTypeVar();
            TypeCheck(sf.body);

            for (auto &back : backup_args.v) back.id->type = back.type;
            for (auto &back : backup_locals.v) back.id->type = back.type;

            Node *last = nullptr;
            for (auto topl = sf.body; topl; topl = topl->tail()) last = topl;
            assert(last);
            if (last->head()->type != T_RETURN) RetVal(last->head(), TopScope(scopes));

            if (!sf.parent->anonymous) named_scopes.pop_back();
            scopes.pop_back();
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
                SubType(stype, Type(V_STRUCT, head->superclassidx), *list->head(), nullptr, "super");
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
            if (verbose) DebugLog(1, "cloned struct: %s", struc->name.c_str());
        }
        struc->typechecked = true;
        int i = 0;
        for (auto &type : argtypes)
        {
            auto &field = struc->fields[i++];
            if (field.flags == AF_ANYTYPE) field.type = type;  // Specialize to arg.
        }
        if (verbose) DebugLog(1, "specialized struct: %s", Signature(struc).c_str());
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
                assert(!f.istype);  // Should not contain any AF_ANYTYPE

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
                    if (verbose) DebugLog(1, "cloning: %s", sf->parent->name.c_str());
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
                        if (verbose) DebugLog(1, "arg: %s:%s", arg.id->name.c_str(), TypeName(arg.type).c_str());
                    }
                }
                for (auto &freevar : sf->freevars.v)
                {
                    freevar.type = freevar.id->type;  // Specialized to current value.
                }
                if (verbose) DebugLog(1, "specialization: %s", SignatureWithFreeVars(sf).c_str());
            }
            match:
            // Here we have a SubFunction witch matching specialized types.
            // First check all the manually typed args.
            int i = 0;
            for (Node *list = call_args; list; list = list->tail())
            {
                auto &arg = sf->args.v[i++];
                if (arg.flags != AF_ANYTYPE) SubType(list->head(), arg.type, ArgName(i).c_str(), f.name.c_str());
            }
            if (!f.istype) TypeCheck(*sf, function_def_node);
            function_def_node->sf() = sf;
            if (verbose) DebugLog(1, "function %s returns %s", Signature(sf).c_str(), TypeName(sf->returntype).c_str());
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

    Type TypeCheckBranch(bool iftrue, Node *condition, Node *fval, Node **args_ptr)
    {
        auto flowstart = CheckFlowTypeChanges(iftrue, condition);
        auto type = TypeCheckDynCall(fval, args_ptr);
        CleanUpFlow(flowstart);
        return type;
    }

    void CheckFlowTypeChangesSub(bool iftrue, Node *condition)
    {
        auto type = Promote(condition->exptype);
        switch (condition->type)
        {
            case T_IDENT:
                if (iftrue)
                {
                    if (type.t == V_NILABLE)
                    {
                        flowstack.push_back(condition);
                    }
                }
                break;
          
            case T_DOT:
                if (iftrue)
                {
                    if (type.t == V_NILABLE && condition->left()->type == T_IDENT)
                    {
                        flowstack.push_back(condition);
                    }
                }
                break;

            case T_NOT:
                CheckFlowTypeChangesSub(!iftrue, condition->child());
                break;
        }
    }

    size_t CheckFlowTypeChanges(bool iftrue, Node *condition)
    {
        auto start = flowstack.size();
        switch (condition->type)
        {
            // FIXME: this doesn't work if you do a & b & c & f(a,b,c) since we only support one nesting level.
            case T_OR:
            case T_AND:
                // AND only works for then, and OR only for else.
                if (iftrue == (condition->type == T_AND))
                {
                    CheckFlowTypeChangesSub(iftrue, condition->left());
                    CheckFlowTypeChangesSub(iftrue, condition->right());
                }
                break;

            default:
                CheckFlowTypeChangesSub(iftrue, condition);
                break;
        }
        return start;
    }

    void AssignFlow(Node *left, const Node *right)
    {
        if (left->exptype.Numeric()) return;  // Early out.

        LookupFlow(*left, true, right->exptype.t == V_NILABLE);
    }

    void UseFlow(Node &n)
    {
        if (n.exptype.t != V_NILABLE) return;  // Early out.

        LookupFlow(n, false, true);
    }

    void LookupFlow(Node &n, bool assign, bool isnil)
    {
        // FIXME: this can in theory find the wrong node, if the same function nests, and the outer one
        // was specialized to a nilable and the inner one was not.
        // This would be very rare though, and benign.
        for (auto it = flowstack.rbegin(); it != flowstack.rend(); ++it)
        {
            auto &in = **it;
            switch (n.type)
            {
                case T_IDENT:
                    if (in.type == T_IDENT && in.ident() == n.ident())
                    {
                        goto found;
                    }
                    else if (in.type == T_DOT && in.left()->ident() == n.ident() && assign)
                    {
                        // We're writing to var V and V.f is in the stack: invalidate regardless.
                        isnil = true;
                        goto found;
                    }
                    break;

                case T_DOT:
                    if (in.type == T_DOT &&
                        in.left()->ident() == n.left()->ident() &&
                        in.right()->fld() == n.right()->fld())
                    {
                        goto found;
                    }
                    break;

                default: assert(0);
            }

            continue;

            found:
            if (assign)
            {
                if (isnil)
                {
                    // FLow based promotion is invalidated.
                    assert(n.exptype.CanWrap());
                    n.exptype = n.exptype.Wrap(V_NILABLE);
                    flowstack.erase(std::next(it).base());
                }
            }
            else
            {
                n.exptype = n.exptype.Element();
            }
            return;
        }
    }

    void CleanUpFlow(size_t start)
    {
        while (flowstack.size() > start) flowstack.pop_back();
    }

    Type TypeCheckAndOr(Node *&n_ptr, bool only_true_type)
    {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.

        Node &n = *n_ptr;

        if (n.type != T_AND && n.type != T_OR)
        {
            TypeCheck(n_ptr);
            auto type = Promote(n.exptype);
            if (type.t == V_NILABLE && only_true_type) return type.Element();
            return type;
        }

        auto tleft = TypeCheckAndOr(n.left(), n.type == T_OR);
        auto flowstart = CheckFlowTypeChanges(n.type == T_AND, n.left());
        auto tright = TypeCheckAndOr(n.right(), only_true_type);
        CleanUpFlow(flowstart);

        n.exptype = only_true_type && n.type == T_AND
            ? tright
            : Union(tleft, tright, false);
        return n.exptype;
    }

    void TypeCheck(Node *&n_ptr)
    {
        Node &n = *n_ptr;
        Type &type = n.exptype;

        switch (n.type)
        {
            case T_STRUCTDEF:
            case T_FUNDEF:
                return;

            case T_CLOSUREDEF:
                type = Type(V_FUNCTION, n.closure_def()->sf()->parent->idx);
                return;

            case T_LIST:
                // Flatten the TypeCheck recursion a bit
                for (Node *stats = &n; stats; stats = stats->b())
                    TypeCheck(stats->a());
                return;

            case T_OR:
            case T_AND:
                TypeCheckAndOr(n_ptr, false);
                return;
        }

        if (n.HasChildren())
        {
            if (n.a()) TypeCheck(n.a());
            if (n.b()) TypeCheck(n.b());
        }

        switch (n.type)
        {
            case T_INT:   type = Type(V_INT); break;
            case T_FLOAT: type = Type(V_FLOAT); break;
            case T_STR:   type = Type(V_STRING); break;
            case T_NIL:   type = NewTypeVar().Wrap(V_NILABLE); break;

            case T_PLUS:
            case T_MINUS:
            case T_MULT:
            case T_DIV:
            case T_MOD:
            {
                type = Union(n.left(), n.right(), true);
                MathError(type, n, n.type);
                SubTypeLR(type, n);
                break;
            }

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:
            {
                type = Promote(n.left()->exptype);
                MathError(type, n, n.type - T_PLUSEQ + T_PLUS);
                SubType(n.right(), type, "right", n);
                break;
            }

            case T_NEQ: 
            case T_EQ: 
            case T_GTEQ: 
            case T_LTEQ: 
            case T_GT:  
            case T_LT:
            {
                auto u = Union(n.left(), n.right(), true);
                if (!u.Numeric() && u.t != V_STRING)
                {
                    // FIXME: rather than nullptr, these TypeError need to figure out which side caused the error much like MathError
                    if (n.type == T_EQ || n.type == T_NEQ)
                    {
                        if (u.t != V_VECTOR && u.t != V_STRUCT && u.t != V_NILABLE)
                            TypeError("numeric/string/vector/struct", u, n, nullptr);
                    }
                    else
                    {
                        TypeError("numeric/string", u, n, nullptr);
                    }
                }
                SubTypeLR(u, n);
                type = Type(V_INT);
                break;
            }

            case T_NOT:
                type = Type(V_INT);
                break;

            case T_POSTDECR:
            case T_POSTINCR:
            case T_DECR:  
            case T_INCR:
            {
                type = Promote(n.child()->exptype);
                if (!type.Numeric())
                    TypeError("numeric", type, n, nullptr);
                break;
            }

            case T_UMINUS:
            {
                type = Promote(n.child()->exptype);
                if (!type.Numeric() && type.t != V_VECTOR)
                    TypeError("numeric/vector", type, n, nullptr);
                break;
            }

            case T_IDENT:
                type = n.ident()->type;
                UseFlow(n);
                break;

            case T_DEF:
            {
                auto id = n.left()->ident();
                type = n.right()->exptype;
                n.left()->exptype = type;
                id->type = type;
                if (verbose) DebugLog(1, "var: %s:%s", id->name.c_str(), TypeName(type).c_str());
                break;
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
                                    ArgName(i).c_str(), nf->name.c_str());
                            }
                            else
                            {
                                SubType(list->head(), argtypes[0], ArgName(i).c_str(), nf->name.c_str());
                            }
                            break;

                        case NF_ANYVAR:
                            if (argtype.t == V_VECTOR) argtype = NewTypeVar().Wrap();
                            else if (argtype.t == V_ANY) argtype = NewTypeVar();
                            else assert(0);
                            break;
                    }
                    SubType(list->head(), argtype, ArgName(i).c_str(), nf->name.c_str());
                    argtypes.push_back(list->head()->exptype);
                    i++;
                }
                type = Type();  // no retvals
                if (nf->retvals.v.size())
                {
                    // FIXME: multiple retvals
                    switch (nf->retvals.v[0].flags)
                    {
                        case NF_SUBARG1:
                            type = argtypes[0]; 
                            break;
                        case NF_ANYVAR: 
                            type = nf->retvals.v[0].type.t == V_VECTOR ? NewTypeVar().Wrap() : NewTypeVar(); 
                            break;
                        default:
                            type = nf->retvals.v[0].type; 
                            break;
                    }
                }
                break;
            }

            case T_CALL:
            {
                auto &f = *n.call_function()->sf()->parent;
                type = TypeCheckCall(f, n.call_args(), n.call_function());
                break;
            }

            case T_DYNCALL:
                type = TypeCheckDynCall(n.dcall_fval(),
                                        &n.dcall_info()->dcall_args(),
                                        n.dcall_info()->dcall_function());
                break;

            case T_RETURN:
            {
                // FIXME multiret
                auto sf = TopScope(named_scopes);
                if (sf == st.functiontable[n.return_function_idx()->integer()]->subf)
                {
                    RetVal(n.return_value(), sf);
                }
                else
                {
                    // FIXME: return from .. target function may not be typechecking right now?
                    assert(0);
                }
                type = n.return_value()->exptype;
                break;
            }

            case T_IF:
            {
                Node *args = nullptr;
                if (n.if_branches()->right()->type != T_NIL)
                {
                    auto tleft  = TypeCheckBranch(true, n.if_condition(), n.if_branches()->left(), &args);
                    auto tright = TypeCheckBranch(true, n.if_condition(), n.if_branches()->right(), &args);
                    type = Union(tleft, tright, false);
                    // FIXME: we would want to allow coercions here, but we can't do so without changing
                    // these closure to a T_DYNCALL or inlining them
                    SubType(tleft, type, *n.if_branches()->left(), "then branch", nullptr);
                    SubType(tright, type, *n.if_branches()->right(), "else branch", nullptr);
                }
                else
                {
                    TypeCheckDynCall(n.if_branches()->left(), &args);
                    // No else: this currently returns either the condition or the branch value.
                    type = Type();
                }
                break;
            }

            case T_WHILE:
            {
                Node *args = nullptr;
                TypeCheckDynCall(n.while_condition(), &args);
                TypeCheckBranch(true, n.while_condition()->closure_body()->head(), n.while_body(), &args);
                // Currently always return V_UNDEFINED
                type = Type();
                break;
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
                type = Type();
                break;
            }

            case T_TYPE:
                type = *n.typenode();
                break;

            case T_IS:
                // FIXME If the typecheck fails statically, we can replace this node with false
                type = Type(V_INT);
                break;

            case T_FIELD:
                break;  // Already set by the parser.

            case T_CONSTRUCTOR:
            {
                type = *n.constructor_type()->typenode();
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
                    SubType(list->head(), elemtype, ArgName(i).c_str(), n);
                    i++;
                }
                break;
            }

            case T_DOT:
            {
                auto stype = Promote(n.left()->exptype);
                if (stype.t != V_STRUCT)
                    TypeError("struct/value", stype, n, "object");
                auto struc = st.structtable[stype.idx];
                auto sf = n.right()->fld();
                auto uf = struc->Has(sf);
                if (!uf) TypeError("type " + struc->name + " has no field named " + sf->name, n);
                type = uf->type;
                UseFlow(n);
                break;
            }

            case T_INDEX:
            {
                auto vtype = Promote(n.left()->exptype);
                if (vtype.t != V_VECTOR && vtype.t != V_STRING)
                    TypeError("vector/string", vtype, n, "container");
                auto itype = Promote(n.right()->exptype);
                switch (itype.t)
                {
                    case V_INT: vtype = vtype.t == V_VECTOR ? vtype.Element() : Type(V_INT); break;
                    case V_STRUCT:
                    {
                        auto &struc = *st.structtable[itype.idx];
                        for (auto &field : struc.fields)
                        {
                            if (field.type.t != V_INT) TypeError("int field", field.type, n, "index");
                            if (vtype.t != V_VECTOR) TypeError("nested vector", vtype, n, "container");
                            vtype = vtype.Element();
                        }
                        break;
                    }
                    case V_VECTOR: // FIXME only way to typecheck this correctly is if we knew the length statically
                    default: TypeError("int/struct of int", itype, n, "index");
                }
                type = vtype;
                break;
            }

            case T_SEQ:
                type = n.right()->exptype;
                break;

            case T_ASSIGN:
                AssignFlow(n.left(), n.right());
                SubType(n.right(), n.left()->exptype, "right", n);
                type = n.left()->exptype;
                break;

            case T_COCLOSURE:
                type = Type(V_FUNCTION, -1);
                break;

            case T_COROUTINE:
                type = Type(V_COROUTINE);
                break;

            case T_SUPER:
                type = n.child()->exptype;
                break;



            case T_CO_AT:

            case T_ASSIGNLIST:


            case T_MULTIRET:



            case T_FUN:
            case T_NATIVE:
            case T_LIST:
            case T_BRANCHES:
            case T_DYNINFO:
            case T_STRUCT:
                break;

            case T_I2F:
            case T_A2S:
                // These have been added by another specialization.
                // We could check if they still apply, but even more robust is just to remove them,
                // and let them be regenerated if need be.
                n_ptr = n.child();
                n.child() = nullptr;
                delete &n;
                break;

            default:
                assert(0);
                break;
        }
    }
};

}  // namespace lobster
