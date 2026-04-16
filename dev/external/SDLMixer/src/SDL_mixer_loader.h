#if !defined(MIX_LOADER_FUNCTIONS) || !defined(MIX_LOADER_MODULE)
#error Please define MIX_LOADER_FUNCTIONS and MIX_LOADER_MODULE before including this file.
#else

#define MIX_LOADER_SYMCOMBINE2(a, b) a##b
#define MIX_LOADER_SYMCOMBINE(a, b) MIX_LOADER_SYMCOMBINE2(a, b)

typedef struct {
    int refcount;
    void *handle;
    #define MIX_LOADER_FUNCTION(required,ret,fn,args) ret (*fn) args;
    MIX_LOADER_FUNCTIONS
    #undef MIX_LOADER_FUNCTION
} MIX_LOADER_SYMCOMBINE(MIX_LOADER_MODULE, _loader);

static MIX_LOADER_SYMCOMBINE(MIX_LOADER_MODULE, _loader) MIX_LOADER_MODULE;

static void MIX_LOADER_SYMCOMBINE(UnloadModule_, MIX_LOADER_MODULE)(void)
{
    if (--MIX_LOADER_MODULE.refcount <= 0) {
        #define MIX_LOADER_FUNCTION(required,ret,fn,args) MIX_LOADER_MODULE.fn = NULL;
        MIX_LOADER_FUNCTIONS
        #undef MIX_LOADER_FUNCTION
        #ifdef MIX_LOADER_DYNAMIC
        if (MIX_LOADER_MODULE.handle) {
            SDL_UnloadObject(MIX_LOADER_MODULE.handle);
            MIX_LOADER_MODULE.handle = NULL;
        }
        #endif
        MIX_LOADER_MODULE.refcount = 0;  // just in case.
    }
}

#ifdef __APPLE__  // Need to turn off optimizations so weak framework load check works, apparently.
__attribute__ ((optnone))
#endif
static bool MIX_LOADER_SYMCOMBINE(LoadModule_, MIX_LOADER_MODULE)(void)
{
    bool retval = true;

    if (MIX_LOADER_MODULE.refcount++ == 0) {
        #ifdef MIX_LOADER_DYNAMIC
            MIX_LOADER_MODULE.handle = SDL_LoadObject(MIX_LOADER_DYNAMIC);
            if (MIX_LOADER_MODULE.handle == NULL) {
                MIX_LOADER_MODULE.refcount--;
                return false;
            }

            #define MIX_LOADER_FUNCTION(required,ret,fn,args) \
                MIX_LOADER_MODULE.fn = (ret(*)args) SDL_LoadFunction(MIX_LOADER_MODULE.handle, #fn); \
                if ((MIX_LOADER_MODULE.fn == NULL) && ((required))) { \
                    SDL_SetError("Missing %s in library " MIX_LOADER_DYNAMIC, #fn); \
                    retval = false; \
                }
        #else
            #define MIX_LOADER_FUNCTION(required,ret,fn,args) \
                MIX_LOADER_MODULE.fn = fn; \
                if ((MIX_LOADER_MODULE.fn == NULL) && ((required))) { \
                    SDL_SetError("Missing " SDL_STRINGIFY_ARG(MIX_LOADER_MODULE) " framework"); \
                    retval = false; \
                }
        #endif

        MIX_LOADER_FUNCTIONS
        #undef MIX_LOADER_FUNCTION

        if (!retval) {
            MIX_LOADER_SYMCOMBINE(UnloadModule_, MIX_LOADER_MODULE)();
        }
    }

    return retval;
}

#undef MIX_LOADER_SYMCOMBINE2
#undef MIX_LOADER_SYMCOMBINE
#undef MIX_LOADER_FUNCTIONS
#undef MIX_LOADER_MODULE

#ifdef MIX_LOADER_DYNAMIC
#undef MIX_LOADER_DYNAMIC
#endif

#endif

