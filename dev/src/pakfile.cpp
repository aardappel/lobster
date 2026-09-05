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

#include "lobster/compiler.h"

#define FLATBUFFERS_DEBUG_VERIFICATION_FAILURE
#include "lobster/bytecode_generated.h"

namespace lobster {

bool IsCompressed(string_view filename) {
    auto dot = filename.find_last_of('.');
    if (dot == string_view::npos) return false;
    auto ext = filename.substr(dot);
    return ext == ".c" || ext == ".lbc" || ext == ".lobster" || ext == ".materials" || ext == ".glsl";
}

static const uint8_t *magic = (uint8_t *)"LPAK";
static const size_t magic_size = 4;
static const size_t header_size = magic_size + sizeof(int64_t) * 4;
static const char *mdname = "metadata.lbc";
static const char *ccname = "c_codegen.c";
static const int64_t current_version = 2;

template <typename T> int64_t LE(T x) { return flatbuffers::EndianScalar((int64_t)x); }

string BuildPakFile(string &pakfile, string &metadata_buffer, set<string> &files, uint64_t src_hash,
                    const string &c_codegen) {
    // All offsets in 64bit, just in-case we ever want pakfiles > 4GB :)
    // Since we're building this in memory, they can only be created by a 64bit build.
    vector<int64_t> filestarts;
    vector<int64_t> namestarts;
    vector<int64_t> uncompressed;
    vector<string> filenames;
    auto add_file = [&](string_view buf, string_view filename) {
        filestarts.push_back(LE(pakfile.size()));
        filenames.push_back(string(filename));
        LOG_INFO("adding to pakfile: ", filename);
        if (IsCompressed(filename)) {
            string out;
            WEntropyCoder<true>((uint8_t *)buf.data(), buf.length(), buf.length(), out);
            pakfile += out;
            uncompressed.push_back(buf.length());
        } else {
            pakfile += buf;
            uncompressed.push_back(-1);
        }
    };
    // Start with a magic id, just for the hell of it.
    pakfile.insert(pakfile.end(), magic, magic + magic_size);
    // Metadata always first entry.
    add_file(metadata_buffer, mdname);
    if (!c_codegen.empty()) {
        add_file(c_codegen, ccname);
    }
    // Followed by all files.
    files.insert("data/shaders/default.materials");  // If it hadn't already been added.
    string buf;
    function<string(const string &)> addrec;
    addrec = [&](const string &filename) -> string {
        auto l = LoadFile(filename, &buf);
        if (l >= 0) {
            add_file(buf, filename);
        } else if (LoadFile("modules/" + filename, &buf) >= 0) {
            // A source file a `pakfile` names is found the way an import is, see LoadedFile,
            // and goes in under the name that found it, which is how it is looked up again.
            add_file(buf, "modules/" + filename);
        } else {
            auto base = filename;
            auto pat = string{};
            auto pos = filename.find("#");
            if (pos != filename.npos) {
                pat = filename.substr(pos + 1);
                base = filename.substr(0, pos);
            }
            vector<DirectoryInfo> dir;
            if (!ScanDir(base, dir)) return "cannot load file/dir for pakfile: " + filename;
            for (auto &entry : dir) {
                if (!pat.empty() && entry.name.find(pat) == entry.name.npos) continue;
                auto fn = base;
                if (fn.back() != '/') fn += "/";
                fn += entry.name;
                auto err = addrec(fn);
                if (!err.empty()) return err;
            }
        }
        return "";
    };
    for (auto &filename : files) {
        auto err = addrec(filename);
        if (!err.empty()) return err;
    }
    // Now we can write the directory, first the names:
    auto dirstart = LE(pakfile.size());
    for (auto &filename : filenames) {
        namestarts.push_back(LE(pakfile.size()));
        pakfile.insert(pakfile.end(), filename.c_str(), filename.c_str() + filename.length() + 1);
    }
    // Then the starting offsets and other data:
    pakfile.insert(pakfile.end(), (uint8_t *)uncompressed.data(),
        (uint8_t *)(uncompressed.data() + uncompressed.size()));
    pakfile.insert(pakfile.end(), (uint8_t *)filestarts.data(),
        (uint8_t *)(filestarts.data() + filestarts.size()));
    pakfile.insert(pakfile.end(), (uint8_t *)namestarts.data(),
        (uint8_t *)(namestarts.data() + namestarts.size()));
    auto num = LE(filestarts.size());
    // Finally the "header" (or do we call this a "tailer" ? ;)
    auto header_start = pakfile.size();
    auto version = LE(current_version);
    auto src_hash_le = LE(src_hash);
    pakfile.insert(pakfile.end(), (uint8_t *)&src_hash_le, (uint8_t *)(&src_hash_le + 1));
    pakfile.insert(pakfile.end(), (uint8_t *)&num, (uint8_t *)(&num + 1));
    pakfile.insert(pakfile.end(), (uint8_t *)&dirstart, (uint8_t *)(&dirstart + 1));
    pakfile.insert(pakfile.end(), (uint8_t *)&version, (uint8_t *)(&version + 1));
    pakfile.insert(pakfile.end(), magic, magic + magic_size);
    assert(pakfile.size() - header_start == header_size);
    (void)header_start;
    return "";
}

// This just loads the directory part of a pakfile such that subsequent LoadFile calls know how
// to load from it.
bool LoadPakDir(const char *lpak, uint64_t &src_hash_dest) {
    // This supports reading from a pakfile > 4GB even on a 32bit system! (as long as individual
    // files in it are <= 4GB).
    auto plen = LoadFile(lpak, nullptr, 0, 0);
    if (plen < 0) return false;
    string header;
    if (LoadFile(lpak, &header, plen - (int64_t)header_size, header_size) < 0 ||
        memcmp(header.c_str() + header_size - magic_size, magic, magic_size)) return false;
    auto read_unaligned64 = [](const void *p) {
        int64_t r;
        memcpy(&r, p, sizeof(int64_t));
        return LE(r);
    };
    auto src_hash = (uint64_t)read_unaligned64((int64_t *)header.c_str());
    auto num = read_unaligned64((int64_t *)header.c_str() + 1);
    auto dirstart = read_unaligned64((int64_t *)header.c_str() + 2);
    auto version = read_unaligned64((int64_t *)header.c_str() + 3);
    if (version != current_version) return false;
    if (dirstart > plen) return false;
    string dir;
    if (LoadFile(lpak, &dir, dirstart, plen - dirstart - (int64_t)header_size) < 0)
        return false;
    auto namestarts = (int64_t *)(dir.c_str() + dir.length()) - num;
    auto filestarts = namestarts - num;
    auto uncompressed = filestarts - num;
    for (int64_t i = 0; i < num; i++) {
        auto name = string_view(dir.c_str() + (read_unaligned64(namestarts + i) - dirstart));
        auto off = read_unaligned64(filestarts + i);
        auto end = i < num - 1 ? read_unaligned64(filestarts + i + 1) : dirstart;
        auto len = end - off;
        LOG_INFO("pakfile dir: ", name, " : ", len);
        AddPakFileEntry(lpak, name, off, len, read_unaligned64(uncompressed + i));
    }
    src_hash_dest = src_hash;
    return true;
}

bool LoadMetaDataAndCode(string &metadata, string &c_codegen) {
    if (LoadFile(mdname, &metadata) < 0) return false;
    LoadFile(ccname, &c_codegen);
    flatbuffers::Verifier verifier((const uint8_t *)metadata.c_str(), metadata.length());
    auto ok = metadata::VerifyMetadataFileBuffer(verifier);
    assert(ok);
    return ok;
}

}  // namespace lobster
