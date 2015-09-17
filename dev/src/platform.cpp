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

// misc platform specific stuff

#include "stdafx.h"
#include <stdarg.h>

#ifdef WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #define FILESEP '\\'
    #include <intrin.h>
#else
    #include <sys/time.h>
    #define FILESEP '/'
#endif

#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#ifndef __IOS__
#include <Carbon/Carbon.h>
#endif
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "sdlincludes.h"
#include "sdlinterface.h"


string datadir;  // main dir to load files relative to, on windows this is where lobster.exe resides,
                 // on apple platforms it's the Resource folder in the bundle
string auxdir;   // auxiliary dir to load files from, this is where the bytecode file you're running or
                 // the main .lobster file you're compiling reside
string writedir; // folder to write to, usually the same as auxdir, special folder on mobile platforms

string StripFilePart(const char *filepath)
{
    auto fpos = strrchr(filepath, FILESEP);
    return fpos ? string(filepath, fpos - filepath + 1) : "";
}

string StripDirPart(const char *filepath)
{
    auto fpos = strrchr(filepath, FILESEP);
    if (!fpos) fpos = strrchr(filepath, ':');
    return fpos ? fpos + 1 : filepath;
}

bool SetupDefaultDirs(const char *exefilepath, const char *auxfilepath, bool from_bundle)
{
    datadir = StripFilePart(exefilepath);
    auxdir = auxfilepath ? StripFilePart(SanitizePath(auxfilepath).c_str()) : datadir;
    writedir = auxdir;

    // FIXME: use SDL_GetBasePath() instead?
    #ifdef __APPLE__
        if (from_bundle)
        {
            // default data dir is the Resources folder inside the .app bundle
            CFBundleRef mainBundle = CFBundleGetMainBundle();
            CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
            char path[PATH_MAX];
            auto res = CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX);
            CFRelease(resourcesURL);
            if (!res)
                return false;
            datadir = string(path) + "/";
            #ifdef __IOS__
                writedir = StripFilePart(path) + "Documents/"; // there's probably a better way to do this in CF
            #else
                // FIXME: this should probably be ~/Library/Application Support/AppName,
                // but for now this works for non-app store apps
                writedir = datadir;
            #endif
        }
    #elif defined(__ANDROID__)
        SDL_Init(0); // FIXME, is this needed? bad dependency.
        auto internalstoragepath = SDL_AndroidGetInternalStoragePath();
        auto externalstoragepath = SDL_AndroidGetExternalStoragePath();
        Output(OUTPUT_INFO, internalstoragepath);
        Output(OUTPUT_INFO, externalstoragepath);
        if (internalstoragepath) datadir = internalstoragepath + string("/");
        if (externalstoragepath) writedir = externalstoragepath + string("/");
        // for some reason, the above SDL functionality doesn't actually work,
        // we have to use the relative path only to access APK files:
        datadir = "";
        auxdir = writedir;
    #endif

    (void)from_bundle;
    return true;
}

string SanitizePath(const char *path)
{
    string r;
    while (*path)
    {
        if (*path == '\\' || *path == '/') r += FILESEP;
        else r += *path;
        path++;
    }
    return r;
}

uchar *LoadFilePlatform(const char *absfilename, size_t *lenret)
{
    return SDLLoadFile(absfilename, lenret);
    //return loadfile(absfilename, lenret);
}

uchar *LoadFile(const char *relfilename, size_t *lenret)
{
    auto srfn = SanitizePath(relfilename);
    auto f = LoadFilePlatform((datadir + srfn).c_str(), lenret);
    if (f) return f;
    f = LoadFilePlatform((auxdir + srfn).c_str(), lenret);
    if (f) return f;
    return LoadFilePlatform((writedir + srfn).c_str(), lenret);
}

FILE *OpenForWriting(const char *relfilename, bool binary)
{
    return fopen((writedir + SanitizePath(relfilename)).c_str(), binary ? "wb" : "w");
}

OutputType min_output_level = OUTPUT_WARN;

void Output(OutputType ot, const char *msg, ...)
{
    if (ot < min_output_level) return;
    va_list args;
    va_start(args, msg);
    #ifdef __ANDROID__
        auto tag = "lobster";
        switch (ot)
        {
            case OUTPUT_DEBUG:   __android_log_vprint(ANDROID_LOG_DEBUG, tag, msg, args); break;
            case OUTPUT_INFO:    __android_log_vprint(ANDROID_LOG_INFO,  tag, msg, args); break;
            case OUTPUT_WARN:    __android_log_vprint(ANDROID_LOG_WARN,  tag, msg, args); break;
            case OUTPUT_PROGRAM: __android_log_vprint(ANDROID_LOG_ERROR, tag, msg, args); break;
            case OUTPUT_ERROR:   __android_log_vprint(ANDROID_LOG_ERROR, tag, msg, args); break;
        }
    #elif defined(WIN32)
        char buf[1024 * 16];
        vsnprintf(buf, sizeof(buf), msg, args);
        buf[sizeof(buf) - 1] = 0;
        OutputDebugStringA("LOG: ");
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
        if (ot >= OUTPUT_WARN) printf("%s\n", buf);
    #elif defined(__IOS__)
        extern void IOSLog(const char *msg);
        IOSLog(msg);  // FIXME: args?
    #else
        vprintf(msg, args);
        printf("\n");
    #endif
    va_end(args);
}

void MsgBox(const char *err)
{
    #if defined(__APPLE__) && !defined(__IOS__)
        // FIXME: this code should never be run when running from command line
        DialogRef alertDialog;
        CFStringRef sr = //CFStringCreateWithCharacters(nullptr, err, strlen(err));
        CFStringCreateWithCString(kCFAllocatorDefault, err, ::GetApplicationTextEncoding());
        CreateStandardAlert(kAlertStopAlert, CFSTR("Error:"), sr, nullptr, &alertDialog);
        RunStandardAlert (alertDialog, nullptr, nullptr);
    #else
        (void)err;
    #endif
}

#ifndef WIN32   // emulate QPC on *nix, thanks Lee
    struct LARGE_INTEGER
    {
        long long int QuadPart;
    };

    void QueryPerformanceCounter(LARGE_INTEGER *dst)
    {
        struct timeval t;
        gettimeofday (& t, nullptr);
        dst->QuadPart = t.tv_sec * 1000000LL + t.tv_usec;
    }

    void QueryPerformanceFrequency(LARGE_INTEGER *dst)
    {
        dst->QuadPart = 1000000LL;
    }
#endif

LARGE_INTEGER freq, start;

void InitTime()
{
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);
}

double SecondsSinceStart()
{
    LARGE_INTEGER end;
    QueryPerformanceCounter(&end);
    return double(end.QuadPart - start.QuadPart) / double(freq.QuadPart);
}

// use this instead of assert to break on a condition and still be able to continue in the debugger.

void ConditionalBreakpoint(bool shouldbreak)
{
    if (shouldbreak)
    {
        #ifdef WIN32
            __debugbreak();
        #elif __GCC__
            __builtin_trap();
        #endif
    }
}

