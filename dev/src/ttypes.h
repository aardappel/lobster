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
    T0(T_NONE, "invalid_token", 0) \
    \
    /* These are used both as tokens and node types */ \
    T0(T_PLUS, "+", 2) \
    T0(T_MINUS, "-", 2) \
    T0(T_MULT, "*", 2) \
    T0(T_DIV, "/", 2) \
    T0(T_MOD, "%", 2) \
    T0(T_PLUSEQ, "+=", 2) \
    T0(T_MINUSEQ, "-=", 2) \
    T0(T_MULTEQ, "*=", 2) \
    T0(T_DIVEQ, "/=", 2) \
    T0(T_MODEQ, "%=", 2) \
    T0(T_AND, "&", 2) \
    T0(T_OR, "|", 2) \
    T0(T_NOT, "~", 1) \
    T0(T_INCR, "++", 1) \
    T0(T_DECR, "--", 1) \
    T0(T_EQ, "==", 2) \
    T0(T_NEQ, "!=", 2) \
    T0(T_LT, "<", 2) \
    T0(T_GT, ">", 2) \
    T0(T_LTEQ, "<=", 2) \
    T0(T_GTEQ, ">=", 2) \
    T0(T_ASSIGN, "=", 2) \
    T0(T_DYNASSIGN, "<-", 2) \
    T0(T_LOGASSIGN, "?=", 2) \
    T0(T_DEF, ":=", 3) \
    T0(T_DOT, ".", 2) \
    T0(T_DOTMAYBE, "?.", 2) \
    T0(T_CODOT, "->", 2) \
    T0(T_INT, "integer literal", 0) \
    T0(T_FLOAT, "floating point literal", 0) \
    T0(T_STR, "string literal", 0) \
    T0(T_NIL, "nil", 0) \
    T0(T_DEFAULTVAL, "default value", 0) \
    T0(T_IDENT, "identifier", 0) \
    T0(T_STRUCT, "struct", 0) \
    T0(T_FUN, "def", 0) \
    T2(T_RETURN, "return", 2, return_value, return_function_idx) \
    T0(T_IS, "is", 2) \
    T0(T_TYPEOF, "typeof", 1) \
    T0(T_COROUTINE, "coroutine", 1) \
    \
    /* These are used ONLY as node types: */ \
    T0(T_TYPE, "type", 0) \
    T1(T_STRUCTDEF, "struct definition", 1, struct_id) \
    T0(T_FIELD, "field", 0) \
    T0(T_COCLOSURE, "coroutine yield", 0) \
    T0(T_NATIVE, "native function", 0) \
    T2(T_NATCALL, "native call", 2, ncall_id, ncall_args) \
    T2(T_CALL, "call", 2, call_function, call_args) \
    T3(T_DYNCALL, "dynamic call", 3, dcall_fval, dcall_function, dcall_args) \
    T2(T_LIST, "list", 2, head, tail) \
    T0(T_ASSIGNLIST, "assign list", 2) \
    T2(T_MULTIRET, "multiple return", 2, headexp, tailexps) \
    T0(T_SEQ, "statements", 2) \
    T0(T_INDEX, "indexing operation", 2) \
    T0(T_POSTINCR, "++", 1) \
    T0(T_POSTDECR, "--", 1) \
    T0(T_UMINUS, "-", 1) \
    T0(T_I2F, "tofloat", 1) \
    T0(T_A2S, "tostring", 1) \
    T0(T_E2A, "toany", 1) \
    T0(T_E2N, "tonil", 1) \
    T0(T_E2B, "tobool", 1) \
    T0(T_T2I, "toint", 1) \
    T2(T_CONSTRUCTOR, "constructor", 2, constructor_args, constructor_type) \
    T3(T_IF, "if", 3, if_condition, if_then, if_else) \
    T2(T_WHILE, "while", 2, while_condition, while_body) \
    T2(T_FOR, "for", 2, for_iter, for_body) \
    T0(T_FORLOOPVAR, "for loop variable", 0) \
    T0(T_EMPTY, "empty", 0) \
    \
    /* These are used ONLY as tokens: */ \
    T0(T_LINEFEED, "linefeed", 0) \
    T0(T_ENDOFINCLUDE, "end of include", 0) \
    T0(T_ENDOFFILE, "end of file", 0) \
    T0(T_INDENT, "indentation", 0) \
    T0(T_DEDENT, "de-indentation", 0) \
    T0(T_LEFTPAREN, "(", 0) \
    T0(T_RIGHTPAREN, ")", 0) \
    T0(T_LEFTBRACKET, "[", 0) \
    T0(T_RIGHTBRACKET, "]", 0) \
    T0(T_LEFTCURLY, "{", 0) \
    T0(T_RIGHTCURLY, "}", 0) \
    T0(T_SEMICOLON, ";", 0) \
    T0(T_AT, "@", 0) \
    T0(T_QUESTIONMARK, "?", 0) \
    T0(T_COMMA, ",", 0) \
    T0(T_COLON, ":", 0) \
    T0(T_DEFCONST, ":==", 0) \
    T0(T_DEFTYPEIN, "::=", 0) \
    T0(T_TYPEIN, "::", 0) \
    T0(T_VALUE, "value", 0) \
    T0(T_INCLUDE, "include", 0) \
    T0(T_INTTYPE, "int", 0) \
    T0(T_FLOATTYPE, "float", 0) \
    T0(T_STRTYPE, "string", 0) \
    T0(T_VECTTYPE, "vector", 0) \
    T0(T_FROM, "from", 0) \
    T0(T_PROGRAM, "program", 0) \
    T0(T_PRIVATE, "private", 0) \
    T0(T_ENUM, "enum", 0) \
    T0(T_CALL_NOPARENS, "callnp", 0)

enum TType
{
    #define T0(ENUM, STR, CAT) ENUM,
    #define T1(ENUM, STR, CAT, ONE) ENUM,
    #define T2(ENUM, STR, CAT, ONE, TWO) ENUM,
    #define T3(ENUM, STR, CAT, ONE, TWO, THREE) ENUM,
        TTYPES_LIST
    #undef T0
    #undef T1
    #undef T2
    #undef T3
};

inline const char *TName(TType t)
{
    static const char *names[] =
    {
        #define T0(ENUM, STR, CAT) STR,
        #define T1(ENUM, STR, CAT, ONE) STR,
        #define T2(ENUM, STR, CAT, ONE, TWO) STR,
        #define T3(ENUM, STR, CAT, ONE, TWO, THREE) STR,
            TTYPES_LIST
        #undef T0
        #undef T1
        #undef T2
        #undef T3
    };
    return names[t];
}

inline char TArity(TType t)
{
    static char cats[] =
    {
        #define T0(ENUM, STR, CAT) CAT,
        #define T1(ENUM, STR, CAT, ONE) CAT,
        #define T2(ENUM, STR, CAT, ONE, TWO) CAT,
        #define T3(ENUM, STR, CAT, ONE, TWO, THREE) CAT,
            TTYPES_LIST
        #undef T0
        #undef T1
        #undef T2
        #undef T3
    };
    return cats[t];
}

}  // namespace lobster
