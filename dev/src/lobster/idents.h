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

#ifndef LOBSTER_IDENTS
#define LOBSTER_IDENTS

#include "lobster/natreg.h"

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

namespace lobster {

// FlatBuffers takes care of backwards compatibility of all metadata, but not of the C the
// compiler emits, so this needs to be bumped each time we change the format.
const int LOBSTER_METADATA_FORMAT_VERSION = 28;

struct NativeFun;
struct SymbolTable;

struct Node;
struct Block;

struct Function;
struct SubFunction;

struct SpecIdent;

struct UDT;

extern SlabAlloc *g_current_slaballoc;

#ifdef NDEBUG
    #define USE_CURRENT_SLABALLOCATOR \
        static void *operator new(size_t size) noexcept { \
            return g_current_slaballoc->alloc_small(size); \
        } \
        static void operator delete(void *ptr, size_t) noexcept { \
            return g_current_slaballoc->dealloc_small(ptr); \
        }
#else
    #define USE_CURRENT_SLABALLOCATOR
#endif

struct Ident : Named {
    size_t scopelevel;
    Line line;

    bool isprivate = false;

    // Not declared const but def only, exp may or may not be const.
    bool single_assignment = true;
    // Declared const.
    bool constant = false;
    // Not necessarily declared const but def only, exp is const.
    // NOTE: not exact, since only takes parsed constructor arguments into account, could have non-const
    // fields, but only used so far for reporting unused variables, so ok for now.
    bool static_constant = false;
    // Has been read at least once.
    bool read = false;
    // This is var contains a struct whose fields have been assigned to.
    // Instead of having this bool, we could set single_assignment to false, but since
    // they are set at very different times this is probably clearer for now.
    bool struct_field_assign = false;

    bool predeclaration = false;
    bool preferfree = false;
    // A for loop variable, which is not declared with let or var, so gets no suggestion to.
    bool loop_var = false;

    SpecIdent *cursid = nullptr;

    UnTypeRef giventype = (UnType *)nullptr;
    // For single-assignment top level vars only: the defining expression, so
    // the type of simple constant globals can be derived without having
    // typechecked their definition. Owned by the AST, so only valid during
    // typechecking.
    Node *toplevel_initializer = nullptr;

    Ident(string_view _name, int _idx, size_t _sl, Line &_line)
        : Named(_name, _idx), scopelevel(_sl), line(_line) {}

    void Assign(Lex &lex) {
        single_assignment = false;
        if (constant) lex.Report("variable " + name + " is constant");
    }

    void StructAssign(Lex &lex,Line *ln = nullptr) {
        struct_field_assign = true;
        if (constant) lex.Report("variable " + name + " is a constant struct, cannot modify its fields", ln);
    }

    Ident *Read() {
        read = true;
        return this;
    }

    flatbuffers::Offset<metadata::Ident> Serialize(flatbuffers::FlatBufferBuilder &fbb,
                                                   bool is_top_level) const {
        return metadata::CreateIdent(fbb, fbb.CreateString(name), constant, is_top_level);
    }
};

struct SharedField;
struct Define;

struct SpecIdent {
    Ident *id;
    TypeRef type;
    Lifetime lt = LT_UNDEF;
    int idx, sidx = -1;             // Into specidents, and into vm ordering.
    SubFunction *sf_def = nullptr;  // Where it is defined, including anonymous functions.
    bool used_as_freevar = false;   // determined in codegen.
    bool withtype = false;
    Node *constprop = nullptr;      // We are going to constant propagate this var, which avoids it being a freevar, and the optimizer will replace it.
    // For a borrowed parameter: the variable (and field path from it) the current call passed,
    // which is then where the fields of this parameter really live, see LValContext::Step and
    // TypeChecker::BindParamAliases. Null when the argument was not a variable or field path.
    const SpecIdent *alias_sid = nullptr;
    small_vector<SharedField *, 3> alias_derefs;
    // With the alias: where the argument of the current call sits in the call, so that when
    // what the parameter names gets written while it is in use, the caller can be made to
    // keep the value alive instead, see TypeChecker::KeepArgAlive.
    Node **arg_slot = nullptr;
    // A variable that borrows what it was initialized with rather than owning a reference of
    // its own (a single-assignment variable initialized from a variable, field or element,
    // or a for loop element), as long as nothing writes to what it borrows from while it is
    // alive: a write that does makes it own after all, see TypeChecker::FlipSpeculative,
    // which uses the Define to give the initializer an inc. Null for a for loop element,
    // whose loop does the inc, see ForLoopElem::Generate.
    bool speculative = false;
    Define *spec_define = nullptr;

    SpecIdent(Ident *_id, TypeRef _type, int idx, bool withtype)
        : id(_id), type(_type), idx(idx), withtype(withtype) {}
    int Idx() const {
        assert(sidx >= 0);
        return sidx;
    }
    SpecIdent *&Current() {
        return id->cursid;
    }

    USE_CURRENT_SLABALLOCATOR
};

struct Enum;

struct EnumVal : Named {
    bool isprivate = false;
    int64_t val = 0;
    Enum *e = nullptr;

    EnumVal(string_view _name, int _idx) : Named(_name, _idx) {}
};

struct Enum : Named {
    bool isprivate = false;
    vector<unique_ptr<EnumVal>> vals;
    Type thistype;
    bool flags = false;

    Enum(string_view _name, int _idx) : Named(_name, _idx) {
        thistype = Type{ this };
    }

    EnumVal *Lookup(int64_t q) {
        for (auto &v : vals)
            if (v.get()->val == q) return v.get();
        return nullptr;
    }

    flatbuffers::Offset<metadata::Enum> Serialize(flatbuffers::FlatBufferBuilder &fbb) {
        vector<flatbuffers::Offset<metadata::EnumVal>> valoffsets;
        for (auto &v : vals)
            valoffsets.push_back(metadata::CreateEnumVal(fbb, fbb.CreateString(v->name), v->val));
        return metadata::CreateEnum(fbb, fbb.CreateString(name), fbb.CreateVector(valoffsets),
                                    flags);
    }
};

// Only still needed because we have no idea which struct it refers to at parsing time.
struct SharedField : Named {
    SharedField(string_view _name, int _idx) : Named(_name, _idx) {}
    SharedField() : SharedField("", 0) {}
};

// Stands for an element in a path of derefs (see LValContext), where indexing a vector sits.
// Indexing with a constant gets a marker of its own per value, so elements with different
// constant indices are different locations; anything else is "any element", which the borrow
// check treats as the same location as every element (see LValContext::IsPrefix). All markers
// have idx -1, which is what tells them apart from real fields.
inline SharedField elem_field { "[..]", -1 };
inline SharedField *ElemField(int64_t i) {
    // Map nodes never move, so the pointers stay valid for the life of the program.
    static map<int64_t, SharedField> fields;
    auto it = fields.find(i);
    if (it == fields.end()) it = fields.try_emplace(i, cat("[", i, "]"), -1).first;
    return &it->second;
}
inline bool IsElemField(const SharedField *f) { return f->idx == -1; }

struct Overload;

struct Field {
    SharedField *id;
    UnTypeRef giventype;
    // Inherited fields share the declaration's default, including the lexical bindings
    // the declchecker adds there. Typechecking and construction only work on clones.
    shared_ptr<Node> gdefaultval;
    bool isprivate;
    bool in_scope;  // Whether the `member` statement declaring it has been reached.
    // The method a `member` declared this in, null for an ordinary field. Only
    // that method (and anything lexically inside it) may access it.
    Overload *member_of = nullptr;
    // The storage width given as `int<8>` / `float<32>`, 0 for a whole slot. Only the
    // storage: a value read from the field is a plain int/float, see SField::bits.
    int bits = 0;
    Line defined_in;

    Field(SharedField *_id, UnTypeRef _type, Node *_gdefaultval, bool isprivate,
          bool in_scope, const Line &defined_in);
};

struct SField {
    TypeRef type;
    Node *defaultval = nullptr;
    int slot = -1;
    // A field stored in part of its slot (see Field::bits): how many bits, at which bit of
    // the slot, which it then shares with the other such fields there. The slot is held as
    // an int whatever the fields in it are (see SlotTypeOf), a float field being the bits
    // of the 32-bit float in it. 0 bits is a field that is its whole slot.
    int bits = 0;
    int bitoff = 0;

    bool isfloat() const { return type->t == V_FLOAT; }
};

struct TypeVariable {
    string_view name;
    UnType thistype;

    TypeVariable(string_view name) : name(name), thistype(this) {}
};

struct GenericTypeVariable {
    TypeRef type;
    TypeVariable *tv;

    GenericTypeVariable() : tv(nullptr) {}
    GenericTypeVariable(TypeRef _type, TypeVariable *_tv) : type(_type), tv(_tv) {}
};

struct DispatchEntry {
    SubFunction *sf = nullptr;          // if !is_switch_dispatch
    int case_index = -1;                // if is_switch_dispatch
    bool is_switch_dispatch = false;
    UDT *dispatch_root = nullptr;
    // Shared return type if root of dispatch.
    TypeRef returntype = nullptr;
    int returned_thru_to_max = -1;
    size_t subudts_size = 0;  // At time of creation.
    int vtable_idx = -1;
};

// This contains the declaration-side stuff for any UDT, and may contain
// generics and generic types.
struct GUDT : Named {
    Line line;
    vector<GenericTypeVariable> generics;
    UDT *first = nullptr;  // Specializations
    vector<Field> fields;
    UnTypeRef gsuperclass;
    bool is_struct = false;
    bool is_abstract = false;
    bool isprivate = false;
    bool predeclaration = false;
    bool has_subclasses = false;
    bool has_constructor_function = false;
    // On the root of an abstract struct family (see UDT::family_root): whether any struct in
    // the family can have a field holding a reference, decided over all their declarations
    // by the declchecker. All members are then structs of references, since a value of any
    // of their types may hold any member, see DeclChecker::FinalizeFamilies.
    bool family_hasref = false;
    SpecUDT unspecialized;
    UnType unspecialized_type;
    map<string_view, string_view> attributes;

    GUDT(string_view _name, int _idx, bool is_struct, Line &line)
        : Named(_name, _idx),
          line(line),
          is_struct(is_struct),
          unspecialized(this),
          unspecialized_type(&unspecialized) {
    }

    bool IsGeneric() const {
        return !generics.empty();
    }

