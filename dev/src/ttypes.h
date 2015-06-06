// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

namespace lobster
{

// Enum used both for token and node types

#define TTYPES_LIST \
    T(T_NONE, "invalid_token", TT_NONE, NO, a, NO, b) \
    \
    /* These are used both as tokens and node types */ \
    T(T_PLUS, "+", TT_BINARY, NO, a, NO, b) \
    T(T_MINUS, "-", TT_BINARY, NO, a, NO, b) \
    T(T_MULT, "*", TT_BINARY, NO, a, NO, b) \
    T(T_DIV, "/", TT_BINARY, NO, a, NO, b) \
    T(T_MOD, "%", TT_BINARY, NO, a, NO, b) \
    T(T_PLUSEQ, "+=", TT_BINARY, NO, a, NO, b) \
    T(T_MINUSEQ, "-=", TT_BINARY, NO, a, NO, b) \
    T(T_MULTEQ, "*=", TT_BINARY, NO, a, NO, b) \
    T(T_DIVEQ, "/=", TT_BINARY, NO, a, NO, b) \
    T(T_MODEQ, "%=", TT_BINARY, NO, a, NO, b) \
    T(T_AND, "&", TT_BINARY, NO, a, NO, b) \
    T(T_OR, "|", TT_BINARY, NO, a, NO, b) \
    T(T_NOT, "~", TT_UNARY, NO, a, NO, b) \
    T(T_INCR, "++", TT_UNARY, NO, a, NO, b) \
    T(T_DECR, "--", TT_UNARY, NO, a, NO, b) \
    T(T_EQ, "==", TT_BINARY, NO, a, NO, b) \
    T(T_NEQ, "!=", TT_BINARY, NO, a, NO, b) \
    T(T_LT, "<", TT_BINARY, NO, a, NO, b) \
    T(T_GT, ">", TT_BINARY, NO, a, NO, b) \
    T(T_LTEQ, "<=", TT_BINARY, NO, a, NO, b) \
    T(T_GTEQ, ">=", TT_BINARY, NO, a, NO, b) \
    T(T_ASSIGN, "=", TT_BINARY, NO, a, NO, b) \
    T(T_DYNASSIGN, "<-", TT_BINARY, NO, a, NO, b) \
    T(T_LOGASSIGN, "?=", TT_BINARY, NO, a, NO, b) \
    T(T_DEF, ":=", TT_TERNARY, NO, a, NO, b) \
    T(T_DOT, ".", TT_BINARY, NO, a, NO, b) \
    T(T_DOTMAYBE, "?.", TT_BINARY, NO, a, NO, b) \
    T(T_CODOT, "->", TT_BINARY, NO, a, NO, b) \
    T(T_INT, "integer literal", TT_NOCHILD, NO, a, NO, b) \
    T(T_FLOAT, "floating point literal", TT_NOCHILD, NO, a, NO, b) \
    T(T_STR, "string literal", TT_NOCHILD, NO, a, NO, b) \
    T(T_NIL, "nil", TT_NOCHILD, NO, a, NO, b) \
    T(T_DEFAULTVAL, "default value", TT_NOCHILD, NO, a, NO, b) \
    T(T_IDENT, "identifier", TT_NOCHILD, NO, a, NO, b) \
    T(T_STRUCT, "struct", TT_NOCHILD, NO, a, NO, b) \
    T(T_FUN, "def", TT_NOCHILD, NO, a, NO, b) \
    T(T_RETURN, "return", TT_NONE, YES, return_value, YES, return_function_idx) \
    T(T_IS, "is", TT_BINARY, NO, a, NO, b) \
    T(T_COROUTINE, "coroutine", TT_UNARY, NO, a, NO, b) \
    \
    /* These are used ONLY as node types: */ \
    T(T_TYPE, "type", TT_NOCHILD, NO, a, NO, b) \
    T(T_STRUCTDEF, "struct definition", TT_NONE, YES, struct_id, NO, b) \
    T(T_FIELD, "field", TT_NOCHILD, NO, a, NO, b) \
    T(T_COCLOSURE, "coroutine yield", TT_NOCHILD, NO, a, NO, b) \
    T(T_NATIVE, "native function", TT_NOCHILD, NO, a, NO, b) \
    T(T_NATCALL, "native call", TT_NONE, YES, ncall_id, YES, ncall_args) \
    T(T_CALL, "call", TT_NONE, YES, call_function, YES, call_args) \
    T(T_DYNCALL, "dynamic call", TT_NONE, YES, dcall_fval, YES, dcall_info) \
    T(T_DYNINFO, "dynamic info", TT_NONE, YES, dcall_function, YES, dcall_args) \
    T(T_LIST, "list", TT_NONE, YES, head, YES, tail) \
    T(T_ASSIGNLIST, "assign list", TT_BINARY, NO, a, NO, b) \
    T(T_MULTIRET, "multiple return", TT_NONE, YES, headexp, YES, tailexps) \
    T(T_SEQ, "statements", TT_BINARY, NO, a, NO, b) \
    T(T_INDEX, "indexing operation", TT_BINARY, NO, a, NO, b) \
    T(T_POSTINCR, "++", TT_UNARY, NO, a, NO, b) \
    T(T_POSTDECR, "--", TT_UNARY, NO, a, NO, b) \
    T(T_UMINUS, "-", TT_UNARY, NO, a, NO, b) \
    T(T_I2F, "tofloat", TT_UNARY, NO, a, NO, b) \
    T(T_A2S, "tostring", TT_UNARY, NO, a, NO, b) \
    T(T_CONSTRUCTOR, "constructor", TT_NONE, YES, constructor_args, YES, constructor_type) \
    T(T_IF, "if", TT_NONE, YES, if_condition, YES, if_branches) \
    T(T_BRANCHES, "if branch", TT_BINARY, NO, a, NO, b) \
    T(T_WHILE, "while", TT_NONE, YES, while_condition, YES, while_body) \
    T(T_FOR, "for", TT_NONE, YES, for_iter, YES, for_body) \
    T(T_FORLOOPVAR, "for loop variable", TT_NOCHILD, NO, a, NO, b) \
    \
    /* These are used ONLY as tokens: */ \
    T(T_LINEFEED, "linefeed", TT_NONE, NO, a, NO, b) \
    T(T_ENDOFINCLUDE, "end of include", TT_NONE, NO, a, NO, b) \
    T(T_ENDOFFILE, "end of file", TT_NONE, NO, a, NO, b) \
    T(T_INDENT, "indentation", TT_NONE, NO, a, NO, b) \
    T(T_DEDENT, "de-indentation", TT_NONE, NO, a, NO, b) \
    T(T_LEFTPAREN, "(", TT_NONE, NO, a, NO, b) \
    T(T_RIGHTPAREN, ")", TT_NONE, NO, a, NO, b) \
    T(T_LEFTBRACKET, "[", TT_NONE, NO, a, NO, b) \
    T(T_RIGHTBRACKET, "]", TT_NONE, NO, a, NO, b) \
    T(T_LEFTCURLY, "{", TT_NONE, NO, a, NO, b) \
    T(T_RIGHTCURLY, "}", TT_NONE, NO, a, NO, b) \
    T(T_SEMICOLON, ";", TT_NONE, NO, a, NO, b) \
    T(T_AT, "@", TT_NONE, NO, a, NO, b) \
    T(T_QUESTIONMARK, "?", TT_NONE, NO, a, NO, b) \
    T(T_COMMA, ",", TT_NONE, NO, a, NO, b) \
    T(T_COLON, ":", TT_NONE, NO, a, NO, b) \
    T(T_DEFCONST, ":==", TT_NONE, NO, a, NO, b) \
    T(T_DEFTYPEIN, "::=", TT_NONE, NO, a, NO, b) \
    T(T_TYPEIN, "::", TT_NONE, NO, a, NO, b) \
    T(T_VALUE, "value", TT_NONE, NO, a, NO, b) \
    T(T_INCLUDE, "include", TT_NONE, NO, a, NO, b) \
    T(T_INTTYPE, "int", TT_NONE, NO, a, NO, b) \
    T(T_FLOATTYPE, "float", TT_NONE, NO, a, NO, b) \
    T(T_STRTYPE, "string", TT_NONE, NO, a, NO, b) \
    T(T_VECTTYPE, "vector", TT_NONE, NO, a, NO, b) \
    T(T_FROM, "from", TT_NONE, NO, a, NO, b) \
    T(T_PROGRAM, "program", TT_NONE, NO, a, NO, b) \
    T(T_PRIVATE, "private", TT_NONE, NO, a, NO, b) \
    T(T_ENUM, "enum", TT_NONE, NO, a, NO, b) \

enum TType
{
    #define T(ENUM, STR, CAT, HASLEFT, LEFT, HASRIGHT, RIGHT) ENUM,
    TTYPES_LIST
    #undef T
};

inline const char *TName(TType t)
{
    static const char *names[] =
    {
        #define T(ENUM, STR, CAT, HASLEFT, LEFT, HASRIGHT, RIGHT) STR,
        TTYPES_LIST
        #undef T
    };
    return names[t];
}

enum TTypeCategory { TT_NONE, TT_NOCHILD, TT_UNARY, TT_BINARY, TT_TERNARY };

inline TTypeCategory TCat(TType t)
{
    static TTypeCategory cats[] =
    {
        #define T(ENUM, STR, CAT, HASLEFT, LEFT, HASRIGHT, RIGHT) CAT,
        TTYPES_LIST
        #undef T
    };
    return cats[t];
}

}  // namespace lobster
