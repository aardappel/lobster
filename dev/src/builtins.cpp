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

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "lobster/wfc.h"

namespace lobster {

static int IntCompare(Value a, Value b) {
    return a.ival() < b.ival() ? -1 : a.ival() > b.ival();
}

static int FloatCompare(Value a, Value b) {
    return a.fval() < b.fval() ? -1 : a.fval() > b.fval();
}

static int StringCompare(Value a, Value b) {
    auto _a = a.sval()->strv();
    auto _b = b.sval()->strv();
    return (_a > _b) - (_b > _a);
}

static int ObjectCompare(Value a, Value b) {
    return a.any() < b.any() ? -1 : a.any() > b.any();
}

static int FirstStringCompare(Value key, Value elem) {
    auto _a = key.sval()->strv();
    auto _b = elem.oval()->At(0).sval()->strv();
    return (_a > _b) - (_b > _a);
}

static int FirstObjectCompare(Value key, Value elem) {
    auto _a = key.any();
    auto _b = elem.oval()->At(0).any();
    return _a < _b ? -1 : _a > _b;
}

template<typename T> iint BinarySearch(StackPtr &sp, Value l, Value key, T comparefun) {
    iint size = l.vval()->len;
    iint i = 0;
    for (;;) {
        if (!size) break;
        iint mid = size / 2;
        iint comp = comparefun(key, l.vval()->AtS(i + mid));
        if (comp) {
            if (comp < 0) size = mid;
            else { mid++; i += mid; size -= mid; }
        } else {
            i += mid;
            size = 1;
            while (i && !comparefun(key, l.vval()->AtS(i - 1))) { i--; size++; }
            while (i + size < l.vval()->len && !comparefun(key, l.vval()->AtS(i + size))) {
                size++;
            }
            break;
        }
    }
    Push(sp, Value(size));
    return i;
}

BuiltinGroup core_builtins;
#define BUILTIN_GROUP core_builtins
#define BUILTIN_SYM(name) builtin_##name

BUILTIN(print, "x", "Ss", "",
    "output any value to the console (with linefeed).")
(VM &vm, LString *a) {
    vm.s_reuse.clear();
    RefToString(vm, vm.s_reuse, a, vm.programprintprefs);
    if (vm.evalret.second) {
        LOG_ERROR(vm.s_reuse);
    } else {
        LOG_PROGRAM(vm.s_reuse);
    }
}

// This is now the identity function, but still useful to force a coercion.
BUILTIN(string, "x", "Ssk", "S",
    "convert any value to string")
(VM &, LString *a) {
    return a;
}

BUILTIN(set_print_depth, "depth", "I", "I",
    "for printing / string conversion: sets max vectors/objects recursion depth (default 10), "
    "returns old value")
(VM &vm, iint a) {
    auto old = vm.programprintprefs.depth;
    vm.programprintprefs.depth = a;
    return old;
}

BUILTIN(set_print_length, "len", "I", "I",
    "for printing / string conversion: sets max string length (default 100000), "
    "returns old value")
(VM &vm, iint a) {
    auto old = vm.programprintprefs.budget;
    vm.programprintprefs.budget = a;
    return old;
}

BUILTIN(set_print_quoted, "quoted", "B", "B",
    "for printing / string conversion: if the top level value is a string, whether to convert"
    " it with escape codes and quotes (default false), returns old value")
(VM &vm, iint a) {
    auto old = vm.programprintprefs.quoted;
    vm.programprintprefs.quoted = a != 0;
    return old;
}

BUILTIN(set_print_decimals, "decimals", "I", "I",
    "for printing / string conversion: number of decimals for any floating point output"
    " (default -1, meaning all), returns old value")
(VM &vm, iint a) {
    auto old = vm.programprintprefs.decimals;
    vm.programprintprefs.decimals = a;
    return old;
}

BUILTIN(set_print_indent, "spaces", "I", "I",
    "for printing / string conversion: number of spaces to indent with. default is 0:"
    " no indent / no multi-line, returns old value")
(VM &vm, iint a) {
    auto old = vm.programprintprefs.indent;
    vm.programprintprefs.indent = (int)a;
    return old;
}

BUILTIN(get_line, "prefix", "S", "S",
    "reads a string from the console if possible (followed by enter). Prefix will be"
    " printed before the input")
(VM &vm, LString *prefix) {
    fputs(prefix->data(), stdout);
    const int MAXSIZE = 1000;
    char buf[MAXSIZE];
    if (!fgets(buf, MAXSIZE, stdin)) buf[0] = 0;
    buf[MAXSIZE - 1] = 0;
    // Anything past the terminator is uninitialized.
    for (int i = 0; i < MAXSIZE; i++) if (buf[i] == '\n' || !buf[i]) { buf[i] = 0; break; }
    return vm.NewString(buf);
}

BUILTIN(append, "xs,ys", "A]*cA]*u1c", "A]1",
    "creates a new vector by appending all elements of 2 input vectors")
(VM &vm, LVector *v1, LVector *v2) {
    auto type = v1->tti;
    auto nv = (LVector *)vm.NewVec(0, v1->len + v2->len, type);
    nv->Append(vm, v1, 0, v1->len);
    nv->Append(vm, v2, 0, v2->len);
    return nv;
}

BUILTIN(append_into, "dest,src", "A]*A]1c", "Ab]1",
    "appends all elements of the second vector into the first")
(VM &vm, LVector *v1, LVector *v2) {
    v1->Append(vm, v2, 0, v2->len);
    return v1;
}

BUILTIN(vector_capacity, "xs,len", "A]*I", "Ab]1",
    "ensures the vector capacity (number of elements it can contain before re-allocating)"
    " is at least \"len\". Does not actually add (or remove) elements. This function is"
    " just for efficiency in the case the amount of \"push\" operations is known."
    " returns original vector.")
(VM &vm, LVector *vec, iint len) {
    vec->MinCapacity(vm, len);
    return vec;
}

BUILTIN_OVERLOAD(length_int, "length", "x", "I", "I",
    "length of int (identity function, useful in combination with string/vector version)")
(VM &, iint a) {
    return a;
}

BUILTIN_OVERLOAD(length_string, "length", "s", "S", "I",
    "length of string")
(VM &, LString *a) {
    auto len = a->len;
    return len;
}

BUILTIN_OVERLOAD(length_vector, "length", "xs", "A]*", "I",
    "length of vector")
(VM &, LVector *a) {
    auto len = a->len;
    return len;
}

BUILTIN(equal, "a,b", "AA", "B",
    "structural equality between any two values (recurses into vectors/objects,"
    " unlike == which is only true for vectors/objects if they are the same object)")
(VM &vm, Value a, Value b) {
    bool eq = RefEqual(vm, a.refnil(), b.refnil(), true);
    return eq;
}

BUILTIN(push, "xs,x", "A]*Akw1", "Ab]1",
    "appends one element to a vector, returns existing vector")
(VM &vm, LVector *l, Value *x) {
    l->PushVW(vm, x);
    return l;
}

BUILTIN_V(pop, "xs", "A]*", "A1",
    "removes last element from vector and returns it")
(StackPtr &sp, VM &vm, LVector *l) {
    if (!l->len) vm.BuiltinError("pop: empty vector");
    l->PopVW(TopPtr(sp));
    PushN(sp, l->width);
}

BUILTIN_V(top, "xs", "A]*", "Ab1",
    "returns last element from vector")
(StackPtr &sp, VM &vm, LVector *l) {
    if (!l->len) vm.BuiltinError("top: empty vector");
    l->TopVW(TopPtr(sp));
    PushN(sp, l->width);
}

BUILTIN(insert, "xs,i,x", "A]*IAkw1", "Ab]1",
    "inserts a value into a vector at index i, existing elements shift upward,"
    " returns original vector")
(VM &vm, LVector *l, iint i, Value *x) {
    if (i < 0 || i > l->len)
        vm.BuiltinError("insert: index or n out of range");  // note: i==len is legal
    l->Insert(vm, x, i);
    return l;
}

BUILTIN_V(remove, "xs,i", "A]*I", "A1",
    "remove element at index i, following elements shift down. returns the element removed.")
(StackPtr &sp, VM &vm, LVector *l, iint i) {
    if (i < 0 || i >= l->len)
        vm.BuiltinError(cat("remove: index (", i, ") out of range (", l->len, ")"));
    l->RemovePush(sp, i);
}

BUILTIN(remove_range, "xs,i,n", "A]*II", "",
    "remove n elements at index i, following elements shift down.")
(VM &vm, LVector *l, iint i, iint amount) {
    if (amount < 0 || amount > l->len || i < 0 || i > l->len - amount)
        vm.BuiltinError(
            cat("remove: index (", i, ") or amount (", amount, ") out of range (", l->len, ")"));
    l->Remove(vm, i, amount);
}

BUILTIN(remove_obj, "xs,obj", "A]*A1", "Ab2",
    "remove all elements equal to obj (==), returns obj.")
(VM &vm, LVector *l, Value o) {
    auto vt = vm.GetTypeInfo(l->ti(vm).subt).t;
    for (iint i = 0; i < l->len; i++) {
        auto e = l->AtS(i);
        if (e.Equal(vm, vt, o, vt, false)) {
            l->Remove(vm, i--, 1);
        }
    }
    return o;
}

BUILTIN(truncate, "xs,i", "A]*I", "",
    "removes all elements starting from index i, does nothing if i >= len")
(VM &vm, LVector *l, iint i) {
    if (i < 0 || i >= l->len) return;
    l->Truncate(vm, i);
}

BUILTIN_OVERLOAD(binary_search_int, "binary_search", "xs,key", "I]I", "II",
    "does a binary search for key in a sorted vector, returns as first return value how many"
    " matches were found, and as second the index in the array where the matches start (so you"
    " can read them, overwrite them, or remove them), or if none found, where the key could be"
    " inserted such that the vector stays sorted. This overload is for int vectors and keys.")
(StackPtr &sp, VM &, LVector *l, iint key) {
    auto r = BinarySearch(sp, l, key, IntCompare);
    return r;
}

BUILTIN_OVERLOAD(binary_search_float, "binary_search", "xs,key", "F]F", "II",
    "float version.")
(StackPtr &sp, VM &, LVector *l, double key) {
    auto r = BinarySearch(sp, l, key, FloatCompare);
    return r;
}

BUILTIN_OVERLOAD(binary_search_string, "binary_search", "xs,key", "S]S", "II",
    "string version.")
(StackPtr &sp, VM &, LVector *l, LString *key) {
    auto r = BinarySearch(sp, l, key, StringCompare);
    return r;
}

BUILTIN(binary_search_object, "xs,key", "A]*A1", "II",
    "object version. compares by reference rather than contents.")
(StackPtr &sp, VM &, LVector *l, Value key) {
    auto r = BinarySearch(sp, l, key, ObjectCompare);
    return r;
}

// TODO: add int/float versions of this?
BUILTIN(binary_search_first_field_string, "xs,key", "A]*S", "II",
    "object version where key is the first field (must be string, runtime error if it is not)")
(StackPtr &sp, VM &vm, LVector *l, LString *key) {
    auto &et = vm.GetTypeInfo(l->ti(vm).subt);
    if (et.t != RTT_CLASS || !et.len || vm.GetTypeInfo(et.elemtypes[0].type).t != RTT_STRING)
        vm.BuiltinError(
            "binary_search_first_field_string: elements not objects with first string field");
    auto r = BinarySearch(sp, l, key, FirstStringCompare);
    return r;
}

BUILTIN(binary_search_first_field_object, "xs,key", "A]*A", "II",
    "object version where key is the first field (must be object, runtime error if it is not)")
(StackPtr &sp, VM &vm, LVector *l, Value key) {
    auto &et = vm.GetTypeInfo(l->ti(vm).subt);
    if (et.t != RTT_CLASS || !et.len || vm.GetTypeInfo(et.elemtypes[0].type).t != RTT_CLASS)
        vm.BuiltinError(
            "binary_search_first_field_object: elements not objects with first object field");
    auto r = BinarySearch(sp, l, key, FirstObjectCompare);
    return r;
}

BUILTIN(copy, "x", "A", "A1",
    "makes a shallow copy of any object/vector/string.")
(VM &vm, Value v) {
    return v.CopyRef(vm, 1);
}

BUILTIN(deepcopy, "x,depth", "AI", "A1",
    "makes a deep copy of any object/vector/string. DAGs become trees, and cycles will"
    " clone until it reach the given depth. depth == 1 would do the same as copy.")
(VM &vm, Value v, iint depth) {
    return v.CopyRef(vm, max((iint)1, depth));
}

BUILTIN(slice, "xs,start,size", "A]*II", "A]1",
    "returns a sub-vector of size elements from index start."
    " size can be negative to indicate the rest of the vector.")
(VM &vm, LVector *l, iint start, iint e) {
    auto size = e;
    if (start < 0)
        vm.BuiltinError(cat("slice: start cannot be negative: ", start));
    if (size < 0) size = std::max((iint)0, l->len - start);
    if (start + size > l->len)
        vm.BuiltinError(cat("slice: range extends beyond the end: ", start + size, " > ", l->len));
    auto nv = (LVector *)vm.NewVec(0, size, l->tti);
    nv->Append(vm, l, start, size);
    return nv;
}

BUILTIN_OVERLOAD(any_vector, "any", "xs", "A]*", "B",
    "returns whether any elements of the vector are true values")
(VM &, LVector *v) {
    iint l = v->len;
    for (auto i = 0; i < l; i++) {
        if (v->AtS(i).True()) return true;
    }
    return false;
}

// A builtin that takes a numeric struct exists for each width Lobster code uses it with, since
// the generated code passes the values of a struct in exactly that many stack slots, with no
// length to go with them. The symbol of each is the given one with its width appended, and its
// return value goes in the slots its arguments came in.
#define VECBOOLW(sym, name, W, help, init, test) \
    BUILTIN_OVERLOAD(sym##W, name, "xs", "I}:" #W, "B", help) \
    (VM &, vec<iint, W> v) { \
        auto r = init; \
        for (int i = 0; i < W; i++) if (test) r = !init; \
        return r; \
    }
#define VECBOOL1234(sym, name, help, init, test) \
    VECBOOLW(sym, name, 1, help, init, test) \
    VECBOOLW(sym, name, 2, help, init, test) \
    VECBOOLW(sym, name, 3, help, init, test) \
    VECBOOLW(sym, name, 4, help, init, test)

VECBOOL1234(any_ivec, "any",
    "returns whether any elements of the numeric struct are true values",
    false, v[i] != 0)

BUILTIN_OVERLOAD(all_vector, "all", "xs", "A]*", "B",
    "returns whether all elements of the vector are true values")
(VM &, LVector *v) {
    for (iint i = 0; i < v->len; i++) {
        if (v->AtS(i).False()) return false;
    }
    return true;
}

VECBOOL1234(all_ivec, "all",
    "returns whether all elements of the numeric struct are true values",
    true, v[i] == 0)

BUILTIN(substring, "s,start,size", "SII", "S",
    "returns a substring of size characters from index start."
    " size can be negative to indicate the rest of the string.")
(VM &vm, LString *l, iint start, iint e) {
    iint size = e;
    if (start < 0) vm.BuiltinError(cat("substring: start cannot be negative: ", start));
    if (size < 0) size = std::max((iint)0, l->len - start);
    if (start + size > l->len)
        vm.BuiltinError(cat("substring: range extends beyond the end: ", start + size, " > ", l->len));
    auto ns = vm.NewString(string_view(l->data() + start, (size_t)size));
    return ns;
}

BUILTIN(find_string, "s,substr,offset", "SSI?", "I",
    "finds the index at which substr first appears, or -1 if none."
    " optionally start at a position other than 0")
(VM &, LString *s, LString *sub, iint offset) {
    return (ssize_t)s->strv().find(sub->strv(), (size_t)offset);
}

BUILTIN(find_string_reverse, "s,substr,offset", "SSI?", "I",
    "finds the index at which substr first appears when searching from the end, or -1 if none."
    " optionally start at a position other than the end of the string")
(VM &, LString *s, LString *sub, iint offset) {
    auto sv = s->strv();
    auto lim = offset ? (size_t)offset : sv.size();
    // Cut sv, because the "pos" arg to rfind has the weird behavior that it
    // will go over that limit by the size of the search string (wtf?)
    sv = sv.substr(0, lim);
    return (ssize_t)sv.rfind(sub->strv());
}

BUILTIN(split_string, "s,delimiter", "SS", "SS",
    "returns two strings, the parts of the input before and after the first delimiter."
    " if not found, returns the input and an empty string")
(StackPtr &sp, VM &vm, LString *input, LString *delim) {
    auto s = input->strv();
    auto d = delim->strv();
    auto pos = s.find(d, 0);
    if (pos == string_view::npos) {
        input->Inc();
        Push(sp, input);
        return vm.NewString(0);  // FIXME: need to have a way to not allocate empty strings.
    } else {
        auto left = vm.NewString(string_view(s.data(), pos));
        auto right = vm.NewString(string_view(s.data() + (pos + d.size()), s.size() - (pos + d.size())));
        Push(sp, Value(left));
        return right;
    }
}

BUILTIN(split_string_reverse, "s,delimiter", "SS", "SS",
    "returns two strings, the parts of the input before and after the last delimiter."
    " if not found, returns an empty string and the input")
(StackPtr &sp, VM &vm, LString *input, LString *delim) {
    auto s = input->strv();
    auto d = delim->strv();
    auto pos = s.rfind(d);
    if (pos == string_view::npos) {
        Push(sp, Value(vm.NewString(0)));  // FIXME: need to have a way to not allocate empty strings.
        input->Inc();
        return input;
    } else {
        auto left = vm.NewString(string_view(s.data(), pos));
        auto right = vm.NewString(string_view(s.data() + (pos + d.size()), s.size() - (pos + d.size())));
        Push(sp, Value(left));
        return right;
    }
}

BUILTIN(replace_string, "s,a,b,count", "SSSI?", "S",
    "returns a copy of s where all occurrences of a have been replaced with b."
    " if a is empty, no replacements are made."
    " if count is specified, makes at most that many replacements")
(VM &vm, LString *is, LString *ia, LString *ib, iint count) {
    string s;
    auto sv = is->strv();
    auto a = ia->strv();
    auto b = ib->strv();
    auto c = count ? count : numeric_limits<iint>::max();
    if (a.empty()) {
        // We could error here, but more useful to just return the input.
        s = sv;
    } else {
        for (size_t i = 0;;) {
            auto j = std::min(sv.find(a, i), sv.size());
            auto prefix = sv.substr(i, j - i);;
            s += prefix;
            i += prefix.size();
            if (j == sv.size()) break;
            s += b;
            i += a.size();
            if (!--c) {
                s += sv.substr(i);
                break;
            }
        }
    }
    auto ns = vm.NewString(s);
    return ns;
}

BUILTIN(string_to_int, "s,base", "SI?", "IB",
    "converts a string to an int given the base (2..36, e.g. 16 for hex, default is 10)."
    "returns 0 if no numeric data could be parsed (or overflow); second return value is true if all"
    "characters of the string were parsed (and no overflow).")
(StackPtr &sp, VM &vm, LString *s, iint b) {
    int base = (b != 0) ? (int)b : 10;
    if (base < 2 || base > 36)
        vm.BuiltinError("string_to_int: values out of range");
    std::errc ec;
    auto svnt = s->strv();
    auto i = parse_int<iint>(svnt, base, &ec);
    Push(sp,  i);
    return ec == std::errc();
}

BUILTIN(string_to_float, "s", "S", "FB",
    "converts a string to a float. returns 0.0 if no numeric data could be parsed;"
    "second return value is true if all characters of the string were parsed.")
(StackPtr &sp, VM &, LString *s) {
    const char *end;
    auto sv = s->strv();
    auto f = parse_float<double>(sv, &end);
    Push(sp, f);
    return end == sv.data() + sv.size();
}

BUILTIN(tokenize, "s,delimiters,whitespace,dividing", "SSSI?", "S]",
    "splits a string into a vector of strings, by splitting into segments upon each dividing or"
    " terminating delimiter. Segments are stripped of leading and trailing whitespace."
    " Example: \"; A ; B C;; \" becomes [ \"\", \"A\", \"B C\", \"\" ] with \";\" as delimiter and"
    " \" \" as whitespace. If dividing was true, there would be a 5th empty string element.")
(VM &vm, LString *s, LString *delims, LString *whitespace, iint dividing) {
    auto v = (LVector *)vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
    auto ws = whitespace->strv();
    auto dl = delims->strv();
    auto p = s->strv();
    p.remove_prefix(std::min(p.find_first_not_of(ws), p.size()));
    bool has_delim = false;
    while (!p.empty() || (has_delim && (dividing != 0))) {
        auto delim = std::min(p.find_first_of(dl), p.size());
        auto delimstr = p.substr(0, delim);
        auto end = std::min(delimstr.find_last_not_of(ws) + 1, delim);
        v->Push(vm, vm.NewString(string_view(p.data(), end)));
        p.remove_prefix(delim);
        has_delim = std::min(p.find_first_not_of(dl), p.size()) != 0;
        if (has_delim) p.remove_prefix(1);
        p.remove_prefix(std::min(p.find_first_not_of(ws), p.size()));
    }
    return v;
}

BUILTIN(unicode_to_string, "us", "I]", "S",
    "converts a vector of ints representing unicode values to a UTF-8 string.")
(VM &vm, LVector *v) {
    char buf[7];
    string s;
    for (iint i = 0; i < v->len; i++) {
        auto c = v->AtS(i);
        auto len = ToUTF8((int)c.ival(), buf);
        s += string_view(buf, len);
    }
    return vm.NewString(s);
}

BUILTIN(string_to_unicode, "s", "S", "I]B",
    "converts a UTF-8 string into a vector of unicode values. second return value is false"
    " if there was a decoding error, and the vector will only contain the characters up to the"
    " error")
(StackPtr &sp, VM &vm, LString *s) {
    auto v = (LVector *)vm.NewVec(0, s->len, TYPE_ELEM_VECTOR_OF_INT);
    Push(sp, v);
    auto p = s->strv();
    while (!p.empty()) {
        int u = FromUTF8(p);
        if (u < 0) return false;
        v->Push(vm, u);
    }
    return true;
}

BUILTIN(number_to_string, "number,base,minchars", "III", "S",
    "converts the (unsigned version) of the input integer number to a string given the base"
    " (2..36, e.g. 16 for hex) and outputting a minimum of characters (padding with 0).")
(VM &vm, iint n, iint b, iint mc) {
    if (b < 2 || b > 36 || mc > 32)
        vm.BuiltinError("number_to_string: values out of range");
    auto i = (uint64_t)n;
    string s;
    auto from = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while (i || ssize(s) < mc) {
        s.insert(0, 1, from[i % b]);
        i /= b;
    }
    return vm.NewString(s);
}

BUILTIN(lowercase, "s", "S", "S",
    "converts a UTF-8 string from any case to lower case, affecting only A-Z")
(VM &vm, LString *s) {
    auto ns = vm.NewString(s->strv());
    for (auto &c : ns->strv()) {
        // This is unicode-safe, since all unicode chars are in bytes >= 128
        if (c >= 'A' && c <= 'Z') (char &)c += 'a' - 'A';
    }
    return ns;
}

BUILTIN(uppercase, "s", "S", "S",
    "converts a UTF-8 string from any case to upper case, affecting only a-z")
(VM &vm, LString *s) {
    auto ns = vm.NewString(s->strv());
    for (auto &c : ns->strv()) {
        // This is unicode-safe, since all unicode chars are in bytes >= 128
        if (c >= 'a' && c <= 'z') (char &)c -= 'a' - 'A';
    }
    return ns;
}

BUILTIN(escape_string, "s,set,prefix,postfix", "SSSS", "S",
    "prefixes & postfixes any occurrences or characters in set in string s")
(VM &vm, LString *s, LString *set, LString *prefix, LString *postfix) {
    string out;
    for (auto p = s->strv();;) {
        auto loc = p.find_first_of(set->strv());
        if (loc != string_view::npos) {
            out.append(p.data(), loc);
            auto presv = prefix->strv();
            out.append(presv.data(), presv.size());
            out += p[loc++];
            auto postsv = postfix->strv();
            out.append(postsv.data(), postsv.size());
            p.remove_prefix(loc);
        } else {
            out += p;
            break;
        }
    }
    return vm.NewString(out);
}

BUILTIN(concat_string, "v,sep", "S]S", "S",
    "concatenates all elements of the string vector, separated with sep.")
(VM &vm, LVector *v, LString *sep) {
    string s;
    auto sepsv = sep->strv();
    for (iint i = 0; i < v->len; i++) {
        if (i) s.append(sepsv);
        auto esv = v->AtS(i).sval()->strv();
        s.append(esv);
    }
    return vm.NewString(s);
}

BUILTIN(repeat_string, "s,n", "SI", "S",
    "returns a string consisting of n copies of the input string.")
(VM &vm, LString *s, iint _n) {
    auto n = std::max(iint(0), _n);
    auto len = s->len;
    const iint max_len = 1024 * 1024 * 1024;
    // Divide rather than multiply, so the check itself can't overflow.
    if (len && n > max_len / len)
        vm.BuiltinError(cat("repeat_string: resulting string too big: ", len, " * ", n));
    auto ns = vm.NewString(len * n);
    for (iint i = 0; i < n; i++) {
        memcpy((char *)ns->data() + i * len, s->data(), (size_t)len);
    }
    return ns;
}


// An elementwise operation on a numeric struct `v`: `op` computes from the element in `f` the
// one it becomes, of element type RCT. See VECBOOL1234 above for the widths.
#define VECTOROPW(sym, name, ids, T, CT, RT, RCT, W, help, op) \
    BUILTIN_V_OVERLOAD(sym##W, name, ids, T "}:" #W, RT "}:" #W, help) \
    (StackPtr &sp, VM &, vec<CT, W> v) { \
        vec<RCT, W> r((RCT)0); \
        for (int i = 0; i < W; i++) { auto f = v[i]; r[i] = (RCT)(op); } \
        PushVec(sp, r); \
    }
#define VECTOROP1234(sym, name, ids, T, CT, RT, RCT, help, op) \
    VECTOROPW(sym, name, ids, T, CT, RT, RCT, 1, help, op) \
    VECTOROPW(sym, name, ids, T, CT, RT, RCT, 2, help, op) \
    VECTOROPW(sym, name, ids, T, CT, RT, RCT, 3, help, op) \
    VECTOROPW(sym, name, ids, T, CT, RT, RCT, 4, help, op)

// The same with the VM available, which only the random one needs.
#define VECTOROPVMW(sym, name, ids, T, CT, RT, RCT, W, help, op) \
    BUILTIN_V_OVERLOAD(sym##W, name, ids, T "}:" #W, RT "}:" #W, help) \
    (StackPtr &sp, VM &vm, vec<CT, W> v) { \
        vec<RCT, W> r((RCT)0); \
        for (int i = 0; i < W; i++) { auto f = v[i]; r[i] = (RCT)(op); } \
        PushVec(sp, r); \
    }
#define VECTOROPVM1234(sym, name, ids, T, CT, RT, RCT, help, op) \
    VECTOROPVMW(sym, name, ids, T, CT, RT, RCT, 1, help, op) \
    VECTOROPVMW(sym, name, ids, T, CT, RT, RCT, 2, help, op) \
    VECTOROPVMW(sym, name, ids, T, CT, RT, RCT, 3, help, op) \
    VECTOROPVMW(sym, name, ids, T, CT, RT, RCT, 4, help, op)

// A builtin that works on its numeric struct argument as a vector `v` of that width: VECMATH
// pushes the single value `expr` makes of it, VECVEC a struct of the same width.
#define VECMATHW(sym, name, ids, T, CT, RT, W, help, expr) \
    BUILTIN_OVERLOAD(sym##W, name, ids, T "}:" #W, RT, help) \
    (VM &, vec<CT, W> v) { \
        return expr; \
    }
#define VECMATH1234(sym, name, ids, T, CT, RT, help, expr) \
    VECMATHW(sym, name, ids, T, CT, RT, 1, help, expr) \
    VECMATHW(sym, name, ids, T, CT, RT, 2, help, expr) \
    VECMATHW(sym, name, ids, T, CT, RT, 3, help, expr) \
    VECMATHW(sym, name, ids, T, CT, RT, 4, help, expr)

#define VECVECW(sym, name, ids, T, CT, W, help, expr) \
    BUILTIN_V_OVERLOAD(sym##W, name, ids, T "}:" #W, T "}:" #W, help) \
    (StackPtr &sp, VM &, vec<CT, W> v) { \
        PushVec(sp, expr); \
    }
#define VECVEC1234(sym, name, ids, T, CT, help, expr) \
    VECVECW(sym, name, ids, T, CT, 1, help, expr) \
    VECVECW(sym, name, ids, T, CT, 2, help, expr) \
    VECVECW(sym, name, ids, T, CT, 3, help, expr) \
    VECVECW(sym, name, ids, T, CT, 4, help, expr)

// The same for two numeric structs of that width, as vectors `a` and `b`.
#define VEC2MATHW(sym, name, ids, T, CT, RT, W, help, expr) \
    BUILTIN_OVERLOAD(sym##W, name, ids, T "}:" #W T "}:" #W "1", RT, help) \
    (VM &, vec<CT, W> a, vec<CT, W> b) { \
        return expr; \
    }
#define VEC2MATH1234(sym, name, ids, T, CT, RT, help, expr) \
    VEC2MATHW(sym, name, ids, T, CT, RT, 1, help, expr) \
    VEC2MATHW(sym, name, ids, T, CT, RT, 2, help, expr) \
    VEC2MATHW(sym, name, ids, T, CT, RT, 3, help, expr) \
    VEC2MATHW(sym, name, ids, T, CT, RT, 4, help, expr)

#define VEC2VECW(sym, name, ids, T, CT, W, help, expr) \
    BUILTIN_V_OVERLOAD(sym##W, name, ids, T "}:" #W T "}:" #W "1", T "}:" #W, help) \
    (StackPtr &sp, VM &, vec<CT, W> a, vec<CT, W> b) { \
        PushVec(sp, expr); \
    }
#define VEC2VEC1234(sym, name, ids, T, CT, help, expr) \
    VEC2VECW(sym, name, ids, T, CT, 1, help, expr) \
    VEC2VECW(sym, name, ids, T, CT, 2, help, expr) \
    VEC2VECW(sym, name, ids, T, CT, 3, help, expr) \
    VEC2VECW(sym, name, ids, T, CT, 4, help, expr)

// And for three of them, as `a`, `b` and `c`.
#define VEC3VECW(sym, name, ids, T, CT, W, help, expr) \
    BUILTIN_V_OVERLOAD(sym##W, name, ids, T "}:" #W T "}:" #W "1" T "}:" #W "1", \
                       T "}:" #W, help) \
    (StackPtr &sp, VM &, vec<CT, W> a, vec<CT, W> b, vec<CT, W> c) { \
        PushVec(sp, expr); \
    }
#define VEC3VEC1234(sym, name, ids, T, CT, help, expr) \
    VEC3VECW(sym, name, ids, T, CT, 1, help, expr) \
    VEC3VECW(sym, name, ids, T, CT, 2, help, expr) \
    VEC3VECW(sym, name, ids, T, CT, 3, help, expr) \
    VEC3VECW(sym, name, ids, T, CT, 4, help, expr)

BUILTIN_OVERLOAD(pow_int, "pow", "a,b", "II", "I",
    "a raised to the power of b, for integers, using exponentiation by squaring")
(VM &, iint a, iint b) {
    return b >= 0 ? ipow<iint>(a, b) : 0;
}

BUILTIN_OVERLOAD(pow_float, "pow", "a,b", "FF", "F",
    "a raised to the power of b")
(VM &, double a, double b) { return pow(a, b); }

#define POWW(W) \
    BUILTIN_V_OVERLOAD(pow_fvec##W, "pow", "a,b", "F}:" #W "F", "F}:" #W, \
        "struct elements raised to the power of b") \
    (StackPtr &sp, VM &, vec<double, W> a, double exp) { \
        auto r = vec<double, W>(0.0); \
        for (int i = 0; i < W; i++) r[i] = pow(a[i], exp); \
        PushVec(sp, r); \
    }
POWW(2) POWW(3) POWW(4)
#undef POWW

BUILTIN(log, "a", "F", "F",
    "natural logaritm of a")
(VM &, double a) { return log(a); }

BUILTIN(log2, "a", "F", "F",
    "base 2 logaritm of a")
(VM &, double a) { return log2(a); }

BUILTIN(sqrt, "f", "F", "F",
    "square root")
(VM &, double a) { return sqrt(a); }

BUILTIN_OVERLOAD(ceiling_float, "ceiling", "f", "F", "I",
    "the nearest int >= f")
(VM &, double a) { return fceil<iint>(a); }
VECTOROP1234(ceiling_fvec, "ceiling", "v", "F", double, "I", iint,
    "the nearest ints >= each component of v",
    iint(fceil<iint>(f)))

BUILTIN_OVERLOAD(floor_float, "floor", "f", "F", "I",
    "the nearest int <= f")
(VM &, double a) { return ffloor<iint>(a); }
VECTOROP1234(floor_fvec, "floor", "v", "F", double, "I", iint,
    "the nearest ints <= each component of v",
    ffloor<iint>(f))

BUILTIN_OVERLOAD(int_float, "int", "f", "F", "I",
    "converts a float to an int by dropping the fraction")
(VM &, double a) { return iint(a); }
VECTOROP1234(int_fvec, "int", "v", "F", double, "I", iint,
    "converts a struct of floats to ints by dropping the fraction",
    iint(f))

BUILTIN_OVERLOAD(round_float, "round", "f", "F", "I",
    "converts a float to the closest int")
(VM &, double a) { return iint(a + (double(a >= 0) - 0.5)); }
VECTOROP1234(round_fvec, "round", "v", "F", double, "I", iint,
    "converts a struct of floats to the closest ints",
    iint(f + (double(f >= 0) - 0.5)))

BUILTIN_OVERLOAD(fraction_float, "fraction", "f", "F", "F",
    "returns the fractional part of a float: short for f - floor(f)")
(VM &, double a) { return a - floor(a); }
VECTOROP1234(fraction_fvec, "fraction", "v", "F", double, "F", double,
    "returns the fractional part of a struct of floats",
    f - floor(f))

BUILTIN_OVERLOAD(float_int, "float", "i", "I", "F",
    "converts an int to float")
(VM &, iint a) { return double(a); }
VECTOROP1234(float_ivec, "float", "v", "I", iint, "F", double,
    "converts a struct of ints to floats",
    double(f))

BUILTIN_OVERLOAD(sin_float, "sin", "angle", "F", "F",
    "the y coordinate of the normalized vector indicated by angle (in degrees)")
(VM &, double a) { return sin(a * RAD_D); }
VECTOROP1234(sin_fvec, "sin", "angle", "F", double, "F", double,
    "the y coordinates of the normalized vector indicated by the angles (in degrees)",
    sin(f * RAD_D))
BUILTIN_OVERLOAD(cos_float, "cos", "angle", "F", "F",
    "the x coordinate of the normalized vector indicated by angle (in degrees)")
(VM &, double a) { return cos(a * RAD_D); }
VECTOROP1234(cos_fvec, "cos", "angle", "F", double, "F", double,
    "the x coordinates of the normalized vector indicated by the angles (in degrees)",
    cos(f * RAD_D))
BUILTIN_OVERLOAD(tan_float, "tan", "angle", "F", "F",
    "the tangent of an angle (in degrees)")
(VM &, double a) { return tan(a * RAD_D); }
VECTOROP1234(tan_fvec, "tan", "angle", "F", double, "F", double,
    "the tangents of the angles (in degrees)",
    tan(f * RAD_D))

BUILTIN_V(sincos, "angle", "F", "F}:2",
    "the normalized vector indicated by angle (in degrees), same as float2 { cos(angle), sin(angle) }")
(StackPtr &sp, VM &, double a) {
    PushVec(sp, double2(cos(a * RAD_D), sin(a * RAD_D)));
}

BUILTIN(asin, "y", "F", "F",
    "the angle (in degrees) indicated by the y coordinate projected to the unit circle")
(VM &, double y) { return asin(y) / RAD_D; }
BUILTIN(acos, "x", "F", "F",
    "the angle (in degrees) indicated by the x coordinate projected to the unit circle")
(VM &, double x) { return acos(x) / RAD_D; }
BUILTIN(atan, "x", "F", "F",
    "the angle (in degrees) indicated by the y coordinate of the tangent projected to the unit circle")
(VM &, double x) { return atan(x) / RAD_D; }

BUILTIN(radians, "angle", "F", "F",
    "converts an angle in degrees to radians")
(VM &, double a) { return a * RAD_D; }
BUILTIN(degrees, "angle", "F", "F",
    "converts an angle in radians to degrees")
(VM &, double a) { return a / RAD_D; }

BUILTIN(atan2, "vec", "F}:2" , "F",
    "the angle (in degrees) corresponding to a normalized 2D vector")
(VM &, double2 vec) {
    auto v = ToVec<double2>(vec);
    return atan2(v.y, v.x) / RAD_D;
}

VECVEC1234(normalize_f, "normalize", "vec", "F", double,
    "returns a vector of unit length",
    (squaredlength(v) == 0.0 ? decltype(v)(0.0) : normalize(v)))

VEC2MATH1234(dot_f, "dot", "a,b", "F", double, "F",
    "the length of vector a when projected onto b (or vice versa)", dot(a, b))

VECMATH1234(magnitude_f, "magnitude", "v", "F", double, "F",
    "the geometric length of a vector", length(v))

VECMATH1234(magnitude_squared_fvec, "magnitude_squared", "v", "F", double, "F",
    "the geometric length of a vector squared", squaredlength(v))

VECMATH1234(magnitude_squared_ivec, "magnitude_squared", "v", "I", iint, "I",
    "the geometric length of a vector squared", squaredlength(v))

VECMATH1234(manhattan_i, "manhattan", "v", "I", iint, "I",
    "the manhattan distance of a vector", manhattan(v))

BUILTIN_V(cross, "a,b", "F}:3F}:3", "F}:3",
    "a perpendicular vector to the 2D plane defined by a and b (swap a and b for its inverse)")
(StackPtr &sp, VM &, double3 a_, double3 b_) {
    auto b = ToVec<double3>(b_);
    auto a = ToVec<double3>(a_);
    PushVec(sp, cross(a, b));
}

VECMATH1234(volume_fvec, "volume", "v", "F", double, "F",
    "the volume of the area spanned by the vector", v.volume())

VECMATH1234(volume_ivec, "volume", "v", "I", iint, "I",
    "the volume of the area spanned by the vector", v.volume())

BUILTIN_OVERLOAD(rnd_int, "rnd", "max", "I", "I",
    "a random value [0..max).")
(VM &vm, iint a) {
    return vm.rndx[vm.active_rng].rnd_int64(std::max((iint)1, a));
}

VECTOROPVM1234(rnd_ivec, "rnd", "max", "I", iint, "I", iint,
    "a random struct within the range of an input struct.",
    vm.rndx[vm.active_rng].rnd_int64(std::max((iint)1, f)))

BUILTIN(rnd_float, "", "", "F",
    "a random float [0..1)")
(VM &vm) {
    return vm.rndx[vm.active_rng].rnd_double();
}

BUILTIN(rnd_gaussian, "", "", "F",
    "a random float in a gaussian distribution with mean 0 and stddev 1")
(VM &vm) {
    return vm.rndx[vm.active_rng].rnd_gaussian();
}

BUILTIN(rnd_seed, "seed", "I", "",
    "explicitly set a random seed for reproducable randomness")
(VM &vm, iint seed) {
    vm.rndx[vm.active_rng].seed(seed);
}

BUILTIN(rnd_select, "index", "I", "I",
    "select a different random number generator to be active. default is 0, max is 1000000."
    " returns previous value.")
(VM &vm, iint i) {
    if (i > 1000000) vm.BuiltinError("rnd_select: too many random number generators");
    auto old = vm.active_rng;
    vm.active_rng = (size_t)i;
    if (vm.active_rng >= vm.rndx.size())
        vm.rndx.resize(vm.active_rng + 1, RandomNumberGenerator<Xoshiro256SS>());
    return old;
}

BUILTIN(rndm, "max", "I", "I",
    "deprecated: old mersenne twister version of the above for backwards compat.")
(VM &vm, iint a) {
    return vm.rndm.rnd_int(std::max(1, (int)a));
}

BUILTIN(rndm_seed, "seed", "I", "",
    "deprecated: old mersenne twister version of the above for backwards compat.")
(VM &vm, iint seed) {
    vm.rndm.seed((int)seed);
}

BUILTIN(div, "a,b", "II", "F",
    "forces two ints to be divided as floats")
(VM &, iint a, iint b) { return double(a) / double(b); }

BUILTIN_OVERLOAD(clamp_int, "clamp", "x,min,max", "III", "I",
    "forces an integer to be in the range between min and max (inclusive)")
(VM &, iint a, iint b, iint c) {
    return geom::clamp(a, b, c);
}

BUILTIN_OVERLOAD(clamp_float, "clamp", "x,min,max", "FFF", "F",
    "forces a float to be in the range between min and max (inclusive)")
(VM &, double a, double b, double c) {
    return geom::clamp(a, b, c);
}

VEC3VEC1234(clamp_ivec, "clamp", "x,min,max", "I", iint,
    "forces an integer struct to be in the range between min and max (inclusive)",
    clamp(a, b, c))

VEC3VEC1234(clamp_fvec, "clamp", "x,min,max", "F", double,
    "forces a float struct to be in the range between min and max (inclusive)",
    clamp(a, b, c))

BUILTIN_OVERLOAD(in_range_int, "in_range", "x,range,bias", "III?", "B",
    "checks if an integer is >= bias and < bias + range. Bias defaults to 0.")
(VM &, iint x, iint range, iint bias) {
    return x >= bias && x < bias + range;
}

BUILTIN_OVERLOAD(in_range_float, "in_range", "x,range,bias", "FFF?", "B",
    "checks if a float is >= bias and < bias + range. Bias defaults to 0.")
(VM &, double x, double range, double bias) {
    return x >= bias && x < bias + range;
}

// A left out bias is a struct of zeroes the typechecker adds, so it is always there.
#define INRANGEW(sym, D, T, CT, W) \
    BUILTIN_OVERLOAD(sym##W, "in_range", "x,range,bias", \
        T "}:" #W T "}:" #W "1" T "}:" #W "1?", "B", \
        "checks if a " #W "d " D " vector is >= bias and < bias + range." \
        " Bias defaults to 0.") \
    (VM &, vec<CT, W> x, vec<CT, W> range, vec<CT, W> bias) { \
        return in_range(x, range, bias); \
    }
INRANGEW(in_range_ivec, "integer", "I", iint, 2)
INRANGEW(in_range_ivec, "integer", "I", iint, 3)
INRANGEW(in_range_fvec, "float", "F", double, 2)
INRANGEW(in_range_fvec, "float", "F", double, 3)
#undef INRANGEW


BUILTIN_OVERLOAD(abs_int, "abs", "x", "I", "I",
    "absolute value of an integer")
(VM &, iint a) { return std::abs(a); }
BUILTIN_OVERLOAD(abs_float, "abs", "x", "F", "F",
    "absolute value of a float")
(VM &, double a) { return fabs(a); }
VECTOROP1234(abs_ivec, "abs", "x", "I", iint, "I", iint,
    "absolute value of an int vector",
    std::abs(f))
VECTOROP1234(abs_fvec, "abs", "x", "F", double, "F", double,
    "absolute value of a float vector",
    fabs(f))

BUILTIN_OVERLOAD(sign_int, "sign", "x", "I", "I",
    "sign (-1, 0, 1) of an integer")
(VM &, iint a) { return signum(a); }
BUILTIN_OVERLOAD(sign_float, "sign", "x", "F", "I",
    "sign (-1, 0, 1) of a float")
(VM &, double a) { return signum(a); }
VECTOROP1234(sign_ivec, "sign", "x", "I", iint, "I", iint,
    "signs of an int vector",
    signum(f))
VECTOROP1234(sign_fvec, "sign", "x", "F", double, "I", iint,
    "signs of a float vector",
    signum(f))

#define VECSCALAROP(type, init, fun, len, at) \
    type v = init; \
    auto l = x->len; \
    for (iint i = 0; i < l; i++) { \
        auto f = x->at; \
        fun; \
    } \
    return v;

BUILTIN_OVERLOAD(min_int, "min", "x,y", "II", "I",
    "smallest of 2 integers.")
(VM &, iint x, iint y) {
    return std::min(x, y);
}
BUILTIN_OVERLOAD(min_float, "min", "x,y", "FF", "F",
    "smallest of 2 floats.")
(VM &, double x, double y) {
    return std::min(x, y);
}
VEC2VEC1234(min_ivec, "min", "x,y", "I", iint,
    "smallest components of 2 int vectors", min(a, b))
VEC2VEC1234(min_fvec, "min", "x,y", "F", double,
    "smallest components of 2 float vectors", min(a, b))
VECMATH1234(min_of_ivec, "min", "v", "I", iint, "I",
    "smallest component of a int vector.", min(v))
VECMATH1234(min_of_fvec, "min", "v", "F", double, "F",
    "smallest component of a float vector.", min(v))
BUILTIN_OVERLOAD(min_of_int_vector, "min", "v", "I]", "I",
    "smallest component of a int vector, or INT64_MAX if length 0.")
(VM &, LVector *x) {
    VECSCALAROP(iint, INT64_MAX, v = std::min(v, f.ival()), len, AtS(i))
}
BUILTIN_OVERLOAD(min_of_float_vector, "min", "v", "F]", "F",
    "smallest component of a float vector, or DBL_MAX if length 0.")
(VM &, LVector *x) {
    VECSCALAROP(double, DBL_MAX, v = std::min(v, f.fval()), len, AtS(i))
}

BUILTIN_OVERLOAD(max_int, "max", "x,y", "II", "I",
    "largest of 2 integers.")
(VM &, iint x, iint y) {
    return std::max(x, y);
}
BUILTIN_OVERLOAD(max_float, "max", "x,y", "FF", "F",
    "largest of 2 floats.")
(VM &, double x, double y) {
    return std::max(x, y);
}
VEC2VEC1234(max_ivec, "max", "x,y", "I", iint,
    "largest components of 2 int vectors", max(a, b))
VEC2VEC1234(max_fvec, "max", "x,y", "F", double,
    "largest components of 2 float vectors", max(a, b))
VECMATH1234(max_of_ivec, "max", "v", "I", iint, "I",
    "largest component of a int vector.", max(v))
VECMATH1234(max_of_fvec, "max", "v", "F", double, "F",
    "largest component of a float vector.", max(v))
BUILTIN_OVERLOAD(max_of_int_vector, "max", "v", "I]", "I",
    "largest component of a int vector, or INT64_MIN if length 0.")
(VM &, LVector *x) {
    VECSCALAROP(iint, INT64_MIN, v = std::max(v, f.ival()), len, AtS(i))
}
BUILTIN_OVERLOAD(max_of_float_vector, "max", "v", "F]", "F",
    "largest component of a float vector, or -DBL_MAX if length 0.")
(VM &, LVector *x) {
    VECSCALAROP(double, -DBL_MAX, v = std::max(v, f.fval()), len, AtS(i))
}

BUILTIN(popcount, "x", "I", "I",
    "number of bits set in an integer")
(VM &, iint a) {
    return PopCount((uint64_t)a);
}

BUILTIN_OVERLOAD(lerp_float, "lerp", "x,y,f", "FFF", "F",
    "linearly interpolates between x and y with factor f [0..1]")
(VM &, double x, double y, double f) {
    return mix(x, y, (float)f);
}

#define LERPW(W) \
    BUILTIN_V_OVERLOAD(lerp_fvec##W, "lerp", "a,b,f", "F}:" #W "F}:" #W "1F", "F}:" #W, \
        "linearly interpolates between a and b vectors with factor f [0..1]") \
    (StackPtr &sp, VM &, vec<double, W> a, vec<double, W> b, double f) { \
        auto r = vec<double, W>(0.0); \
        for (int i = 0; i < W; i++) r.c[i] = mix(a.c[i], b.c[i], (float)f); \
        PushVec(sp, r); \
    }
LERPW(2) LERPW(3) LERPW(4)
#undef LERPW

BUILTIN_V(spherical_lerp, "a,b,f", "F}:4F}:4F", "F}:4",
    "spherically interpolates between a and b quaternions with factor f [0..1]")
(StackPtr &sp, VM &, double4 a, double4 b, double f_) {
    auto f = (float)f_;
    auto y = ToVec<quat>(b);
    auto x = ToVec<quat>(a);
    PushVec(sp, spherical_lerp(x, y, f));
}

BUILTIN(smoothmin, "x,y,k", "FFF", "F",
    "k is the influence range")
(VM &, double x, double y, double k) {
    return smoothmin((float)x, (float)y, (float)k);
}

BUILTIN_OVERLOAD(smoothstep_x, "smoothstep", "x", "F", "F",
    "input must be in range 0..1, https://en.wikipedia.org/wiki/Smoothstep")
(VM &, double x) {
    return smoothstep((float)x);
}

BUILTIN_OVERLOAD(smoothstep_abf, "smoothstep", "a,b,f", "FFF", "F",
    "hermite interpolation between a and b by f [0..1], https://registry.khronos.org/OpenGL-Refpages/gl4/html/smoothstep.xhtml")
(VM &, double a, double b, double f) {
    return smoothstep((float)a, (float)b, (float)f);
}

BUILTIN(smootherstep, "x", "F", "F",
    "input must be in range 0..1, https://en.wikipedia.org/wiki/Smoothstep")
(VM &, double x) {
    return smootherstep((float)x);
}

#define CARDINALSPLINEW(W) \
    BUILTIN_V_OVERLOAD(cardinal_spline_f##W, "cardinal_spline", "z,a,b,c,f,tension", \
        "F}:" #W "F}:" #W "1F}:" #W "1F}:" #W "1FF", "F}:" #W, \
        "computes the position between a and b with factor f [0..1], using z (before a) and c" \
        " (after b) to form a cardinal spline (tension at 0.5 is a good default)") \
    (StackPtr &sp, VM &, vec<double, W> z, vec<double, W> a, vec<double, W> b, \
     vec<double, W> c, double f, double t) { \
        PushVec(sp, cardinal_spline(ToVec<double3>(z), ToVec<double3>(a), \
                                    ToVec<double3>(b), ToVec<double3>(c), f, t), W); \
    }
CARDINALSPLINEW(2) CARDINALSPLINEW(3)
#undef CARDINALSPLINEW

BUILTIN_V(line_intersect, "line1a,line1b,line2a,line2b", "F}:2F}:2F}:2F}:2", "IF}:2",
    "computes if there is an intersection point between 2 line segments, with the point as"
    " second return value")
(StackPtr &sp, VM &, double2 line1a, double2 line1b, double2 line2a, double2 line2b) {
    auto l2b = ToVec<double2>(line2b);
    auto l2a = ToVec<double2>(line2a);
    auto l1b = ToVec<double2>(line1b);
    auto l1a = ToVec<double2>(line1a);
    double2 ipoint(0, 0);
    auto r = line_intersect(l1a, l1b, l2a, l2b, &ipoint);
    Push(sp,  r);
    PushVec(sp, ipoint);
}

BUILTIN(circles_within_range, "dist,positions,radiuses,positions2,radiuses2,gridsize", "FF}:2]F]F}:2]F]I}:2", "I]]",
    "Given a vector of 2D positions (and same size vectors of radiuses), returns a vector of"
    " vectors of indices (to the second set of positions and radiuses) of the circles that are"
    " within dist of eachothers radius. If the second set are [], the first set is used for"
    " both (and the self element is excluded)."
    " gridsize optionally specifies the size of the grid to use for accellerated lookup of nearby"
    " points. This is essential for the algorithm to be fast, too big or too small can cause slowdown."
    " Omit it, and a heuristic will be chosen for you, which is currently sqrt(num_circles) * 2 along"
    " each dimension, e.g. 100 elements would use a 20x20 grid."
    " Efficiency wise this algorithm is fastest if there is not too much variance in the radiuses of"
    " the second set and/or the second set has smaller radiuses than the first.")
(VM &vm, double qdist, LVector *positions1, LVector *radiuses1,
 LVector *positions2v, LVector *radiuses2v, iint2 gridsize) {
    auto ncelld = ToVec<iint2>(gridsize);
    auto radiuses2 = radiuses2v;
    auto positions2 = positions2v;
    if (!radiuses2->len) radiuses2 = radiuses1;
    if (!positions2->len) positions2 = positions1;
    if (ncelld.x <= 0 || ncelld.y <= 0)
        ncelld = iint2((iint)sqrtf(float(positions2->len + 1) * 4));
    if (radiuses1->len != positions1->len || radiuses2->len != positions2->len)
        vm.BuiltinError("circles_within_range: input vectors size mismatch");
    struct Node {
        double2 pos = double2_0;
        double rad = 0.0;
        iint idx = 0;
        Node *next = nullptr;
    };
    vector<Node> nodes(positions2->SLen(), Node());
    double maxrad = 0;
    double2 minpos = double2(DBL_MAX), maxpos(-DBL_MAX);
    for (ssize_t i = 0; i < positions2->SLen(); i++) {
        auto &n = nodes[i];
        auto p = ValueToF<2>(positions2->AtSt(i), positions2->width);
        minpos = min(minpos, p);
        maxpos = max(maxpos, p);
        n.pos = p;
        auto r = radiuses2->AtS(i).fval();
        maxrad = std::max(maxrad, r);
        n.rad = r;
        n.idx = i;
        n.next = nullptr;
    }
    vector<Node *> cells((ssize_t)(ncelld.x * ncelld.y), nullptr);
    auto wsize = max(maxpos - minpos, double2(0.0001f));  // Avoid either dim being 0.
    wsize *= 1.00001f;  // No objects may fall exactly on the far border.
    auto tocellspace = [&](const double2 &pos) {
        return iint2((pos - minpos) / wsize * double2(ncelld));
    };
    for (ssize_t i = 0; i < positions2->SLen(); i++) {
        auto &n = nodes[i];
        auto cp = tocellspace(n.pos);
        auto &c = cells[ssize_t(cp.x + cp.y * ncelld.x)];
        n.next = c;
        c = &n;
    }
    vector<iint> within_range;
    vector<LVector *> results(positions1->SLen(), nullptr);
    for (ssize_t i = 0; i < positions1->SLen(); i++) {
        auto pos = ValueToF<2>(positions1->AtSt(i), positions1->width);
        auto rad = radiuses1->AtS(i).fval();
        auto scanrad = rad + maxrad + qdist;
        auto minc = max(iint2_0, min(ncelld - 1, tocellspace(pos - scanrad)));
        auto maxc = max(iint2_0, min(ncelld - 1, tocellspace(pos + scanrad)));
        for (iint y = minc.y; y <= maxc.y; y++) {
            for (iint x = minc.x; x <= maxc.x; x++) {
                for (auto c = cells[(ssize_t)(x + y * ncelld.x)]; c; c = c->next) {
                    if (c->idx != i || positions1 != positions2) {
                        auto d = length(c->pos - pos) - rad - c->rad;
                        if (d < qdist) {
                            within_range.push_back(c->idx);
                        }
                    }
                }
            }
        }
        auto vec = (LVector *)vm.NewVec(0, (int)within_range.size(),
                                                TYPE_ELEM_VECTOR_OF_INT);
        for (auto i : within_range) vec->Push(vm, Value(i));
        within_range.clear();
        results[i] = vec;
    }
    auto rvec = (LVector *)vm.NewVec(0, positions1->len, TYPE_ELEM_VECTOR_OF_VECTOR_OF_INT);
    for (auto vec : results) rvec->Push(vm, Value(vec));
    return rvec;
}

BUILTIN_V(wave_function_collapse, "tilemap,size", "S]I}:2", "S]I",
    "returns a tilemap of given size modelled after the possible shapes in the input"
    " tilemap. Tilemap should consist of chars in the 0..127 range. Second return value"
    " the number of failed neighbor matches, this should"
    " ideally be 0, but can be non-0 for larger maps. Simply call this function"
    " repeatedly until it is 0")
(StackPtr &sp, VM &vm, LVector *tilemap, iint2 size) {
    auto sz = ToVec<int2>(size);
    auto rows = tilemap->SLen();
    vector<const char *> inmap(rows);
    iint cols = 0;
    for (ssize_t i = 0; i < rows; i++) {
        auto sv = tilemap->AtS(i).sval()->strv();
        if (i) {
            if (ssize(sv) != cols)
                vm.BuiltinError("all columns must be equal length");
        }
        else cols = sv.size();
        inmap[i] = sv.data();
    }
    auto outstrings = ToValueOfVectorOfStringsEmpty(vm, sz, 0);
    vector<char *> outmap(sz.y, nullptr);
    for (int i = 0; i < sz.y; i++) outmap[i] = (char *)outstrings->AtS(i).sval()->data();
    int num_contradictions = 0;
    auto ok = WaveFunctionCollapse(int2(iint2(cols, ssize(inmap))), inmap.data(), sz, outmap.data(),
                                   vm.rndx[vm.active_rng], num_contradictions);
    if (!ok)
        vm.BuiltinError("tilemap contained too many tile ids, or characters outside of 0..127 range");
    Push(sp,  outstrings);
    Push(sp,  num_contradictions);
}

BUILTIN_OVERLOAD(hash_int, "hash", "x", "I", "I",
    "hashes an int value into a positive int; may be the identity function")
(VM &vm, iint a) {
    auto h = positive_bits(Value(a).Hash(vm, RTT_INT));
    return h;
}
BUILTIN_OVERLOAD(hash_any, "hash", "x", "A", "I",
    "hashes any ref value into a positive int")
(VM &vm, Value a) {
    auto h = a.refnil() ? positive_bits(a.ref()->Hash(vm)) : (iint)0;
    return h;
}
BUILTIN_OVERLOAD(hash_function, "hash", "x", "L", "I",
    "hashes a function value into a positive int")
(VM &vm, Value a) {
    auto h = positive_bits(a.Hash(vm, RTT_FUNCTION));
    return h;
}
BUILTIN_OVERLOAD(hash_float, "hash", "x", "F", "I",
    "hashes a float value into a positive int")
(VM &vm, double a) {
    auto h = positive_bits(Value(a).Hash(vm, RTT_FLOAT));
    return h;
}
// The same hash as a struct of these values on the stack would get, see Value::Hash.
#define HASHW(sym, T, CT, W) \
    BUILTIN_OVERLOAD(sym##W, "hash", "v", T "}:" #W, "I", \
        "hashes a numeric struct into a positive int") \
    (VM &, vec<CT, W> v) { \
        auto h = SplitMix64Hash((uint64_t)W); \
        for (int i = 0; i < W; i++) h = h * 31 + SplitMix64Hash(ReadMem<uint64_t>(&v.c[i])); \
        return positive_bits(h); \
    }
HASHW(hash_ivec, "I", iint, 2)
HASHW(hash_ivec, "I", iint, 3)
HASHW(hash_ivec, "I", iint, 4)
HASHW(hash_fvec, "F", double, 2)
HASHW(hash_fvec, "F", double, 3)
HASHW(hash_fvec, "F", double, 4)
#undef HASHW

BUILTIN(call_function_value, "x", "L", "",
    "calls a void / no args function value.. you shouldn't need to use this, it is"
    " a demonstration of how native code can call back into Lobster")
(VM &vm, Value f) {
    vm.CallFunctionValue(f);
}

BUILTIN(type_id, "ref", "A", "I",
    "int uniquely representing the type of the given reference (object/vector/string/resource)."
    " this is the same as typeof, except dynamic (accounts for subtypes of the static type)."
    " useful to compare the types of objects quickly."
    " specializations of a generic type will result in different ids.")
(VM &, Value a) {
    return a.ref()->tti;
}

BUILTIN(type_string, "ref", "A", "S",
    "string representing the type of the given reference (object/vector/string/resource)")
(VM &vm, Value a) {
    return vm.NewString(a.ref()->TypeName(vm));
}

BUILTIN(type_element_string, "v", "A]*", "S",
    "string representing the type of the elements of a vector")
(VM &vm, LVector *a) {
    auto &ti = a->ti(vm);
    string sd;
    vm.GetTypeInfo(ti.subt).Print(vm, sd, nullptr);
    return vm.NewString(sd);
}

BUILTIN(type_field_count, "obj", "A", "I",
    "number of fields in an object, or 0 for other reference types")
(VM &vm, Value a) {
    auto &ti = a.ref()->ti(vm);
    return RTIsUDT(ti.t) ? ti.len : 0;
}

BUILTIN(type_field_string, "obj,idx", "AI", "S",
    "string representing the type of a field in an object, or empty for other reference types")
(VM &vm, Value a, iint i) {
    auto &ti = a.ref()->ti(vm);
    string sd;
    if (RTIsUDT(ti.t) && i >= 0 && i < ti.len) {
        vm.GetTypeInfo(ti.elemtypes[i].type).Print(vm, sd, nullptr);
    }
    return vm.NewString(sd);
}

BUILTIN(type_field_name, "obj,idx", "AI", "S",
    "name of a field in an object, or empty for other reference types")
(VM &vm, Value a, iint idx) {
    auto i = (int)idx;
    auto &ti = a.ref()->ti(vm);
    string sd;
    if (RTIsUDT(ti.t) && i >= 0 && i < ti.len) {
        sd = vm.LookupFieldByOffset(ti.structidx, i);
    }
    return vm.NewString(sd);
}

BUILTIN(type_field_value, "obj,idx", "AI", "S",
    "string representing the value of a field in an object, or empty for other reference types")
(VM &vm, Value a, iint i) {
    auto &ti = a.ref()->ti(vm);
    if (!RTIsUDT(ti.t) || i < 0 || i >= ti.len) return vm.NewString(0);
    auto &sti = vm.GetTypeInfo(ti.elemtypes[i].type);
    return vm.ToString(a.oval()->At(i), sti);
}

BUILTIN(type_enum_value_name, "enum_type_id,idx", "TI", "S",
    "string representing the name of an enum value, belonging to the enum (use typeof)")
(VM &vm, iint id, iint i) {
    auto &ti = vm.GetTypeInfo((type_elem_t)id);
    string sd;
    if (ti.t == RTT_INT && ti.enumidx >= 0) {
        vm.EnumName(sd, i, ti.enumidx);
    }
    return vm.NewString(sd);
}

BUILTIN(type_enum_value_valid, "enum_type_id,idx", "TI", "B",
    "whether an integer is a value of the given enum (use typeof), i.e. whether"
    " type_enum_value_name would give it a name, but without allocating a string. Cheap enough"
    " to range check a value that may come from elsewhere (such as a file written by a newer"
    " version of the program) before switching on it. For an enum_flags, any combination of"
    " declared bits is a value, so this is true for more values than a switch on it has cases")
(VM &vm, iint id, iint i) {
    auto &ti = vm.GetTypeInfo((type_elem_t)id);
    return ti.t == RTT_INT && ti.enumidx >= 0 && vm.EnumValueValid(i, ti.enumidx);
}

BUILTIN(program_name, "", "", "S",
    "returns the name of the main program (e.g. \"foo.lobster\"), \"\" if running from lpak.")
(VM &vm) {
    return vm.NewString(vm.GetProgramName());
}

BUILTIN(vm_compiled_mode, "", "", "B",
    "returns if the VM is running in compiled mode (Lobster -> C++), or false for JIT.")
(VM &) {
    return !VM_JIT_MODE;
}

BUILTIN(seconds_elapsed, "", "", "F",
    "seconds since program start as a float, unlike gl.time() it is calculated every time it is"
    " called")
(VM &vm) {
    return vm.Time();
}

BUILTIN(date_time, "utc", "B?", "I]",
    "a vector of integers representing date & time information (index with date_time.lobster)."
    " By default returns local time, pass true for UTC instead.")
(VM &vm, iint utc) {
    auto time = std::time(nullptr);
    const iint num_elems = 9;
    auto v = vm.NewVec(num_elems, num_elems, TYPE_ELEM_VECTOR_OF_INT);
    for (iint i = 0; i < num_elems; i++) v->AtSR(i) = -1;
    if (!time) return v;
    v->AtSR(0) = (iint)time; // unix epoch in seconds
    auto tm = (utc != 0) ? std::gmtime(&time) : std::localtime(&time);
    if (!tm) return v;
    v->AtSR(1) = tm->tm_year;
    v->AtSR(2) = tm->tm_mon;
    v->AtSR(3) = tm->tm_mday;
    v->AtSR(4) = tm->tm_yday;
    v->AtSR(5) = tm->tm_wday;
    v->AtSR(6) = tm->tm_hour;
    v->AtSR(7) = tm->tm_min;
    v->AtSR(8) = tm->tm_sec;
    return v;
}

BUILTIN(date_time_string, "utc", "B?", "S",
    "a string representing date & time information in the format: \'Www Mmm dd hh:mm:ss yyyy\'."
    " By default returns local time, pass true for UTC instead.")
(VM &vm, iint utc) {
    auto time = std::time(nullptr);
    if (!time) return vm.NewString("");
    auto tm = (utc != 0) ? std::gmtime(&time) : std::localtime(&time);
    if (!tm) return vm.NewString("");
    auto ts = std::asctime(tm);
    auto s = vm.NewString(string_view(ts, 24));
    return s;
}

BUILTIN(date_time_string_format, "format,utc", "SB?", "S",
    "a string representing date & time information using a formatting string according to"
    " https://en.cppreference.com/w/cpp/chrono/c/strftime, for example \"%Y_%m_%d_%H_%M_%S\"."
    " By default returns local time, pass true for UTC instead.")
(VM &vm, LString *fmt, iint utc) {
    auto time = std::time(nullptr);
    if (!time) return vm.NewString("");
    auto tm = (utc != 0) ? std::gmtime(&time) : std::localtime(&time);
    if (!tm) return vm.NewString("");
    const size_t max = 1024;
    char buf[max];
    auto sz = std::strftime(buf, max, fmt->strvnt().c_str(), tm);
    if (!sz) return vm.NewString("");
    auto s = vm.NewString(string_view(buf, sz));
    return s;
}

BUILTIN(date_time_build_info, "", "", "S",
    "a string representing information from when this program was compiled.")
(VM &vm) {
    auto s = vm.NewString(vm.BuildInfo());
    return s;
}

BUILTIN(get_stack_trace, "", "", "S",
    "gets a stack trace of the current location of the program (needs --runtime-stack-trace)"
    " without actually stopping the program.")
(VM &vm) {
    string sd;
    vm.DumpStackTrace(sd, false);
    return vm.NewString(sd);
}

BUILTIN(get_memory_usage, "n", "I", "S",
    "gets a text showing the top n object types that are using the most memory.")
(VM &vm, iint n) {
    return vm.NewString(vm.MemoryUsage((int)n));
}

BUILTIN(pass, "", "", "",
    "does nothing. useful for empty bodies of control structures.")
(VM &) {
}

BUILTIN(reference_count, "val", "A", "I",
    "get the reference count of any value. for compiler debugging, mostly")
(VM &, Value x) {
    auto refc = x.refnil() ? x.refnil()->refc - 1 : -1;
    return refc;
}

BUILTIN(set_console, "on", "B", "",
    "lets you turn on/off the console window (on Windows)")
(VM &, iint x) {
    SetConsole((x != 0));
}

BUILTIN(set_output_level, "level", "I", "",
    "0 = debug, 1 = verbose, 2 = warn (default), 3 = error, 4 = program")
(VM &, iint x) {
    // Do "min", so we can override even lower from command-line.
    min_output_level = std::min(min_output_level, (OutputType)(int)x);
}

BUILTIN(set_exit_code, "code", "I", "",
    "this will be returned when run as a console application")
(VM &vm, iint x) {
    vm.evalret.second = x;
}

BUILTIN(command_line_arguments, "", "", "S]",
    "")
(VM &vm) {
    return ToValueOfVectorOfStrings(vm, vm.vma.program_args);
}

BUILTIN(thread_information, "", "", "II",
    "returns the number of hardware threads, and the number of cores")
(StackPtr &sp, VM &) {
    Push(sp,  NumHWThreads());
    return NumHWCores();
}

BUILTIN(is_worker_thread, "", "", "B",
    "whether the current thread is a worker thread")
(VM &vm) {
    return vm.is_worker;
}

BUILTIN(start_worker_threads, "numthreads", "I", "",
    "launch worker threads")
(VM &vm, iint n) {
    vm.StartWorkers(n);
}

BUILTIN(stop_worker_threads, "", "", "",
    "only needs to be called if you want to stop the worker threads before the end of"
            " the program, or if you want to call start_worker_threads again. workers_alive"
            " will become false inside the workers, which should then exit.")
(VM &vm) {
    vm.TerminateWorkers();
}

BUILTIN(workers_alive, "", "", "B",
    "whether workers should continue doing work. returns false after"
            " stop_worker_threads() has been called.")
(VM &vm) {
    return vm.tuple_space && vm.tuple_space->alive;
}

BUILTIN(thread_write, "object", "A", "",
    "put this object in the thread queue")
(VM &vm, Value s) {
    vm.WorkerWrite(s.refnil());
}

BUILTIN(thread_read, "type", "T", "A1?",
    "get an object from the thread queue. pass the typeof object. blocks if no such"
    " objects available. returns object, or nil if this was the result of thread_wake()"
    " or stop_worker_threads() was called")
(VM &vm, iint t) {
    return vm.WorkerRead((type_elem_t)t);
}

BUILTIN(thread_check, "type", "T", "A1?",
    "tests if an object is available on the thread queue. pass the typeof object. "
    "returns object, or nil if none available, or if stop_worker_threads() was called")
(VM &vm, iint t) {
    return vm.WorkerCheck((type_elem_t)t);
}

BUILTIN(thread_wake, "type", "T", "",
    "wakes up one thread that are currently blocked on a thread_read for this type. "
    "this will cause them to return nil since no object is sent. "
    "it is similar to thread_write(nil)")
(VM &vm, iint t) {
    vm.WorkerWake((type_elem_t)t);
}

BUILTIN(crash_test_cpp_nullptr_exception, "", "", "",
    "only for testing crash dump functionality, don\'t use! :)")
(VM &) {
    char *volatile crash = nullptr;
    *crash = 0;
}

// See also the similar im.show_profiling_stats.
BUILTIN(profiler_dump_stats, "num,histogram", "IB", "S",
    "returns a string with CSV data for the num top items")
(VM &vm, iint i, iint histogram) {
    string s;
    #if LOBSTER_FRAME_PROFILER == 1
        auto &prof_db = prof_db_thread_local;
        vector<pair<const struct ___tracy_source_location_data *, ProfStat *>> display;
        for (auto &it : prof_db.stats) {
            display.push_back({ it.first, &it.second });
        }
        sort(display.begin(), display.end(),
             [](pair<const struct ___tracy_source_location_data *, ProfStat *> &a,
                pair<const struct ___tracy_source_location_data *, ProfStat *> &b) -> bool {
                 return a.second->time >= b.second->time;
            });
        s += "Function,AVG ms,HI ms,TOTAL sec,COUNT\n";  // ,Histogram
        for (auto &it : display) {
            s += cat("\"", it.first->function, "\",",
                it.second->time * 1000.0 / it.second->n, ",",
                it.second->highest * 1000.0, ",",
                it.second->time, ",",
                it.second->n, "\n");
            if ((histogram != 0)) {
                // FIXME: output it.second->window somehow.
            }
            if (!--i) break;
        }
    #else
        (void)i;
        (void)histogram;
    #endif
    return vm.NewString(s);
}

BUILTIN(profiler_paused, "paused", "B", "",
    "")
(VM &, iint paused) {
    #if LOBSTER_FRAME_PROFILER == 1
        auto &prof_db = prof_db_thread_local;
        prof_db.paused = (paused != 0);
    #else
        (void)paused;
    #endif
}

BUILTIN(profiler_reset, "", "", "",
    "")
(VM &) {
    #if LOBSTER_FRAME_PROFILER == 1
        auto &prof_db = prof_db_thread_local;
        prof_db.stats.clear();
    #endif
}


#undef BUILTIN_GROUP
#undef BUILTIN_SYM
BuiltinGroup matrix_builtins;
#define BUILTIN_GROUP matrix_builtins
#define BUILTIN_SYM(name) builtin_matrix_##name

BUILTIN(multiply, "a,b", "F]F]", "F]",
    "input matrices must be 4x4 elements")
(VM &vm, LVector *av, LVector *bv) {
    if (av->len != 16 || bv->len != 16)
        vm.BuiltinError("matrix_multiply: input vectors must be length 16");
    auto r = vm.NewVec(16, 16, TYPE_ELEM_VECTOR_OF_FLOAT);
    InlineVec<double, 16> iva(av->Elems());
    InlineVec<double, 16> ivb(bv->Elems());
    InlineVec<double, 16> ivr(r->Elems(), false);
    (*(double4x4 *)ivr.vals) = (*(double4x4 *)iva.vals) * (*(double4x4 *)ivb.vals);
    ivr.CopyBack(r->Elems());
    return r;
}

BUILTIN(rotate_x, "angle", "F}:2", "F]",
    "")
(VM &vm, double2 angle_) {
    auto angle = ToVec<double2>(angle_);
    auto r = vm.NewVec(16, 16, TYPE_ELEM_VECTOR_OF_FLOAT);
    InlineVec<double, 16> ivr(r->Elems(), false);
    (*(double4x4 *)ivr.vals) = rotationX(angle);
    ivr.CopyBack(r->Elems());
    return r;
}

BUILTIN(rotate_y, "angle", "F}:2", "F]",
    "")
(VM &vm, double2 angle_) {
    auto angle = ToVec<double2>(angle_);
    auto r = vm.NewVec(16, 16, TYPE_ELEM_VECTOR_OF_FLOAT);
    InlineVec<double, 16> ivr(r->Elems(), false);
    (*(double4x4 *)ivr.vals) = rotationY(angle);
    ivr.CopyBack(r->Elems());
    return r;
}

BUILTIN(rotate_z, "angle", "F}:2", "F]",
    "")
(VM &vm, double2 angle_) {
    auto angle = ToVec<double2>(angle_);
    auto r = vm.NewVec(16, 16, TYPE_ELEM_VECTOR_OF_FLOAT);
    InlineVec<double, 16> ivr(r->Elems(), false);
    (*(double4x4 *)ivr.vals) = rotationZ(angle);
    ivr.CopyBack(r->Elems());
    return r;
}

BUILTIN(translation, "trans", "F}:3", "F]",
    "")
(VM &vm, double3 trans_) {
    auto trans = ToVec<double3>(trans_);
    auto r = vm.NewVec(16, 16, TYPE_ELEM_VECTOR_OF_FLOAT);
    InlineVec<double, 16> ivr(r->Elems(), false);
    (*(double4x4 *)ivr.vals) = translation(trans);
    ivr.CopyBack(r->Elems());
    return r;
}

}