    int Has(SharedField *fld) {
        for (auto &uf : fields) {
            if (uf.id == fld) return int(&uf - &fields[0]);
        }
        return -1;
    }
};

// How far a UDT has progressed thru declaration resolving / typechecking.
// Consumers complete a UDT on demand (EnsureUDTChecked) rather than relying
// on the order declarations get visited, and can assert on this rather than
// ever observing partial state.
enum class UDTState {
    DECLARED,         // Specialization exists, sfields not (fully) filled in.
    FIELDS_RESOLVED,  // sfields filled, but inferred ones may be null,
                      // defaultvals not yet typechecked.
    CHECKED,          // Field types complete, defaultvals typechecked, sized.
};

// This is a fully specialized instance of a GUDT.
struct UDT : Named {
    GUDT &g;
    UDT *next = nullptr;  // Other specializations of this GUDT.
    vector<TypeRef> bound_generics;
    vector<SField> sfields;
    UDT *ssuperclass = nullptr;
    // For a struct with an abstract struct among its superclasses (itself included): the
    // topmost one, the root of the "family" whose members all share one slot layout, such
    // that a value of any of their types can hold any member, see SymbolTable::LayoutFamily.
    // Null for every other struct, and for classes.
    UDT *family_root = nullptr;
    // A family member's slots that are not fields: slot 0, which holds the type id of the
    // value's dynamic type, and the padding that lines its fields up with the family
    // layout (0 or nil, by the kind of the slot), see LayoutFamily.
    vector<SField> hidden_sfields;
    // Only on the root: the kind of value each slot after the type slot holds in every member
    // that has a field there (see SymbolTable::SlotKind), and whether the layout was decided
    // (after which it can't change, see LayoutFamily).
    string family_kinds;
    bool family_laid_out = false;
    bool family_in_progress = false;
    // Only on the root: the number of vtable entries per member, the members' vtables
    // sitting at that stride from the root's, by family index, see CodeGen::CodeGen.
    int family_vtable_stride = 0;
    // Only on the root: whether the type slot holds more than the family index (which is its
    // low 8 bits, see FamilyIndexOf): the first fields of the members that fit above it, see
    // LayoutFamily. Decided along with the layout, and part of it. What reads the index
    // masks it out only then, since the whole slot is the index otherwise.
    bool family_type_slot_shared = false;
    UDTState state = UDTState::DECLARED;
    bool in_forest = false;  // Present in the subudts of itself & superclasses.
    bool unnamed_specialization = false;
    Type thistype;  // convenient place to store the type corresponding to this.
    TypeRef sametype = type_undefined;  // If all fields are int/float, this allows vector ops.
    type_elem_t typeinfonon = (type_elem_t)-1;  // Runtime type.
    type_elem_t typeinfonil = (type_elem_t)-1;  // Runtime type.
    int numslots = -1;
    int vtable_start = -1;
    int serializable_id = -1;
    // Pre-order index over the inheritance forest (assigned in CodeGen), such
    // that the subtree of any UDT (including itself) occupies the contiguous
    // range [subtype_dfs, subtype_dfs_end].
    int subtype_dfs = -1;
    int subtype_dfs_end = -1;
    // Intrusive list of direct subclasses (linked up in CodeGen), to traverse
    // the inheritance forest for the above.
    UDT *first_subclass = nullptr;
    UDT *next_subclass = nullptr;
    vector<UDT *> subudts;  // Including self.
    string subudts_dispatched_where;
    // Subset of methods that participate in dynamic dispatch. Order in this table determines
    // vtable layout and is compatible with sub/super classes.
    // Multiple specializations of a method may be in here.
    // Methods whose dispatch can be determined statically for the current program do not end up
    // in here.
    vector<unique_ptr<DispatchEntry>> dispatch_table;

    UDT(string_view _name, int _idx, GUDT &g) : Named(_name, _idx), g(g) {
        thistype = g.is_struct ? Type { V_STRUCT_R, this } : Type { V_CLASS, this };
    }

    ~UDT();

    // ResolveFields and the declaration fixpoint both derive this same property. Keep it
    // in thistype, which every later pass reads, rather than in a second cached flag.
    bool UpdateStructType() {
        assert(g.is_struct);
        bool hasref = family_root && family_root->g.family_hasref;
        for (auto &sfield : sfields) {
            // A field still to be inferred may turn out to hold a reference.
            if (sfield.type.Null() || IsRefNil(sfield.type->t)) hasref = true;
        }
        auto kind = hasref ? V_STRUCT_R : V_STRUCT_S;
        if (thistype.t == kind) return false;
        const_cast<ValueType &>(thistype.t) = kind;
        return true;
    }

    vector<GenericTypeVariable> GetBoundGenerics() {
        auto generics = g.generics;
        for (auto [i, gtv] : enumerate(generics)) {
            gtv.type = { bound_generics[i] };
        }
        return generics;
    }

    // The pre-order index of a member of an abstract struct family within it (0 for the
    // root), which is what the type slot of a value holds (see SymbolTable::LayoutFamily),
    // such that the members below any member are a contiguous range of indices. Only
    // valid once the ids of the inheritance forest are assigned, see CodeGen::CodeGen.
    int FamilyIndex() const {
        assert(family_root && subtype_dfs >= 0);
        return subtype_dfs - family_root->subtype_dfs;
    }

    // Whether any field is stored in part of its slot, see SField::bits.
    bool HasPackedFields() const {
        for (auto &sfield : sfields) if (sfield.bits) return true;
        return false;
    }

    // Assigns the slots the fields occupy and the number of them, which for a member of an
    // abstract struct family is the family's layout, see SymbolTable::LayoutFamily. Returns
    // false for a struct that (transitively) contains itself.
    bool ComputeSizes(SymbolTable &st, int depth = 0);

