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

// Builtin calls and their argument and return types.
// This topic depends only on TypeCheckLocations and TypeCheckBase.

namespace lobster {

struct TypeCheckBuiltin : virtual TypeCheckLocations {
    TypeCheckBuiltin() {}

    // The message for a call that doesn't fit a builtin: the argument types it got, and the
    // overloads there are.
    string NatCallMsg(string_view errstr, const NativeFun *nf, const NativeCall &callnode) {
        auto err = errstr + nf->name;
        err += "\n  got:";
        for (auto c : callnode.children) {
            err += " " + TypeName(c->exptype);
        }
        for (auto cnf = nf->first; cnf; cnf = cnf->overloads) {
            err += "\n  overload: " + Signature(*cnf);
        }
        return err;
    }

    // Whether argument `i` of the call is acceptable to argument `arg` of overload `cnf`.
    bool BuiltinArgFits(const NativeCall &node, const NativeFun *cnf, const Narg &arg, size_t i) {
        if (i >= node.children.size()) {
            // Default args always good for overload match.
            return arg.optional;
        }
        // Special purpose treatment of V_ANY to allow generic vectors in overloaded
        // length() etc.
        auto etype = node.children[i]->exptype;
        auto cf = CF_NUMERIC_NIL;
        if (arg.vttype->t != V_STRING) cf = ConvertFlags(cf | CF_COERCIONS);
        if (arg.vttype->t != V_ANY &&
            (arg.vttype->t != V_VECTOR ||
             etype->t != V_VECTOR ||
             arg.vttype->sub->t != V_ANY) &&
            !ConvertsTo(etype,
                        ActualBuiltinType(arg.vttype, arg.flags, etype, cnf, true, i + 1, node),
                        cf)) return false;
        // A plain V_ANY argument takes a reference and nothing else, same as the check the
        // argument loop does. Anything else has to find its overload elsewhere, and is told
        // what there is when it fits none of them, rather than being reported against this
        // one.
        if (arg.vttype->t == V_ANY && !arg.flags && !IsRefNilNoStruct(etype->t)) return false;
        return true;
    }

    // The numeric structs the overloads of a builtin accept for argument `i`: which element
    // types, and which widths (in slots).
    struct NumStructArg {
        bool ints = false;
        bool floats = false;
        bool any_width = false;
        int min_slots = 0;  // 0 when there are no numeric struct overloads at all.
        int max_slots = 0;

        string Describe() const {
            auto desc = string("numeric struct (all ");
            desc += ints && floats ? "int or all float" : ints ? "int" : "float";
            desc += ") of ";
            if (any_width) desc += "any number of";
            else if (min_slots == max_slots) desc += cat(min_slots);
            else desc += cat(min_slots, " to ", max_slots);
            return desc + " slots";
        }
    };

    NumStructArg NumStructArgOf(const NativeFun *nf, size_t i) {
        NumStructArg nsa;
        for (auto cnf = nf->first; cnf; cnf = cnf->overloads) {
            if (i >= cnf->args.size()) continue;
            auto vt = cnf->args[i].vttype;
            if (vt->t != V_STRUCT_NUM) continue;
            (vt->ns->t == V_INT ? nsa.ints : nsa.floats) = true;
            if (vt->ns->flen < 0) {
                nsa.any_width = true;
            } else {
                if (!nsa.min_slots || vt->ns->flen < nsa.min_slots) nsa.min_slots = vt->ns->flen;
                if (vt->ns->flen > nsa.max_slots) nsa.max_slots = vt->ns->flen;
            }
        }
        if (nsa.any_width && !nsa.min_slots) nsa.min_slots = 1;
        return nsa;
    }

    // A struct argument that fits no overload is reported in terms of the numeric structs the
    // builtin does take, since a list of overloads for the individual widths does not make the
    // restriction obvious, and the other overloads in it can never apply to a struct.
    void NumStructArgError(const NativeCall &node, size_t i) {
        auto nf = node.nf->first;
        auto &c = *node.children[i];
        auto udt = c.exptype->udt;
        auto nsa = NumStructArgOf(nf, i);
        if (!nsa.min_slots) {
            Error(c, "struct value cannot be used in ", Q(nf->name));
            return;
        }
        string why;
        if (!udt->sametype->Numeric()) {
            why = cat(Q(udt->name), " does not have all int or all float fields");
        } else if (!(udt->sametype->t == V_INT ? nsa.ints : nsa.floats)) {
            why = cat(Q(udt->name), " has ", TypeName(udt->sametype), " fields");
        } else {
            why = cat(Q(udt->name), " has ", udt->numslots, " slots");
            // A field that is a struct itself takes as many slots as it has, so a slot count
            // that doesn't match the fields as written needs spelling out.
            if (udt->numslots != ssize(udt->sfields))
                why += cat(", not ", ssize(udt->sfields),
                           ": a struct field counts for as many slots as it has");
        }
        RequiresError(nsa.Describe(), c.exptype, c, i < nf->args.size() ? nf->args[i].name : "",
                      nf->name, nullptr, why);
    }

