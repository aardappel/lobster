#define DECLSPEC 
#define SDL_NO_COMPAT
// hack: so we can share one include folder for SDL
#ifdef __APPLE__
    #ifdef __IOS__
        #include "SDL_config_iphoneos.h"
    #else
        #include "SDL_config_macosx.h"
    #endif
#else
    #ifdef ANDROID
        #include "SDL_config_android.h"
    #else
        #ifdef WIN32
            #include "SDL_config_windows.h"
        #else
            #include "SDL_config.h"
        #endif
    #endif
#endif
#define _SDL_config_h
#include "SDL.h"