    flatbuffers::Offset<metadata::UDT> Serialize(flatbuffers::FlatBufferBuilder &fbb) {
        vector<flatbuffers::Offset<metadata::Field>> fieldoffsets;
        for (auto [i, sfield] : enumerate(sfields))
            fieldoffsets.push_back(
                metadata::CreateField(fbb, fbb.CreateString(g.fields[i].id->name), sfield.slot,
                                      sfield.bitoff, sfield.bits));
        return metadata::CreateUDT(fbb, fbb.CreateString(name), idx, fbb.CreateVector(fieldoffsets),
                                   numslots, ssuperclass ? ssuperclass->idx : -1, typeinfonon);
    }
};

bool SpecUDT::IsGeneric() const {
    assert(specializers.size() == gudt->generics.size());
    return !gudt->generics.empty();
}

bool SpecUDT::Equal(const SpecUDT &o) const {
    if (gudt != o.gudt ||
        IsGeneric() != o.IsGeneric() ||
        specializers.size() != o.specializers.size()) return false;
    for (auto [i, s] : enumerate(specializers)) {
        if (!s->Equal(*o.specializers[i])) return false;
    }
    return true;
}

bool Type::Equal(const Type &o, bool allow_unresolved) const {
    if (this == &o) return true;
    if (t != o.t) {
        if (!allow_unresolved) return false;
        // Special case for V_UUDT, since sometime types are resolved in odd orders.
        // TODO: can the IsGeneric() be removed?
        switch (t) {
            case V_UUDT:
                return IsUDT(o.t) && spec_udt->gudt == &o.udt->g && !spec_udt->IsGeneric();
            case V_CLASS:
            case V_STRUCT_R:
            case V_STRUCT_S:
                return o.t == V_UUDT && o.spec_udt->gudt == &udt->g && !o.spec_udt->IsGeneric();
            default:
                return false;
        }
    }
    if (sub == o.sub) return true;  // Also compares sf/udt
    switch (t) {
        case V_VECTOR:
        case V_NIL:
            return sub->Equal(*o.sub, allow_unresolved);
        case V_UUDT:
            return spec_udt->Equal(*o.spec_udt);
        default:
            return false;
    }
}

// Can this type be upgraded to a different type by flow?
bool Type::FlowSensitive() const {
    switch (t) {
        case V_NIL:
            return true;
        case V_VECTOR:
            return sub->FlowSensitive();
        case V_CLASS:
            return udt->g.has_subclasses;
        case V_STRUCT_R:
        case V_STRUCT_S:
            // A value of an abstract struct family carries its dynamic type, like a class.
            return udt->family_root && udt->g.has_subclasses;
        // We can't be sure:
        case V_VAR:
        case V_TYPEVAR:
        case V_UUDT:
        case V_TUPLE:
            return true;
        default:
            return false;
    }
}

GUDT *GetGUDTAny(UnTypeRef type) {
    return type->t == V_UUDT ? type->spec_udt->gudt : (IsUDT(type->t) ? &type->udt->g : nullptr);
}

inline TypeRef SingleNonGenericSpecialization(GUDT &gudt) {
    // Not generic, so must have just 1 specialization:
    assert(!gudt.IsGeneric() && gudt.first && !gudt.first->next);
    return &gudt.first->thistype;
}

// Distance to the exact type "super".
int SuperDistance(const UDT *super, const UDT *subclass) {
    int dist = 0;
    for (auto t = subclass; t; t = t->ssuperclass) {
        if (t == super) return dist;
        dist++;
    }
    return -1;
}

int DistanceToSpecializedSuper(const GUDT *super, const UDT *subclass) {
    int dist = 0;
    for (auto t = subclass; t; t = t->ssuperclass) {
        for (auto s = super->first; s; s = s->next)
            if (t == s) return dist;
        dist++;
    }
    return -1;
}

int DistanceFromSpecializedSub(const UDT *super, const GUDT *subclass) {
    int dist = 0;
    for (auto t = subclass; t; t = GetGUDTAny(t->gsuperclass)) {
        for (auto u = t->first; u; u = u->next)
            if (u == super) return dist;
        dist++;
    }
    return -1;
}

const UDT *CommonSuperType(const UDT *a, const UDT *b) {
    if (a != b) {
        for (;;) {
            if (SuperDistance(a, b) >= 0) break;
            a = a->ssuperclass;
            if (!a) return nullptr;
        }
    }
    return a;
}

inline int ValWidth(TypeRef type) {
    assert(type->t != V_TUPLE);  // You need ValWidthMulti
    return IsStruct(type->t) ? type->udt->numslots : 1;
}

inline int ValWidthMulti(TypeRef type, size_t nvals) {
    int n = 0;
    for (size_t i = 0; i < nvals; i++) {
        n += ValWidth(type->Get(i));
    }
    return n;
}

inline const SField *FindSlot(const UDT &udt, int i) {
    for (auto &sfield : udt.sfields) {
        if (i >= sfield.slot && i < sfield.slot + ValWidth(sfield.type)) {
            return IsStruct(sfield.type->t) ? FindSlot(*sfield.type->udt, i - sfield.slot) : &sfield;
        }
    }
    for (auto &sfield : udt.hidden_sfields) {
        if (sfield.slot == i) return &sfield;
    }
    assert(false);
    return nullptr;
}

// The static type of slot i of a struct as the generated code holds it. In an abstract struct
// family (see SymbolTable::LayoutFamily) a slot holds a reference field of one member and a
// reference field of another type of the next, so it is held as any reference, and every
// use of it converts from and to that, wherever in the value of a family member the slot
// sits. A slot that holds fields of fewer bits than itself (see SField::bits) is an int,
// whatever those fields are. Other slots are the type of their field.
inline TypeRef SlotTypeOf(const UDT &udt, int i, bool in_family = false) {
    in_family = in_family || udt.family_root;
    for (auto &sfield : udt.sfields) {
        if (i >= sfield.slot && i < sfield.slot + ValWidth(sfield.type)) {
            if (IsStruct(sfield.type->t)) {
                return SlotTypeOf(*sfield.type->udt, i - sfield.slot, in_family);
            }
            if (sfield.bits) return type_int;
            return in_family && IsRefNil(sfield.type->t) ? WrapKnown(type_any, V_NIL)
                                                         : sfield.type;
        }
    }
    for (auto &sfield : udt.hidden_sfields) {
        if (sfield.slot == i) return sfield.type;
    }
    assert(false);
    return type_undefined;
}

// The root of the abstract struct family a struct belongs to: the topmost abstract struct
// among its superclasses, itself included, see UDT::family_root.
inline UDT *FamilyRootOf(UDT *udt) {
    if (!udt->g.is_struct) return nullptr;
    UDT *root = nullptr;
    for (auto u = udt; u; u = u->ssuperclass) {
        if (u->g.is_abstract) root = u;
    }
    return root;
}

// The same over declarations, which is what decides GUDT::family_hasref.
inline GUDT *FamilyRootOf(GUDT *gudt) {
    if (!gudt->is_struct) return nullptr;
    GUDT *root = nullptr;
    for (auto g = gudt; g; g = GetGUDTAny(g->gsuperclass)) {
        if (g->is_abstract) root = g;
    }
    return root;
}

// A struct in an abstract struct family: a value of it carries its dynamic type (in its type
// slot, see LayoutFamily), so it takes part in `switch`, `is` and dynamic dispatch like an
// object does.
inline bool IsFamilyStruct(TypeRef type) {
    return IsStruct(type->t) && type->udt->family_root;
}

// What those three work on.
inline bool IsDynamicType(TypeRef type) {
    return type->t == V_CLASS || IsFamilyStruct(type);
}

typedef small_vector<SharedField *, 3> FieldPath;

struct LValContext {
    // For now, only: ident ( . field )*.
    const SpecIdent *sid;
    FieldPath derefs;
    LValContext(SpecIdent *sid) : sid(sid) {}
    LValContext(const Node &n);
    bool IsValid() const { return sid; }
    bool HasElem() const {
        for (auto f : derefs) if (IsElemField(f)) return true;
        return false;
    }
    bool DerefsEqual(const LValContext &o) const {
        if (derefs.size() != o.derefs.size()) return false;
        for (auto &shf : derefs) if (shf != o.derefs[&shf - &derefs[0]]) return false;
        return true;
    }
    // Whether two derefs can be the same location: the same field, or elements unless both are
    // constant indices that differ.
    static bool MayAlias(const SharedField *a, const SharedField *b) {
        if (a == b) return true;
        if (!IsElemField(a) || !IsElemField(b)) return false;
        return a == &elem_field || b == &elem_field;
    }
    bool IsPrefix(const LValContext &o) {  // Is o a prefix of this?
        if (sid != o.sid || derefs.size() < o.derefs.size()) return false;
        for (auto &shf : o.derefs) if (!MayAlias(shf, derefs[&shf - &o.derefs[0]])) return false;
        return true;
    }
    string Name() {
        auto s = sid ? sid->id->name : "<invalid>";
        for (auto &shf : derefs) {
            if (!IsElemField(shf)) s += ".";
            s += shf->name;
        }
        return s;
    }
    // When the variable is a borrowed parameter that names what its caller passed, rename this
    // to the caller's path to the same location, so the paths of every function on the stack
    // that have a name for it become comparable, see TypeChecker::BindParamAliases.
    bool Step() {
        if (!sid || !sid->alias_sid) return false;
        // A variable that owns a reference (a speculative borrow that got flipped, see
        // SpecIdent::speculative) is a location of its own: what it holds cannot be freed
        // by a write to what it was initialized from. Its fields are still those of the
        // shared object.
        if (derefs.empty() && !IsBorrow(sid->lt)) return false;
        small_vector<SharedField *, 3> d;
        for (auto f : sid->alias_derefs) d.push_back(f);
        for (auto f : derefs) d.push_back(f);
        derefs = d;
        sid = sid->alias_sid;
        return true;
    }
    // The variable that really holds it, i.e. not a borrowed parameter.
    void Canonicalize() {
        // Bindings only ever point at a variable of a caller, so this terminates, and the
        // bound is only a safety net.
        for (int i = 0; i < 64 && Step(); i++) {}
    }
};

struct FlowItem : LValContext {
    TypeRef old, now;
    FlowItem(const Node &n, TypeRef type);
    FlowItem(SpecIdent *sid, TypeRef old, TypeRef now) : LValContext(sid), old(old), now(now) {}
    FlowItem(const LValContext &lv, TypeRef type) : LValContext(lv), old(type), now(type) {}
};

// A write to a reference, recorded for every function on the stack it happened inside of, so
// that when that function gets called again without being typechecked again, the write can be
// checked against the borrows and flow promotions of the new context, see
// TypeChecker::ReplayAssigns. The path is what the function it is recorded in calls the
// location: a variable it can see, or its own parameter that aliases the location.
struct AssignEvent {
    Node *n;
    LValContext lv;
};

struct Arg {
    TypeRef spec_type = type_undefined;
    SpecIdent *sid = nullptr;

    Arg() = default;
    Arg(const Arg &o) = default;
    Arg(SpecIdent *_sid, TypeRef _type)
        : spec_type(_type), sid(_sid) {}
};

struct Function;
struct SubFunction;

struct Caller {
    SubFunction *caller = nullptr;  // Null if this is the call to __top_level_expression
    DispatchEntry *de = nullptr;    // Null if static call.
};

struct ExplicitFreeVarSpec {
    TypeRef bound = nullptr;
    SpecIdent *sid = nullptr;
    SharedField *field = nullptr;  // May be null.
    bool used = false;
};

struct ExplicitFreeVar {
    string_view name;
    UnTypeRef giventype = (UnType *)nullptr;   // May be null if not specified.
    ExplicitFreeVarSpec spec;
};

struct Overload {
    SubFunction *sf = nullptr;
    vector<UnTypeRef> givenargs;
    vector<ExplicitFreeVar *> freevardecls;
    Block *gbody = nullptr;
    Line declared_at;
    bool isprivate;
    GUDT *method_of = nullptr;

    Overload(Line da, bool p) : declared_at(da), isprivate(p) {}

    ~Overload();

    int NumSubf();
};

struct SubFunction {
    int idx;
    vector<Arg> args;
    vector<Arg> locals;
    vector<Arg> freevars;       // any used from outside this scope
    vector<FlowItem> freevarflowfields;
    UnTypeRef returngiventype = (UnType *)nullptr;
    TypeRef returntype = type_undefined;
    size_t num_returns = 0;
    size_t num_returns_non_local = 0;
    size_t reqret = 0;  // Do the caller(s) want values to be returned?
    vector<pair<const SubFunction *, TypeRef>> reuse_return_events;
    vector<AssignEvent> reuse_assign_events;
    bool isrecursivelycalled = false;
    Block *sbody = nullptr;
    SubFunction *next = nullptr;
    Function *parent = nullptr;
    bool typechecked = false;
    bool freevarchecked = false;
    bool mustspecialize = false;
    bool isdynamicfunctionvalue = false;
    bool explicit_generics = false;
    int returned_thru_to_max = -1;  // >=0: there exist return statements that may skip the caller.
    vector<int> returned_thru_function_ids;
    UDT *method_of = nullptr;
    int numcallers = 0;
    Type thistype { V_FUNCTION, this };  // convenient place to store the type corresponding to this
    vector<GenericTypeVariable> generics;
    map<string_view, string_view> attributes;
    Overload *lexical_parent = nullptr;
    Overload *overload = nullptr;
    size_t node_count = 0;
    vector<Caller> callers;

    SubFunction(int _idx) : idx(_idx) {}

    void SetParent(Function &f, Overload &ov) {
        parent = &f;
        next = ov.sf;
        ov.sf = this;
        ov.sf->overload = &ov;
    }

    auto IterFreeVar(const SpecIdent &sid) {
        return std::lower_bound(freevars.begin(), freevars.end(), sid.id,
            [&](const Arg &e, Ident *id) {
                return e.sid->id < id;
            });
    }

    void AddFreeVar(vector<Arg>::iterator lower, SpecIdent &sid, TypeRef flowtype) {
        freevars.insert(lower, Arg(&sid, flowtype));
    }

    bool IsFreeVar(vector<Arg>::iterator lower, const SpecIdent &sid) {
        return lower != freevars.end() && lower->sid->id == sid.id;
    }

    ~SubFunction();
};

int Overload::NumSubf() {
    int sum = 0;
    for (auto csf = sf; csf; csf = csf->next) sum++;
    return sum;
}

struct Function : Named {
    // functions with the same name and args, but different types (dynamic dispatch |
    // specialization)
    vector<Overload *> overloads;
    // functions with the same name but different number of args (overloaded)
    Function *sibf = nullptr;
    Function *first = this;
    // does not have a programmer specified name
    bool anonymous = false;
    // its merely a function type, has no body, but does have a set return type.
    bool istype = false;
    GUDT *is_constructor_of = nullptr;

    size_t scopelevel;

    small_vector<Node *, 4> default_args;
    int first_default_arg = -1;

    Function(string_view _name, int _idx, size_t _sl)
        : Named(_name, _idx), scopelevel(_sl) {
    }

    ~Function();

    size_t nargs() const { return overloads[0]->sf->args.size(); }

    int NumSubf() {
        int sum = 0;
        for (auto ov : overloads) sum += ov->NumSubf();
        return sum;
    }

    bool RemoveSubFunction(SubFunction *sf) {
        for (auto [i, ov] : enumerate(overloads)) {
            for (auto sfp = &ov->sf; *sfp; sfp = &(*sfp)->next) {
                if (*sfp == sf) {
                    *sfp = sf->next;
                    sf->next = nullptr;
                    if (!ov->sf) {
                        delete overloads[i];
                        overloads.erase(overloads.begin() + i);
                    }
                    return true;
                }
            }
        }
        return false;
    }