    Node *Check(NativeCall &node, size_t /*reqret*/, TypeRef /*parent_bound*/) {
        // The arguments were typechecked by the GenericCall this came from.
        assert(node.children.empty() || node.children[0]->exptype->t != V_UNDEFINED);
        // Stands in for a call that doesn't fit the builtin, see TypeChecker::ErrorNode.
        auto give_up = [&]() {
            ReleaseChildren(node);
            return ErrorNode(node);
        };
        if (node.nf->first->overloads) {
            // Multiple overloads available, figure out which we want to call.
            auto cnf = node.nf->first;
            for (; cnf; cnf = cnf->overloads) {
                if (cnf->args.size() < node.Arity()) continue;
                for (auto [i, arg] : enumerate(cnf->args))
                    if (!BuiltinArgFits(node, cnf, arg, i)) goto nomatch;
                node.nf = cnf;
                break;
                nomatch:;
            }
            if (!cnf) {
                // A struct argument no overload can take at all gets told what kinds of struct
                // the builtin does take, rather than a list of overloads it can never fit.
                for (auto [i, c] : enumerate(node.children)) {
                    if (!IsStruct(c->exptype->t)) continue;
                    bool fits = false;
                    for (auto onf = node.nf->first; onf && !fits; onf = onf->overloads)
                        fits = i < onf->args.size() && BuiltinArgFits(node, onf, onf->args[i], i);
                    if (fits) continue;
                    NumStructArgError(node, i);
                    return give_up();
                }
                auto err = NatCallMsg("arguments match no overloads of ", node.nf, node);
                for (auto c : node.children) err += DemotionNote(*c);
                Error(node, err);
                return give_up();
            }
        }
        for (auto [i, arg] : enumerate(node.nf->args)) {
            if (i >= node.Arity()) {
                if (arg.optional) {
                    auto type = ActualBuiltinType(arg.vttype, arg.flags,
                                                     type_undefined,
                                                     node.nf, true, i + 1, node);
                    switch (type->t) {
                        case V_INT: {
                            auto ic = new IntConstant(node.line, arg.default_val);
                            node.Add(ic);
                            if ((arg.flags & NF_BOOL) && st.default_bool_type) {
                                auto ev = st.default_bool_type->Lookup(arg.default_val);
                                if (ev) ic->from = ev;
                            }
                            break;
                        }
                        case V_FLOAT:
                            node.Add(new FloatConstant(node.line, arg.default_val));
                            break;
                        case V_STRUCT_S: {
                            auto cons = new ObjectConstructor(node.line, { type });
                            for (auto &f : type->udt->sfields) {
                                if (f.type->t == V_FLOAT) {
                                    cons->Add(new FloatConstant(node.line, 0.0));
                                } else {
                                    // FIXME: This may be unresolved/ununified, for now assume int,
                                    // since that will convert to float if needed.
                                    cons->Add(new IntConstant(node.line, 0));
                                }
                            }
                            node.Add(cons);
                            break;
                        }
                        default:
                            node.Add(new Nil(node.line, { st.Wrap(type, V_NIL, &node.line) }));
                            break;
                    }
                    TT(node.children.back(), 1, LT_ANY);
                } else {
                    ErrorAlways(node,
                                   NatCallMsg("wrong number of many arguments for ", node.nf, node));
                    return give_up();
                }
            }
        }
        if (node.children.size() != node.nf->args.size()) {
            ErrorAlways(node, NatCallMsg("too many arguments for ", node.nf, node));
            return give_up();
        }
        vector<TypeRef> argtypes(node.children.size());
        for (auto [i, c] : enumerate(node.children)) {
            auto &arg = node.nf->args[i];
            auto argtype = ActualBuiltinType(arg.vttype, arg.flags, node.children[i]->exptype, node.nf, false, i + 1, node);
            // Filter out functions that are not struct aware.
            bool typed = false;
            if (arg.flags & NF_CONVERTANYTOSTRING && c->exptype->t != V_STRING) {
                AdjustLifetime(c, LT_BORROW);  // MakeString wants to borrow.
                MakeString(c, arg.lt);
                argtype = type_string;
                typed = true;
            }
            auto cf_const = arg.flags & NF_CONST ? CF_COVARIANT : CF_NONE;
            int flag = NF_SUBARG1;
            for (int sa = 0; sa < 3; sa++) {
                if (arg.flags & flag) {
                    if (arg.flags & NF_UNION) {
                        argtypes[sa] = Union(argtypes[sa], c->exptype, "left", "right",
                                                CF_NONE, &node);
                    }
                    SubType(c,
                               node.nf->args[sa].vttype->t == V_VECTOR && argtypes[sa]->t == V_VECTOR && argtype->t != V_VECTOR
                                ? argtypes[sa]->sub
                                : argtypes[sa],
                            ArgName(i),
                            node.nf->name,
                            cf_const);
                    // Stop these generic params being turned into any by SubType below.
                    typed = true;
                }
                flag *= 2;
            }
            if (arg.flags & NF_ANYVAR) {
                if (argtype->t == V_VECTOR)
                    argtype = st.Wrap(st.NewTypeVar(), V_VECTOR, &node.line);
                else if (argtype->t == V_ANY)
                    argtype = st.NewTypeVar();
                else assert(0);
            }
            if (argtype->t == V_ANY) {
                if (!arg.flags) {
                    // Special purpose type checking to allow any reference type for functions like
                    // copy/equal/hash etc. Note that this is the only place in the language where
                    // we allow this!
                    if (!IsRefNilNoStruct(c->exptype->t))
                        RequiresError("reference type", c->exptype, *c, node.nf->args[i].name, node.nf->name);
                    typed = true;
                } else if (IsStruct(c->exptype->t) &&
                           !(arg.flags & NF_ANYWIDTH) &&
                           c->exptype->udt->numslots > 1) {
                    // Avoid unsuspecting generic functions taking values as args.
                    // TODO: ideally this does not trigger for any functions.
                    Error(node, "function does not support this struct type");
                }
            }
            if (arg.vttype->t != V_STRUCT_NUM && !(arg.flags & NF_ANYWIDTH))
                NoStruct(*c, node.nf->name);
            if (!typed) {
                SubType(c, argtype, ArgName(i), node.nf->name, cf_const);
            }
            argtypes[i] = c->exptype;
            StorageType(c->exptype, node);
            AdjustLifetime(c, arg.lt);
            DecBorrowers(c->lt, node);
            if (arg.flags & NF_MUTATES) CheckElementWrite(c, &node);
        }

        node.exptype = type_void;  // no retvals
        node.lt = LT_ANY;
        if (node.nf->retvals.size() > 1) node.exptype = st.NewTuple(node.nf->retvals.size());
        for (auto [i, ret] : enumerate(node.nf->retvals)) {
            int sa = 0;
            auto type = ret.vttype;
            auto rlt = ret.lt;
            switch (ret.flags) {
                case NF_SUBARG3: sa++; [[fallthrough]];
                case NF_SUBARG2: sa++; [[fallthrough]];
                case NF_SUBARG1: {
                    type = argtypes[sa];
                    auto nftype = node.nf->args[sa].vttype;

                    if (nftype->t == V_TYPEID) {
                        assert(!sa);  // assumes always first.
                        auto tin = node.children[0];  // Usually a TypeOf, but could be IdentRef or any exp?
                        // Anything but a typeid got an error from the SubType above.
                        type = tin->exptype->t == V_TYPEID ? tin->exptype->sub : type_error;
                    }

                    if (ret.optional) {
                        if (!st.IsNillable(type)) {
                            Error(node, "argument ", sa + 1, " to ", Q(node.nf->name),
                                            " has to be a reference type");
                            type = type_error;
                        }
                        type = st.Wrap(type, V_NIL, &node.line);
                    } else if (nftype->t == V_VECTOR && ret.vttype->t != V_VECTOR) {
                        if (type->t == V_VECTOR) type = type->sub;
                    } else if (nftype->t == V_FUNCTION) {
                        auto csf = type->sf;
                        assert(csf);
                        // In theory it is possible this hasn't been generated yet..
                        type = csf->returntype;
                    }
                    if (rlt == LT_BORROW) {
                        auto alt = node.nf->args[sa].lt;
                        assert(alt >= LT_BORROW);
                        rlt = alt;
                    }
                    break;
                }
                case NF_ANYVAR:
                    type = ret.vttype->t == V_VECTOR ? st.Wrap(st.NewTypeVar(), V_VECTOR, &node.line)
                                                     : st.NewTypeVar();
                    assert(rlt == LT_KEEP);
                    break;
                default:
                    break;
            }
            // This allows the 0th retval to inherit the type of the 0th arg, and is
            // a bit special purpose..
            type = ActualBuiltinType(type, ret.flags,
                                        !i && node.Arity() ? node.children[0]->exptype : nullptr, node.nf, false,
                                        0, node);
            if (!IsRefNilVar(type->t)) rlt = LT_ANY;
            if (node.nf->retvals.size() > 1) {
                node.exptype->Set(i, type.get(), rlt);
                node.lt = LT_MULTIPLE;
            } else {
                node.exptype = type;
                node.lt = rlt;
            }
        }

        node.nattype = node.exptype;
        node.natlt = node.lt;
        return &node;
    }
};

TypeRef NativeCall::SimpleType(SymbolTable &) {
    // Only builtins with a single fixed scalar/string return type, and no
    // overloads whose selection could change that.
    if (nf->first->overloads) return nullptr;
    if (nf->retvals.size() != 1) return nullptr;
    auto &ret = nf->retvals[0];
    if (ret.flags != NF_NONE || ret.optional) return nullptr;
    switch (ret.vttype->t) {
        case V_INT:
        case V_FLOAT:
        case V_STRING:
            return ret.vttype;
        default:
            return nullptr;
    }
}

}  // namespace lobster
