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

// platform independent file access:

extern string StripFilePart(const char *filepath);
extern string StripDirPart(const char *filepath);

// call this at init to determine default folders to load stuff from
extern bool SetupDefaultDirs(const char *exefilepath, const char *auxfilepath, bool from_bundle);

extern uchar *LoadFile(const char *relfilename, size_t *len = nullptr);
extern FILE *OpenForWriting(const char *relfilename, bool binary);
extern string SanitizePath(const char *path);

// logging:

enum OutputType
{
    OUTPUT_DEBUG,    // temp spam, should eventually be removed.
    OUTPUT_INFO,     // output that helps understanding what the code is doing when not under a debugger
    OUTPUT_WARN,     // non-critical issues, e.g. SDL errors.
    OUTPUT_ERROR,    // compiler & vm errors, program terminates after this
    OUTPUT_PROGRAM,  // output by the Lobster code, should always be shown
};

extern OutputType min_output_level;  // defaults to showing OUTPUT_WARN and up.

extern void Output(OutputType ot, const char *msg, ...);

extern void MsgBox(const char *err);

// time:

extern void InitTime();
extern double SecondsSinceStart();


#if defined(__IOS__) || defined(__ANDROID__)
    // This assumes OpenGL ES + touch screen as opposed to Desktop GL + mouse.
    #define PLATFORM_MOBILE
#endif