    flatbuffers::Offset<metadata::Function> Serialize(flatbuffers::FlatBufferBuilder &fbb) const {
        return metadata::CreateFunction(fbb, fbb.CreateString(name));
    }
};

template<typename T> void UnregisterT(const T *x, unordered_map<string_view, T *> &dict) {
    auto it = dict.find(x->name);
    // A declaration that clashed with an existing one is not what is registered under its name.
    if (it != dict.end() && it->second == x) dict.erase(it);
}

template<typename T> void ErasePrivate(unordered_map<string_view, T *> &dict) {
    auto it = dict.begin();
    while (it != dict.end()) {
        auto n = it->second;
        it++;
        if (n->isprivate) UnregisterT(n, dict);
    }
}

template<> void ErasePrivate(unordered_map<string_view, UDT *> &dict) {
    auto it = dict.begin();
    while (it != dict.end()) {
        auto n = it->second;
        it++;
        if (n->g.isprivate) UnregisterT(n, dict);
    }
}

struct SymbolTable {
    Lex &lex;

    unordered_map<string_view, Ident *> idents;  // Key points to value!
    vector<Ident *> identtable;
    vector<Ident *> identstack;
    vector<SpecIdent *> specidents;

    unordered_map<string_view, UDT *> udts;  // Key points to value!
    unordered_map<string_view, GUDT *> gudts;  // Key points to value!
    vector<UDT *> udttable;
    vector<GUDT *> gudttable;

    unordered_map<string_view, SharedField *> fields;  // Key points to value!
    vector<SharedField *> fieldtable;

    unordered_map<string, vector<Function *>> functions;
    // All named functions incl. ones in nested scopes, grouped per name
    // (each entry a sibf chain head), filled in by the declchecker.
    unordered_map<string_view, vector<Function *>> functions_by_name;
    unordered_map<string_view, Function *> operators;  // Key points to value!
    vector<Function *> functiontable;
    vector<SubFunction *> subfunctiontable;
    SubFunction *toplevel = nullptr;

    unordered_map<string_view, Enum *> enums;  // Key points to value!
    unordered_map<string_view, EnumVal *> enumvals;  // Key points to value!
    vector<Enum *> enumtable;

    vector<TypeVariable *> typevars;
    vector<vector<GenericTypeVariable>> bound_typevars_stack;

    vector<size_t> scopelevels;

    // A scope in which the fields of an object can be named without it: a `::` argument or
    // loop variable, a `case T::`, or a pattern variable of a `case T(name)`. The object is
    // `id`, followed by the fields in `derefs` (a switch value that is a field path).
    struct WithStackElem {
        GUDT *gudt = nullptr;
        Ident *id = nullptr;
        SubFunction *sf = nullptr;
        UDT *udt_tc = nullptr;  // Only in TC.
        FieldPath derefs;
        // For a pattern variable: its name, and the one field of the object it stands for.
        // Otherwise empty/null, and all fields of `gudt` are in scope.
        string_view alias;
        SharedField *fld = nullptr;
        bool SameObject(const WithStackElem &o) const {
            if (id != o.id || derefs.size() != o.derefs.size()) return false;
            for (size_t i = 0; i < derefs.size(); i++) if (derefs[i] != o.derefs[i]) return false;
            return true;
        }
        // The object, as an expression that reads it.
        Node *Object(const Line &line) const;
    };
    vector<WithStackElem> withstack;
    vector<size_t> withstacklevels;

    enum { NUM_VECTOR_TYPE_WRAPPINGS = 3 };
    vector<TypeRef> default_int_vector_types[NUM_VECTOR_TYPE_WRAPPINGS],
                    default_float_vector_types[NUM_VECTOR_TYPE_WRAPPINGS];
    Enum *default_bool_type = nullptr;

    // Used during parsing.
    vector<SubFunction *> defsubfunctionstack;

    vector<Type *> typelist;  // Used for constructing new vector types, variables, etc.
    vector<UnType *> untypelist;
    vector<vector<Type::TupleElem> *> tuplelist;
    vector<SpecUDT *> specudts;

    string_view current_namespace;
    vector<string_view> namespace_stack;

    set<string_view> declared_explicit_free_variables;

    // FIXME: because we cleverly use string_view's into source code everywhere, we now have
    // no way to refer to constructed strings, and need to store them seperately :(
    // TODO: instead use larger buffers and constuct directly into those, so no temp string?
    vector<const char *> stored_names;

    function<void(UDT &)> type_check_call_back;

    SymbolTable(Lex &lex) : lex(lex) {
        type_check_call_back = [](UDT &) {};
        namespace_stack.push_back({});
    }

    ~SymbolTable() {
        for (auto id  : identtable)       delete id;
        for (auto sid : specidents)       delete sid;
        for (auto u   : udttable)         delete u;
        for (auto gu  : gudttable)        delete gu;
        for (auto f   : functiontable)    delete f;
        for (auto e   : enumtable)        delete e;
        for (auto sf  : subfunctiontable) delete sf;
        for (auto f   : fieldtable)       delete f;
        for (auto t   : typelist)         delete t;
        for (auto t   : untypelist)       delete t;
        for (auto t   : tuplelist)        delete t;
        for (auto n   : stored_names)     delete[] n;
        for (auto tv  : typevars)         delete tv;
        for (auto su  : specudts)         delete su;
    }

    bool MaybeNameSpace(string_view name) const {
        return !current_namespace.empty() && name.find(".") == name.npos;
    }

    string NameSpaced(string_view name, string_view ns) {
        return cat(ns, ".", name);
    }

    string NameSpaced(string_view name) {
        assert(MaybeNameSpace(name));
        return NameSpaced(name, current_namespace);
    }

    string_view StoreName(const string &s) {
        auto buf = new char[s.size()];
        memcpy(buf, s.data(), s.size());  // Look ma, no terminator :)
        stored_names.push_back(buf);
        return string_view(buf, s.size());
    }

    string_view MaybeMakeNameSpace(string_view name, bool other_conditions) {
        return other_conditions && scopelevels.size() == 1 && MaybeNameSpace(name)
            ? StoreName(NameSpaced(name))
            : name;
    }

    Ident *Lookup(string_view name) {
        if (MaybeNameSpace(name)) {
            auto it = idents.find(NameSpaced(name));
            if (it != idents.end()) return it->second->Read();
        }
        auto it = idents.find(name);
        if (it != idents.end()) return it->second->Read();
        return nullptr;
    }

    Ident *NewId(string_view name, SubFunction *sf, bool withtype, size_t scopelevel, Line &line) {
        auto ident = new Ident(name, (int)identtable.size(), scopelevel, line);
        ident->cursid = NewSid(ident, sf, withtype);
        identtable.push_back(ident);
        idents[ident->name /* must be in value */] = ident;
        identstack.push_back(ident);
        return ident;
    }

    Ident *LookupDefWS(string_view name) {
        WithStackElem *wse = nullptr;
        if (LookupWithStruct(name, wse)) {
            if (wse->alias.empty())
                lex.Report("cannot define variable with same name as field in this scope: " + name);
            else
                lex.Report(cat("identifier shadowing: ", name));
        }
        return Lookup(name);
    }

    Ident *LookupDef(string_view name, bool islocal, bool withtype) {
        auto ident = LookupDefWS(name);
        if (ident) {
            if (scopelevels.size() != ident->scopelevel)
                lex.Report(cat("identifier shadowing: ", name));
            else if (!ident->predeclaration)
                lex.Report(cat("identifier redefinition: ", name));
            return ident;
        }
        auto sf = defsubfunctionstack.back();
        ident = NewId(name, sf, withtype, scopelevels.size(), lex);
        (islocal ? sf->locals : sf->args).push_back(
            Arg(ident->cursid, type_undefined));
        return ident;
    }

    Ident *LookupDefStatic(string_view name) {
        auto ident = LookupDefWS(name);
        if (ident) {
            lex.Report(cat("identifier shadowing/redefinition: ", name));
            return ident;
        }
        auto sf = defsubfunctionstack[0];
        // Is going to get removed as if it was part of the current function.
        ident = NewId(name, sf, false, 1, lex);
        sf->locals.push_back(Arg(ident->cursid, type_undefined));
        return ident;
    }

    void AddWithStruct(GUDT *gudt, Ident *id, SubFunction *sf, const FieldPath &derefs = {}) {
        if (!gudt) {
            lex.Report(":: can only be used with struct/class types");
            return;
        }
        WithStackElem wse { gudt, id, sf, nullptr, derefs };
        for (auto &wp : withstack) {
            // The same object again (a `case T::` on a `::` variable) names the same fields
            // either way, so is not ambiguous.
            if (wp.gudt == gudt && wp.alias.empty() && !wp.SameObject(wse)) {
                lex.Report("type used twice in the same scope with ::");
                return;
            }
        }
        // FIXME: should also check if variables have already been defined in this scope that clash
        // with the struct, or do so in LookupUse
        withstack.push_back(wse);
    }

    void AddWithStructTT(TypeRef type, Ident *id, SubFunction *sf,
                         const FieldPath &derefs = {}) {
        assert(type->t != V_UUDT);
        withstack.push_back({ &type->udt->g, id, sf, type->udt, derefs });
    }

    // Declares `name` in the current scope as a pattern variable: another name for field
    // `fld` of the object `id`/`derefs` (see WithStackElem). Like a variable, it may not
    // shadow anything.
    void AddPatternVar(string_view name, GUDT *gudt, SharedField *fld, Ident *id,
                       SubFunction *sf, const FieldPath &derefs) {
        WithStackElem *wse = nullptr;
        if (LookupWithStruct(name, wse)) {
            if (wse->alias.empty())
                lex.Report("cannot define variable with same name as field in this scope: " + name);
            else if (size_t(wse - withstack.data()) >= withstacklevels.back())
                lex.Report(cat("identifier redefinition: ", name));
            else
                lex.Report(cat("identifier shadowing: ", name));
        } else if (Lookup(name)) {
            lex.Report(cat("identifier shadowing: ", name));
        }
        withstack.push_back({ gudt, id, sf, nullptr, derefs, name, fld });
    }

    // The field `name` stands for in this scope, if any: that of a pattern variable, or one
    // of a type in scope thru `::`, with the entry providing it. A pattern variable shadows
    // whatever is declared outside it, while two types in scope with the same field make
    // it ambiguous, unless they are the same object.
    SharedField *LookupWithStruct(string_view name, WithStackElem *&found) {
        assert(!found);
        auto fld = FieldUse(name);
        for (auto &wse : reverse(withstack)) {
            if (!wse.alias.empty()) {
                if (wse.alias != name) continue;
                if (!found) found = &wse;
                break;
            }
            if (!fld || wse.gudt->Has(fld) < 0) continue;
            if (!found) found = &wse;
            else if (!found->SameObject(wse)) lex.Report("access to ambiguous field: " + fld->name);
        }
        return found ? (found->fld ? found->fld : fld) : nullptr;
    }

