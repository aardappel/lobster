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

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "unicode.h"

using namespace lobster;

static RandomNumberGenerator<MersenneTwister> rnd;

static int IntCompare(const Value &a, const Value &b)
{
    return a.ival() < b.ival() ? -1 : a.ival() > b.ival();
}

static int FloatCompare(const Value &a, const Value &b)
{
    return a.fval() < b.fval() ? -1 : a.fval() > b.fval();
}

static int StringCompare(const Value &a, const Value &b)
{
    return strcmp(a.sval()->str(), b.sval()->str());
}

template<typename T> Value BinarySearch(Value &l, Value &key, T comparefun)
{
    ValueRef lref(l), kref(key);

    int size = l.vval()->len;
    int i = 0;

    for (;;)
    {
        if (!size) break;

        int mid = size / 2;
        int comp = comparefun(key, l.vval()->at(i + mid));

        if (comp)
        {
            if (comp < 0) size = mid;
            else { mid++; i += mid; size -= mid; }
        }
        else
        {
            i += mid;
            size = 1;
            while (i                        && !comparefun(key, l.vval()->at(i - 1   ))) { i--; size++; }
            while (i + size < l.vval()->len && !comparefun(key, l.vval()->at(i + size))) {      size++; }
            break;
        }
    }

    g_vm->Push(Value(size));
    return Value(i);
}

type_elem_t GetVectType(type_elem_t available, type_elem_t existing)
{
    auto &ti = g_vm->GetTypeInfo(existing);
    // FIXME: this is not entirely correct, it could replace a color by an xyzw_i
    return ti.t != V_STRUCT || available < 0 ? existing : available;
}

