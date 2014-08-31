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

#include "stdint.h"

#ifdef WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#else
    #ifndef __ANDROID__
        #include <glob.h>
    #endif
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

void AddDirItem(LVector *list, const char *filename, int64_t size, int divisor)
{
    auto elem = g_vm->NewVector(2, V_VECTOR);
    elem->push(Value(g_vm->NewString(filename, strlen(filename))));
    if (size >= 0)
    {
        size /= divisor;
        if (size > 0x7FFFFFFF) size = 0x7FFFFFFF;
    }
    elem->push(Value(int(size)));
    list->push(Value(elem));
}

void AddFileOps()
{
    STARTDECL(scan_folder) (Value &fld, Value &divisor)
    {
        string folder = SanitizePath(fld.sval->str());
        fld.DEC();

        if (divisor.ival <= 0) divisor.ival = 1;

        #ifdef WIN32

        WIN32_FIND_DATA fdata;
        HANDLE fh = FindFirstFile((folder + "\\*.*").c_str(), &fdata);
        if (fh == INVALID_HANDLE_VALUE) return Value(0, V_NIL);

        auto list = g_vm->NewVector(0, V_VECTOR);

        do
        {
            if (strcmp(fdata.cFileName, ".") && strcmp(fdata.cFileName, ".."))
            {
                ULONGLONG size = (static_cast<ULONGLONG>(fdata.nFileSizeHigh) << (sizeof(uint) * 8)) | 
                                 fdata.nFileSizeLow;
                AddDirItem(list, fdata.cFileName, fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? -1 : size,
                           divisor.ival);
            }
        }
        while(FindNextFile(fh, &fdata));
        FindClose(fh);
        return Value(list);

        #elif !defined(__ANDROID__)

        glob_t gl;
        string mask = folder + "/*";
        if (glob(mask.c_str(), GLOB_MARK | GLOB_TILDE, NULL, &gl)) return Value(0, V_NIL);

        auto list = g_vm->NewVector(0, V_VECTOR);

        for (size_t fi = 0; fi < gl.gl_pathc; fi++)
        {
            string xFileName = gl.gl_pathv[fi];
            bool isDir = xFileName[xFileName.length()-1] == '/';
            if (isDir) xFileName = xFileName.substr(0, xFileName.length() - 1);
            string cFileName = xFileName.substr(xFileName.find_last_of('/') + 1);
            struct stat st;
            stat(gl.gl_pathv[fi], &st);

            AddDirItem(list, cFileName.c_str(), isDir ? -1 : st.st_size, divisor.ival);
        }
        globfree(&gl);
        return Value(list);

        #else

        return Value(0, V_NIL);

        #endif
    }
    ENDDECL2(scan_folder, "folder,divisor", "SI", "I",
        "returns a vector of all elements in a folder, each element is [ name,  filesize (-1 if directory) ]."
        " Specify 1 as divisor to get sizes in bytes, 1024 for kb etc. Values > 0x7FFFFFFF will be clamped."
        " Returns nil if folder couldn't be scanned.");

    STARTDECL(read_file) (Value &file)
    {
        size_t sz = 0;
        auto buf = (char *)LoadFile(file.sval->str(), &sz);
        file.DEC();
        if (!buf) return Value(0, V_NIL);
        auto s = g_vm->NewString(buf, sz);
        free(buf);
        return Value(s);
    }
    ENDDECL1(read_file, "file", "S", "S",
        "returns the contents of a file as a string, or nil if the file can't be found."
        " you may use either \\ or / as path separators");

    STARTDECL(write_file) (Value &file, Value &contents)
    {
        FILE *f = OpenForWriting(file.sval->str(), true);
        file.DEC();
        size_t written = 0;
        if (f)
        {
            written = fwrite(contents.sval->str(), contents.sval->len, 1, f);
            fclose(f);
        }
        contents.DEC();
        return Value(written == 1);
    }
    ENDDECL2(write_file, "file,contents", "SS", "I",
        "creates a file with the contents of a string, returns false if writing wasn't possible");
}

AutoRegister __afo("file", AddFileOps);