    void BlockScopeStart() {
        scopelevels.push_back(identstack.size());
        withstacklevels.push_back(withstack.size());
    }

    void BlockScopeCleanup() {
        while (identstack.size() > scopelevels.back()) {
            auto ident = identstack.back();
            auto it = idents.find(ident->name);
            if (it != idents.end()) {  // can already have been removed by private var cleanup
                idents.erase(it);
            }
            identstack.pop_back();
        }
        scopelevels.pop_back();
        while (withstack.size() > withstacklevels.back()) withstack.pop_back();
        withstacklevels.pop_back();
    }

    SubFunction *FunctionScopeStart() {
        BlockScopeStart();
        auto sf = CreateSubFunction();
        if (!defsubfunctionstack.empty())
            sf->lexical_parent = defsubfunctionstack.back()->parent->overloads.back();
        defsubfunctionstack.push_back(sf);
        return sf;
    }

    void FunctionScopeCleanup(size_t count) {
        auto sf = defsubfunctionstack.back();
        sf->node_count = count;
        defsubfunctionstack.pop_back();
        BlockScopeCleanup();
    }

    template<typename F> void PopOutOfFunctionScope(F f) {
        // This is a bit of a hack, but we have the need to parse default expressions while
        // already inside a function scope, but that exp shouldn't touch the function scope.
        // So temp remove and put back is the easiest way around that, since starting the scope
        // later would affect the args being parsed (if this ever is a problem, could attempt to
        // parse those without processing first).
        auto sf = defsubfunctionstack.back();
        auto sl = scopelevels.back();
        auto ws = withstacklevels.back();
        defsubfunctionstack.pop_back();
        scopelevels.pop_back();
        withstacklevels.pop_back();
        f();
        withstacklevels.push_back(ws);
        scopelevels.push_back(sl);
        defsubfunctionstack.push_back(sf);
    }

    void UnregisterEnum(const Enum *e) {
        auto it = enums.find(e->name);
        // A declaration that clashed with an existing one is not what is registered under its
        // name, and neither are its values, see EnumLookup.
        if (it == enums.end() || it->second != e) return;
        for (auto &ev : e->vals) {
            auto evit = enumvals.find(ev->name);
            if (evit != enumvals.end() && evit->second == ev.get()) enumvals.erase(evit);
        }
        enums.erase(it);
    }

    void Unregister(const Function *f) {
        auto it = functions.find(f->name);
        if (it != functions.end() && !it->second.empty() && it->second.back() == f) {
            it->second.pop_back();
        }
    }

    void StartOfInclude() {
        namespace_stack.push_back(current_namespace);
        current_namespace = {};
    }

    void EndOfInclude() {
        current_namespace = namespace_stack.back();
        namespace_stack.pop_back();
        ErasePrivate(idents);
        ErasePrivate(gudts);
        ErasePrivate(udts);
        ErasePrivate(enums);
        // Note: can't remove functions here, because final function lookup is in typechecker.
    }

    // A declaration that clashes with an existing one gets parsed into an enum (or value) of its
    // own that nothing can refer to (`registered` false), rather than into the existing one.
    Enum *NewEnum(string_view name, bool registered) {
        auto e = new Enum(name, (int)enumtable.size());
        enumtable.push_back(e);
        if (registered) enums[e->name /* must be in value */] = e;
        return e;
    }

    Enum *EnumLookup(string_view name, bool decl) {
        auto eit = enums.find(name);
        if (eit != enums.end()) {
            if (!decl) return eit->second;
            lex.Report("double declaration of enum: " + name);
            return NewEnum(name, false);
        }
        if (!decl) {
            if (MaybeNameSpace(name)) {
                eit = enums.find(NameSpaced(name));
                if (eit != enums.end()) return eit->second;
            }
            return nullptr;
        }
        return NewEnum(name, true);
    }

    EnumVal *EnumValLookup(string_view name, bool decl) {
        if (!decl) {
            if (MaybeNameSpace(name)) {
                auto evit = enumvals.find(NameSpaced(name));
                if (evit != enumvals.end()) return evit->second;
            }
        }
        auto evit = enumvals.find(name);
        if (evit != enumvals.end()) {
            if (!decl) return evit->second;
            lex.Report("double declaration of enum value: " + name);
            return new EnumVal(name, 0);
        }
        if (!decl) {
            return nullptr;
        }
        auto ev = new EnumVal(name, 0);
        enumvals[ev->name /* must be in value */] = ev;
        return ev;
    }

    // A declaration that clashes with an existing one gets parsed into a type of its own that
    // nothing can refer to (`registered` false), rather than into the existing one.
    GUDT *NewStruct(string_view name, bool is_struct, Line &line, bool registered) {
        auto st = new GUDT(name, (int)gudttable.size(), is_struct, line);
        if (registered) gudts[st->name /* must be in value */] = st;
        gudttable.push_back(st);
        return st;
    }

    GUDT &StructDecl(string_view name, bool is_struct, Line &line) {
        auto udt = LookupSpecialization(name);
        if (udt && !udt->g.predeclaration) {
            lex.Report("type previously declared as specialization: " + name);
            return *NewStruct(name, is_struct, line, false);
        }
        auto uit = gudts.find(name);
        if (uit != gudts.end()) {
            if (!uit->second->predeclaration) {
                lex.Report("double declaration of type: " + name);
                return *NewStruct(name, is_struct, line, false);
            }
            if (uit->second->is_struct != is_struct) {
                lex.Report("class/struct previously declared as different kind");
                return *NewStruct(name, is_struct, line, false);
            }
            uit->second->predeclaration = false;
            return *uit->second;
        }
        return *NewStruct(name, is_struct, line, true);
    }

    // Stands in for a type the parser could not resolve or declare, such that it can go on as
    // if it had one: a pre-declared class, a state everything in the parser already accepts,
    // not registered under any name.
    GUDT *error_gudt = nullptr;

    GUDT &ErrorStruct() {
        if (!error_gudt) {
            error_gudt = NewStruct("<error>", false, lex, false);
            error_gudt->predeclaration = true;
            MakeSpecialization(*error_gudt, error_gudt->name, false, true);
        }
        return *error_gudt;
    }

    GUDT *LookupStruct(string_view name) {
        if (MaybeNameSpace(name)) {
            auto uit = gudts.find(NameSpaced(name));
            if (uit != gudts.end()) return uit->second;
        }
        auto uit = gudts.find(name);
        if (uit != gudts.end()) return uit->second;
        return nullptr;
    }
    GUDT *LookupStructQuery(string_view name) {
        GUDT* res = LookupStruct(name);
        if(res) return res;
        //Try to search out of scope when doing query
        for (auto gudt = gudttable.rbegin(); gudt != gudttable.rend(); ++gudt) {
            if((*gudt)->name == name) {
                return *gudt;
            }
        }
        return nullptr;
    }

    GUDT &StructUse(string_view name) {
        auto gudt = LookupStruct(name);
        if (gudt) return *gudt;
        lex.Report("unknown type: " + name);
        return ErrorStruct();
    }

    UDT *MakeSpecialization(GUDT &gudt, string_view sname, bool named, bool from_generic) {
        auto st = new UDT(sname, (int)udttable.size(), gudt);
        st->thistype.udt = st;
        st->unnamed_specialization = !named;
        st->next = gudt.first;
        gudt.first = st;
        udttable.push_back(st);
        if (named) {
            if (LookupStruct(sname)) {
                lex.Report("specialization previously declared as type: " + sname);
                return st;
            }
            auto uit = udts.find(sname);
            if (uit != udts.end()) {
                lex.Report("double declaration of specialization: " + sname);
                return st;
            }
        }
        if (named || !from_generic) {
            udts[st->name /* must be in value */] = st;
        }
        return st;
    }

    UDT *LookupSpecialization(string_view name) {
        if (MaybeNameSpace(name)) {
            auto uit = udts.find(NameSpaced(name));
            if (uit != udts.end()) return uit->second;
        }
        auto uit = udts.find(name);
        if (uit != udts.end()) return uit->second;
        return nullptr;
    }

    pair<GUDT *, UDT *> StructOrSpecializationUse(string_view name) {
        auto udt = LookupSpecialization(name);
        if (udt) return { &udt->g, udt };
        auto gudt = LookupStruct(name);
        if (gudt) return { gudt, nullptr };
        lex.Report("unknown type: " + name);
        return { &ErrorStruct(), nullptr };
    }

    SharedField &FieldDecl(string_view name, GUDT *gudt) {
        auto fld = FieldUse(name);
        if (!fld) {
            fld = new SharedField(name, (int)fieldtable.size());
            fields[fld->name /* must be in value */] = fld;
            fieldtable.push_back(fld);
        }
        if (gudt->Has(fld) >= 0) {
            lex.Report("double declaration of field: " + name);
        }
        return *fld;
    }

    SharedField *FieldUse(string_view name) {
        auto it = fields.find(name);
        return it != fields.end() ? it->second : nullptr;
    }

    SubFunction *CreateSubFunction() {
        auto sf = new SubFunction((int)subfunctiontable.size());
        subfunctiontable.push_back(sf);
        return sf;
    }

    Function &CreateFunction(string_view name) {
        auto fname = name.length() ? string(name) : cat("function", functiontable.size());
        auto f = new Function(fname, (int)functiontable.size(), scopelevels.size());
        functiontable.push_back(f);
        return *f;
    }

    Function &FunctionDecl(const string &name, size_t nargs) {
        auto &v = functions[name];
        if (!v.empty() && v.back()->scopelevel == scopelevels.size()) {
            for (auto f = v.back(); f; f = f->sibf) {
                if (f->nargs() == nargs) {
                    return *f;
                }
            }
            auto &f = CreateFunction(name);
            f.first = v.back()->first;
            // Insert in sibf linked list such that highest nargs variants come first,
            // this is useful later when deciding which variant to use.
            Function **it = &v.back();
            for (; *it; it = &(*it)->sibf) {
                if (nargs > (*it)->nargs()) {
                    // Insert before this element.
                    f.sibf = *it;
                    if (it == &v.back()) {
                        // We have a new first.
                        for (auto g = &f; g; g = g->sibf) g->first = &f;
                    }
                    break;
                }
            }
            // If we got to the end of loop, just insert last.
            *it = &f;
            return f;
        } else {
            auto &f = CreateFunction(name);
            v.push_back(&f);
            // Store top level functions, for now only operators needed.
            if (scopelevels.size() == 2 && name.substr(0, 8) == TName(T_OPERATOR)) {
                operators[f.name /* must be in value */] = &f;
            }
            return f;
        }
    }

