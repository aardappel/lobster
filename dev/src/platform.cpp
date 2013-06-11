// misc platform specific stuff

#include "stdafx.h"

#ifdef WIN32
    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #define FILESEP '\\'
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

#ifdef ANDROID
#include <android/log.h>
#endif

string datadir;     // main dir to load files relative to, on windows this is where lobster.exe resides, on apple platforms it's the Resource folder in the bundle
string auxdir;      // auxiliary dir to load files from, this is where the bytecode file you're running or the main .lobster file you're compiling reside
string writedir;    // folder to write to, usually the same as auxdir, special folder on mobile platforms

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

bool SetupDefaultDirs(const char *exefilepath, const char *auxfilepath, bool forcecommandline)
{
    datadir = StripFilePart(exefilepath);
    auxdir = auxfilepath ? StripFilePart(SanitizePath(auxfilepath).c_str()) : datadir;
    writedir = auxdir;

    #ifdef __APPLE__
        if (!forcecommandline)
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
                writedir = datadir; // FIXME: this should probably be ~/Library/Application Support/AppName, but for now this works for non-app store apps
            #endif
        }
    #elif defined(ANDROID)
        datadir = "/Documents/Lobster/";  // FIXME: temp solution
        writedir = datadir;
    #endif

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

uchar *LoadFile(const char *relfilename, size_t *len)
{
    auto srfn = SanitizePath(relfilename);
    auto f = loadfile((datadir + srfn).c_str(), len);
    if (f) return f;
    f = loadfile((auxdir + srfn).c_str(), len);
    if (f) return f;
    return loadfile((writedir + srfn).c_str(), len);
}

FILE *OpenForWriting(const char *relfilename, bool binary)
{
    return fopen((writedir + SanitizePath(relfilename)).c_str(), binary ? "wb" : "w");
}

void DebugLog(int lev, const char *msg)
{
    #ifdef ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "lobster", msg)
    #elif defined(WIN32)
        if (lev >= MINLOGLEVEL) { OutputDebugStringA("LOG: "); OutputDebugStringA(msg); OutputDebugStringA("\n"); }
    #elif defined(__IOS__)
        extern void IOSLog(const char *msg);
        //IOSLog(msg);
    #else
        //printf("LOG: %s\n", msg)
    #endif
}

void MsgBox(const char *err)
{
    #if defined(__APPLE__) && !defined(__IOS__)
        DialogRef alertDialog;
        CFStringRef sr = //CFStringCreateWithCharacters(NULL, err, strlen(err));
        CFStringCreateWithCString(kCFAllocatorDefault, err, ::GetApplicationTextEncoding());
        CreateStandardAlert(kAlertStopAlert, CFSTR("Error:"), sr, NULL, &alertDialog);
        RunStandardAlert (alertDialog, NULL, NULL);
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
        gettimeofday (& t, NULL);
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
