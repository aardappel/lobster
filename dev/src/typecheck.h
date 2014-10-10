
struct TypeChecker
{
    Lex &lex;
    Parser &parser;
    SymbolTable &st;

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st)
    {
        parser.root->exptype = TypeCheck(parser.root);
    }

    Type Union(const Node *a, const Node *b)
    {
        if (a->exptype.t == b->exptype.t)
        {
            switch (a->exptype.t)
            {
                case V_INT:
                case V_FLOAT:
                case V_STRING:
                    return a->exptype;
            }
        }
        return Type(V_UNKNOWN);
    }

    Type SubType(const Node *a, const Type &sub, const char *context)
    {
        const Type &type = a->exptype;
        if (type.t == sub.t) return type;
        switch (sub.t)
        {
            case V_UNKNOWN: return type;
            //case V_INT:
        }
        parser.Error("\"" + string(context) + "\" requires type: " + sub.Name() + ", got: " + type.Name(), a);
        return Type();
    }

    Type TypeCheck(Node *n)
    {
        if (n->HasChildren())
        {
            if (n->a) n->a->exptype = TypeCheck(n->a);
            if (n->b) n->b->exptype = TypeCheck(n->b);
        }

        switch (n->type)
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
                return Union(n->a, n->b);

            case T_NATCALL:
            {
                auto nf = n->a->nf;
                int i = 0;
                for (Node *list = n->b; list; list = list->b)
                {
                    list->a->exptype = SubType(list->a, nf->args[i++].type, nf->name.c_str());
                }
                return nf->nretvalues ? nf->retvals[0].type : Type();  // FIXME: multiple retvals
            }

            case T_IDENT:
                return n->ident->type;

            case T_DEF:
                return n->a->ident->type = n->b->exptype;

            case T_CALL:
            {
                auto &f = *n->a->f;
                return Type();

            }


            case T_DOT:

            case T_CO_AT:

            case T_DEFLIST:

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

            case T_NEQ: 
            case T_EQ: 
            case T_GTEQ: 
            case T_LTEQ: 
            case T_GT:  
            case T_LT:  

            case T_CLOSURE: 

            case T_STRUCTDEF:

            case T_DYNCALL:

            case T_FUNDEF:
            case T_FUN:

            case T_NATIVE:

            case T_LIST:
            case T_ARGLIST:

            case T_SEQ:

            case T_MULTIRET:

            case T_AND:

            case T_OR:

            case T_NOT:

            case T_CONSTRUCTOR:

            case T_IS:

            case T_RETURN:

            case T_COCLOSURE:

            case T_COROUTINE:
                return n->exptype;

            default:
                assert(0);
                return n->exptype;
        }
    }
};
