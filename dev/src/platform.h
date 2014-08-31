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
extern bool SetupDefaultDirs(const char *exefilepath, const char *auxfilepath, bool forcecommandline);

extern uchar *LoadFile(const char *relfilename, size_t *len = NULL);
extern FILE *OpenForWriting(const char *relfilename, bool binary);
extern string SanitizePath(const char *path);

// logging:

#define MINLOGLEVEL 0  // -1 = location trace, 0 = information, 1 = important
extern void DebugLog(int lev, const char *msg, ...);
extern void ProgramOutput(const char *msg);

extern void MsgBox(const char *err);

// time:

extern void InitTime();
extern double SecondsSinceStart();

