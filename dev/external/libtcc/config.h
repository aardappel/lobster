#define TCC_VERSION "0.9.27"

#if defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
    #define TCC_TARGET_X86_64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define TCC_TARGET_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
    #define TCC_TARGET_ARM 1
#elif defined(__i386__) || defined(_M_IX86)
    #define TCC_TARGET_I386 1
#else
    #define TCC_TARGET_X86_64 1
#endif

#ifdef _WIN32
    #define TCC_TARGET_PE 1
#endif

#ifdef __APPLE__
    #define TCC_IS_NATIVE
#endif

#define CONFIG_TCC_PREDEFS 1