    // Any function of this name, once parsing has cleaned up the scoped lookup above: the top
    // level one if there is one, else the first declared. For the IDE queries.
    Function *FindFunctionAnywhere(string_view name) {
        if (MaybeNameSpace(name)) {
            auto f = FindFunctionAnywhere(NameSpaced(name));
            if (f) return f;
        }
        auto it = functions_by_name.find(name);
        if (it == functions_by_name.end()) return nullptr;
        // Top level functions are at scopelevel 2 (1 is the file scope).
        for (auto f : it->second) if (f->scopelevel == 2) return f;
        return it->second[0];
    }

    // The function of this name in the innermost scope that has one, while parsing.
    Function *GetFirstFunction(const string &name) {
        auto it = functions.find(name);
        return it == functions.end() || it->second.empty() ? nullptr : it->second.back();
    }

    Function *FindFunction(string_view name) {
        if (MaybeNameSpace(name)) {
            auto f = GetFirstFunction(NameSpaced(name));
            if (f) return f;
        }
        return GetFirstFunction(string(name));
    }

    SpecIdent *NewSid(Ident *id, SubFunction *sf, bool withtype, TypeRef type = nullptr) {
        auto sid = new SpecIdent(id, type, (int)specidents.size(), withtype);
        sid->sf_def = sf;
        specidents.push_back(sid);
        return sid;
    }

    void CloneSids(vector<Arg> &av, SubFunction *sf) {
        for (auto &a : av) {
            a.sid = NewSid(a.sid->id, sf, a.sid->withtype);
        }
    }

    void CloneIds(SubFunction &sf, const SubFunction &o) {
        sf.args = o.args;     CloneSids(sf.args, &sf);
        sf.locals = o.locals; CloneSids(sf.locals, &sf);
        // Don't clone freevars, these will be accumulated in the new copy anew.
    }

    Type *NewType() {
        // These get allocated for very few nodes, given that most types are shared or stored in
        // their own struct.
        auto t = new Type();
        typelist.push_back(t);
        return t;
    }

    UnType *NewUnType() {
        // These get allocated for very few nodes, given that most types are shared or stored in
        // their own struct.
        auto t = new UnType();
        untypelist.push_back(t);
        return t;
    }

    TypeRef NewTypeVar() {
        auto var = NewType();
        *var = Type(V_VAR);
        // Vars store a cycle of all vars its been unified with, starting with itself.
        var->sub = var;
        return var;
    }

    TypeRef NewTuple(size_t sz) {
        auto type = NewType();
        *type = Type(V_TUPLE);
        type->tup = new vector<Type::TupleElem>(sz);
        tuplelist.push_back(type->tup);
        return type;
    }

    UnTypeRef NewSpecUDT(GUDT *gudt) {
        auto su = new SpecUDT(gudt);
        specudts.push_back(su);
        auto nt = NewUnType();
        *nt = UnType(su);
        return nt;
    }

    template<typename T> T Wrap(T elem, ValueType with, const Line *errl = nullptr) {
        if (with == V_NIL) {
            if (elem->t == V_NIL) return elem;
            if (elem->t != V_VAR && elem->t != V_TYPEVAR && !IsNillable(elem)) {
                lex.Report("cannot construct nillable type from " + Q(TypeName(elem)), errl);
                return T(&*type_error);
            }
        }
        auto wt = WrapKnown(elem, with);
        if (!wt.Null()) return wt;
        return elem->Wrap(NewType(), with);
    }

    bool RegisterTypeVector(vector<TypeRef> *sv, const char **names) {
        if (sv[0].size()) return true;  // Already initialized.
        for (size_t i = 0; i < NUM_VECTOR_TYPE_WRAPPINGS; i++) {
            sv[i].push_back(nullptr);
            sv[i].push_back(nullptr);
        }
        for (auto name = names; *name; name++) {
            // Can't use stucts.find, since all are out of scope.
            for (auto udt : udttable) if (udt->name == *name) {
                for (size_t i = 0; i < NUM_VECTOR_TYPE_WRAPPINGS; i++) {
                    auto vt = TypeRef(&udt->thistype);
                    for (size_t j = 0; j < i; j++) vt = Wrap(vt, V_VECTOR);
                    sv[i].push_back(vt);
                }
                goto found;
            }
            return false;
            found:;
        }
        return true;
    }

    static const char **DefaultIntVectorTypeNames() {
        static const char *names[] = { "int2", "int3", "int4", nullptr };
        return names;
    }

    static const char **DefaultFloatVectorTypeNames() {
        static const char *names[] = { "float2", "float3", "float4", nullptr };
        return names;
    }

    static const char *GetVectorName(ValueType t, int flen) {
        if (flen < 2 || flen > 4) return nullptr;
        if (t == V_INT) return DefaultIntVectorTypeNames()[flen - 2];
        if (t == V_FLOAT) return DefaultFloatVectorTypeNames()[flen - 2];
        return nullptr;
    }

    bool RegisterDefaultTypes() {
        // TODO: This isn't great hardcoded in the compiler, would be better if it was declared in
        // lobster code.
        for (auto e : enumtable) {
            if (e->name == "bool") {
                default_bool_type = e;
                break;
            }
        }
        return RegisterTypeVector(default_int_vector_types, DefaultIntVectorTypeNames()) &&
               RegisterTypeVector(default_float_vector_types, DefaultFloatVectorTypeNames()) &&
               default_bool_type;
    }

    TypeRef GetVectorType(ValueType et, size_t level, int arity) const {
        if (arity > 4) return nullptr;
        return et == V_INT
            ? default_int_vector_types[level][arity]
            : default_float_vector_types[level][arity];
    }

    bool IsGeneric(UnTypeRef type) {
        auto u = type->UnWrapAll();
        return u->t == V_TYPEVAR ||
               (u->t == V_UUDT && u->spec_udt->IsGeneric());
    }

    bool IsNillable(UnTypeRef type) {
        return (IsRef(type->t) && type->t != V_STRUCT_R) ||
               (type->t == V_UUDT && !type->spec_udt->gudt->is_struct) ||
               type->t == V_ERROR;
    }

    TypeVariable *NewGeneric(string_view name) {
        auto tv = new TypeVariable { name };
        typevars.push_back(tv);
        return tv;
    }

    TypeRef ResolveTypeVars(UnTypeRef type, const Line &errl) {
        switch (type->t) {
            case V_NIL:
            case V_VECTOR: {
                auto nt = ResolveTypeVars({ type->Element() }, errl);
                if (&*nt != &*type->Element()) {
                    return Wrap(nt, type->t, &errl);
                }
                return &*type;
            }
            case V_TUPLE: {
                vector<TypeRef> types;
                bool same = true;
                for (auto [i, te] : enumerate(*type->tup)) {
                    auto tr = ResolveTypeVars({ te.type }, errl);
                    types.push_back(tr);
                    if (!tr->Equal(*te.type)) same = false;
                }
                if (same) return &*type;
                auto nt = NewTuple(type->tup->size());
                for (auto [i, te] : enumerate(*type->tup)) {
                    nt->Set(i, &*types[i], te.lt);
                }
                return nt;
            }
            case V_UUDT: {
                vector<TypeRef> types;
                for (auto s : type->spec_udt->specializers) {
                    auto t = ResolveTypeVars({ s }, errl);
                    types.push_back(&*t);
                }
                for (auto udti = type->spec_udt->gudt->first; udti; udti = udti->next) {
                    assert(udti->bound_generics.size() == types.size());
                    for (auto [i, gtype] : enumerate(udti->bound_generics)) {
                        if (!gtype->Equal(*types[i])) goto nomatch;
                    }
                    return &udti->thistype;
                    nomatch:;
                }
                // No existing specialization found, create a new one.
                auto udt =
                    MakeSpecialization(*type->spec_udt->gudt, type->spec_udt->gudt->name, false, true);
                if (udt->g.generics.size() != types.size()) {
                    // FIXME: this can happen for class foo<T> : bar<T, .. > where generics
                    // are inherited from bar.
                    lex.Report(cat("internal: missing specializers for ", Q(TypeName(type))),
                               &errl);
                    // Everything downstream assumes one bound type per generic.
                    types.resize(udt->g.generics.size(), type_error);
                }
                udt->bound_generics = types;
                ResolveFields(*udt, errl);
                type_check_call_back(*udt);
                return &udt->thistype;
            }
            case V_TYPEVAR: {
                for (auto &bvec : reverse(bound_typevars_stack)) {
                    for (auto &gtv : bvec) {
                        if (gtv.tv == type->tv && !gtv.type.Null()) return gtv.type;
                    }
                }
                lex.Report(cat("could not resolve type variable ", Q(type->tv->name)), &errl);
                return type_error;
            }
            default:
                return &*type;
        }
    }

    void PushSuperGenerics(UDT *u) {
        for (; u; u = u->ssuperclass) {
            bound_typevars_stack.push_back(u->GetBoundGenerics());
        }
    }

    void PopSuperGenerics(UDT *u) {
        for (; u; u = u->ssuperclass) {
            bound_typevars_stack.pop_back();
        }
    }

    size_t CheckUDTSameTypeRec(UDT &cudt, TypeRef &sametype, size_t slot, size_t rec) {
        for (size_t i = 0; i < cudt.sfields.size(); i++) {
            // Can't use Union here since it will bind variables, use simplified
            // alternative:
            auto ftype = cudt.sfields[i].type;
            if (ftype.Null()) {
                // Field type still to be inferred from its default value, so
                // it can't participate in sametype.
                sametype = type_undefined;
                break;
            }
            if (cudt.sfields[i].bits) {
                // A field that shares its slot is not a slot of the type it is, which
                // is what everything that acts on sametype (vector math, indexing, the
                // numeric struct arguments of builtins) takes the slots as.
                sametype = type_undefined;
                break;
            }
            if (IsStruct(ftype->t)) {
                if (rec == 16) {  // We only for self-referential in the TypeChecker :(
                    sametype = type_undefined;
                    break;
                }
                slot = CheckUDTSameTypeRec(*ftype->udt, sametype, slot, rec + 1);
                if (sametype->t == V_UNDEFINED) break;
            } else if (slot == 0) {
                sametype = ftype;
                slot++;
            } else if (!ftype->Equal(*sametype)) {
                sametype = type_undefined;
                slot++;
                break;
            }
        }
        return slot;
    }

