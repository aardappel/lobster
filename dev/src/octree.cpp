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

#include "lobster/octree.h"

#include "lobster/glinterface.h"

namespace lobster {

inline ResourceType *GetOcTreeType() {
    static ResourceType voxel_type = { "octree", [](void *v) { delete (OcTree *)v; } };
    return &voxel_type;
}

inline OcTree &GetOcTree(StackPtr &sp, VM &vm, const Value &res) {
    return *GetResourceDec<OcTree *>(sp, vm, res, GetOcTreeType());
}

const int cur_version = 3;

}  // namespace lobster



bool FileWriteBytes(FILE *f, const void *v, size_t len) {
    return fwrite(v, len, 1, f) == 1;
}

template<typename T> bool FileWriteVal(FILE *f, const T &v) {
    return fwrite(&v, sizeof(T), 1, f) == 1;
}

template<typename T> bool FileWriteVec(FILE *f, const T &v) {
    return FileWriteVal(f, (uint64_t)v.size()) &&
        fwrite(v.data(), sizeof(typename T::value_type), v.size(), f) == v.size();
}

template<typename T> void ReadVec(const uint8_t *&p, T &v) {
    auto len = ReadMemInc<uint64_t>(p);
    v.resize((size_t)len);
    auto blen = sizeof(typename T::value_type) * v.size();
    memcpy(v.data(), p, blen);
    p += blen;
}

using namespace lobster;

static const char *magic = "CWFF";

