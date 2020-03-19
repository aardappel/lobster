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

inline OcTree &GetOcTree(VM &vm, const Value &res) {
    return *GetResourceDec<OcTree *>(vm, res, GetOcTreeType());
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

template<typename T> void ReadVec(const uchar *&p, T &v) {
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
    [](VM &vm, Value &oc) {
        return Value(GetOcTree(vm, oc).world_bits);
    });

nfr("oc_buf", "octree", "R", "S", "",
    [](VM &vm, Value &oc) {
        auto &ocworld = GetOcTree(vm, oc);
        auto s = vm.NewString(string_view((char *)ocworld.nodes.data(),
                                          ocworld.nodes.size() * sizeof(OcVal)));
        return Value(s);
    });

nfr("oc_load", "name", "S", "R?", "",
    [](VM &vm, Value &name) {
        string buf;
        auto r = LoadFile(name.sval()->strv(), &buf);
        if (r < 0) return Value();
        auto p = (const uchar *)buf.c_str();
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
    [](VM &vm, Value &oc, Value &name) {
        auto &ocworld = GetOcTree(vm, oc);
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
    [](VM &vm, Value &world_bits, Value &fix_bits) {
        auto ocworld = new OcTree(world_bits.intval(), fix_bits.intval());
        return Value(vm.NewResource(ocworld, GetOcTreeType()));
    });

nfr("oc_set", "octree,pos,val", "RI}:3I", "", "",
    [](VM &vm) {
        auto val = vm.Pop().intval();
        auto pos = vm.PopVec<int3>();
        auto &ocworld = GetOcTree(vm, vm.Pop());
        OcVal v;
        v.SetLeafData(max(val, 0));
        ocworld.Set(pos, v);
    });

nfr("oc_get", "octree,pos", "RI}:3", "I", "",
    [](VM &vm) {
        auto pos = vm.PopVec<int3>();
        auto &ocworld = GetOcTree(vm, vm.Pop());
        vm.Push(ocworld.nodes[ocworld.Get(pos).first].LeafData());
    });

nfr("oc_buffer_update", "octree,uname,ssbo", "RSI", "I", "",
    [](VM &vm) {
        auto ssbo = vm.Pop().True();
        auto name = vm.Pop().sval()->strv();
        auto &ocworld = GetOcTree(vm, vm.Pop());
        extern Shader *currentshader;
        intp num_updates = 0;
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
        vm.Push(num_updates);
    });
}