    void ResolveFields(UDT &udt, const Line &errl) {
        bound_typevars_stack.push_back(udt.GetBoundGenerics());
        auto supertype = ResolveTypeVars(udt.g.gsuperclass, errl);
        if (supertype->t != V_UNDEFINED) {
            assert(IsUDT(supertype->t));
            udt.ssuperclass = supertype->udt;
            // An inheritance cycle can be declared thru pre-declarations, and
            // would make the walks over ssuperclass everywhere run forever.
            // Checking at the moment the link is added means the chain below
            // is guaranteed cycle-free.
            for (auto u = udt.ssuperclass; u; u = u->ssuperclass) {
                if (u == &udt) {
                    lex.Report(cat("inheritance cycle in type ", Q(udt.g.name)), &errl);
                    // Left without a superclass, at both levels, which breaks the cycle.
                    udt.ssuperclass = nullptr;
                    udt.g.gsuperclass = UnTypeRef();
                    break;
                }
            }
        }
        PushSuperGenerics(udt.ssuperclass);
        for (size_t i = udt.sfields.size(); i < udt.g.fields.size(); i++) {
            auto &field = udt.g.fields[i];
            // A field with no type given whose type could not be derived from
            // its default value at parse time stays null until EnsureUDTChecked
            // derives it by typechecking the default, so partially resolved
            // state can never be silently read (V_ANY, the previous sentinel,
            // is also a legitimate type).
            udt.sfields.push_back({ field.gdefaultval && field.giventype->t == V_ANY
                                        ? TypeRef(nullptr)
                                        : ResolveTypeVars(field.giventype, errl) });
            udt.sfields.back().bits = field.bits;
        }
        PopSuperGenerics(udt.ssuperclass);
        bound_typevars_stack.pop_back();
        udt.family_root = FamilyRootOf(&udt);
        // NOTE: all users of sametype will only act on it if it is numeric, since
        // otherwise it would a scalar field to become any without boxing.
        // A member of an abstract struct family is never numeric: its layout has slots
        // that are not its fields.
        if (udt.sfields.size() >= 1 && !udt.family_root) {
            CheckUDTSameTypeRec(udt, udt.sametype, 0, 0);
        }
        // Update the type to the correct struct type.
        if (udt.g.is_struct) {
            // A member of an abstract struct family is a struct of references if any member
            // can be (see GUDT::family_hasref), since a value of its type may hold any
            // member. The declchecker decides that over all declarations and applies it to
            // the specializations known then (see DeclChecker::FinalizeFamilies); one
            // created after that follows it here.
            udt.UpdateStructType();
        }
        if (udt.state == UDTState::DECLARED && udt.sfields.size() == udt.g.fields.size())
            udt.state = UDTState::FIELDS_RESOLVED;
    }

    // The kind of value a slot of this type holds, as far as the generated code is concerned:
    // an int, a float, a function, or a reference (or nil).
    static char SlotKind(TypeRef type) {
        if (IsRefNil(type->t)) return 'R';
        if (type->t == V_FLOAT) return 'F';
        if (type->t == V_FUNCTION) return 'N';
        return 'I';
    }

    // Lays out an abstract struct family: every member (the root and every struct below it)
    // gets the same number of slots, with slot 0 holding the family index of the value's
    // dynamic type (see UDT::FamilyIndex), such that a value of any member type can hold
    // any member. The generated code holds and increments/decrements the slots of a value
    // by the slot types of its static type, so any slot must hold the same kind of value
    // (see SlotKind) in every member that has a field there, and nothing in the rest:
    // family_kinds on the root says which. The kinds come grouped, each group as wide as
    // the member that needs the most slots of that kind, in the order the kinds first
    // appear in the members' fields, and a field goes to the first free run of slots of its
    // kinds, which is only the next slot when the fields are declared in that order (the
    // common case, which the constructor code takes advantage of, see
    // ObjectConstructor::GenerateFamilyStruct). A field that is a struct of several kinds
    // stays one block, which gets extra slots at the end when no run fits it. The slots a
    // member has no field in are padding (0 or nil), its hidden_sfields along with the type
    // slot. A member inherits the slots of its superclass's fields, so a method of the
    // superclass reads them from any member. Once decided, the layout only ever grows at
    // the end, so a specialization created during typechecking still fits after the sizes
    // computed from it were used, unless it needs a run of kinds the family does not have,
    // which is an error. Returns false when a member holds a value of the family itself,
    // which no layout can accommodate.
    bool LayoutFamily(UDT &root, int depth) {
        assert(root.family_root == &root);
        if (root.family_in_progress) return false;
        root.family_in_progress = true;
        auto &kinds = root.family_kinds;
        vector<UDT *> members;
        for (auto udt : udttable) {
            if (udt->family_root == &root) members.push_back(udt);
        }
        // How many fields a member inherits from a superclass in the family, which keep the
        // slots (and bits) they have there.
        auto inherited_of = [&](UDT *udt) {
            if (!udt->ssuperclass || udt->ssuperclass->family_root != &root) return size_t(0);
            return std::min(udt->ssuperclass->sfields.size(), udt->sfields.size());
        };
        // The family index takes only the low 8 bits of the type slot (see FamilyIndexOf),
        // so the first fields of a member that are stored in part of a slot (see
        // SField::bits) can have the other 56, when they fit, which may make a whole family
        // of small structs a single slot. It is decided once for the family whether any
        // member does (see UDT::family_type_slot_shared), since reading the index then takes
        // a mask the whole slot does not need.
        if (members.size() > (size_t(1) << FAMILY_INDEX_BITS)) {
            lex.Report(cat("abstract struct ", Q(root.name), " has more than ",
                           1 << FAMILY_INDEX_BITS, " members"),
                       &root.g.line);
        }
        if (!root.family_laid_out) {
            for (auto udt : members) {
                if (inherited_of(udt) || udt->sfields.empty()) continue;
                auto &first = udt->sfields[0];
                if (first.bits && first.bits <= 64 - FAMILY_INDEX_BITS) {
                    root.family_type_slot_shared = true;
                }
            }
        }
        // The kinds of the slots each field of each member occupies, see SlotKind. A field
        // stored in part of a slot shares an int slot with such fields declared right before
        // it while they fit, as in UDT::ComputeSizes: the first of them has the slot as its
        // block, the rest an empty block, at the bit offsets decided here. Fields in the
        // type slot have a block of their own, "T", which is not a kind. The fields after
        // the inherited ones start a slot of their own.
        map<UDT *, vector<string>> blocks;
        for (auto udt : members) {
            auto inherited = inherited_of(udt);
            int lastbits = 64;
            int capacity = 64;
            for (auto [i, sfield] : enumerate(udt->sfields)) {
                if (i == inherited) lastbits = capacity = 64;
                string block;
                if (sfield.type.Null()) {
                    // A field whose type was never inferred, which the typechecker reports
                    // for any member it completes (see EnsureUDTChecked); the rest are in
                    // code that never ran, and only need some slot to sit in.
                    block = "I";
                    lastbits = capacity = 64;
                } else if (sfield.bits) {
                    if (!i && !inherited && root.family_type_slot_shared &&
                        sfield.bits <= 64 - FAMILY_INDEX_BITS) {
                        // Above the family index in the type slot.
                        block = "T";
                        sfield.bitoff = FAMILY_INDEX_BITS;
                        lastbits = FAMILY_INDEX_BITS + sfield.bits;
                        capacity = 64;
                    } else if (lastbits + sfield.bits <= capacity) {
                        sfield.bitoff = lastbits;
                        lastbits += sfield.bits;
                    } else {
                        block = "I";
                        sfield.bitoff = 0;
                        lastbits = sfield.bits;
                        capacity = 64;
                    }
                } else if (IsStruct(sfield.type->t)) {
                    lastbits = capacity = 64;
                    auto &fudt = *sfield.type->udt;
                    if (fudt.ComputeSizes(*this, depth + 1)) {
                        for (int j = 0; j < fudt.numslots; j++) {
                            block += SlotKind(SlotTypeOf(fudt, j));
                        }
                    } else {
                        lex.Report(cat("struct ", Q(udt->name), " cannot contain ", Q(fudt.name),
                                       " (self-referential)"),
                                   &udt->g.line);
                        block = "I";
                    }
                } else {
                    block = SlotKind(sfield.type);
                    lastbits = capacity = 64;
                }
                blocks[udt].push_back(block);
            }
        }
        if (!root.family_laid_out) {
            string order;
            map<char, int> group;
            for (auto udt : members) {
                map<char, int> need;
                for (auto &block : blocks[udt]) {
                    if (block == "T") continue;
                    for (auto kind : block) {
                        if (order.find(kind) == string::npos) order += kind;
                    }
                    // A block of one kind goes in that kind's group; one of several stays
                    // together and gets its own slots if no run happens to fit it.
                    if (block.find_first_not_of(block[0]) == string::npos) {
                        need[block[0]] += (int)block.size();
                    }
                }
                for (auto [kind, n] : need) group[kind] = std::max(group[kind], n);
            }
            for (auto kind : order) kinds += string(group[kind], kind);
        }
        set<UDT *> placed;
        std::function<void(UDT &)> place = [&](UDT &udt) {
            if (udt.numslots >= 0 || !placed.insert(&udt).second) return;
            // Which of the family's slots this member's fields already sit in.
            vector<bool> used(kinds.size() + 1, false);
            auto take = [&](int slot, int width) {
                for (int j = 0; j < width; j++) {
                    if (slot + j < (int)used.size()) used[slot + j] = true;
                }
            };
            auto inherited = inherited_of(&udt);
            if (inherited) {
                auto &sup = *udt.ssuperclass;
                place(sup);
                for (size_t i = 0; i < inherited; i++) {
                    udt.sfields[i].slot = sup.sfields[i].slot;
                    udt.sfields[i].bitoff = sup.sfields[i].bitoff;
                    if (udt.sfields[i].slot) {
                        take(udt.sfields[i].slot, (int)blocks[&udt][i].size());
                    }
                }
            }
            for (size_t i = inherited; i < udt.sfields.size(); i++) {
                auto &block = blocks[&udt][i];
                if (block.empty()) {
                    // Shares the slot of the field before it, see above.
                    udt.sfields[i].slot = udt.sfields[i - 1].slot;
                    continue;
                }
                if (block == "T") {
                    udt.sfields[i].slot = 0;
                    continue;
                }
                // The first free run of slots of the block's kinds.
                size_t q = 0;
                for (;; q++) {
                    if (q + block.size() > kinds.size()) {
                        if (root.family_laid_out) {
                            lex.Report(cat("struct ", Q(udt.name), " does not fit the layout of"
                                           " abstract struct ", Q(root.name), ", which its first use"
                                           " already decided (declare a named specialization of it"
                                           " before that use)"),
                                       &udt.g.line);
                        }
                        q = kinds.size();
                        kinds += block;
                        used.resize(kinds.size() + 1, false);
                        break;
                    }
                    if (kinds.compare(q, block.size(), block) != 0) continue;
                    auto free = true;
                    for (size_t j = 0; j < block.size(); j++) {
                        if (used[q + 1 + j]) free = false;
                    }
                    if (free) break;
                }
                udt.sfields[i].slot = (int)q + 1;
                take(udt.sfields[i].slot, (int)block.size());
            }
        };
        for (auto udt : members) place(*udt);
        auto width = 1 + (int)kinds.size();
        for (auto udt : members) {
            if (udt->numslots == width) continue;
            udt->numslots = width;
            udt->hidden_sfields.clear();
            vector<bool> covered(width, false);
            for (auto &sfield : udt->sfields) {
                auto w = sfield.type.Null() ? 1 : ValWidth(sfield.type);
                for (int j = 0; j < w; j++) {
                    if (sfield.slot + j < width) covered[sfield.slot + j] = true;
                }
            }
            for (int s = 0; s < width; s++) {
                if (covered[s]) continue;
                SField h;
                auto kind = s ? kinds[s - 1] : 'I';
                h.type = kind == 'R' ? WrapKnown(type_any, V_NIL)
                       : kind == 'F' ? type_float
                       : kind == 'N' ? type_function_null_void
                                     : type_int;
                h.slot = s;
                udt->hidden_sfields.push_back(h);
            }
        }
        root.family_laid_out = true;
        root.family_in_progress = false;
        return true;
    }

