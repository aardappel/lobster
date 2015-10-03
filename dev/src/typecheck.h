
namespace lobster
{

struct TypeChecker
{
    Parser &parser;
    SymbolTable &st;

    struct Scope { SubFunction *sf; const Node *call_context; };
    vector<Scope> scopes, named_scopes;

    struct FlowItem
    {
        const Node *item;  // For now, only for ident and dot nodes.
        TypeRef old, now;

        FlowItem(const Node *_item, TypeRef type) : item(_item), old(_item->exptype), now(type) {}
    };
    vector<FlowItem> flowstack;

    TypeChecker(Parser &_p, SymbolTable &_st) : parser(_p), st(_st)
    {
        st.RegisterDefaultVectorTypes();
        for (auto &struc : st.structtable)
        {
            if (!struc->generic) ComputeStructVectorType(struc);
            if (struc->superclass) for (auto &field : struc->fields.v)
            {
                // If this type refers to the super struct type, make it refer to this type instead.
                // There may be corner cases where this is not what you want, but generally you do.
                PromoteStructIdx(field.type, struc->superclass, struc);
            }
        }
            
        TypeCheck(parser.root, T_LIST);
        CleanUpFlow(0);

        assert(!scopes.size());
        assert(!named_scopes.size());
        
        Stats();
    }

    void ComputeStructVectorType(Struct *struc)
    {
        // FIXME: this is broken. It would a scalar field to become any without boxing.
        if (struc->fields.size())
        {
            TypeRef vectortype = struc->fields.v[0].type;
            for (size_t i = 1; i < struc->fields.size(); i++)
            {
                // FIXME: Can't use Union here since it will bind variables
                //vectortype = Union(vectortype, struc->fields[i].type, false);
                // use simplified alternative:
                if (!ExactType(struc->fields.v[i].type, vectortype)) vectortype = type_any;
            }
            struc->vectortype = vectortype->Wrap(NewType());
        }
    }

    void PromoteStructIdx(TypeRef &type, const Struct *olds, const Struct *news)
    {
        auto u = type;
        while (u->Wrapped()) u = u->Element();
        if (u->t == V_STRUCT && u->struc == olds) type = PromoteStructIdxRec(type, news);
    }

    TypeRef PromoteStructIdxRec(TypeRef type, const Struct *news)
    {
        return type->Wrapped() ? PromoteStructIdxRec(type->sub, news)->Wrap(NewType(), type->t) : &news->thistype;
    }

    Type *NewType() { return parser.NewType(); }

    string TypedArg(const GenericArgs &args, size_t i, bool withtype = true)
    {
        string s = args.GetName(i);
        if (args.GetType(i)->type->t != V_ANY && withtype) s += ":" + TypeName(args.GetType(i)->type);
        return s;
    }

    string Signature(const GenericArgs &args, bool withtype = true)
    {
        string s = "(";
        for (size_t i = 0; i < args.size(); i++)
        {
            if (i) s += ", ";
            s += TypedArg(args, i, withtype);
        }
        return s + ")"; 
    }

    string Signature(const Struct &struc)                         { return struc.name      + Signature(struc.fields); }
    string Signature(const SubFunction &sf, bool withtype = true) { return sf.parent->name + Signature(sf.args, withtype); }
    string Signature(const NativeFun &nf)                         { return nf.name         + Signature(nf.args); }

    string SignatureWithFreeVars(const SubFunction &sf, set<Ident *> *already_seen, bool withtype = true)
    {
        string s = Signature(sf, withtype) + " { ";
        size_t i = 0;
        for (auto &freevar : sf.freevars.v)
        {
            if (freevar.type->t != V_FUNCTION &&
                !freevar.id->static_constant &&
                (!already_seen || already_seen->find(freevar.id) == already_seen->end()))
            {
                s += TypedArg(sf.freevars, i) + " ";
                if (already_seen) already_seen->insert(freevar.id);
            }
            i++;
        }
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
            default: return to_string(i + 1) + string("th");
        }
    }

