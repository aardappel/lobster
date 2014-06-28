
struct TypeChecker
{
    Lex &lex;
    Parser &parser;
    SymbolTable &st;

    TypeChecker(Parser &_p, SymbolTable &_st) : lex(_p.lex), parser(_p), st(_st)
    {
        parser.root->exptype = TypeCheck(parser.root);
    }

    Type Union(Node *a, Node *b)
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

    Type TypeCheck(Node *n)
    {
        if (n->HasChildren())
        {
            if (n->a) n->a->exptype = TypeCheck(n->a);
            if (n->b) n->b->exptype = TypeCheck(n->b);
        }

        switch (n->type)
        {
            case 'INT': return Type(V_INT);
            case 'FLT': return Type(V_FLOAT);
            case 'STR': return Type(V_STRING);
            case 'NIL': return Type(V_NIL);

            case '/':
            case '*':
            case '-':
            case '+':
                return Union(n->a, n->b);

            case 'ID': 


            case '.':

            case 'IDX':

            case '.@':

            case ':=':
            case ',=':

            case '=': 

            case '+=':
            case '*=':
            case '-=':
            case '/=':
            case '%=':

            case '---': 
            case '+++': 
            case '--':  
            case '++':  

            case '!=': 
            case '==': 
            case '>=': 
            case '<=': 
            case '>':  
            case '<':  
            case '%':  

            case '{}': 

            case 'ADT':

            case 'NATC':
            case 'CALL':
            case '->':

            case 'FUN':

            case ':':

            case ',':

            case 'BUT':

            case 'MR':

            case '&':

            case '|':

            case '!':

            case '[]':

            case 'IS':

            case 'RET':

            case 'COCL':

            case 'CORO':

                return n->exptype;
                break;

            default:
                //assert(0);
                return Type();
        }
    }
};