void AddOcTree(NativeRegistry &nfr) {

nfr("oc_world_bits", "octree", "R", "I", "",
    [](StackPtr &sp, VM &vm, Value &oc) {
        return Value(GetOcTree(sp, vm, oc).world_bits);
    });

nfr("oc_buf", "octree", "R", "S", "",
    [](StackPtr &sp, VM &vm, Value &oc) {
        auto &ocworld = GetOcTree(sp, vm, oc);
        auto s = vm.NewString(string_view((char *)ocworld.nodes.data(),
                                          ocworld.nodes.size() * sizeof(OcVal)));
        return Value(s);
    });

nfr("oc_optimize", "octree", "R", "", "",
    [](StackPtr &sp, VM &vm, Value &oc) {
        auto &ocworld = GetOcTree(sp, vm, oc);
        auto numnodes1 = ocworld.NumNodes();
        ocworld.Merge();  // This should in theory not change anything.
        auto numnodes2 = ocworld.NumNodes();
        vector<OcVal> dest;
        dest.reserve(ocworld.nodes.size());
        dest.push_back(OcVal());
        ocworld.Copy(dest, OcTree::ROOT_INDEX, 0);
        ocworld.nodes = dest;
        ocworld.freelist.clear();
        ocworld.dirty.clear();
        ocworld.all_dirty = true;
        auto numnodes3 = ocworld.NumNodes();
        LOG_INFO("oc_optimize: ", numnodes1, ", ", numnodes2, ", ", numnodes3, ", ");
        return Value();
    });

nfr("oc_load", "name", "S", "R?", "",
    [](StackPtr &, VM &vm, Value &name) {
        string buf;
        auto r = LoadFile(name.sval()->strv(), &buf);
        if (r < 0) return Value();
        auto p = (const uint8_t *)buf.c_str();
        if (strncmp((const char *)p, magic, strlen(magic))) return Value();
        p += strlen(magic);
        auto version = ReadMemInc<int>(p);
        if (version > cur_version) return Value();
        auto bits = ReadMemInc<int>(p);
        auto ocworld = new OcTree(bits);
        ReadVec(p, ocworld->nodes);
        ReadVec(p, ocworld->freelist);
        assert(p == (void *)(buf.data() + buf.size()));
        return Value(vm.NewResource(ocworld, GetOcTreeType()));
    });

nfr("oc_save", "octree,name", "RS", "B", "",
    [](StackPtr &sp, VM &vm, Value &oc, Value &name) {
        auto &ocworld = GetOcTree(sp, vm, oc);
        auto f = OpenForWriting(name.sval()->strv(), true);
        if (!f) return Value(false);
        auto ok = FileWriteBytes(f, magic, strlen(magic)) &&
            FileWriteVal(f, cur_version) &&
            FileWriteVal(f, ocworld.world_bits) &&
            FileWriteVec(f, ocworld.nodes) &&
            FileWriteVec(f, ocworld.freelist);
        fclose(f);
        return Value(ok);
    });

nfr("oc_new", "world_bits,fix_bits", "II?", "R", "",
    [](StackPtr &, VM &vm, Value &world_bits, Value &fix_bits) {
        auto ocworld = new OcTree(world_bits.intval(), fix_bits.intval());
        return Value(vm.NewResource(ocworld, GetOcTreeType()));
    });

nfr("oc_set", "octree,pos,val", "RI}:3I", "", "",
    [](StackPtr &sp, VM &vm) {
        auto val = Pop(sp).intval();
        auto pos = PopVec<int3>(sp);
        auto &ocworld = GetOcTree(sp, vm, Pop(sp));
        OcVal v;
        v.SetLeafData(max(val, 0));
        if (!ocworld.Set(pos, v)) vm.BuiltinError(sp, "OcTree: grown too big (>2GB)");
    });

nfr("oc_get", "octree,pos", "RI}:3", "I", "",
    [](StackPtr &sp, VM &vm) {
        auto pos = PopVec<int3>(sp);
        auto &ocworld = GetOcTree(sp, vm, Pop(sp));
        Push(sp,  ocworld.nodes[ocworld.Get(pos).first].LeafData());
    });

nfr("oc_buffer_update", "octree,uname,ssbo", "RSI", "I", "",
    [](StackPtr &sp, VM &vm) {
        auto ssbo = Pop(sp).True();
        auto name = Pop(sp).sval()->strv();
        auto &ocworld = GetOcTree(sp, vm, Pop(sp));
        extern Shader *currentshader;
        iint num_updates = 0;
        if (ocworld.all_dirty) {
            // Can't do partial update.
            UniformBufferObject(currentshader, ocworld.nodes.data(),
                sizeof(OcVal) * ocworld.nodes.size(), -1, name, ssbo, 0);
            num_updates = -1;
            ocworld.all_dirty = false;
        } else {
            for (auto [i, is_dirty] : enumerate(ocworld.dirty)) {
                if (!is_dirty) continue;
                num_updates++;
                // TODO: experiment with uploading more data per call.
                auto offset = i * OcTree::NODES_PER_DIRTY_BIT * OcTree::ELEMENTS_PER_NODE + OcTree::ROOT_INDEX;
                auto size = min((size_t)OcTree::NODES_PER_DIRTY_BIT * OcTree::ELEMENTS_PER_NODE, ocworld.nodes.size() - offset);
                UniformBufferObject(currentshader, ocworld.nodes.data() + offset,
                    sizeof(OcVal) * size, offset * sizeof(OcVal), name, ssbo, 0);
            }
        }
        ocworld.dirty.clear();
        Push(sp,  num_updates);
    });

nfr("aabb_tree", "positions,values,cam", "F}:3]I]F}:3", "S", "",
    [](StackPtr &sp, VM &vm) {
        auto cam = PopVec<float3>(sp);
        auto values = Pop(sp).vval();
        auto positions = Pop(sp).vval();
        if (!positions->len) {
            Push(sp, vm.NewString(0));
            return;
        }
        struct S {
            float3 lo;
            int left_or_val;  // Negative is val.
            float3 hi;
            int fail;
        };
        struct N {
            N *left, *right;
            iint val;
            float3 lo, hi, center;
            float cam_dist;
            N *insert(N *nn, vector<N> &nodes) {
                if (left) {
                    if (squaredlength(left->center - nn->center) <
                        squaredlength(right->center - nn->center)) {
                        left = left->insert(nn, nodes);
                    } else {
                        right = right->insert(nn, nodes);
                    }
                    lo = min(left->lo, right->lo);
                    hi = max(left->hi, right->hi);
                    center = (lo + hi) / 2;
                    return this;
                } else {
                    auto nlo = min(lo, nn->lo);
                    auto nhi = max(hi, nn->hi);
                    nodes.emplace_back(N { this, nn, -1, nlo, nhi, (nlo + nhi) / 2, 0 });
                    return &nodes.back();
                }
            }
            void balance(float3 cam) {
                // Sorting by closest point is significantly faster than center?
                cam_dist = length(point_to_box(cam, lo, hi));
                if (!left) return;
                left->balance(cam);
                right->balance(cam);
                if (left->cam_dist >
                    right->cam_dist) {
                    swap(left, right);
                }
            };
            int serialize(int fail, S *&p, int &i) {
                if (left) {
                    auto b = right->serialize(fail, p, i);
                    auto a = left->serialize(b, p, i);
                    *p++ = { lo, a, hi, fail };
                } else {
                    *p++ = { lo, -(int)val, hi, fail };
                }
                return i++;
            }
        };
        vector<N> nodes;
        nodes.reserve(positions->len + positions->len - 1);
        N *root = nodes.data();
        for (iint i = 0; i < positions->len; i++) {
            auto pos = ValueToFLT<3>(positions->AtSt(i), positions->width);
            nodes.emplace_back(N {
                nullptr, nullptr, i < values->len ? values->At(i).ival() : 0,
                pos - 0.5, pos + 0.5, pos, 0
            });
            if (!i) continue;
            root = root->insert(&nodes.back(), nodes);
        }
        root->balance(cam);
        auto s = vm.NewString(nodes.size() * sizeof(S));
        auto p = (S *)s->data();
        int i = 0;
        root->serialize(-1, p, i);
        Push(sp, s);
    });
}