    void TypeError(const char *required, TypeRef got, const Node &n, const char *argname, const char *context = nullptr)
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
        set<Ident *> already_seen;
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            auto &scope = *it;
            err += "\n  in " + parser.lex.Location(scope.call_context->line) + ": ";
            err += SignatureWithFreeVars(*scope.sf, &already_seen);
            for (Node *list = scope.sf->body; list; list = list->tail())
            {
                for (auto dl = list->head(); dl->type == T_DEF; dl = dl->right())
                {
                    auto id = dl->left()->ident();
                    //if (id->type->t != V_ANY)
                    {
                        err += ", " + id->name + ":" + TypeName(id->cursid->type);
                    }
                }
            }
        }
        parser.Error(err, &n);
    }

    void NatCallError(const char *errstr, const NativeFun *nf, const Node &callnode)
    {
        auto err = errstr + nf->name;
        err += "\n  got:";
        for (auto list = callnode.ncall_args(); list; list = list->tail())
        {
            err += " " + TypeName(list->head()->exptype);
        }
        for (auto cnf = nf->first; cnf; cnf = cnf->overloads)
        {
            err += "\n  overload: " + Signature(*cnf);
        }
        TypeError(err, callnode);
    }

    TypeRef VectorStructElement(TypeRef type)
    {
        if (type->t == V_VECTOR) return type->sub;
        if (type->t == V_STRUCT) return type->struc->vectortype->sub;
        assert(0);
        return type;
    }

    TypeRef NewTypeVar()
    {
        auto var = NewType();
        *var = Type(V_VAR);
        var->sub = var;  // Vars store a cycle of all vars its been unified with, starting with itself.
        return var;
    }

    TypeRef NewNilTypeVar()
    {
        auto nil = NewType();
        *nil = Type(V_NIL);
        nil->sub = &*NewTypeVar();
        return nil;
    }

    void UnifyVar(TypeRef type, TypeRef hasvar)
    {
        // Typically Type is const, but this is the one place we overwrite them.
        // Type objects that are V_VAR are seperate heap instances, so overwriting them has no side-effects on
        // non-V_VAR Type instances.
        assert(hasvar->t == V_VAR);
        if (type->t == V_VAR)
        {
            swap((Type *&)hasvar->sub, (Type *&)type->sub);  // Combine two cyclic linked lists.. elegant!
        }
        else
        {
            auto v = hasvar;
            do  // Loop thru all vars in unification cycle.
            {
                auto next = v->sub;
                *(Type *)&*v = *type;  // Overwrite Type struct!
                v = next;
            }
            while (&*v != &*hasvar);  // Force TypeRef pointer comparison.
        }
    }

    bool ConvertsTo(TypeRef type, TypeRef sub, bool coercions, bool unifications = true, bool genericany = false)
    {
        if (sub == type) return true;
        if (type->t == V_VAR)
        {
            if (unifications) UnifyVar(sub, type);
            return true;
        }
        switch (sub->t)
        {
            case V_ANY:       return genericany || coercions || !IsScalar(type->t);
            case V_VAR:       UnifyVar(type, sub); return true;
            case V_FLOAT:     return type->t == V_INT && coercions;
            case V_INT:       return type->t == V_TYPEID && coercions;
            case V_STRING:    return false; //coercions;
            case V_FUNCTION:  return type->t == V_FUNCTION && !sub->sf;
            case V_NIL:       return (type->t == V_NIL && ConvertsTo(type->Element(), sub->Element(), false, unifications, genericany)) ||
                                     (!type->Numeric() && ConvertsTo(type, sub->Element(), false, unifications, genericany));
            case V_VECTOR:    return ((type->t == V_VECTOR && ConvertsTo(type->Element(), sub->Element(), false, unifications, genericany)) ||
                                      (type->t == V_STRUCT && ConvertsTo(type->struc->vectortype, sub, false, unifications, genericany)));
            case V_STRUCT:    return type->t == V_STRUCT && st.IsSuperTypeOrSame(sub->struc, type->struc);
            case V_COROUTINE: return type->t == V_COROUTINE &&
                                     (sub->sf == type->sf ||
                                      (!sub->sf && ConvertsTo(type->sf->coresumetype, NewNilTypeVar(), false)));
        }
        return false;
    }

    TypeRef Union(const Node *a, const Node *b, bool coercions)
    {
         return Union(a->exptype, b->exptype, coercions);
    }
    TypeRef Union(TypeRef at, TypeRef bt, bool coercions)
    {
        if (ConvertsTo(at, bt, coercions)) return bt;
        if (ConvertsTo(bt, at, coercions)) return at;
        if (at->t == V_VECTOR && bt->t == V_VECTOR) return type_vector_any;
        if (at->t == V_STRUCT && bt->t == V_STRUCT)
        {
            auto sstruc = st.CommonSuperType(at->struc, bt->struc);
            if (sstruc) return &sstruc->thistype;
        }
        return type_any;
    }

    bool ExactType(TypeRef a, TypeRef b)
    {
        return a == b;  // Not inlined for documentation purposes.
    }

    void MakeString(Node *&a)
    {
        assert(a->exptype->t != V_STRING);
        MakeAny(a);  // Could instead make a T_I2S etc, but string() goes thru any also.
        a = (Node *)new Unary(a->line, T_A2S, a);
        a->exptype = type_string;
    }

    void MakeAny(Node *&a)
    {
        if (a->exptype->t == V_FUNCTION) TypeError("cannot convert a function value to any", *a);
        if (a->exptype->t == V_TYPEID) TypeError("cannot convert a typeid to any", *a);
        if (IsScalar(a->exptype->t)) a = (Node *)new Unary(a->line, T_E2A, a);
        a->exptype = type_any;
    }

    void MakeNil(Node *&a)
    {
        a = (Node *)new Unary(a->line, T_E2N, a);
        a->exptype = type_any;
    }

    void MakeBool(Node *&a)
    {
        if (a->exptype->t != V_INT) a = (Node *)new Unary(a->line, T_E2B, a);
        a->exptype = type_int;
    }

    void MakeInt(Node *&a)
    {
        a = (Node *)new Unary(a->line, T_T2I, a);
        a->exptype = type_int;
    }

    void MakeFloat(Node *&a)
    {
        a = (Node *)new Unary(a->line, T_I2F, a);
        a->exptype = type_float;
    }

    void SubTypeLR(TypeRef sub, Node &n) { SubType(n.left(), sub, "left", n); SubType(n.right(), sub, "right", n); }

    void SubType(Node *&a, TypeRef sub, const char *argname, const Node &context)
    {
        SubType(a, sub, argname, TName(context.type));
    }
    void SubType(Node *&a, TypeRef sub, const char *argname, const char *context)
    {
        if (ConvertsTo(a->exptype, sub, false)) return;

        switch (sub->t)
        {
            case V_FLOAT:
                if (a->exptype->t == V_INT)
                {
                    MakeFloat(a);
                    return;
                }
                break;
            case V_ANY:
                MakeAny(a);
                return;
            case V_INT:
                if (a->exptype->t == V_TYPEID)
                {
                    MakeInt(a);
                    return;
                }
                break;
            case V_FUNCTION:
                if (a->exptype->IsFunction() && sub->sf)
                {
                    // See if these functions can be made compatible. Specialize and typecheck if needed.
                    auto sf = a->exptype->sf;
                    auto ss = sub->sf;
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
                        if (!ConvertsTo(ss->args.v[i].type, arg.type, false))
                            goto error;
                        i++;
                    }
                    TypeCheckFunctionDef(*sf, sf->body);
                    // Covariant again.
                    if (sf->returntypes.size() != ss->returntypes.size() ||
                        !ConvertsTo(sf->returntypes[0], ss->returntypes[0], false))
                            break;
                    assert(ss->returntypes.size() == 1);  // Parser only parses one ret type for function types.
                    return;
                }
                break;
        }
        error:
        TypeError(TypeName(sub).c_str(), a->exptype, *a, argname, context);
    }

    void SubTypeT(TypeRef type, TypeRef sub, const Node &n, const char *argname, const char *context = nullptr)
    {
        if (!ConvertsTo(type, sub, false)) TypeError(TypeName(sub).c_str(), type, n, argname, context);
    }

    // Used by type-checker to and optimizer.
    // If it returns true, sets val to a value that gives the correct True().
    // Also sets correct scalar values.
    bool ConstVal(const Node &n, Value &val)
    {
        switch (n.type)
        {
            case T_INT:   val = Value(n.integer());    return true;
            case T_FLOAT: val = Value((float)n.flt()); return true;
            case T_NIL:   val = Value(nullptr, V_NIL); return true;
            case T_IS:
            {
                if (n.left()->exptype == n.right()->exptype ||
                    n.right()->exptype->t == V_ANY)                            { val = Value(true);  return true; }
                if (!ConvertsTo(n.right()->exptype, n.left()->exptype, false)) { val = Value(false); return true; }
                // This means it is always a reference type, since int/float/function don't convert
                // into anything without coercion.
                return false;
            }
            case T_NOT:
            {
                auto isconst = ConstVal(*n.child(), val);
                val = Value(!val.True());
                return isconst;
            }
            case T_AND:
            case T_OR:
            {
                auto isconst = ConstVal(*n.left(), val);
                if (!isconst) return false;
                return val.True() == (n.type == T_OR) ? true : ConstVal(*n.right(), val);
            }
            // TODO: support more? strings?
            default: 
                return false;
        }
    }

    TypeRef StructTypeFromVector(TypeRef vectortype, const Struct *struc)
    {
        for (struc = struc->first->next; struc; struc = struc->next)
        {
            if (struc->vectortype->sub->t == vectortype->sub->t)
            {
                return &struc->thistype;
            }
        }
        return vectortype;
    }

    bool MathCheckVector(TypeRef &type, Node *&left, Node *&right)
    {
        TypeRef ltype = left->exptype;
        TypeRef rtype = right->exptype; 
        // Special purpose check for vector * scalar etc.
        if (rtype->Numeric())
        {
            TypeRef vtype;
            if (ltype->t == V_VECTOR)
            {
                vtype = ltype->Element();
            }
            else if (ltype->t == V_STRUCT)
            {
                vtype = ltype->struc->vectortype->Element();
            }

            if (vtype->Numeric())
            {
                if (vtype->t == V_INT)
                {
                    type = type_vector_int;
                    if (rtype->t == V_FLOAT) return false;  // Don't implicitly convert int vectors to float.
                }
                else
                {
                    type = type_vector_float;
                    if (rtype->t == V_INT) SubType(right, type_float, "right", *right);
                }
                // Recover struct type if possible.
                // We don't rely on default_int_vector_types etc here, since we also want other
                // numeric vectors to be preserved, e.g. "color"
                // At run-time, the left argument is type preserving, the right is not.
                if (ltype->t == V_STRUCT) type = StructTypeFromVector(type, ltype->struc);
                return true;
            }
        }

        return false;
    }

    const char *MathCheck(TypeRef &type, Node &n, TType op, bool &unionchecked, bool typechangeallowed)
    {
        if (op == T_MOD)
        {
            if (type->t != V_INT) return "int";
        }
        else
        {
            if (!type->Numeric() && type->t != V_VECTOR && type->t != V_STRUCT)
            {
                if (MathCheckVector(type, n.left(), n.right()))
                {
                    unionchecked = true;
                    return nullptr;
                }

                if (op == T_PLUS)
                {
                    auto ltype = n.left()->exptype;
                    auto rtype = n.right()->exptype;
                    if (ltype->t == V_STRING)
                    {
                        if (rtype->t != V_STRING)
                        {
                            // Anything can be added to a string on the right (because of +=).
                            MakeString(n.right());
                            // Make sure the overal type is string.
                            type = type_string;
                            unionchecked = true;
                        }
                    }
                    else if (rtype->t == V_STRING && ltype->t != V_STRING && typechangeallowed)
                    {
                        // Only if not in a +=
                        MakeString(n.left());
                        type = type_string;
                        unionchecked = true;
                    }
                    else
                    {
                        return "numeric/string/vector/struct";
                    }
                }
                else
                {
                    return "numeric/vector/struct";
                }
            }
        }
        return nullptr;
    }

    void MathError(TypeRef &type, Node &n, TType op, bool &unionchecked, bool typechangeallowed)
    {
        auto err = MathCheck(type, n, op, unionchecked, typechangeallowed);
        if (err)
        {
            if (MathCheck(n.left()->exptype, n, op, unionchecked, typechangeallowed))
                TypeError(err, n.left()->exptype, n, "left");
            if (MathCheck(n.right()->exptype, n, op, unionchecked, typechangeallowed))
                TypeError(err, n.right()->exptype, n, "right");
            TypeError(string("can\'t use \"") +
                      TName(n.type) + 
                      "\" on " + 
                      TypeName(n.left()->exptype) + 
                      " and " + 
                      TypeName(n.right()->exptype), n);
        }
    }

    SubFunction *TopScope(vector<Scope> &_scopes) { return _scopes.empty() ? nullptr : _scopes.back().sf; }

    void RetVal(Node *&a, SubFunction *sf, size_t i, TypeRef *exacttype = nullptr)
    {
        if (!sf) return;
        if (i >= sf->returntypes.size())
        {
            if (sf->fixedreturntype) TypeError("number of returned value must correspond to declared return type", *a);
            assert(i == sf->returntypes.size());
            sf->returntypes.push_back(exacttype ? *exacttype : a->exptype);
        }
        else
        {
            auto argname = "return value";
            if (exacttype) SubTypeT(*exacttype, sf->returntypes[i], *a, argname);
            else if (a) SubType(a, sf->returntypes[i], argname, *a);
            else SubTypeT(type_any, sf->returntypes[i], *a, argname);  // FIXME: this allows "return" followed by "return 1" ?
        }
    }

    SpecIdent *NewSid(Ident *id, TypeRef type)
    {
        auto sid = new SpecIdent(id, type, (int)st.specidents.size());
        st.specidents.push_back(sid);
        return sid;
    }

    void TypeCheckFunctionDef(SubFunction &sf, const Node *call_context)
    {
        if (sf.typechecked) return;
        
        Output(OUTPUT_DEBUG, "function start: %s", SignatureWithFreeVars(sf, nullptr).c_str());

        Scope scope;
        scope.sf = &sf;
        scope.call_context = call_context;
        scopes.push_back(scope);
        if (!sf.parent->anonymous) named_scopes.push_back(scope);

        sf.typechecked = true;

        for (auto &fv : sf.freevars.v) fv.sid = fv.id->cursid;
        for (auto &dyn : sf.dynscoperedefs.v) dyn.sid = dyn.id->cursid;

        auto backup_vars = [&](ArgVector &in, ArgVector &backup)
        {
            int i = 0;
            for (auto &arg : in.v)
            {
                // Need to not overwrite nested/recursive calls. e.g. map(): map(): ..
                backup.v[i].sid = arg.id->cursid;
                arg.id->cursid = arg.sid = NewSid(arg.id, arg.type);
                i++;
            }
        };

        auto backup_args = sf.args; backup_vars(sf.args, backup_args);
        auto backup_locals = sf.locals; backup_vars(sf.locals, backup_locals);

        // FIXME: this would not be able to typecheck recursive functions with multiret.
        if (!sf.fixedreturntype) sf.returntypes[0] = NewTypeVar();
        sf.coresumetype = sf.iscoroutine ? NewTypeVar() : type_any;

        auto start_promoted_vars = flowstack.size();

        TypeCheck(sf.body, T_LIST);

        CleanUpFlow(start_promoted_vars);

        for (auto &back : backup_args.v)   { back.id->cursid = back.sid; }
        for (auto &back : backup_locals.v) { back.id->cursid = back.sid; }

        Node *last = Parser::LastInList(sf.body);
        if (last->head()->type != T_RETURN || last->head()->return_function_idx()->integer() != sf.parent->idx) 
            RetVal(last->head(), &sf, 0);

        for (auto type : sf.returntypes) if (type->HasVariable())
        {
            // If this function return something with a variable in it, then it likely will get bound by the
            // caller. If the function then gets reused without specialization, it will get the wrong return type,
            // so we force specialization for subsequent calls of this function.
            // FIXME: check in which cases this is typically true, since its expensive if done without reason.
            sf.mustspecialize = true;
        }

        if (!sf.parent->anonymous) named_scopes.pop_back();
        scopes.pop_back();

        Output(OUTPUT_DEBUG, "function end %s returns %s", Signature(sf).c_str(), TypeName(sf.returntypes[0]).c_str());
    }

    Struct *FindStructSpecialization(Struct *given, const Node *args, const Node &n)
    {
        // This code is somewhat similar to function specialization, but not similar enough to share.
        // If they're all typed, we bail out early:
        if (!given->generic) return given;

        auto head = given->first;

        // First collect types for all args.
        vector<TypeRef> argtypes;
        for (auto list = args; list; list = list->tail())
        {
            argtypes.push_back(list->head()->exptype);
        }
        assert(argtypes.size() == head->fields.size());

        // Now find a match:
        auto struc = head->next;
        for (; struc; struc = struc->next)
        {
            int i = 0;
            for (auto &type : argtypes)
            {
                auto &field = struc->fields.v[i++];
                if (field.flags == AF_ANYTYPE && !ExactType(type, field.type)) goto fail;
            }
            return struc;  // Found a match.
            fail:;
        }

        string s;
        for (auto type : argtypes) s += " " + TypeName(type);
        TypeError("no specialization of " + given->first->name + " matches these types:" + s, n);
        return nullptr;
    }

    void CheckIfSpecialization(Struct *spec_struc, TypeRef given, const Node &n, const char *argname,
                               const char *req = nullptr, bool subtypeok = false, const char *context = nullptr)
    {
        auto givenu = given->UnWrapped();
        if (given->t != V_STRUCT ||
            (!spec_struc->IsSpecialization(givenu->struc) &&
             (!subtypeok || !st.IsSuperTypeOrSame(spec_struc, givenu->struc))))
        {
            TypeError(req ? req : spec_struc->name.c_str(), given, n, argname, context);
        }
    }

    void CheckGenericArg(TypeRef otype, TypeRef argtype, const char *argname, const Node &n, const char *context)
    {
        if (otype->t != V_ANY)  // Argument is a generic struct type, or wrapped in vector/nilable
        {
            auto u = otype->UnWrapped();
            assert(u->t == V_STRUCT);
            if (otype->EqNoIndex(*argtype))
            {
                CheckIfSpecialization(u->struc, argtype, n, argname, TypeName(otype).c_str(), true, context);
            }
            else
            {
                // This likely generates either an error, or contains an unbound var that will get bound.
                SubTypeT(argtype, otype, n, argname, context);
                //TypeError(TypeName(otype).c_str(), argtype, n, argname, context);
            }
        }
    }
    
    bool FreeVarsSameAsCurrent(SubFunction *sf, bool prespecialize)
    {
        for (auto &freevar : sf->freevars.v)
        {
            //auto atype = Promote(freevar.id->type);
            if (freevar.sid != freevar.id->cursid || !ExactType(freevar.type, freevar.id->cursid->type))
            {
                (void)prespecialize;
                assert(prespecialize ||
                       freevar.sid == freevar.id->cursid ||
                       (freevar.sid && freevar.id->cursid));
                return false;
            }
            //if (atype->t == V_FUNCTION) return false;
        }
        return true;
    }
    
    SubFunction *CloneFunction(SubFunction *csf)
    {
        Output(OUTPUT_DEBUG, "cloning: %s", csf->parent->name.c_str());
        auto sf = st.CreateSubFunction();
        sf->SetParent(*csf->parent, csf->parent->subf);
        sf->CloneIds(*csf);
        sf->body = csf->body->Clone();
        sf->freevarchecked = true;
        return sf;
    }
    
    TypeRef TypeCheckCall(SubFunction *csf, Node *call_args, Node &function_def_node)
    {
        Function &f = *csf->parent;
        if (f.multimethod)
        {
            if (!f.subf->numcallers)
            {
                // Simplistic: typechecked with actual argument types.
                // Should attempt static picking as well, if static pick succeeds, specialize.
                for (auto sf = f.subf; sf; sf = sf->next)
                {
                    sf->numcallers++;
                    TypeCheckFunctionDef(*sf, &function_def_node);
                }
                // FIXME: Don't need to do this on every call.
                f.multimethodretval = f.subf->returntypes[0];
                for (auto sf = f.subf->next; sf; sf = sf->next)
                {
                    // FIXME: Lift these limits?
                    if (sf->returntypes.size() != 1)
                        TypeError("multi-methods can currently return only 1 value.", function_def_node);
                    if (!ConvertsTo(sf->returntypes[0], f.multimethodretval, false))
                        TypeError("multi-method " + f.name + " has inconsistent return value types: " +
                                  TypeName(sf->returntypes[0]) + " and " + TypeName(f.multimethodretval),
                                  *call_args);
                }
            }
            return f.multimethodretval;
        }
        else
        {
            SubFunction *sf = csf;
            // First see any args are untyped, this means we must specialize.
            for (auto &arg : csf->args.v) if (arg.flags == AF_ANYTYPE) goto specialize;
            // If we didn't find any such args, and we also don't have any freevars, we don't specialize.
            if (!csf->freevars.v.size() && !csf->mustspecialize) goto match;
            
            specialize:
            {
                assert(!f.istype);  // Should not contain any AF_ANYTYPE

                if (sf->typechecked)
                {
                    // Check if any existing specializations match.
                    for (sf = f.subf; sf; sf = sf->next) if (sf->typechecked && !sf->mustspecialize)
                    {
                        int i = 0;
                        for (Node *list = call_args; list && i < f.nargs(); list = list->tail())
                        {
                            auto &arg = sf->args.v[i++];
                            if (arg.flags == AF_ANYTYPE && !ExactType(list->head()->exptype, arg.type)) goto fail;
                        }
                        if (FreeVarsSameAsCurrent(sf, false)) goto match;
                        fail:;
                    }
                    // No fit. Specialize existing function, or its clone.
                    sf = CloneFunction(csf);
                }

                // Now specialize.
                int i = 0;
                for (Node *list = call_args; list && i < f.nargs(); list = list->tail())
                {
                    auto &arg = sf->args.v[i];
                    if (arg.flags == AF_ANYTYPE)
                    {
                        arg.type = list->head()->exptype;  // Specialized to arg.
                        CheckGenericArg(f.orig_args.v[i].type, arg.type, arg.id->name.c_str(), *list->head(),
                                        f.name.c_str());
                        Output(OUTPUT_DEBUG, "arg: %s:%s", arg.id->name.c_str(), TypeName(arg.type).c_str());
                    }
                    i++;
                }

                // This must be the correct freevar specialization.
                assert(!f.anonymous || sf->freevarchecked);
                assert(!sf->freevars.v.size());

                Output(OUTPUT_DEBUG, "specialization: %s", Signature(*sf).c_str());
            }
            match:
            // Here we have a SubFunction witch matching specialized types.
            sf->numcallers++;
            // First check all the manually typed args.
            int i = 0;
            for (Node *list = call_args; list && i < f.nargs(); i++, list = list->tail())
            {
                auto &arg = sf->args.v[i];
                if (arg.flags != AF_ANYTYPE)
                    SubType(list->head(), arg.type, ArgName(i).c_str(), f.name.c_str());
                if (list->head()->type == T_COCLOSURE)
                    sf->iscoroutine = true;
            }
            if (!f.istype) TypeCheckFunctionDef(*sf, &function_def_node);
            function_def_node.sf() = sf;

            for (auto &freevar : sf->freevars.v)
            {
                // New freevars may have been added during the function def typecheck above.
                // In case their types differ from the flow-sensitive value at the callsite (here),
                // we want to override them.
                freevar.type = freevar.id->cursid->type;
            }

            return sf->returntypes[0];
        }
    }

    SubFunction *PreSpecializeFunction(SubFunction *hsf)
    {
        // Don't pre-specialize named functions, because this is not their call-site.
        if (!hsf->parent->anonymous) return hsf;
        
        hsf = hsf->parent->subf;
        
        auto sf = hsf;
        if (sf->freevarchecked)
        {
            // See if there's an existing match.
            for (; sf; sf = sf->next) if (sf->freevarchecked)
            {
                if (FreeVarsSameAsCurrent(sf, true)) return sf;
            }
            
            sf = CloneFunction(hsf);
        }
        else
        {
            // First time this function has ever been touched.
            sf->freevarchecked = true;
        }
        
        assert(!sf->freevars.v.size());

        // Output without arg types, since those are yet to be overwritten.
        Output(OUTPUT_DEBUG, "pre-specialization: %s", SignatureWithFreeVars(*sf, nullptr, false).c_str());

        return sf;
    }

    TypeRef TypeCheckDynCall(Node &fval, Node *args, Node *fspecnode)
    {
        auto ftype = fval.exptype;
        auto nargs = Parser::CountList(args);
        if (ftype->IsFunction())
        {
            // We can statically typecheck this dynamic call. Happens for almost all non-escaping closures.
            auto sf = ftype->sf;

            if ((int)nargs < sf->parent->nargs())
                TypeError("function value called with too few arguments", fval);
            // In the case of too many args, TypeCheckCall will ignore them (and optimizer will remove them).

            auto type = TypeCheckCall(sf, args, *fspecnode);
            auto nsf = fspecnode->sf();
            fspecnode->exptype = fval.exptype = &nsf->thistype;
            return type;
        }
        else if (ftype->t == V_YIELD)
        {
            // V_YIELD must have perculated up from a coroutine call.
            if (nargs > 1)
                TypeError("coroutine yield call must at most one argument", fval);

            for (auto scope = named_scopes.rbegin(); scope != named_scopes.rend(); ++scope)
            {
                auto sf = scope->sf;
                if (!sf->iscoroutine) continue;

                // What yield returns to returnvalue()
                auto type = args ? args->head()->exptype : type_any;
                RetVal(sf->body, sf, 0, &type);

                // Now collect all ids between coroutine and yield, so that we can save these in the VM
                bool foundstart = false;
                for (auto savescope = scopes.begin(); savescope != scopes.end(); ++savescope)
                {
                    auto ssf = savescope->sf;
                    if (ssf == sf) foundstart = true;
                    if (!foundstart) continue;

                    for (auto &arg : ssf->args.v) sf->coyieldsave.Add(arg);
                    for (auto &loc : ssf->locals.v) sf->coyieldsave.Add(Arg(loc.id, loc.sid, loc.sid->type, false));
                    for (auto &dyn : ssf->dynscoperedefs.v) sf->coyieldsave.Add(dyn);
                }

                for (auto &cys : sf->coyieldsave.v) cys.sid = cys.id->cursid;

                return sf->coresumetype;
            }
            TypeError("yield function called outside scope of coroutine", fval);
            return type_any;
        }
        else
        {
            // We have to do this call entirely at runtime. We take any args, and return any.
            // FIXME: the body T_FUN that created this function value hasn't been typechecked
            // at all, meaning its contents is all T_ANY. This is not necessary esp if the function
            // had no args, or all typed args, but we have no way of telling which T_FUN's
            // will end up this way.
            // For now, just error.
            TypeError("cannot infer type of function in call", fval);
            return type_any;
        }
    }

    TypeRef TypeCheckBranch(bool iftrue, const Node &condition, Node *&bodycall, TType parent_type)
    {
        auto flowstart = CheckFlowTypeChanges(iftrue, condition);
        TypeCheck(bodycall, parent_type);
        CleanUpFlow(flowstart);
        return bodycall->exptype;
    }

    void CheckFlowTypeIdOrDot(const Node &n, TypeRef type)
    {
        if (n.type == T_IDENT ||
            ((n.type == T_DOT || n.type == T_DOTMAYBE) && n.left()->type == T_IDENT))
        {
            flowstack.push_back(FlowItem(&n, type));
        }
    }

    void CheckFlowTypeChangesSub(bool iftrue, const Node &condition)
    {
        auto type = condition.exptype;
        switch (condition.type)
        {
            case T_IS:
                if (iftrue) CheckFlowTypeIdOrDot(*condition.left(), condition.right()->typenode());
                break;

            case T_NOT:
                CheckFlowTypeChangesSub(!iftrue, *condition.child());
                break;

            case T_E2B:
                CheckFlowTypeChangesSub(iftrue, *condition.child());
                break;

            default:
                if (iftrue && type->t == V_NIL) CheckFlowTypeIdOrDot(condition, type->Element());
                break;
        }
    }

    size_t CheckFlowTypeChanges(bool iftrue, const Node &condition)
    {
        auto start = flowstack.size();
        switch (condition.type)
        {
            case T_OR:
            case T_AND:
                // AND only works for then, and OR only for else.
                if (iftrue == (condition.type == T_AND))
                {
                    // This is a bit clumsy, but allows for a chain of &'s without allowing mixed operators
                    if (condition.left()->type == condition.type ||
                        (condition.left()->type == T_E2B && condition.left()->child()->type == condition.type))
                    {
                        CheckFlowTypeChanges(iftrue, *condition.left());
                    }
                    else
                    {
                        CheckFlowTypeChangesSub(iftrue, *condition.left());
                    }
                    CheckFlowTypeChangesSub(iftrue, *condition.right());
                }
                break;

            case T_E2B:
                CheckFlowTypeChanges(iftrue, *condition.child());
                break;

            default:
                CheckFlowTypeChangesSub(iftrue, condition);
                break;
        }
        return start;
    }

    void AssignFlowPromote(Node &left, TypeRef right)
    {
        if ((left.exptype->t == V_ANY && right->t != V_ANY) ||
            (left.exptype->t == V_NIL && right->t != V_NIL))
        {
            CheckFlowTypeIdOrDot(left, right);
        }
    }

    void AssignFlowDemote(Node &left, TypeRef overwritetype, bool coercions)
    {
        // Early out, numeric types are not nillable, nor do they make any sense for "is"
        if (left.exptype->Numeric()) return;

        LookupFlow(left, true, &overwritetype, coercions);
    }

    void UseFlow(Node &n)
    {
        if (n.exptype->Numeric()) return;  // Early out, same as above.

        LookupFlow(n, false, nullptr, false);
    }

    void LookupFlow(Node &n, bool assign, const TypeRef *overwritetype, bool coercions)
    {
        // FIXME: this can in theory find the wrong node, if the same function nests, and the outer one
        // was specialized to a nilable and the inner one was not.
        // This would be very rare though, and benign.
        for (auto it = flowstack.rbegin(); it != flowstack.rend(); ++it)
        {
            auto &flow = *it;
            auto &in = *flow.item;
            switch (n.type)
            {
                case T_IDENT:
                    if (in.type == T_IDENT && in.ident() == n.ident())
                    {
                        n.exptype = flow.old;  // only for assign
                        goto found;
                    }
                    else if ((in.type == T_DOT || in.type == T_DOTMAYBE) && in.left()->ident() == n.ident() && assign)
                    {
                        // We're writing to var V and V.f is in the stack: invalidate regardless.
                        goto found;
                    }
                    break;

                case T_DOT:
                case T_DOTMAYBE:
                    if (n.left()->type == T_IDENT &&
                        (in.type == T_DOT || in.type == T_DOTMAYBE) &&
                        in.left()->ident() == n.left()->ident() &&
                        in.right()->fld() == n.right()->fld())
                    {
                        n.exptype = flow.old;  // only for assign
                        goto found;
                    }
                    break;

                default: assert(0);
            }

            continue;

            found:
            if (assign)
            {
                assert(overwritetype);
                if (!ConvertsTo(*overwritetype, flow.now, coercions))
                {
                    // FLow based promotion is invalidated.
                    flow.now = flow.old;
                    // TODO: it be cool to instead overwrite with whatever type is currently being assigned.
                    // That currently doesn't work, since our flow analysis is a conservative approximation,
                    // so if this assignment happens conditionally it wouldn't work.
                }
                // We continue with the loop here, since a single assignment may invalidate multiple promotions.
            }
            else
            {
                n.exptype = flow.now;
                return;
            }
        }
    }

    void CleanUpFlow(size_t start)
    {
        while (flowstack.size() > start) flowstack.pop_back();
    }

    TypeRef TypeCheckAndOr(Node *&n_ptr, bool only_true_type, TType parent_type)
    {
        // only_true_type supports patterns like ((a & b) | c) where the type of a doesn't matter,
        // and the overal type should be the union of b and c.
        // Or a? | b, which should also be the union of a and b.

        if (n_ptr->type != T_AND && n_ptr->type != T_OR)
        {
            TypeCheck(n_ptr, parent_type);
        }

        Node &n = *n_ptr;

        // We check again, since TypeCheck may have removed a coercion op.
        if (n.type != T_AND && n.type != T_OR)
        {
            auto type = n.exptype;
            if (type->t == V_NIL && only_true_type) return type->Element();
            return type;
        }

        auto tleft = TypeCheckAndOr(n.left(), n.type == T_OR, n.type);
        auto flowstart = CheckFlowTypeChanges(n.type == T_AND, *n.left());
        auto tright = TypeCheckAndOr(n.right(), only_true_type, n.type);
        CleanUpFlow(flowstart);

        if (only_true_type && n.type == T_AND)
        {
            n.exptype = tright;
        }
        else
        {
            n.exptype = Union(tleft, tright, false);
            if (n.exptype->t == V_ANY)
            {
                // Special case: we may have just merged scalar and reference types, and unlike elsewhere, we don't
                // want to box the scalars to make everything references, since typically they are just tested and
                // thrown away. Instead, we force all values to bools.
                MakeBool(n.left());
                MakeBool(n.right());
                n.exptype = type_int;
            }
        }
        return n.exptype;
    }

    void CheckReturnValues(size_t nretvals, size_t i, const string &name, const Node &n)
    {
        if (nretvals <= i)
        {
            parser.Error("function " + name + " returns " + to_string(nretvals) + " values, " + to_string(i + 1) + " requested", &n);
        }
    }

    void CheckReadOnly(Node &n)
    {
        if (n.type == T_DOT && n.left()->exptype->t == V_STRUCT && n.left()->exptype->struc->readonly)
            TypeError("cannot write to field of value: " + n.left()->exptype->struc->name, n);
    }

    void CheckFreeVariable(Ident &id)
    {
        // If this is a free variable, record it in all parents up to the definition point.
        // FIXME: this is technically not the same as a "free variable" in the literature,
        // since HOFs get marked with freevars of their functionvalue this way.
        // This is benign, since the HOF will be specialized to the function value anyway,
        // but would be good to clean up.
        // We currently don't have an easy way to test for lexically enclosing functions.
        for (int i = (int)scopes.size() - 1; i >= 0; i--)
        {
            auto sf = scopes[i].sf;
            // Check if we arrived at the definition point.
            // Since this function may have been cloned since, we also accept any specialization.
            if (id.sf_def == sf ||
                (id.sf_def && id.sf_def->parent == sf->parent && !sf->parent->multimethod))
                break;
            // We use the id's type, not the flow sensitive type, just in case there's multiple uses of the var.
            // this will get corrected after the call this is part of.
            if (sf->freevars.Add(Arg(&id, id.cursid, id.cursid->type, true)))
            {
                //Output(OUTPUT_DEBUG, "freevar added: %s (%s) in %s",
                //       id.name.c_str(), TypeName(id.type).c_str(), sf->parent->name.c_str());
            }
        }
    }

    void TypeCheck(Node *&n_ptr, TType parent_type)
    {
        Node &n = *n_ptr;
        TypeRef &type = n.exptype;

        switch (n.type)
        {
            case T_STRUCTDEF:
                return;

            // Clumsy code, but this pre-specializes for all occurences except being the dcall_function arg.
            case T_DYNCALL:
                if (n.dcall_fval()->type == T_FUN) n.dcall_fval()->sf() = PreSpecializeFunction(n.dcall_fval()->sf());
                break;
            case T_FUN:
                if (n.sf())
                {
                    if (parent_type != T_DYNCALL) n.sf() = PreSpecializeFunction(n.sf());
                    type = &n.sf()->thistype;
                }
                else
                {
                    type = type_any;
                }
                return;

            case T_LIST:
                // Flatten the TypeCheck recursion a bit
                for (Node *stats = &n; stats; stats = stats->b()) TypeCheck(stats->aref(), T_LIST);
                return;

            case T_OR:
            case T_AND:
                TypeCheckAndOr(n_ptr, false, parent_type);
                return;

            case T_IF:
            {
                TypeCheck(n.if_condition(), T_IF);
                Value cval;
                bool isconst = ConstVal(*n.if_condition(), cval);
                if (n.if_else()->type != T_DEFAULTVAL)
                {
                    if (!isconst)
                    {
                        auto tleft = TypeCheckBranch(true, *n.if_condition(), n.if_then(), T_IF);
                        auto tright = TypeCheckBranch(false, *n.if_condition(), n.if_else(), T_IF);
                        type = Union(tleft, tright, false);
                        // These will potentially make either body from T_CALL into some coercion.
						SubType(n.if_then(), type, "then branch", n);
                        SubType(n.if_else(), type, "else branch", n);
                    }
                    else if (cval.True())  // Ignore the else part, optimizer guaranteed to cull it.
                    {
                        type = TypeCheckBranch(true, *n.if_condition(), n.if_then(), T_IF);
                    }
                    else  // Ignore the then part, optimizer guaranteed to cull it.
                    {
                        type = TypeCheckBranch(false, *n.if_condition(), n.if_else(), T_IF);
                    }
                }
                else
                {
                    // No else: this always returns nil.
                    if (!isconst || cval.True())
                    {
                        TypeCheckBranch(true, *n.if_condition(), n.if_then(), T_IF);
                        MakeNil(n.if_then());
                    }
                    // else constant == false: this if-then will get optimized out entirely, ignore it.
                    // bind the var in T_DEFAULTVAL regardless, since it will stay in the AST.
                    TypeCheck(n.if_else(), T_IF);
                    SubType(n.if_else(), type_function_nil, "else", n);
                    type = type_any;
                }
                return;
            }

            case T_WHILE:
            {
                TypeCheck(n.while_condition(), T_WHILE);
                TypeCheckBranch(true, *n.while_condition()->call_function()->sf()->body->head(), n.while_body(), T_WHILE);
                // Currently always return V_NIL
                type = type_any;
                return;
            }

            case T_FOR:
            {
                TypeCheck(n.for_iter(), T_FOR);
                auto itertype = n.for_iter()->exptype;
                if (itertype->t == V_INT || itertype->t == V_STRING) itertype = type_int;
                else if (itertype->t == V_VECTOR) itertype = itertype->Element();
                else if (itertype->t == V_STRUCT) itertype = itertype->struc->vectortype->Element();
                else TypeError("for can only iterate over int/string/vector/struct, not: " + TypeName(itertype), n);
                auto args = n.for_body()->call_args();
                if (args)
                {
                    args->head()->exptype = itertype;
                    if (args->tail()) args->tail()->head()->exptype = type_int;
                }
                TypeCheck(n.for_body(), T_FOR);
                // Currently always return V_NIL
                type = type_any;
                return;
            }

            case T_FORLOOPVAR:
                // Ignore, they've already been assigned a type in T_FOR.
                return;

            case T_CODOT:
            {
                TypeCheck(n.left(), n.type);
                // Leave right ident untypechecked.

                SubType(n.left(), type_coroutine, "coroutine", n);
                auto sf = n.left()->exptype->sf;
                Arg *uarg = nullptr;
                auto &name = n.right()->ident()->name;  // This ident is not necessarily the right one.
                for (auto &arg : sf->coyieldsave.v) if (arg.id->name == name)
                {
                    if (uarg) TypeError("multiple coroutine variables named: " + name, n);
                    uarg = &arg;
                }
                if (!uarg) TypeError("no coroutine variables named: " + name, n);
                n.right()->ident() = uarg->id;
                n.right()->sid() = uarg->sid;
                n.right()->exptype = type = uarg->type;
                return;
            }

            case T_DEF:
            case T_ASSIGNLIST:
            {
                auto dl = &n;
                vector<Node *> idnodes;
                for (; dl->type == T_DEF || dl->type == T_ASSIGNLIST; dl = dl->right())
                {
                    idnodes.push_back(dl->left());
                    if (dl->type == T_ASSIGNLIST)
                    {
                        TypeCheck(dl->left(), T_ASSIGNLIST);
                    }
                    else  // T_DEF
                    {
                        auto id = dl->left()->ident();
                        if (!id->cursid)
                        {
                            assert(!id->sf_def);  // Must be a global;
                            id->cursid = NewSid(id, type_any);
                        }
                        // We have to use a variable here because the init exp may be a function call that causes this
                        // variable to be used/assigned.
                        id->cursid->type = n.c() ? n.c()->typenode() : NewTypeVar();
                    }
                }
                TypeCheck(dl, n.type);
                size_t i = 0;
                for (auto idn : idnodes)
                {
                    if (!dl) parser.Error("right hand side does not return enough values", &n);
                    auto type = dl->exptype;
                    switch (dl->type)
                    {
                        case T_CALL:
                        {
                            auto sf = dl->call_function()->sf();
                            CheckReturnValues(sf->returntypes.size(), i, sf->parent->name, n);
                            type = sf->returntypes[i];
                            break;
                        }

                        case T_NATCALL:
                        {
                            if (i)  // For the 0th value, we prefer the existing type, see T_NATCALL below.
                            {
                                auto nf = dl->ncall_id()->nf();
                                CheckReturnValues(nf->retvals.v.size(), i, nf->name, n);
                                assert(nf->retvals.v[i].flags == AF_NONE);  // See T_NATCALL below.
                                type = nf->retvals.v[i].type;
                            }
                            break;
                        }

                        case T_MULTIRET:
                            type = dl->headexp()->exptype;
                            dl = dl->tailexps();
                            break;
                    }
                    if (n.type == T_DEF)
                    {
                        if (n.c()) type = n.c()->typenode();
                        auto id = idn->ident();
                        // Must SubType here rather than assignment, since id->cursid->type is var that may have
                        // been bound by the initializer already.
                        SubTypeT(type, id->cursid->type, n, "initializer");
                        idn->exptype = type;
                        idn->sid() = id->cursid;
                        Output(OUTPUT_DEBUG, "var: %s:%s", id->name.c_str(), TypeName(type).c_str());
                        if (id->logvaridx >= 0)
                        {
                            for (auto &sc : scopes)
                                if (sc.sf->iscoroutine)
                                    TypeError("can\'t use log variable inside coroutine", n);
                        }
                    }
                    else
                    {
                        AssignFlowDemote(*idn, type, false);
                        SubTypeT(type, idn->exptype, n, "right");
                    }
                    i++;
                }
                return;
            }
        }

        if (n.a()) TypeCheck(n.aref(), n.type);
        if (n.b()) TypeCheck(n.bref(), n.type);
        if (n.c()) TypeCheck(n.cref(), n.type);

        switch (n.type)
        {
            case T_INT:   type = type_int; break;
            case T_FLOAT: type = type_float; break;
            case T_STR:   type = type_string; break;
            case T_NIL:   type = n.typenode()->t != V_ANY ? n.typenode() : NewTypeVar()->Wrap(NewType(), V_NIL);
                          break;

            case T_PLUS:
            case T_MINUS:
            case T_MULT:
            case T_DIV:
            case T_MOD:
            {
                type = Union(n.left(), n.right(), true);
                bool unionchecked = false;
                MathError(type, n, n.type, unionchecked, true);
                if (!unionchecked) SubTypeLR(type, n);
                break;
            }

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:
            {
                CheckReadOnly(*n.left());
                type = n.left()->exptype;
                if (!MathCheckVector(type, n.left(), n.right()))
                {
                    bool unionchecked = false;
                    MathError(type, n, TType(n.type - T_PLUSEQ + T_PLUS), unionchecked, false);
                    if (!unionchecked) SubType(n.right(), type, "right", n);
                }
                break;
            }

            case T_NEQ: 
            case T_EQ: 
            case T_GTEQ: 
            case T_LTEQ: 
            case T_GT:  
            case T_LT:
            {
                type = type_int;
                auto u = Union(n.left(), n.right(), true);
                if (!u->Numeric() && u->t != V_STRING)
                {
                    // FIXME: rather than nullptr, these TypeError need to figure out which side caused the error much like MathError
                    if (n.type == T_EQ || n.type == T_NEQ)
                    {
                        // pointer comparison
                        if (u->t != V_VECTOR && u->t != V_STRUCT && u->t != V_NIL)
                            TypeError("numeric/string/vector/struct", u, n, nullptr);
                    }
                    else
                    {
                        // comparison vector op
                        if ((u->t == V_VECTOR && u->Element()->Numeric()) ||
                            (u->t == V_STRUCT && u->struc->vectortype->Element()->Numeric()))
                        {
                            type = type_vector_int;
                        }
                        else if (MathCheckVector(type, n.left(), n.right()))
                        {
                            type = type_vector_int;
                            break; // don't do SubTypeLR since type already verified and `u` not appropriate anyway.
                        }
                        else
                        {
                            TypeError("numeric/string/vector/struct", u, n, nullptr);
                        }
                    }
                }
                SubTypeLR(u, n);
                break;
            }

            case T_NOT:
                type = type_int;
                break;

            case T_POSTDECR:
            case T_POSTINCR:
            case T_DECR:  
            case T_INCR:
            {
                CheckReadOnly(*n.child());
                type = n.child()->exptype;
                if (!type->Numeric())
                    TypeError("numeric", type, n, nullptr);
                break;
            }

            case T_UMINUS:
            {
                type = n.child()->exptype;
                if (!type->Numeric() &&
                    type->t != V_VECTOR &&
                    (type->t != V_STRUCT || !type->struc->vectortype->Element()->Numeric()))
                    TypeError("numeric/vector", type, n, nullptr);
                break;
            }

            case T_IDENT:
            {
                auto id = n.ident();
                assert(id->cursid);
                type = id->cursid->type;
                n.sid() = id->cursid;
                CheckFreeVariable(*id);
                UseFlow(n);
                break;
            }

            case T_ASSIGN:
                CheckReadOnly(*n.left());
                if (n.left()->type != T_INDEX) AssignFlowDemote(*n.left(), n.right()->exptype, true);
                SubType(n.right(), n.left()->exptype, "right", n);
                if (n.left()->type != T_INDEX) AssignFlowPromote(*n.left(), n.right()->exptype);
                type = n.left()->exptype;
                break;

            case T_DEFAULTVAL:
                // This is used only as a default value for native call arguments.
                // The variable makes it equal to whatever the function expects, then codegen can use that type
                // to generate a correct value.
                type = NewTypeVar();
                break;

            case T_NATCALL:
            {
                auto nf = n.ncall_id()->nf();

                if (nf->first->overloads)
                {
                    // Multiple overloads available, figure out which we want to call.
                    auto cnf = nf->first;
                    nf = nullptr;
                    auto nargs = Parser::CountList(n.ncall_args());
                    for (; cnf; cnf = cnf->overloads)
                    {
                        if (cnf->args.v.size() != nargs) continue;

                        Node *list = n.ncall_args();
                        for (auto &arg : cnf->args.v)
                        {
                            if (!ConvertsTo(list->head()->exptype, arg.type, arg.type->t != V_STRING, false, true)) goto nomatch;
                            list = list->tail();
                        }
                        //if (nf) NatCallError("arguments match more than one overload of ", cnf, n);
                        nf = cnf;
                        n.ncall_id()->nf() = nf;
                        break;
                        nomatch:;
                    }
                    if (!nf) NatCallError("arguments match no overloads of ", n.ncall_id()->nf(), n);
                }

                int i = 0;
                vector<TypeRef> argtypes;
                for (Node *list = n.ncall_args(); list; list = list->tail())
                {
                    auto &arg = nf->args.v[i];
                    auto argtype = arg.type;
                    bool typed = false;

                    if (argtype->t == V_NIL && argtype->sub->Numeric() && list->head()->type != T_DEFAULTVAL)
                    {
                        // This is somewhat of a hack, because we conflate V_NIL with being optional for 
                        // native functions, but we don't want numeric types to be nilable.
                        // Codegen has a special case for T_DEFAULTVAL however.
                        argtype = argtype->sub;
                    }
                    
                    int flag = NF_SUBARG1;
                    for (int sa = 0; sa < 3; sa++)
                    {
                        if (arg.flags & flag)
                        {
                            SubType(list->head(),
                                    nf->args.v[sa].type->t == V_VECTOR && argtype->t != V_VECTOR
                                        ? VectorStructElement(argtypes[sa])
                                        : argtypes[sa],
                                    ArgName(i).c_str(),
                                    nf->name.c_str());
                            typed = true;  // Stop these generic params being turned into any by SubType below.
                        }
                        flag *= 2;
                    }

                    if (arg.flags & NF_ANYVAR)
                    {
                        if (argtype->t == V_VECTOR) argtype = NewTypeVar()->Wrap(NewType());
                        else if (argtype->t == V_ANY) argtype = NewTypeVar();
                        else assert(0);
                    }

                    if (arg.flags & NF_CORESUME)
                    {
                        // Specialized typechecking for resume()
                        assert(argtypes[0]->t == V_COROUTINE);  
                        auto sf = argtypes[0]->sf;
                        if (sf)
                        {
                            SubType(list->head(), sf->coresumetype, "resume value", *list->head());
                        }
                        else
                        {
                            if (list->head()->type != T_DEFAULTVAL)
                                TypeError("cannot resume a generic coroutine type with an argument", n);
                        }
                        if (list->head()->exptype->t == V_VAR)
                        {
                            // No value supplied to resume, and none expected at yield either.
                            // nil will be supplied, so make type reflect that.
                            UnifyVar(NewNilTypeVar(), list->head()->exptype);
                        }
                        typed = true;
                    }
                    
                    if (!typed) SubType(list->head(), argtype, ArgName(i).c_str(), nf->name.c_str());

                    auto actualtype = list->head()->exptype;
                    if (actualtype->IsFunction())
                    {
                        // We must assume this is going to get called and type-check it
                        auto sf = actualtype->sf;
                        Node *args = nullptr;
                        if (sf->args.v.size())
                        {
                            // we have no idea what args.
                            assert(0);
                            TypeError("function passed to " + nf->name + " cannot take any arguments", n);
                        }
                        auto fake_function_def = (Node *)new FunRef(n.line, sf);
                        TypeCheckCall(sf, args, *fake_function_def);
                        assert(sf == fake_function_def->sf());
                        delete fake_function_def;
                    }
                    argtypes.push_back(actualtype);
                    i++;
                }

                if (nf->name == "assert")
                {
                    // Special case, add to flow:
                    CheckFlowTypeChanges(true, *n.ncall_args()->head());
                }

                type = type_any;  // no retvals
                if (nf->retvals.v.size())
                {
                    // multiple retvals taken care of by T_DEF / T_ASSIGNLIST
                    auto &ret = nf->retvals.v[0];
                    int sa = 0;
                    switch (ret.flags)
                    {
                        case NF_SUBARG3: sa++;
                        case NF_SUBARG2: sa++;
                        case NF_SUBARG1:
                        {
                            type = argtypes[sa];
                            auto nftype = nf->args.v[sa].type;

                            if (nftype->t == V_TYPEID)
                            {
                                assert(!sa);  // assumes always first.
                                auto tin = n.ncall_args()->head();
                                assert(tin->type == T_TYPEOF);
                                if (tin->child()) type = tin->child()->exptype;
                            }

                            if (ret.type->t == V_NIL)
                            {
                                if (!IsRef(type->t))
                                    TypeError("argument " + to_string(sa + 1) + " to " + nf->name + " can't be scalar", n);
                                type = type->Wrap(NewType(), V_NIL);
                            }
                            else if (nftype->t == V_VECTOR && ret.type->t != V_VECTOR)
                            {
                                type = VectorStructElement(type);
                            }
                            else if (nftype->t == V_COROUTINE || nftype->t == V_FUNCTION)
                            {
                                auto sf = type->sf;
                                assert(sf);
                                type = sf->returntypes[0];  // in theory it is possible this hasn't been generated yet..
                            }
                            break;
                        }
                        case NF_ANYVAR: 
                            type = ret.type->t == V_VECTOR ? NewTypeVar()->Wrap(NewType()) : NewTypeVar();
                            break;
                        default:
                            assert(ret.flags == AF_NONE);
                            type = ret.type; 
                            break;
                    }
                    // See if we can promote the return type to one of the standard vector types (xy/xyz/xyzw).
                    int flen = ret.fixed_len;
                    if (flen == -1 && n.ncall_args() && n.ncall_args()->head()->exptype->t == V_STRUCT)
                    {
                        // Special case: type is promised to be the same as the input arg 0, so if it was a struct,
                        // we can find the length from that.
                        flen = (int)n.ncall_args()->head()->exptype->struc->fields.size();
                    }
                    if (flen >= 2 && flen <= 4 && type->t == V_VECTOR && type->sub->Numeric())
                    {
                        type = type->sub->t == V_INT ? st.default_int_vector_types[flen]
                                                     : st.default_float_vector_types[flen];
                    }
                }
                break;
            }

            case T_CALL:
            {
                auto sf = n.call_function()->sf();
                type = TypeCheckCall(sf, n.call_args(), *n.call_function());
                break;
            }

            case T_DYNCALL:
                type = TypeCheckDynCall(*n.dcall_fval(), n.dcall_args(), n.dcall_function());
                break;

            case T_RETURN:
            {
                auto fid = n.return_function_idx()->integer();

                for (int i = (int)scopes.size() - 1; i >= 0; i--)
                {
                    if (scopes[i].sf->parent->idx == fid) break;
                    if (scopes[i].sf->iscoroutine) TypeError("cannot return out of coroutine", n);
                }

                if (fid < 0) break;  // return from program

                auto sf = TopScope(named_scopes);
                // Even if this function is specialized, the current one should be the top one.
                auto sf_req = st.functiontable[fid]->subf;
                if (sf->parent != sf_req->parent)
                {
                    // This is a non-local "return from".
                    // FIXME: this is returnining from a specialized version, should really make VM implementation use
                    // SubFunction too.
                    if (!sf_req->typechecked)
                        parser.Error("return from " + sf_req->parent->name + " called out of context", &n);
                    // Do typechecking below against non-local one:
                    sf = sf_req;
                }
                if (n.return_value())
                {
                    type = n.return_value()->exptype;
                    if (n.return_value()->type == T_MULTIRET)
                    {
                        int i = 0;
                        for (auto mr = n.return_value(); mr; mr = mr->tailexps())
                        {
                            RetVal(mr->headexp(), sf, i++);
                            if (!i) type = mr->headexp()->exptype;
                        }
                    }
                    else if (n.return_value()->type == T_CALL &&
                             n.return_value()->call_function()->sf()->returntypes.size() > 1)
                    {
                        // Multi-ret pass-thru:
                        size_t i = 0;
                        for (auto &type : n.return_value()->call_function()->sf()->returntypes)
                            RetVal(n.return_value(), sf, i++, &type);
                    }
                    else
                    {
                        RetVal(n.return_value(), sf, 0);
                    }
                }
                else
                {
                    type = type_any;
                    RetVal(n_ptr, sf, 0, &type);
                }
                break;
            }

            case T_TYPE:
                type = n.typenode();
                break;

            case T_IS:
                type = type_int;
                break;

            case T_FIELD:
                break;  // Already set by the parser.

            case T_CONSTRUCTOR:
            {
                type = n.constructor_type()->typenode();
                if (type->t == V_VECTOR && type->sub->t == V_ANY)
                {
                    // No type was specified.. first find union of all elements.
                    TypeRef u;
                    int i = 0;
                    for (auto list = n.constructor_args(); list; list = list->tail())
                    {
                        u = i ? Union(u, list->head()->exptype, true) : list->head()->exptype;
                        i++;
                    }
                    type = u->Wrap(NewType());
                    if (!i) type = NewTypeVar()->Wrap(NewType());  // special case for empty vectors
                }
                if (type->t == V_STRUCT)
                {
                    auto struc = FindStructSpecialization(type->struc, n.constructor_args(), n);
                    type = &struc->thistype;
                }
                int i = 0;
                for (auto list = n.constructor_args(); list; list = list->tail())
                {
                    TypeRef elemtype = type->t == V_STRUCT ? type->struc->fields.v[i].type : type->Element();
                    SubType(list->head(), elemtype, ArgName(i).c_str(), n);
                    i++;
                }
                break;
            }

            case T_DOT:
            case T_DOTMAYBE:
            {
                auto smtype = n.left()->exptype;
                auto stype = n.type == T_DOTMAYBE && smtype->t == V_NIL
                             ? smtype->Element()
                             : smtype;
                if (stype->t != V_STRUCT)
                    TypeError("struct/value", stype, n, "object");
                auto struc = stype->struc;
                auto sf = n.right()->fld();
                auto fieldidx = struc->Has(sf);
                if (fieldidx < 0) TypeError("type " + struc->name + " has no field named " + sf->name, n);
                auto &uf = struc->fields.v[fieldidx];
                type = n.type == T_DOTMAYBE && smtype->t == V_NIL && uf.type->t != V_NIL
                       ? uf.type->Wrap(NewType(), V_NIL)
                       : uf.type;
                n.right()->exptype = stype;  // Store struct type here for codegen.
                UseFlow(n);
                break;
            }

            case T_INDEX:
            {
                auto vtype = n.left()->exptype;
                if (vtype->t == V_STRUCT) vtype = vtype->struc->vectortype;
                if (vtype->t != V_VECTOR && vtype->t != V_STRING)
                    TypeError("vector/string", vtype, n, "container");
                auto itype = n.right()->exptype;
                switch (itype->t)
                {
                    case V_INT: vtype = vtype->t == V_VECTOR ? vtype->Element() : type_int; break;
                    case V_STRUCT:
                    {
                        auto &struc = *itype->struc;
                        for (auto &field : struc.fields.v)
                        {
                            if (field.type->t != V_INT) TypeError("int field", field.type, n, "index");
                            if (vtype->t != V_VECTOR) TypeError("nested vector", vtype, n, "container");
                            vtype = vtype->Element();
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

            case T_COCLOSURE:
                type = type_function_cocl;
                break;

            case T_COROUTINE:
            {
                if (n.child()->type != T_CALL)  // could be dyn call
                    TypeError("coroutine constructor must be regular function call", n);
                auto sf = n.child()->call_function()->sf();
                assert(sf->iscoroutine);
                auto ct = NewType();
                *ct = Type(V_COROUTINE, sf);
                type = ct;
                break;
            }

            case T_TYPEOF:
                type = type_typeid;
                break;

            case T_MULTIRET:
            case T_FUN:
            case T_NATIVE:
            case T_LIST:
            case T_STRUCT:
                break;

            case T_I2F:
            case T_A2S:
            case T_E2A:
            case T_E2N:
            case T_E2B:
            case T_T2I:
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

        // FIXME: expensive check, but helps track down generic type bugs.
        //assert(n.type == T_TYPE || type.UnWrapped()->t != V_STRUCT || !st.structtable[type.UnWrapped().idx]->generic);
    }
    
    void Stats()
    {
        if (min_output_level > OUTPUT_INFO) return;

        int origsf = 0, multisf = 0, clonesf = 0;
        int orignodes = 0, clonenodes = 0;
        typedef pair<int, Function *> Pair;
        vector<Pair> funstats;
        for (auto f : st.functiontable) funstats.push_back(make_pair(0, f));
        for (auto sf : st.subfunctiontable)
        {
            if (sf->parent->multimethod) { multisf++; orignodes += CountNodes(sf->body); }
            else if (!sf->next)          { origsf++;  orignodes += CountNodes(sf->body); }
            else
            {
                clonesf++;
                auto count = CountNodes(sf->body);
                clonenodes += count;
                funstats[sf->parent->idx].first += count;
            }
        }
        Output(OUTPUT_INFO, "SF count: multi: %d, orig: %d, cloned: %d", multisf, origsf, clonesf);
        Output(OUTPUT_INFO, "Node count: orig: %d, cloned: %d", orignodes, clonenodes);
        sort(funstats.begin(), funstats.end(), [](const Pair &a, const Pair &b) { return a.first > b.first; });
        for (auto &p : funstats) if (p.first > orignodes / 50)
        {
            auto &pos = p.second->subf->body->line;
            Output(OUTPUT_INFO, "Most clones: %s (%s:%d) -> %d nodes accross %d clones (+1 orig)",
                   p.second->name.c_str(),
                   st.filenames[pos.fileidx].c_str(),
                   pos.line,
                   p.first,
                   p.second->NumSubf() - 1);
        }
    }
};

}  // namespace lobster
