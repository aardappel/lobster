// Copyright 2023 Wouter van Oortmerssen. All rights reserved.
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

// -------------------------------------------------------------------

// small_vector, similar to llvm::SmallVector, stores N elements in-line, only dynamically
// allocated if more than that.
// Uses memcpy on growth, so not for elements with non-trivial copy constructors.
// It stores a pointer overlapping with the fixed elements, so there is no point to
// make N smaller than sizeof(T *) / sizeof(T).

template<typename T, int N> class small_vector {
    // These could be 16-bit, but there's no easy portable way to stop the T* below from
    // padding the struct in that case.
    uint32_t len = 0;
    uint32_t cap = N;

    union {
        T elems[N];
        T *buf;
    };

    void grow() {
        uint32_t nc = len * 2;
        auto b = new T[nc];
        t_memcpy(b, data(), len);
        if (cap > N) delete[] buf;
        cap = nc;
        buf = b;
    }

    public:
    small_vector() {}

    ~small_vector() {
        if (cap > N) delete[] buf;
    }

    size_t size() {
        return len;
    }

    T *data() {
        return cap == N ? elems : buf;
    }

    bool empty() {
        return len == 0;
    }

    T &operator[](size_t i) {
        assert(i < len);
        return data()[i];
    }

    T *begin() {
        return data();
    }
    T *end() {
        return data() + len;
    }

    T &back() {
        assert(len);
        return data()[len - 1];
    }

    void push_back(const T &e) {
        if (len == cap) grow();
        data()[len++] = e;
    }

    void insert(size_t at, const T &e) {
        assert(at <= len);
        if (len == cap) grow();
        if (at != len) t_memmove(data() + at + 1, data() + at, size() - at);
        data()[at] = e;
        len++;
    }

    void erase(size_t at) {
        assert(at < len);
        if (at != len - 1) t_memmove(data() + at, data() + at + 1, size() - at - 1);
        len--;
    }
};