    // Register a specialization in the subudts list of itself and all its
    // superclasses. Called by the declchecker for all specializations known
    // after parsing, and from EnsureUDTChecked for specializations created
    // during typechecking.
    void RegisterSubUDT(UDT *udt) {
        if (udt->in_forest) return;
        udt->in_forest = true;
        for (auto u = udt; u; u = u->ssuperclass) {
            u->subudts.push_back(udt);
            // An abstract class can never be a dispatch case itself, it only
            // needs to be present as a dispatch root.
            if (udt->g.is_abstract) break;
        }
    }

    void Serialize(vector<type_elem_t> &typetable,
                   vector<metadata::SpecIdent> &sids,
                   vector<string_view> &stringtable,
                   string &bytecode,
                   vector<pair<string, string>> &filenames,
                   vector<type_elem_t> &ser_ids,
                   uint64_t src_hash) {
        flatbuffers::FlatBufferBuilder fbb;
        vector<flatbuffers::Offset<flatbuffers::String>> fns;
        for (auto &f : filenames) fns.push_back(fbb.CreateString(f.first));
        vector<flatbuffers::Offset<metadata::Function>> functionoffsets;
        for (auto f : functiontable) functionoffsets.push_back(f->Serialize(fbb));
        vector<flatbuffers::Offset<metadata::UDT>> udtoffsets;
        for (auto u : udttable) udtoffsets.push_back(u->Serialize(fbb));
        vector<flatbuffers::Offset<metadata::Ident>> identoffsets;
        for (auto i : identtable) identoffsets.push_back(i->Serialize(fbb, i->scopelevel == 1));
        vector<flatbuffers::Offset<metadata::Enum>> enumoffsets;
        for (auto e : enumtable) enumoffsets.push_back(e->Serialize(fbb));
        vector<int> subfunctions_to_function;
        for (auto sf : subfunctiontable) subfunctions_to_function.push_back(sf->parent->idx);
        string build_info;
        auto time = std::time(nullptr);
        if (time) {
            auto tm = std::localtime(&time);
            if (tm) {
                auto ts = std::asctime(tm);
                build_info = string(ts, 24);
            }
        }
        auto bcf = metadata::CreateMetadataFile(fbb,
            LOBSTER_METADATA_FORMAT_VERSION,
            fbb.CreateVector((vector<int> &)typetable),
            fbb.CreateVector<flatbuffers::Offset<flatbuffers::String>>(stringtable.size(),
                [&](size_t i) {
                    return fbb.CreateString(stringtable[i].data(), stringtable[i].size());
                }
            ),
            fbb.CreateVector(fns),
            fbb.CreateVector(functionoffsets),
            fbb.CreateVector(udtoffsets),
            fbb.CreateVector(identoffsets),
            fbb.CreateVectorOfStructs(sids),
            fbb.CreateVector(enumoffsets),
            fbb.CreateVector((vector<int> &)ser_ids),
            fbb.CreateString(build_info.c_str(), build_info.size()),
            src_hash,
            fbb.CreateVector(subfunctions_to_function));
        metadata::FinishMetadataFileBuffer(fbb, bcf);
        bytecode.assign(fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
    }
};

bool UDT::ComputeSizes(SymbolTable &st, int depth) {
    if (numslots >= 0) return true;
    if (depth > 16) return false;  // Simple protection against recursive references.
    if (family_root) return st.LayoutFamily(*family_root, depth);
    int size = 0;
    // How many bits of the last slot the fields stored in part of it (see SField::bits) use
    // up, 64 when the last slot is a whole field, or when there is none yet.
    int lastbits = 64;
    for (auto &sfield : sfields) {
        sfield.slot = size;
        sfield.bitoff = 0;
        if (sfield.type.Null()) {
            // Field type still to be inferred, can only be reached in
            // recursive situations that will error elsewhere.
            size++;
            lastbits = 64;
        } else if (IsStruct(sfield.type->t)) {
            if (!sfield.type->udt->ComputeSizes(st, depth + 1)) return false;
            size += sfield.type->udt->numslots;
            lastbits = 64;
        } else if (sfield.bits) {
            // Fields declared next to each other share a slot while they fit, in
            // declaration order, so the program decides which of them get the bits
            // left over (a field never straddles two slots).
            if (lastbits + sfield.bits <= 64) {
                sfield.slot = size - 1;
                sfield.bitoff = lastbits;
                lastbits += sfield.bits;
            } else {
                size++;
                lastbits = sfield.bits;
            }
        } else {
            size++;
            lastbits = 64;
        }
    }
    numslots = size;
    return true;
}

inline void FormatArg(string &r, string_view name, size_t i, UnTypeRef type, int depth = 0) {
    if (i) r += ", ";
    r += name;
    if (type->t != V_ANY) {
        r += ":";
        r += TypeName(type, true, depth);
    }
}

inline string Signature(const NativeFun &nf) {
    string r = nf.name;
    r += "(";
    for (auto [i, arg] : enumerate(nf.args)) {
        FormatArg(r, arg.name, i, arg.vttype);
    }
    r += ")";
    if (nf.retvals.size() > 0) {
        r += " -> ";
        for (auto [i, retval] : enumerate(nf.retvals)) {
            if (i) r += ", ";
            r += TypeName(retval.vttype);
        }
    }
    return r;
}

inline string Signature(const UDT &udt) {
    string r = udt.name;
    r += "{";
    for (auto [i, f] : enumerate(udt.g.fields)) {
        // A null type is a field whose type is still to be inferred from its
        // default value.
        FormatArg(r, f.id->name, i,
                  udt.sfields[i].type.Null() ? f.giventype
                                             : UnTypeRef(udt.sfields[i].type.get()));
    }
    r += "}";
    return r;
}

inline string Signature(const GUDT &gudt) {
    string r = gudt.name;
    r += "{";
    for (auto [i, f] : enumerate(gudt.fields)) {
        FormatArg(r, f.id->name, i, f.giventype);
    }
    r += "}";
    return r;
}

inline string Signature(const SubFunction &sf, int depth = 0) {
    string r = sf.parent->name;
    if (!sf.generics.empty() && sf.explicit_generics) {
        r += "<";
        for (auto [i, gtv] : enumerate(sf.generics)) {
            if (i) r += ",";
            r += gtv.type.Null() ? gtv.tv->name : TypeName(gtv.type);
        }
        r += ">";
    }
    r += "(";
    for (auto [i, arg] : enumerate(sf.args)) {
        auto type = arg.spec_type;
        if (arg.spec_type->t == V_FUNCTION && depth > 1) {
            // Avoid recursive function types.
            type = type_function_null_void;
        }
        FormatArg(r, arg.sid->id->name, i, type, depth + 1);
    }
    r += ")";
    if (sf.returntype->t != V_VOID && sf.returntype->t != V_UNDEFINED && sf.returntype->t != V_VAR) {
        r += " -> ";
        r += TypeName(sf.returntype, false);
    }
    return r;
}

// Declared in compiler.h, so that the files outside the compiler can name a type too.
string TypeName(UnTypeRef type, bool tuple_brackets, int depth) {
    // A type name followed by its type arguments, when it has any.
    auto specialized = [](string_view name, const auto &types) {
        string s(name);
        if (types.empty()) return s;
        s += "<";
        for (auto [i, t] : enumerate(types)) {
            if (i) s += ", ";
            s += TypeName(t);
        }
        return s + ">";
    };
    switch (type->t) {
        case V_STRUCT_NUM: {
            auto nvt = SymbolTable::GetVectorName(type->ns->t, type->ns->flen);
            if (nvt) return nvt;
            // FIXME: better names?
            return type->ns->t == V_INT ? "intN" : "floatN";
        }
        case V_STRUCT_R:
        case V_STRUCT_S:
        case V_CLASS: {
            auto udt = type->udt;
            return udt->unnamed_specialization ? specialized(udt->name, udt->bound_generics)
                                               : udt->name;
        }
        case V_UUDT:
            return specialized(type->spec_udt->gudt->name, type->spec_udt->specializers);
        case V_VECTOR:
            return type->Element()->t == V_VAR
                        ? "[]"
                        : "[" + TypeName(type->Element()) + "]";
        case V_FUNCTION:
            return type->sf
                ? Signature(*type->sf, depth)
                : "function";

        case V_NIL:
            return type->Element()->t == V_VAR
                ? "nil"
                : TypeName(type->Element()) + "?";
        case V_TUPLE: {
            string s;
            if (tuple_brackets) s += "(";
            for (auto [i, te] : enumerate(*type->tup)) {
                if (i) s += ", ";
                s += TypeName(te.type);
            }
            if (tuple_brackets) s += ")";
            return s;
        }
        case V_INT:
            return type->e ? type->e->name : "int";
        case V_TYPEID:
            return "typeid(" + TypeName(type->sub) + ")";
        case V_TYPEVAR:
            return string(type->tv->name);
        case V_RESOURCE:
            return type->rt ? cat("resource<", type->rt->name, ">") : "resource";
        case V_STRING:
            return "string";
        case V_FLOAT:
            return "float";
        case V_VOID:
            return "void";
        case V_VAR:
            return "unknown";
        case V_UNDEFINED:
            return "undefined";
        case V_ANY:
            return "any";
        case V_ERROR:
            return "error";
        default:
            return "<internal-error-type>";
    }
}

}  // namespace lobster

#endif  // LOBSTER_IDENTS
