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

#ifndef LOBSTER_OCTREE_H
#define LOBSTER_OCTREE_H

class OcVal {
    int32_t node;
public:
    OcVal() { SetLeafData(0); }
    OcVal(int32_t n) { SetNodeIdx(n); }
    bool IsLeaf() const { return node < 0; }
    int32_t NodeIdx() const { assert(node >= 0); return node; }
    void SetNodeIdx(int32_t n) { assert(n >= 0); node = n; }
    // leaf data is a 31-bit usigned integer.
    int32_t LeafData() const { assert(node < 0); return node & 0x7FFFFFFF; }
    void SetLeafData(int32_t v) { assert(v >= 0); node = v | 0x80000000; }
    bool operator==(const OcVal &o) const { return node == o.node; }
    bool operator!=(const OcVal &o) const { return node != o.node; }
};

struct OcTree {
    vector<OcVal> nodes;
    vector<int> freelist;
    vector<uint8_t> dirty;
    int world_bits;
    int fix_bits;
    bool all_dirty = true;
    enum {
        OCTREE_SUBDIV = 8,  // This one is kind of a given..
        PARENT_INDEX = OCTREE_SUBDIV,
        ELEMENTS_PER_NODE = OCTREE_SUBDIV + 1,  // Last is parent pointer.
        ROOT_INDEX = 1,  // Such that index 0 means "no parent".
        NUM_NEW_NODES_PER_RESIZE = 10000,
        NODES_PER_DIRTY_BIT = 64
    };

    size_t NumNodes() {
        return nodes.size() / ELEMENTS_PER_NODE - freelist.size();
    }

    OcTree(int world_bits, int fix_bits = 0) : world_bits(world_bits), fix_bits(fix_bits) {
        nodes.push_back(OcVal());  // Unused element before root at offset 0.
        RecInit(fix_bits, 0);
    }

    int RecInit(int fbitsr, int parent) {
        int cur = (int)nodes.size();
        for (int i = 0; i < OCTREE_SUBDIV; i++) nodes.push_back(OcVal());
        nodes.push_back({ parent });
        if (fbitsr) {
            for (int i = 0; i < OCTREE_SUBDIV; i++) {
                auto child = RecInit(fbitsr - 1, cur + i);
                nodes[cur + i].SetNodeIdx(child);
            }
        }
        return cur;
    }

    int ToParent(int i) { return i - ((i - ROOT_INDEX) % ELEMENTS_PER_NODE); }
    int Deref(int children) { return nodes[children + PARENT_INDEX].NodeIdx(); }

    void Dirty(int i) {
        // FIXME: simplify representation to simplify this.
        // ROOT_INDEX could be remove in favor of -1
        // node should be a struct of 9 elements.
        i -= ROOT_INDEX;
        i /= ELEMENTS_PER_NODE;
        i /= NODES_PER_DIRTY_BIT;
        if (i >= (int)dirty.size()) dirty.resize(i + 1, false);
        dirty[i] = true;
    }

    int Step(const int3 &pos, int bit, int cur) {
        auto size = 1 << bit;
        auto off = pos & size;
        auto bv = off >> bit;
        return cur + dot(bv, int3(1, 2, 4));
    }

    bool Set(const int3 &pos, OcVal val) {
        int cur = ROOT_INDEX;
        for (auto bit = world_bits - 1; ; bit--) {
            auto ccur = Step(pos, bit, cur);
            auto oval = nodes[ccur];
            if (oval == val) return true;
            if (bit) {  // Not at bottom yet.
                if (oval.IsLeaf()) {  // Values are not equal, so we must subdivide.
                    int ncur;
                    OcVal parent(ccur);
                    if (freelist.empty()) {
                        auto newsize = nodes.size() + NUM_NEW_NODES_PER_RESIZE * ELEMENTS_PER_NODE;
                        if (newsize >= 0x80000000) {
                            return false;
                        }
                        freelist.resize(NUM_NEW_NODES_PER_RESIZE);
                        for (int i = 0; i < NUM_NEW_NODES_PER_RESIZE; i++) {
                            // Backwards, so they get consumed forwards.
                            freelist[NUM_NEW_NODES_PER_RESIZE - 1 - i] =
                                (int)nodes.size() + i * ELEMENTS_PER_NODE;
                        }
                        nodes.resize(newsize);
                        all_dirty = true;
                    }
                    ncur = freelist.back();
                    freelist.pop_back();
                    for (int i = 0; i < OCTREE_SUBDIV; i++) nodes[ncur + i] = oval;
                    nodes[ncur + OCTREE_SUBDIV] = parent;
                    Dirty(ncur);
                    nodes[ccur].SetNodeIdx(ncur);
                    Dirty(ccur);
                    cur = ncur;
                } else {
                    cur = oval.NodeIdx();
                }
            } else {  // Bottom level.
                assert(val.IsLeaf());
                nodes[ccur] = val;
                Dirty(ccur);
                // Try to merge this level all the way to the top.
                for (int pbit = 1 + fix_bits; pbit < world_bits; pbit++) {
                    for (int i = 1; i < OCTREE_SUBDIV; i++) {  // If all 8 are the same..
                        if (nodes[cur] != nodes[cur + i]) return true;
                    }
                    // Merge.
                    auto parent = Deref(cur);
                    assert(cur == nodes[parent].NodeIdx());
                    nodes[parent] = nodes[cur];
                    Dirty(parent);
                    freelist.push_back(cur);
                    cur = ToParent(parent);
                }
                return true;
            }
        }
        return true;
    }

    pair<int, int> Get(const int3 &pos) {
        int bit = world_bits;
        int i = Get(pos, bit, ROOT_INDEX);
        return { i, bit };
    }

    int Get(const int3 &pos, int &bit, int cur) {
        for (;;) {
            bit--;
            auto ccur = Step(pos, bit, cur);
            auto oval = nodes[ccur];
            if (oval.IsLeaf()) {
                return ccur;
            } else {
                cur = oval.NodeIdx();
            }
        }
    }

    int GetLeaf(const int3 &pos) {
        return nodes[Get(pos).first].LeafData();
    }

    int Copy(vector<OcVal> &dest, int src, int parent) {
        int cur = (int)dest.size();
        for (int i = 0; i < OCTREE_SUBDIV; i++) {
            dest.push_back(nodes[src + i]);
        }
        dest.push_back(parent);
        for (int i = 0; i < OCTREE_SUBDIV; i++) {
            auto v = nodes[src + i];
            if (!v.IsLeaf()) dest[cur + i] = Copy(dest, v.NodeIdx(), cur + i);
        }
        return cur;
    }

    // This function is only needed when creating nodes without Set, as Set already merges
    // on the fly.
    void Merge() { Merge(ROOT_INDEX, fix_bits); }
    OcVal Merge(int cur, int fbitsr) {
        for (int i = 0; i < OCTREE_SUBDIV; i++) {
            auto &n = nodes[cur + i];
            if (!n.IsLeaf()) {
                auto nn = Merge(n.NodeIdx(), fbitsr - 1);
                if (n != nn) {
                    n = nn;
                    Dirty(cur);
                }
            }
        }
        OcVal ov(cur);
        if (!nodes[cur].IsLeaf()) return ov;
        for (int i = 1; i < OCTREE_SUBDIV; i++) {
            if (nodes[cur] != nodes[cur + i]) return ov;
        }
        if (fbitsr < 0) {
            freelist.push_back(cur);
            ov = nodes[cur];
        }
        return ov;
    }

};

namespace lobster {

extern OcTree &GetOcTree(VM &vm, const Value &res);

}

#endif  // LOBSTER_OCTREE_H
