
namespace lobster
{

struct TypeChecker
{
    Lex &lex;
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

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st)
    {
        st.RegisterDefaultVectorTypes();
        for (auto &struc : st.structtable)
        {
            if (!struc->generic) ComputeStructVectorType(struc);
            if (struc->superclass) for (auto &field : struc->fields)
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
        if (struc->fields.size())
        {
            TypeRef vectortype = struc->fields[0].type;
            for (size_t i = 1; i < struc->fields.size(); i++)
            {
                // FIXME: Can't use Union here since it will bind variables
                //vectortype = Union(vectortype, struc->fields[i].type, false);
                // use simplified alternative:
                if (!ExactType(struc->fields[i].type, vectortype)) vectortype = type_any;
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

    template<typename T> string TypedArg(const T &arg, bool withtype = true)
    {
        string s = arg.id ? arg.id->name : "arg";  // FIXME: use ArgVector::GetName here instead
        if (arg.type->t != V_ANY && withtype) s += ":" + TypeName(arg.type);
        return s;
    }

    template<typename T> string Signature(const vector<T> &v, bool withtype = true)
    {
        string s = "(";
        int i = 0;
        for (auto &arg : v)
        {
            if (i++) s += ", ";
            s += TypedArg(arg, withtype);
        }
        return s + ")"; 
    }

    string Signature(const Struct &struc)                         { return struc.name      + Signature(struc.fields); }
    string Signature(const SubFunction &sf, bool withtype = true) { return sf.parent->name + Signature(sf.args.v, withtype); }
    string Signature(const NativeFun &nf)                         { return nf.name         + Signature(nf.args.v); }

    string SignatureWithFreeVars(const SubFunction &sf, bool withtype = true)
    {
        string s = Signature(sf, withtype) + " { ";
        for (auto &freevar : sf.freevars.v) if (freevar.type->t != V_FUNCTION) s += TypedArg(freevar) + " ";
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
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it)
        {
            auto &scope = *it;
            err += "\n  in " + parser.lex.Location(scope.call_context->fileidx, scope.call_context->linenumber) + ": ";
            err += SignatureWithFreeVars(*scope.sf);
            for (Node *list = scope.sf->body; list; list = list->tail())
            {
                for (auto dl = list->head(); dl->type == T_DEF; dl = dl->right())
                {
                    auto id = dl->left()->ident();
                    //if (id->type->t != V_ANY)
                    {
                        err += ", " + id->name + ":" + TypeName(id->type);
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
        return var;
    }

    TypeRef UnifyVar(TypeRef type, TypeRef hasvar)
    {
        // Typically Type is const, but this is the one place we overwrite them.
        // Type objects that are V_VAR are seperate heap instances, so overwriting them has no side-effects on
        // non-V_VAR Type instances.
        assert(hasvar->t == V_VAR);
        if (type->t != V_VAR || type != hasvar) *(Type *)&*hasvar = *type;
        return hasvar;
    }

    bool ConvertsTo(TypeRef type, TypeRef sub, bool coercions)
    {
        if (sub == type) return true;
        if (type->t == V_VAR) return ConvertsTo(UnifyVar(sub, type), sub, coercions);
        switch (sub->t)
        {
            case V_ANY:       return true;
            case V_VAR:       return ConvertsTo(type, UnifyVar(type, sub), coercions);
            case V_FLOAT:     return type->t == V_INT && coercions;
            case V_STRING:    return coercions;
            case V_FUNCTION:  return type->t == V_FUNCTION && !sub->sf;
            case V_NILABLE:   return type->t == V_NIL ||
                                     (type->t == V_NILABLE && ConvertsTo(type->Element(), sub->Element(), false)) ||
                                     (!type->Numeric() && ConvertsTo(type, sub->Element(), false));
            case V_VECTOR:    return ((type->t == V_VECTOR && ConvertsTo(type->Element(), sub->Element(), false)) ||
                                      (type->t == V_STRUCT && ConvertsTo(type->struc->vectortype, sub, false)));
            case V_STRUCT:    return type->t == V_STRUCT && st.IsSuperTypeOrSame(sub->struc, type->struc);
            case V_COROUTINE: return type->t == V_COROUTINE && (sub->sf == type->sf || !sub->sf);
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

    void SubTypeLR(TypeRef sub, Node &n) { SubType(n.left(), sub, "left", n); SubType(n.right(), sub, "right", n); }

    void SubType(Node *&a, TypeRef sub, const char *argname, const Node &context)
    {
        SubType(a, sub, argname, TName(context.type));
    }
    void SubType(Node *&a, TypeRef sub, const char *argname, const char *context)
    {
        TypeRef type = a->exptype;
        if (ConvertsTo(type, sub, false)) { a->exptype = type; return; }
        switch (sub->t)
        {
            case V_FLOAT:
                if (type->t == V_INT)
                {
                    a = new Node(lex, T_I2F, a, nullptr);
                    a->exptype = type_float;
                    return;
                }
                break;
            case V_STRING:
                a = new Node(lex, T_A2S, a, nullptr);
                a->exptype = type_string;
                return;
            case V_FUNCTION:
                if (type->IsFunction() && sub->sf)
                {
                    // See if these functions can be made compatible. Specialize and typecheck if needed.
                    auto sf = type->sf;
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
                        if (!ConvertsTo(ss->args.v[i].type, arg.type, false)) goto error;
                        i++;
                    }
                    TypeCheckFunctionDef(*sf, sf->body);
                    // Covariant again.
                    if (sf->returntypes.size() != ss->returntypes.size() ||
                        !ConvertsTo(sf->returntypes[0], ss->returntypes[0], false)) break;
                    assert(ss->returntypes.size() == 1);  // Parser only parses one ret type for function types.
                    return;
                }
                break;
        }
        error:
        TypeError(TypeName(sub).c_str(), type, *a, argname, context);
    }

    void SubTypeT(TypeRef type, TypeRef sub, const Node &n, const char *argname, const char *context = nullptr)
    {
        if (!ConvertsTo(type, sub, false)) TypeError(TypeName(sub).c_str(), type, n, argname, context);
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

    bool MathCheckVector(TypeRef &type, TypeRef ltype, TypeRef rtype, bool flipalso)
    {
        // Special purpose check for vector * scalar etc, needs to be improved further.
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
                type = vtype->t == V_INT && rtype->t == V_INT ? type_vector_int : type_vector_float;
                // Recover struct type if possible.
                if (ltype->t == V_STRUCT) type = StructTypeFromVector(type, ltype->struc);
                return true;
            }
        }

        if (flipalso)
        {
            // Now check scalar * vector instead.
            return MathCheckVector(type, rtype, ltype, false);
        }

        return false;
    }

    const char *MathCheck(TypeRef &type, const Node &n, TType op, bool &unionchecked)
    {
        if (op == T_MOD)
        {
            if (type->t != V_INT) return "int";
        }
        else
        {
            if (!type->Numeric() && type->t != V_VECTOR && type->t != V_STRUCT)
            {
                auto ltype = n.left()->exptype;
                auto rtype = n.right()->exptype;
                
                if (MathCheckVector(type, ltype, rtype, true))
                {
                    unionchecked = true;
                    return nullptr;
                }

                if (op == T_PLUS)
                {
                    if (type->t != V_STRING && 
                        // Anything nilable can be added to a string, but only on one side:
                        (type->t != V_NILABLE ||
                         type->sub->t != V_STRING ||
                         (ltype->t == V_NILABLE && rtype->t == V_NILABLE)))
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

    void MathError(TypeRef &type, const Node &n, TType op, bool &unionchecked)
    {
        auto err = MathCheck(type, n, op, unionchecked);
        if (err)
        {
            if (MathCheck(n.left()->exptype, n, op, unionchecked)) TypeError(err, n.left()->exptype, n, "left");
            if (MathCheck(n.right()->exptype, n, op, unionchecked)) TypeError(err, n.right()->exptype, n, "right");
            TypeError(string("can\'t use \"") +
                      TName(n.type) + 
                      "\" on " + 
                      TypeName(n.left()->exptype) + 
                      " and " + 
                      TypeName(n.right()->exptype), n);
        }
    }

    SubFunction *TopScope(vector<Scope> &scopes) { return scopes.empty() ? nullptr : scopes.back().sf; }

    void RetVal(Node *a, SubFunction *sf, size_t i, TypeRef *exacttype = nullptr)
    {
        if (!sf) return;
        if (i >= sf->returntypes.size())
        {
            assert(i == sf->returntypes.size());
            sf->returntypes.push_back(exacttype ? *exacttype : a->exptype);
        }
        else
        {
            if (exacttype) sf->returntypes[i] = Union(*exacttype, sf->returntypes[i], false);
            else if (a) sf->returntypes[i] = Union(a->exptype, sf->returntypes[i], false);
            else sf->returntypes[i] = type_any;  // FIXME: this allows "return" followed by "return 1" ?
        }
    }

    void TypeCheckFunctionDef(SubFunction &sf, const Node *call_context)
    {
        if (sf.typechecked) return;
        
        Output(OUTPUT_DEBUG, "function start: %s", SignatureWithFreeVars(sf).c_str());

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
            // but will become an issue when we want to store values non-uniformly (i.e. different sizes).
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

        // FIXME: this would not be able to typecheck recursive functions with multiret.
        sf.returntypes[0] = NewTypeVar();
        sf.coresumetype = sf.iscoroutine ? NewTypeVar() : type_any;

        auto start_promoted_vars = flowstack.size();

        TypeCheck(sf.body, T_LIST);

        CleanUpFlow(start_promoted_vars);

        for (auto &back : backup_args.v) back.id->type = back.type;
        for (auto &back : backup_locals.v) back.id->type = back.type;

        Node *last = nullptr;
        for (auto topl = sf.body; topl; topl = topl->tail()) last = topl;
        assert(last);
        if (last->head()->type != T_RETURN) RetVal(last->head(), &sf, 0);

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
                auto &field = struc->fields[i++];
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
    
    bool FreeVarsSameAsCurrent(SubFunction *sf)
    {
        for (auto &freevar : sf->freevars.v)
        {
            //auto atype = Promote(freevar.id->type);
            if (!ExactType(freevar.type, freevar.id->type)) return false;
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
            // Simplistic: typechecked with actual argument types.
            // Should attempt static picking as well, if static pick succeeds, specialize.
            // FIXME: no need to repeat this on every call.
            for (auto sf = f.subf; sf; sf = sf->next) TypeCheckFunctionDef(*sf, &function_def_node);
            TypeRef type = f.subf->returntypes[0];
            for (auto sf = f.subf->next; sf; sf = sf->next) type = Union(type, sf->returntypes[0], false);
            return type;
        }
        else
        {
            SubFunction *sf = csf;
            // First see any args are untyped, this means we must specialize.
            for (auto &arg : csf->args.v) if (arg.flags == AF_ANYTYPE) goto specialize;
            // If we didn't find any such args, and we also don't have any freevars, we don't specialize.
            if (!csf->freevars.v.size()) goto match;
            
            specialize:
            {
                assert(!f.istype);  // Should not contain any AF_ANYTYPE

                if (sf->typechecked)
                {
                    // Check if any existing specializations match.
                    for (sf = f.subf; sf; sf = sf->next) if (sf->typechecked)
                    {
                        int i = 0;
                        for (Node *list = call_args; list && i < f.nargs(); list = list->tail())
                        {
                            auto &arg = sf->args.v[i++];
                            if (arg.flags == AF_ANYTYPE && !ExactType(list->head()->exptype, arg.type)) goto fail;
                        }
                        if (FreeVarsSameAsCurrent(sf)) goto match;
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
                for (auto &freevar : sf->freevars.v)
                {
                    if (f.anonymous)
                    {
                        // cases where these might not be equal:
                        // - if the original freevar wasn't bound yet:
                        //if (freevar.type->t == V_VAR) UnifyVar(freevar.id->type, freevar.type);
                        // - in the case of a dynamic scope assignment? FIXME: make a test case for this.
                        // is it easier to just overwrite the freevar by the one in the id in all cases? does that work?

                        assert(ExactType(freevar.type, freevar.id->type));
                        (void)freevar;
                    }
                    else  // Named function.
                    {
                        freevar.type = freevar.id->type;  // Specialized to current value.
                    }
                }
                Output(OUTPUT_DEBUG, "specialization: %s", Signature(*sf).c_str());
            }
            match:
            // Here we have a SubFunction witch matching specialized types.
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
                if (FreeVarsSameAsCurrent(sf)) return sf;
            }
            
            sf = CloneFunction(hsf);
        }
        else
        {
            // First time this function has ever been touched.
            sf->freevarchecked = true;
        }
        
        // Copy freevars.
        for (auto &freevar : sf->freevars.v)
        {
            freevar.type = freevar.id->type;  // Specialized to current value.
        }

        // Output without types, since those are yet to be overwritten.
        Output(OUTPUT_DEBUG, "pre-specialization: %s", SignatureWithFreeVars(*sf, false).c_str());

        return sf;
    }

    TypeRef TypeCheckDynCall(Node &fval, Node *args, Node *fspecnode = nullptr)
    {
        auto ftype = fval.exptype;
        if (ftype->IsFunction())
        {
            // We can statically typecheck this dynamic call. Happens for almost all non-escaping closures.
            auto sf = ftype->sf;

            if (Parser::CountList(args) < sf->parent->nargs())
                TypeError("function value called with too few arguments", fval);
            // In the case of too many args, TypeCheckCall will ignore them (and codegen also).

            auto &fnode = fspecnode ? *fspecnode : fval;
            auto type = TypeCheckCall(sf, args, fnode);
            auto nsf = fnode.sf();
            fnode.exptype = fval.exptype = &nsf->thistype;
            return type;
        }
        else if (ftype->t == V_YIELD)
        {
            // V_YIELD must have perculated up from a coroutine call.
            if (Parser::CountList(args) > 1)
                TypeError("coroutine yield call must at most one argument", fval);

            for (auto scope = named_scopes.rbegin(); scope != named_scopes.rend(); ++scope)
            {
                auto sf = scope->sf;
                if (!sf->iscoroutine) continue;

                // What yield returns to returnvalue()
                auto type = args ? args->head()->exptype : type_any;
                RetVal(nullptr, sf, 0, &type);

                // Now collect all ids between coroutine and yield, so that we can save these in the VM
                bool foundstart = false;
                for (auto savescope = scopes.begin(); savescope != scopes.end(); ++savescope)
                {
                    auto ssf = savescope->sf;
                    if (ssf == sf) foundstart = true;
                    if (!foundstart) continue;

                    for (auto &arg : ssf->args.v) sf->coyieldsave.Add(arg);
                    for (auto &loc : ssf->locals.v) sf->coyieldsave.Add(Arg(loc.id, loc.id->type, false));
                    for (auto &dyn : ssf->dynscoperedefs.v) sf->coyieldsave.Add(dyn);
                }

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
            SubTypeT(ftype, type_function_null, fval, "function value", "function call");
            // If this magically succeeds, its because ftype was a variable, likely from a nil function value.
            // If so, we don't care, because this will never get executed at runtime?
            return type_any;
        }
    }

    TypeRef TypeCheckBranch(bool iftrue, const Node &condition, Node &fval, Node *args)
    {
        auto flowstart = CheckFlowTypeChanges(iftrue, condition);
        auto type = TypeCheckDynCall(fval, args);
        CleanUpFlow(flowstart);
        return type;
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

            default:
                if (iftrue && type->t == V_NILABLE) CheckFlowTypeIdOrDot(condition, type->Element());
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
                    if (condition.left()->type == condition.type)
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

            default:
                CheckFlowTypeChangesSub(iftrue, condition);
                break;
        }
        return start;
    }

    void AssignFlowPromote(Node &left, TypeRef right)
    {
        if ((left.exptype->t == V_ANY && right->t != V_ANY) ||
            (left.exptype->t == V_NILABLE && right->t != V_NILABLE))
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

        Node &n = *n_ptr;

        if (n.type != T_AND && n.type != T_OR)
        {
            TypeCheck(n_ptr, parent_type);
            auto type = n.exptype;
            if (type->t == V_NILABLE && only_true_type) return type->Element();
            return type;
        }

        auto tleft = TypeCheckAndOr(n.left(), n.type == T_OR, n.type);
        auto flowstart = CheckFlowTypeChanges(n.type == T_AND, *n.left());
        auto tright = TypeCheckAndOr(n.right(), only_true_type, n.type);
        CleanUpFlow(flowstart);

        n.exptype = only_true_type && n.type == T_AND
            ? tright
            : Union(tleft, tright, false);
        return n.exptype;
    }

    void CheckReturnValues(size_t nretvals, size_t i, const string &name, const Node &n)
    {
        if (nretvals <= i)
        {
            string nvals = inttoa((int)nretvals);
            parser.Error("function " + name + " returns " + nvals + " values, " + inttoa((int)i + 1) + " requested", &n);
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

            case T_FUN:
                type = n.sf()
                    ? &(parent_type == T_DYNINFO
                        ? n.sf()
                        : (n.sf() = PreSpecializeFunction(n.sf())))->thistype
                    : type_any;
                return;

            case T_LIST:
                // Flatten the TypeCheck recursion a bit
                for (Node *stats = &n; stats; stats = stats->b())
                    TypeCheck(stats->a(), T_LIST);
                return;

            case T_OR:
            case T_AND:
                TypeCheckAndOr(n_ptr, false, parent_type);
                return;
        }

        auto nc = n.NumChildren();
        if (nc > 0 && n.a()) TypeCheck(n.a(), n.type);
        if (nc > 1 && n.b()) TypeCheck(n.b(), n.type);
        if (nc > 2 && n.c()) TypeCheck(n.c(), n.type);

        switch (n.type)
        {
            case T_INT:   type = type_int; break;
            case T_FLOAT: type = type_float; break;
            case T_STR:   type = type_string; break;
            case T_NIL:   type = n.typenode()->t != V_ANY ? n.typenode() : NewTypeVar()->Wrap(NewType(), V_NILABLE); break;

            case T_PLUS:
            case T_MINUS:
            case T_MULT:
            case T_DIV:
            case T_MOD:
            {
                type = Union(n.left(), n.right(), true);
                bool unionchecked = false;
                MathError(type, n, n.type, unionchecked);
                if (!unionchecked) SubTypeLR(type, n);
                break;
            }

            case T_PLUSEQ:
            case T_MULTEQ:
            case T_MINUSEQ:
            case T_DIVEQ:
            case T_MODEQ:
            {
                type = n.left()->exptype;
                if (!MathCheckVector(type, n.left()->exptype, n.right()->exptype, false))
                {
                    bool unionchecked = false;
                    MathError(type, n, TType(n.type - T_PLUSEQ + T_PLUS), unionchecked);
                    SubType(n.right(), type, "right", n);
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
                        if (u->t != V_VECTOR && u->t != V_STRUCT && u->t != V_NILABLE)
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
                        else if (MathCheckVector(type, n.left()->exptype, n.right()->exptype, false))
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
                type = n.ident()->type;
                UseFlow(n);
                break;

            case T_DEF:
            case T_ASSIGNLIST:
            {
                auto dl = &n;
                vector<Node *> ids;
                for (; dl->type == T_DEF || dl->type == T_ASSIGNLIST; dl = dl->right())
                {
                    ids.push_back(dl->left());
                }
                size_t i = 0;
                for (auto id : ids)
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
                        id->exptype = type;
                        id->ident()->type = type;
                        Output(OUTPUT_DEBUG, "var: %s:%s", id->ident()->name.c_str(), TypeName(type).c_str());
                    }
                    else
                    {
                        AssignFlowDemote(*id, type, false);
                        SubTypeT(type, id->exptype, n, "right");
                    }
                    i++;
                }
                break;
            }

            case T_ASSIGN:
                if (n.left()->type != T_INDEX) AssignFlowDemote(*n.left(), n.right()->exptype, true);
                SubType(n.right(), n.left()->exptype, "right", n);
                if (n.left()->type != T_INDEX) AssignFlowPromote(*n.left(), n.right()->exptype);
                type = n.left()->exptype;
                break;

            case T_NATCALL:
            {
                auto nf = n.ncall_id()->nf();

                if (nf->first->overloads)
                {
                    // Multiple overloads available, figure out which we want to call.
                    auto cnf = nf->first;
                    nf = nullptr;
                    for (; cnf; cnf = cnf->overloads)
                    {
                        Node *list = n.ncall_args();
                        for (auto &arg : cnf->args.v)
                        {
                            if (!ConvertsTo(list->head()->exptype, arg.type, true)) goto nomatch;
                            list = list->tail();
                        }
                        // TODO: list possible alternatives, also in error below.
                        if (nf) NatCallError("arguments match more than one overload of ", cnf, n);
                        nf = cnf;
                        n.ncall_id()->nf() = nf;
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
                    if (argtype->t == V_NILABLE)
                    {
                        // This is somewhat of a hack, because we conflate V_NILABLE with being optional for 
                        // native functions, but we don't want numeric types to be nilable.
                        auto subt = argtype->Element();
                        if (subt->Numeric()) argtype = subt;
                    }
                    switch (arg.flags)
                    {
                        case NF_SUBARG1:
                            SubType(list->head(),
                                    nf->args.v[0].type->t == V_VECTOR && argtype->t != V_VECTOR
                                        ? VectorStructElement(argtypes[0])
                                        : argtypes[0],
                                    ArgName(i).c_str(),
                                    nf->name.c_str());
                            break;

                        case NF_ANYVAR:
                            if (argtype->t == V_VECTOR) argtype = NewTypeVar()->Wrap(NewType());
                            else if (argtype->t == V_ANY) argtype = NewTypeVar();
                            else assert(0);
                            break;

                        case NF_CORESUME:
                        {
                            // Specialized typechecking for resume()
                            assert(argtypes[0]->t == V_COROUTINE);  
                            auto sf = argtypes[0]->sf;
                            SubType(list->head(), sf->coresumetype, "resume value", *list->head());
                        }
                    }
                    SubType(list->head(), argtype, ArgName(i).c_str(), nf->name.c_str());
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
                        auto fake_function_def = (Node *)new FunRef(parser.lex, sf);
                        fake_function_def->linenumber = n.linenumber;
                        fake_function_def->fileidx = n.fileidx;
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
                    switch (ret.flags)
                    {
                        case NF_SUBARG1:
                            if (ret.type->t == V_NILABLE)
                            {
                                type = argtypes[0]->Wrap(NewType(), V_NILABLE);
                            }
                            else if (nf->args.v[0].type->t == V_VECTOR && ret.type->t != V_VECTOR)
                            {
                                type = VectorStructElement(argtypes[0]);
                            }
                            else if (nf->args.v[0].type->t == V_COROUTINE)
                            {
                                auto sf = argtypes[0]->sf;
                                assert(sf);
                                type = sf->returntypes[0];  // in theory it is possible this hasn't been generated yet..
                            }
                            else
                            {
                                type = argtypes[0];
                            }
                            break;
                        case NF_ANYVAR: 
                            type = ret.type->t == V_VECTOR ? NewTypeVar()->Wrap(NewType()) : NewTypeVar();
                            break;
                        default:
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
                        auto struc = st.GetVectorType(flen);
                        if (struc) type = StructTypeFromVector(type, struc);
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
                type = TypeCheckDynCall(*n.dcall_fval(),
                                        n.dcall_info()->dcall_args(),
                                        n.dcall_info()->dcall_function());
                break;

            case T_RETURN:
            {
                auto fid = n.return_function_idx()->integer();
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
                    if (n.return_value()->type == T_MULTIRET)
                    {
                        int i = 0;
                        for (auto mr = n.return_value(); mr; mr = mr->tailexps())
                        {
                            RetVal(mr->headexp(), sf, i++);
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
                    type = n.return_value()->exptype;
                }
                else
                {
                    type = type_any;
                    RetVal(n_ptr, sf, 0, &type);
                }
                break;
            }

            case T_IF:
            {
                if (n.if_branches()->right()->type != T_NIL)
                {
                    auto tleft = TypeCheckBranch(true, *n.if_condition(), *n.if_branches()->left(), nullptr);
                    auto tright = TypeCheckBranch(false, *n.if_condition(), *n.if_branches()->right(), nullptr);
                    // FIXME: we would want to allow coercions here, but we can't do so without changing
                    // these closure to a T_DYNCALL or inlining them
                    // bad, because currently even if(a) 1 else: 1.0 doesn't work.
                    type = Union(tleft, tright, false);
                    SubTypeT(tleft, type, *n.if_branches()->left(), "then branch", nullptr);
                    SubTypeT(tright, type, *n.if_branches()->right(), "else branch", nullptr);
                }
                else
                {
                    TypeCheckBranch(true, *n.if_condition(), *n.if_branches()->left(), nullptr);
                    // No else: this currently returns either the condition or the branch value.
                    type = type_any;
                }
                break;
            }

            case T_WHILE:
            {
                TypeCheckDynCall(*n.while_condition(), nullptr);
                TypeCheckBranch(true, *n.while_condition()->sf()->body->head(), *n.while_body(), nullptr);
                // Currently always return V_UNDEFINED
                type = type_any;
                break;
            }

            case T_FOR:
            {
                // We create temp arg nodes just for typechecking this:
                auto args = new Node(lex, T_LIST,
                                new AST(lex, T_FORLOOPVAR),
                                new Node(lex, T_LIST,
                                    new AST(lex, T_FORLOOPVAR),
                                    nullptr));
                auto itertype = n.for_iter()->exptype;
                if (itertype->t == V_INT || itertype->t == V_STRING) itertype = type_int;
                else if (itertype->t == V_VECTOR) itertype = itertype->Element();
                else if (itertype->t == V_STRUCT) itertype = itertype->struc->vectortype->Element();
                else TypeError("for can only iterate over int/string/vector/struct, not: " + TypeName(itertype), n);
                args->head()->exptype = itertype;
                args->tail()->head()->exptype = type_int;
                TypeCheckDynCall(*n.for_body(), args);
                delete args;
                // Currently always return V_UNDEFINED
                type = type_any;
                break;
            }

            case T_TYPE:
                type = n.typenode();
                break;

            case T_IS:
                // FIXME If the typecheck fails statically, we can replace this node with false
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
                    n.constructor_type()->typenode() = type;
                }
                int i = 0;
                for (auto list = n.constructor_args(); list; list = list->tail())
                {
                    TypeRef elemtype = type->t == V_STRUCT ? type->struc->fields[i].type : type->Element();
                    SubType(list->head(), elemtype, ArgName(i).c_str(), n);
                    i++;
                }
                break;
            }

            case T_DOT:
            case T_DOTMAYBE:
            {
                auto smtype = n.left()->exptype;
                auto stype = n.type == T_DOTMAYBE && smtype->t == V_NILABLE
                             ? smtype->Element()
                             : smtype;
                if (stype->t != V_STRUCT)
                    TypeError("struct/value", stype, n, "object");
                auto struc = stype->struc;
                auto sf = n.right()->fld();
                auto fieldidx = struc->Has(sf);
                if (fieldidx < 0) TypeError("type " + struc->name + " has no field named " + sf->name, n);
                auto &uf = struc->fields[fieldidx];
                type = n.type == T_DOTMAYBE && smtype->t == V_NILABLE && uf.type->t != V_NILABLE
                       ? uf.type->Wrap(NewType(), V_NILABLE)
                       : uf.type;
                n.right()->exptype = stype;  // Store struct type here for codegen.
                UseFlow(n);
                break;
            }

            case T_CODOT:
            {
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
                n.right()->exptype = type = uarg->type;
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
                        for (auto &field : struc.fields)
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

        // FIXME: expensive check, but helps track down generic type bugs.
        //assert(n.type == T_TYPE || type.UnWrapped()->t != V_STRUCT || !st.structtable[type.UnWrapped().idx]->generic);
    }
    
    void Stats()
    {
        int origsf = 0, multisf = 0, clonesf = 0;
        int orignodes = 0, clonenodes = 0;
        for (auto sf : st.subfunctiontable)
        {
            if (sf->parent->multimethod) { multisf++; orignodes += sf->body->Count(); }
            else if (!sf->next)          { origsf++;  orignodes += sf->body->Count(); }
            else                         { clonesf++; clonenodes += sf->body->Count(); }
        }
        Output(OUTPUT_DEBUG, "SF count: multi: %d, orig: %d, cloned: %d", multisf, origsf, clonesf);
        Output(OUTPUT_DEBUG, "Node count: orig: %d, cloned: %d", orignodes, clonenodes);
    }
};

}  // namespace lobster