void AddBuiltins()
{
    STARTDECL(print) (Value &a)
    {
        Output(OUTPUT_PROGRAM, a.ref()->ToString(g_vm->programprintprefs).c_str());
        return a;
    }
    ENDDECL1(print, "x", "A", "A1",
        "output any value to the console (with linefeed). returns its argument.");

    STARTDECL(string) (Value &a)
    {
        auto str = g_vm->NewString(a.ref()->ToString(g_vm->programprintprefs));
        a.DECRT();
        return str;
    }
    ENDDECL1(string, "x", "A", "S",
        "convert any value to string");

    STARTDECL(set_print_depth) (Value &a) { g_vm->programprintprefs.depth = a.ival(); return a; } 
    ENDDECL1(set_print_depth, "A?", "I", "", 
        "for printing / string conversion: sets max vectors/objects recursion depth (default 10)");

    STARTDECL(set_print_length) (Value &a) { g_vm->programprintprefs.budget = a.ival(); return a; } 
    ENDDECL1(set_print_length, "A?", "I", "", 
        "for printing / string conversion: sets max string length (default 10000)");

    STARTDECL(set_print_quoted) (Value &a) { g_vm->programprintprefs.quoted = a.ival() != 0; return a; } 
    ENDDECL1(set_print_quoted, "A?", "I", "", 
        "for printing / string conversion: if the top level value is a string, whether to convert it with escape codes"
        " and quotes (default false)");

    STARTDECL(set_print_decimals) (Value &a) { g_vm->programprintprefs.decimals = a.ival(); return a; } 
    ENDDECL1(set_print_decimals, "A?", "I", "", 
        "for printing / string conversion: number of decimals for any floating point output (default -1, meaning all)");

    STARTDECL(getline) ()
    {
        const int MAXSIZE = 1000;
        char buf[MAXSIZE];
        if (!fgets(buf, MAXSIZE, stdin)) buf[0] = 0;
        buf[MAXSIZE - 1] = 0;
        for (int i = 0; i < MAXSIZE; i++) if (buf[i] == '\n') { buf[i] = 0; break; }
        return Value(g_vm->NewString(buf, strlen(buf)));
    }
    ENDDECL0(getline, "", "", "S",
        "reads a string from the console if possible (followed by enter)");

    STARTDECL(if) (Value &c, Value &t, Value &e)
    {
        assert(0);  // Special case implementation in the VM
        (void)c;
        (void)t;
        (void)e;
        return Value();
    }
    ENDDECL3(if, "cond,then,else", "ACC?", "A",
        "evaluates then or else depending on cond, else is optional");

    STARTDECL(while) (Value &c, Value &b)
    {
        assert(0);  // Special case implementation in the VM
        (void)c;
        (void)b;
        return Value();
    }
    ENDDECL2(while, "cond,do", "C@C", "A",
        "evaluates body while cond (converted to a function) holds true, returns last body value");

    STARTDECL(for) (Value &iter, Value &body)
    {
        assert(0);  // Special case implementation in the VM
        (void)iter;
        (void)body;
        return Value();
    }
    ENDDECL2(for, "iter,do", "AC", "",
        "iterates over int/vector/string, body may take [ element [ , index ] ] arguments");

    STARTDECL(append) (Value &v1, Value &v2)
    {
        assert(v1.vval()->typeoff == v2.vval()->typeoff);  // FIXME: need to guarantee this in typechecking
        auto nv = g_vm->NewVector(v1.vval()->len + v2.vval()->len, v1.vval()->typeoff);
        nv->Append(v1.vval(), 0, v1.vval()->len); v1.DECRT();
        nv->Append(v2.vval(), 0, v2.vval()->len); v2.DECRT();
        return Value(nv);
    }
    ENDDECL2(append, "xs,ys", "V*V*1", "V1",
        "creates a new vector by appending all elements of 2 input vectors");

    STARTDECL(vector_reserve) (Value &type, Value &len)
    {
        return Value(g_vm->NewVector(len.ival(), (type_elem_t)type.ival()));
    }
    ENDDECL2(vector_reserve, "typeid,len", "TI", "V*",
        "creates a new empty vector much like [] would, except now ensures"
        " it will have space for len push() operations without having to reallocate."
        " pass \"typeof return\" as typeid.");

    STARTDECL(length) (Value &a)
    {
        return a;
    }
    ENDDECL1(length, "x", "I", "I",
        "length of int (identity function, useful in combination with string/vector version)");

    STARTDECL(length) (Value &a)
    {
        auto len = a.lobj()->len;
        a.DECRT();
        return Value(len);
    }
    ENDDECL1(length, "s", "S", "I",
        "length of string");

    STARTDECL(length) (Value &a)
    {
        auto len = a.lobj()->len;
        a.DECRT();
        return Value(len);
    }
    ENDDECL1(length, "xs", "V*", "I",
        "length of vector");

    STARTDECL(equal) (Value &a, Value &b)
    {
        bool eq = a.refnil()->Equal(b.refnil(), true);
        a.DECRTNIL();
        b.DECRTNIL();
        return Value(eq);
    }
    ENDDECL2(equal, "a,b", "AA", "I",
        "structural equality between any two values (recurses into vectors/objects,"
        " unlike == which is only true for vectors/objects if they are the same object)");

    STARTDECL(push) (Value &l, Value &x)
    {
        l.vval()->Push(x);
        return l;
    }
    ENDDECL2(push, "xs,x", "V*A1", "V1",
        "appends one element to a vector, returns existing vector");

    STARTDECL(pop) (Value &l)
    {
        if (!l.vval()->len) { l.DECRT(); g_vm->BuiltinError("pop: empty vector"); }
        auto v = l.vval()->Pop();
        l.DECRT();
        return v;
    }
    ENDDECL1(pop, "xs", "V*", "A1",
        "removes last element from vector and returns it");

    STARTDECL(top) (Value &l)
    {
        if (!l.vval()->len) { l.DECRT(); g_vm->BuiltinError("top: empty vector"); }
        auto v = l.vval()->Top();
        l.DECRT();
        return v;
    }
    ENDDECL1(top, "xs", "V*", "A1",
        "returns last element from vector");

    STARTDECL(replace) (Value &l, Value &i, Value &a)
    {
        if (i.ival() < 0 || i.ival() >= l.vval()->len) g_vm->BuiltinError("replace: index out of range");

        auto nv = g_vm->NewVector(l.vval()->len, l.vval()->typeoff);
        nv->Append(l.vval(), 0, l.vval()->len);
        l.DECRT();

        Value &dest = nv->at(i.ival());
        dest.DEC();
        dest = a;

        return Value(nv);
    }
    ENDDECL3(replace, "xs,i,x", "V*IA1", "V1",
        "returns a copy of a vector with the element at i replaced by x");

    STARTDECL(insert) (Value &l, Value &i, Value &a, Value &n)
    {
        if (n.ival() < 0 || i.ival() < 0 || i.ival() > l.vval()->len)
            g_vm->BuiltinError("insert: index or n out of range");  // note: i==len is legal
        l.vval()->Insert(a, i.ival(), max(n.ival(), 1));
        return l;
    }
    ENDDECL4(insert, "xs,i,x,n", "V*IA1I?", "V1",
        "inserts n copies (default 1) of x into a vector at index i, existing elements shift upward,"
        " returns original vector");

    STARTDECL(remove) (Value &l, Value &i, Value &n)
    {
        int amount = max(n.ival(), 1);
        if (n.ival() < 0 || amount > l.vval()->len || i.ival() < 0 || i.ival() > l.vval()->len - amount)
            g_vm->BuiltinError("remove: index (" + to_string(i.ival()) + 
                               ") or n (" + to_string(amount) +
                               ") out of range (" + to_string(l.vval()->len) + ")");
        auto v = l.vval()->Remove(i.ival(), amount);
        l.DECRT();
        return v;
    }
    ENDDECL3(remove, "xs,i,n", "V*II?", "A1",
        "remove element(s) at index i, following elements shift down. pass the number of elements to remove"
        " as an optional argument, default 1. returns the first element removed.");

    STARTDECL(removeobj) (Value &l, Value &o)
    {
        int removed = 0;
        for (int i = 0; i < l.vval()->len; i++)
        {
            auto e = l.vval()->at(i);
            if (e.Equal(e.type, o, o.type, false))
            {
                l.vval()->Remove(i--, 1).DEC();
                removed++;
            }
        }
        o.DEC();
        l.DECRT();
        return Value(removed);
    }
    ENDDECL2(removeobj, "xs,obj", "V*A1", "I",
        "remove all elements equal to obj (==), returns amount of elements removed.");

    STARTDECL(binarysearch) (Value &l, Value &key)
    {
        return BinarySearch(l, key, IntCompare);
    }
    ENDDECL2(binarysearch, "xs,key", "I]I", "II",
        "does a binary search for key in a sorted vector, returns as first return value how many matches were found,"
        " and as second the index in the array where the matches start (so you can read them, overwrite them,"
        " or remove them), or if none found, where the key could be inserted such that the vector stays sorted."
        " This overload is for int vectors and keys.");

    STARTDECL(binarysearch) (Value &l, Value &key)
    {
        return BinarySearch(l, key, FloatCompare);
    }
    ENDDECL2(binarysearch, "xs,key", "F]F", "II",
        "float version.");

    STARTDECL(binarysearch) (Value &l, Value &key)
    {
        return BinarySearch(l, key, StringCompare);
    }
    ENDDECL2(binarysearch, "xs,key", "S]S", "II",
        "string version.");

    STARTDECL(copy) (Value &v)
    {
        auto nv = g_vm->NewVector(v.vval()->len, v.vval()->typeoff);
        nv->Append(v.vval(), 0, v.vval()->len);
        v.DECRT();
        return Value(nv);
    }
    ENDDECL1(copy, "xs", "V*", "V1",
        "makes a shallow copy of vector/object.");

    STARTDECL(slice) (Value &l, Value &s, Value &e)
    {
        int size = e.ival();
        if (size < 0) size = l.vval()->len + size;
        int start = s.ival();
        if (start < 0) start = l.vval()->len + start;
        if (start < 0 || start + size > (int)l.vval()->len)
            g_vm->BuiltinError("slice: values out of range");
        auto nv = g_vm->NewVector(size, l.vval()->typeoff);
        nv->Append(l.vval(), start, size);
        l.DECRT();
        return Value(nv);
    }
    ENDDECL3(slice,
        "xs,start,size", "V*II", "V1", "returns a sub-vector of size elements from index start."
        " start & size can be negative to indicate an offset from the vector length.");

    STARTDECL(any) (Value &v)
    {
        Value r(false);
        for (int i = 0; i < v.vval()->len; i++)
        {
            if (v.vval()->at(i).True())
            {
                r = Value(true);
                break;
            }
        }
        v.DECRT();
        return r;
    }
    ENDDECL1(any, "xs", "V*", "I",
        "returns wether any elements of the vector are true values");

    STARTDECL(all) (Value &v)
    {
        Value r(true);
        for (int i = 0; i < v.vval()->len; i++)
        {
            if (!v.vval()->at(i).True())
            {
                r = Value(false);
                break;
            }
        }
        v.DECRT();
        return r;
    }
    ENDDECL1(all, "xs", "V*", "I",
        "returns wether all elements of the vector are true values");

    STARTDECL(substring) (Value &l, Value &s, Value &e)
    {
        int size = e.ival();
        if (size < 0) size = l.sval()->len + size;
        int start = s.ival();
        if (start < 0) start = l.sval()->len + start;
        if (start < 0 || start + size > (int)l.sval()->len)
            g_vm->BuiltinError("substring: values out of range");

        auto ns = g_vm->NewString(l.sval()->str() + start, size);
        l.DECRT();
        return Value(ns);
    }
    ENDDECL3(substring, "s,start,size", "SII", "S", 
        "returns a substring of size characters from index start."
        " start & size can be negative to indicate an offset from the string length.");

    STARTDECL(string2int) (Value &s)
    {
        auto i = atoi(s.sval()->str());
        s.DECRT();
        return Value(i);
    }
    ENDDECL1(string2int, "s", "S", "I",
        "converts a string to an int. returns 0 if no numeric data could be parsed");

    STARTDECL(string2float) (Value &s)
    {
        auto f = (float)atof(s.sval()->str());
        s.DECRT();
        return Value(f);
    }
    ENDDECL1(string2float, "s", "S", "F",
        "converts a string to a float. returns 0.0 if no numeric data could be parsed");

    STARTDECL(tokenize) (Value &s, Value &delims, Value &whitespace)
    {
        auto v = g_vm->NewVector(0, TYPE_ELEM_VECTOR_OF_STRING);
        auto ws = whitespace.sval()->str();
        auto dl = delims.sval()->str();
        auto p = s.sval()->str();
        p += strspn(p, ws);
        auto strspn1 = [](char c, const char *set) { while (*set) if (*set == c) return 1; return 0; };
        while (*p)
        {
            auto delim = p + strcspn(p, dl);
            auto end = delim;
            while (end > p && strspn1(end[-1], ws)) end--;
            v->Push(g_vm->NewString(p, end - p));
            p = delim + strspn(delim, dl);
            p += strspn(p, ws);
        }
        s.DECRT();
        delims.DECRT();
        whitespace.DECRT();
        return Value(v);
    }
    ENDDECL3(tokenize, "s,delimiters,whitespace", "SSS", "S]",
        "splits a string into a vector of strings, by splitting into segments upon each dividing or terminating"
        " delimiter. Segments are stripped of leading and trailing whitespace."
        " Example: \"; A ; B C; \" becomes [ \"\", \"A\", \"B C\" ] with \";\" as delimiter and \" \" as whitespace." );

    STARTDECL(unicode2string) (Value &v)
    {
        char buf[7];
        string s;
        for (int i = 0; i < v.vval()->len; i++)
        {
            auto &c = v.vval()->at(i);
            TYPE_ASSERT(c.type == V_INT);
            ToUTF8(c.ival(), buf);
            s += buf;
        }
        v.DECRT();
        return Value(g_vm->NewString(s));
    }
    ENDDECL1(unicode2string, "us", "I]", "S",
        "converts a vector of ints representing unicode values to a UTF-8 string.");

    STARTDECL(string2unicode) (Value &s)
    {
        auto v = g_vm->NewVector(s.sval()->len, TYPE_ELEM_VECTOR_OF_INT);
        const char *p = s.sval()->str();
        while (*p)
        {
            int u = FromUTF8(p);
            if (u < 0) { s.DECRT(); Value(v).DECRT(); return Value(0, V_NIL); }
            v->Push(u);
        }
        s.DECRT();
        return Value(v);
    }
    ENDDECL1(string2unicode, "s", "S", "I]?",
        "converts a UTF-8 string into a vector of unicode values, or nil upon a decoding error");

    STARTDECL(number2string) (Value &n, Value &b, Value &mc)
    {
        if (b.ival() < 2 || b.ival() > 36 || mc.ival() > 32)
            g_vm->BuiltinError("number2string: values out of range");

        uint i = (uint)n.ival();
        string s;
        const char *from = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        while (i || (int)s.length() < mc.ival())
        {
            s.insert(0, 1, from[i % b.ival()]);
            i /= b.ival();
        }

        return Value(g_vm->NewString(s));
    }
    ENDDECL3(number2string, "number,base,minchars", "III", "S",
        "converts the (unsigned version) of the input integer number to a string given the base (2..36, e.g. 16 for"
        " hex) and outputting a minimum of characters (padding with 0).");

    STARTDECL(lowercase) (Value &s)
    {
        auto ns = g_vm->NewString(s.sval()->str(), s.sval()->len);
        for (auto p = ns->str(); *p; p++)
        {
            // This is unicode-safe, since all unicode chars are in bytes >= 128
            if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
        }
        s.DECRT();
        return Value(ns);
    }
    ENDDECL1(lowercase, "s", "S", "S",
             "converts a UTF-8 string from any case to lower case, affecting only A-Z");

    STARTDECL(uppercase) (Value &s)
    {
        auto ns = g_vm->NewString(s.sval()->str(), s.sval()->len);
        for (auto p = ns->str(); *p; p++)
        {
            // This is unicode-safe, since all unicode chars are in bytes >= 128
            if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
        }
        s.DECRT();
        return Value(ns);
    }
    ENDDECL1(uppercase, "s", "S", "S",
             "converts a UTF-8 string from any case to upper case, affecting only a-z");

    STARTDECL(pow) (Value &a, Value &b) { return Value(powf(a.fval(), b.fval())); } ENDDECL2(pow, "a,b", "FF", "F",
        "a raised to the power of b");

    STARTDECL(log) (Value &a) { return Value(logf(a.fval())); } ENDDECL1(log, "a", "F", "F", 
        "natural logaritm of a");

    STARTDECL(sqrt) (Value &a) { return Value(sqrtf(a.fval())); } ENDDECL1(sqrt, "f", "F", "F", 
        "square root");

    STARTDECL(and) (Value &a, Value &b) { return Value(a.ival() & b.ival());  } ENDDECL2(and, "a,b", "II", "I",
        "bitwise and");
    STARTDECL(or)  (Value &a, Value &b) { return Value(a.ival() | b.ival());  } ENDDECL2(or,  "a,b", "II", "I", 
        "bitwise or");
    STARTDECL(xor) (Value &a, Value &b) { return Value(a.ival() ^ b.ival());  } ENDDECL2(xor, "a,b", "II", "I",
        "bitwise exclusive or");
    STARTDECL(not) (Value &a)           { return Value(~a.ival());            } ENDDECL1(not, "a",   "I",  "I",
        "bitwise negation");
    STARTDECL(shl) (Value &a, Value &b) { return Value(a.ival() << b.ival()); } ENDDECL2(shl, "a,b", "II", "I", 
        "bitwise shift left");
    STARTDECL(shr) (Value &a, Value &b) { return Value(a.ival() >> b.ival()); } ENDDECL2(shr, "a,b", "II", "I", 
        "bitwise shift right");

    #define GETVECTYPE(accessor) GetVectType(len <= 4 ? g_vm->accessor(len) : (type_elem_t)-1, a.vval()->typeoff)
        
    #define VECTOROPT(op, typeoff) \
        TYPE_ASSERT(IsVector(a.type)); \
        auto len = a.vval()->len; \
        auto v = g_vm->NewVector(len, typeoff); \
        for (int i = 0; i < a.vval()->len; i++) { \
            auto f = a.vval()->at(i); \
            v->Push(Value(op)); \
        } \
        a.DECRT(); \
        return Value(v);
    #define VECTOROP(op) VECTOROPT(op, a.vval()->typeoff)

    STARTDECL(ceiling) (Value &a) { return Value(int(ceilf(a.fval()))); } ENDDECL1(ceiling, "f", "F", "I",
        "the nearest int >= f");
    STARTDECL(ceiling) (Value &a) { VECTOROP(int(ceilf(f.fval()))); } ENDDECL1(ceiling, "v", "F]", "I]:/",
        "the nearest ints >= each component of v");

    STARTDECL(floor)   (Value &a) { return Value(int(floorf(a.fval()))); } ENDDECL1(floor, "f", "F", "I",
        "the nearest int <= f");
    STARTDECL(floor)   (Value &a) { VECTOROP(int(floorf(f.fval()))); } ENDDECL1(floor, "v", "F]", "I]:/",
        "the nearest ints <= each component of v");

    STARTDECL(int)(Value &a) { return Value(int(a.fval())); } ENDDECL1(int, "f", "F", "I",
        "converts a float to an int by dropping the fraction");
    STARTDECL(int)(Value &a) { VECTOROPT(int(f.fval()), GETVECTYPE(GetIntVectorType)); } ENDDECL1(int, "v", "F]", "I]:/",
        "converts a vector of floats to ints by dropping the fraction");

    STARTDECL(round)   (Value &a) { return Value(int(a.fval() + 0.5f)); } ENDDECL1(round, "f", "F", "I",
        "converts a float to the closest int");
    STARTDECL(round)   (Value &a) { VECTOROP(int(f.fval() + 0.5f)); } ENDDECL1(round, "v", "F]", "I]:/",
        "converts a vector of floats to the closest ints");

    STARTDECL(fraction)(Value &a) { return Value(a.fval() - floorf(a.fval())); } ENDDECL1(fraction, "f", "F", "F",
        "returns the fractional part of a float: short for f - floor(f)");
    STARTDECL(fraction)(Value &a) { VECTOROP(f.fval() - floorf(f.fval())); } ENDDECL1(fraction, "v", "F]", "F]:/",
        "returns the fractional part of a vector of floats");

    STARTDECL(float)(Value &a) { return Value(float(a.ival())); } ENDDECL1(float, "i", "I", "F",
        "converts an int to float");
    STARTDECL(float)(Value &a) { VECTOROPT(float(f.ival()), GETVECTYPE(GetFloatVectorType)); } ENDDECL1(float, "v", "I]", "F]:/",
        "converts a vector of ints to floats");

    STARTDECL(sin) (Value &a) { return Value(sinf(a.fval() * RAD)); } ENDDECL1(sin, "angle", "F", "F",
        "the y coordinate of the normalized vector indicated by angle (in degrees)");
    STARTDECL(cos) (Value &a) { return Value(cosf(a.fval() * RAD)); } ENDDECL1(cos, "angle", "F", "F",
        "the x coordinate of the normalized vector indicated by angle (in degrees)");

    STARTDECL(sincos) (Value &a) { return ToValueF(float3(cosf(a.fval() * RAD), sinf(a.fval() * RAD), 0.0f)); }
    ENDDECL1(sincos, "angle", "F", "F]:3",
        "the normalized vector indicated by angle (in degrees), same as [ cos(angle), sin(angle), 0 ]");

    STARTDECL(arcsin) (Value &y) { return Value(asinf(y.fval()) / RAD); } ENDDECL1(arcsin, "y", "F", "F",
        "the angle (in degrees) indicated by the y coordinate projected to the unit circle");
    STARTDECL(arccos) (Value &x) { return Value(acosf(x.fval()) / RAD); } ENDDECL1(arccos, "x", "F", "F",
        "the angle (in degrees) indicated by the x coordinate projected to the unit circle");

    STARTDECL(atan2) (Value &vec) { auto v = ValueDecToF<3>(vec); return Value(atan2f(v.y(), v.x()) / RAD); } 
    ENDDECL1(atan2, "vec",  "F]" , "F",
        "the angle (in degrees) corresponding to a normalized 2D vector");

    STARTDECL(normalize) (Value &vec)
    {
        switch (vec.vval()->len)
        {
            case 2: { auto v = ValueDecToF<2>(vec); return ToValueF(v == float2_0 ? v : normalize(v)); }
            case 3: { auto v = ValueDecToF<3>(vec); return ToValueF(v == float3_0 ? v : normalize(v)); }
            case 4: { auto v = ValueDecToF<4>(vec); return ToValueF(v == float4_0 ? v : normalize(v)); }
            default: return g_vm->BuiltinError("normalize() only works on vectors of length 2 to 4");
        }
    }
    ENDDECL1(normalize, "vec",  "F]" , "F]:/",
        "returns a vector of unit length");

    STARTDECL(dot) (Value &a, Value &b) { return Value(dot(ValueDecToF<4>(a), ValueDecToF<4>(b))); }
    ENDDECL2(dot,   "a,b", "F]F]", "F",
        "the length of vector a when projected onto b (or vice versa)");

    STARTDECL(magnitude) (Value &a)  { return Value(length(ValueDecToF<4>(a))); } ENDDECL1(magnitude, "v", "F]", "F",
        "the geometric length of a vector");

    STARTDECL(cross) (Value &a, Value &b) { return ToValueF(cross(ValueDecToF<3>(a), ValueDecToF<3>(b))); }
    ENDDECL2(cross, "a,b", "F]F]", "F]:3",
        "a perpendicular vector to the 2D plane defined by a and b (swap a and b for its inverse)");

    STARTDECL(rnd) (Value &a) { return Value(rnd(max(1, a.ival()))); } ENDDECL1(rnd, "max", "I", "I",
        "a random value [0..max).");
    STARTDECL(rnd) (Value &a) { VECTOROP(rnd(max(1, f.ival()))); } ENDDECL1(rnd, "max", "I]", "I]:/",
        "a random vector within the range of an input vector.");
    STARTDECL(rndfloat)() { return Value((float)rnd.rnddouble()); } ENDDECL0(rndfloat, "", "", "F",
        "a random float [0..1)");
    STARTDECL(rndseed) (Value &seed) { rnd.seed(seed.ival()); return Value(); } ENDDECL1(rndseed, "seed", "I", "",
        "explicitly set a random seed for reproducable randomness");

    STARTDECL(div) (Value &a, Value &b) { return Value(float(a.ival()) / float(b.ival())); } ENDDECL2(div, "a,b", "II", "F",
        "forces two ints to be divided as floats");

    STARTDECL(clamp) (Value &a, Value &b, Value &c)
    {
        return Value(max(min(a.ival(), c.ival()), b.ival()));
    }
    ENDDECL3(clamp, "x,min,max", "III", "I",
        "forces an integer to be in the range between min and max (inclusive)");

    STARTDECL(clamp) (Value &a, Value &b, Value &c)
    {
        return Value(max(min(a.fval(), c.fval()), b.fval()));
    }
    ENDDECL3(clamp, "x,min,max", "FFF", "F",
             "forces a float to be in the range between min and max (inclusive)");

    STARTDECL(inrange) (Value &x, Value &range, Value &bias)
    {
        return Value(x.ival() >= bias.ival() && x.ival() < bias.ival() + range.ival());
    }
    ENDDECL3(inrange, "x,range,bias", "III?", "I",
             "checks if an integer is >= bias and < bias + range. Bias defaults to 0.");

    STARTDECL(inrange) (Value &xv, Value &rangev, Value &biasv)
    {
        auto x     = ValueDecToI<2>(xv);
        auto range = ValueDecToI<2>(rangev);
        auto bias  = biasv.True() ? ValueDecToI<2>(biasv) : int2_0;
        return Value(x >= bias && x < bias + range);
    }
    ENDDECL3(inrange, "x,range,bias", "I]:2I]:2I]:2?", "I",
             "checks if a 2d integer vector is >= bias and < bias + range. Bias defaults to 0.");

    STARTDECL(inrange) (Value &xv, Value &rangev, Value &biasv)
    {
        auto x     = ValueDecToF<2>(xv);
        auto range = ValueDecToF<2>(rangev);
        auto bias  = ValueDecToF<2>(biasv);
        return Value(x >= bias && x < bias + range);
    }
    ENDDECL3(inrange, "x,range,bias", "F]:2F]:2F]:2?", "I",
        "checks if a 2d float vector is >= bias and < bias + range. Bias defaults to 0.");

    STARTDECL(abs) (Value &a) { return Value(abs(a.ival())); } ENDDECL1(abs, "x", "I", "I",
        "absolute value of an integer");
    STARTDECL(abs) (Value &a) { return Value(fabsf(a.fval())); } ENDDECL1(abs, "x", "F", "F",
        "absolute value of a float");
    STARTDECL(abs) (Value &a) { VECTOROP(abs(f.ival())); } ENDDECL1(abs, "x", "I]", "I]:/",
        "absolute value of an int vector");
    STARTDECL(abs) (Value &a) { VECTOROP(fabsf(f.fval())); } ENDDECL1(abs, "x", "F]", "F]:/",
        "absolute value of a float vector");

    // FIXME: need to guarantee this assert in typechecking
    #define VECBINOP(name,access) \
        if (x.vval()->len != y.vval()->len) g_vm->BuiltinError(#name ## "() arguments must be equal length"); \
        assert(x.vval()->typeoff == y.vval()->typeoff); \
        auto v = g_vm->NewVector(x.vval()->len, x.vval()->typeoff); \
        for (int i = 0; i < x.vval()->len; i++) { \
            v->Push(Value(name(x.vval()->at(i).access(), y.vval()->at(i).access()))); \
        } \
        x.DECRT(); y.DECRT(); \
        return Value(v);

    STARTDECL(min) (Value &x, Value &y) { return Value(min(x.ival(), y.ival())); } ENDDECL2(min, "x,y", "II", "I",
        "smallest of 2 integers.");
    STARTDECL(min) (Value &x, Value &y) { return Value(min(x.fval(), y.fval())); } ENDDECL2(min, "x,y", "FF", "F",
        "smallest of 2 floats.");
    STARTDECL(min) (Value &x, Value &y) { VECBINOP(min,ival) } ENDDECL2(min, "x,y", "I]I]", "I]:/",
        "smallest components of 2 int vectors");
    STARTDECL(min) (Value &x, Value &y) { VECBINOP(min,fval) } ENDDECL2(min, "x,y", "F]F]", "F]:/",
        "smallest components of 2 float vectors");

    STARTDECL(max) (Value &x, Value &y) { return Value(max(x.ival(), y.ival())); } ENDDECL2(max, "x,y", "II", "I",
        "largest of 2 integers.");
    STARTDECL(max) (Value &x, Value &y) { return Value(max(x.fval(), y.fval())); } ENDDECL2(max, "x,y", "FF", "F",
        "largest of 2 floats.");
    STARTDECL(max) (Value &x, Value &y) { VECBINOP(max,ival) } ENDDECL2(max, "x,y", "I]I]", "I]:/",
        "largest components of 2 int vectors");
    STARTDECL(max) (Value &x, Value &y) { VECBINOP(max,fval) } ENDDECL2(max, "x,y", "F]F]", "F]:/",
        "largest components of 2 float vectors");

    STARTDECL(cardinalspline) (Value &z, Value &a, Value &b, Value &c, Value &f, Value &t)
    {
        return ToValueF(cardinalspline(ValueDecToF<3>(z),
                                       ValueDecToF<3>(a),
                                       ValueDecToF<3>(b),
                                       ValueDecToF<3>(c), f.fval(), t.fval()));
    }
    ENDDECL6(cardinalspline, "z,a,b,c,f,tension", "F]F]F]F]FF", "F]:3",
        "computes the position between a and b with factor f [0..1], using z (before a) and c (after b) to form a"
        " cardinal spline (tension at 0.5 is a good default)");

    STARTDECL(lerp) (Value &x, Value &y, Value &f)
    {
        return Value(mix(x.fval(), y.fval(), f.fval()));
    }
    ENDDECL3(lerp, "x,y,f", "FFF", "F",
        "linearly interpolates between x and y with factor f [0..1]");

    STARTDECL(lerp) (Value &x, Value &y, Value &f)
    {
        auto numelems = x.vval()->len;
        return ToValueF(mix(ValueDecToF<4>(x), ValueDecToF<4>(y), f.fval()), numelems);
    }
    ENDDECL3(lerp, "x,y,f", "F]F]F", "F]:/",
        "linearly interpolates between x and y vectors with factor f [0..1]");

    STARTDECL(resume) (Value &co, Value &ret)
    {
        g_vm->CoResume(co.cval());
        // By the time CoResume returns, we're now back in the context of co, meaning that the return value below
        // is what is returned from yield.
        return ret;
    }
    ENDDECL2(resume, "coroutine,returnvalue", "RA%?", "A",
        "resumes execution of a coroutine, passing a value back or nil");

    STARTDECL(returnvalue) (Value &co)
    {
        Value &rv = co.cval()->Current().INC();
        co.DECRT();
        return rv;
    }
    ENDDECL1(returnvalue, "coroutine", "R", "A1",
        "gets the last return value of a coroutine");

    STARTDECL(active) (Value &co)
    {
        bool active = co.cval()->active;
        co.DECRT();
        return Value(active);
    }
    ENDDECL1(active, "coroutine", "R", "I",
        "wether the given coroutine is still active");

    STARTDECL(program_name) ()
    {
        return Value(g_vm->NewString(g_vm->GetProgramName()));
    }
    ENDDECL0(program_name, "", "", "S",
        "returns the name of the main program (e.g. \"foo.lobster\".");

    STARTDECL(caller_id) ()
    {
        return Value(g_vm->CallerId());
    }
    ENDDECL0(caller_id, "", "", "I",
        "returns an int that uniquely identifies the caller to the current function.");

    STARTDECL(seconds_elapsed) ()
    {
        return Value((float)g_vm->Time());
    }
    ENDDECL0(seconds_elapsed, "", "", "F",
        "seconds since program start as a float, unlike gl_time() it is calculated every time it is called");

    STARTDECL(assert) (Value &c)
    {
        if (!c.True()) g_vm->BuiltinError("assertion failed");
        return c;
    }
    ENDDECL1(assert, "condition", "A*", "",
        "halts the program with an assertion failure if passed false. returns its input");

    STARTDECL(trace_bytecode) (Value &i)
    {
        g_vm->Trace(i.ival() != 0);
        return Value();
    }
    ENDDECL1(trace_bytecode, "on", "I", "",
        "tracing shows each bytecode instruction as it is being executed, not very useful unless you are trying to"
        " isolate a compiler bug");

    STARTDECL(collect_garbage) ()
    {
        return Value(g_vm->GC());
    }
    ENDDECL0(collect_garbage, "", "", "I",
        "forces a garbage collection to re-claim cycles. slow and not recommended to be used. instead, write code"
        " to clear any back pointers before abandoning data structures. Watch for a \"LEAKS FOUND\" message in the"
        " console upon program exit to know when you've created a cycle. returns amount of objects collected.");

    STARTDECL(set_max_stack_size) (Value &max)
    {
        g_vm->SetMaxStack(max.ival() * 1024 * 1024 / sizeof(Value));
        return max;
    }
    ENDDECL1(set_max_stack_size, "max",  "I", "",
        "size in megabytes the stack can grow to before an overflow error occurs. defaults to 1");
}

AutoRegister __abi("builtins", AddBuiltins);

