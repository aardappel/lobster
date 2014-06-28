// OpenGL platform definitions

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #ifdef __IOS__
        //#include <SDL_opengles2.h>
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #else
        #include <OpenGL/gl.h>
    #endif
#else
    #ifdef __ANDROID__
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>    
    #else   // WIN32 & Linux
        #ifdef WIN32
            #define VC_EXTRALEAN
            #define WIN32_LEAN_AND_MEAN
            #define NOMINMAX
            #include <windows.h>
        #endif
        #include <GL/gl.h>
        #include <GL/glext.h>
        #ifdef WIN32
            #define GLBASEEXTS \
                GLEXT(PFNGLACTIVETEXTUREARBPROC       , glActiveTexture           )
        #else
            #define GLBASEEXTS
        #endif
        #define GLEXTS \
            GLEXT(PFNGLGENBUFFERSARBPROC              , glGenBuffers              ) \
            GLEXT(PFNGLBINDBUFFERARBPROC              , glBindBuffer              ) \
            GLEXT(PFNGLMAPBUFFERARBPROC               , glMapBuffer               ) \
            GLEXT(PFNGLUNMAPBUFFERARBPROC             , glUnmapBuffer             ) \
            GLEXT(PFNGLBUFFERDATAARBPROC              , glBufferData              ) \
            GLEXT(PFNGLBUFFERSUBDATAARBPROC           , glBufferSubData           ) \
            GLEXT(PFNGLDELETEBUFFERSARBPROC           , glDeleteBuffers           ) \
            GLEXT(PFNGLGETBUFFERSUBDATAARBPROC        , glGetBufferSubData        ) \
            GLEXT(PFNGLVERTEXATTRIBPOINTERARBPROC     , glVertexAttribPointer     ) \
            GLEXT(PFNGLENABLEVERTEXATTRIBARRAYARBPROC , glEnableVertexAttribArray ) \
            GLEXT(PFNGLDISABLEVERTEXATTRIBARRAYARBPROC, glDisableVertexAttribArray) \
            GLEXT(PFNGLCREATEPROGRAMPROC              , glCreateProgram           ) \
            GLEXT(PFNGLDELETEPROGRAMPROC              , glDeleteProgram           ) \
            GLEXT(PFNGLDELETESHADERPROC               , glDeleteShader            ) \
            GLEXT(PFNGLUSEPROGRAMPROC                 , glUseProgram              ) \
            GLEXT(PFNGLCREATESHADERPROC               , glCreateShader            ) \
            GLEXT(PFNGLSHADERSOURCEPROC               , glShaderSource            ) \
            GLEXT(PFNGLCOMPILESHADERPROC              , glCompileShader           ) \
            GLEXT(PFNGLGETPROGRAMIVARBPROC            , glGetProgramiv            ) \
            GLEXT(PFNGLGETSHADERIVPROC                , glGetShaderiv             ) \
            GLEXT(PFNGLGETPROGRAMINFOLOGPROC          , glGetProgramInfoLog       ) \
            GLEXT(PFNGLGETSHADERINFOLOGPROC           , glGetShaderInfoLog        ) \
            GLEXT(PFNGLATTACHSHADERPROC               , glAttachShader            ) \
            GLEXT(PFNGLLINKPROGRAMARBPROC             , glLinkProgram             ) \
            GLEXT(PFNGLGETUNIFORMLOCATIONARBPROC      , glGetUniformLocation      ) \
            GLEXT(PFNGLUNIFORM1FARBPROC               , glUniform1f               ) \
            GLEXT(PFNGLUNIFORM2FARBPROC               , glUniform2f               ) \
            GLEXT(PFNGLUNIFORM3FARBPROC               , glUniform3f               ) \
            GLEXT(PFNGLUNIFORM4FARBPROC               , glUniform4f               ) \
            GLEXT(PFNGLUNIFORM1FVARBPROC              , glUniform1fv              ) \
            GLEXT(PFNGLUNIFORM2FVARBPROC              , glUniform2fv              ) \
            GLEXT(PFNGLUNIFORM3FVARBPROC              , glUniform3fv              ) \
            GLEXT(PFNGLUNIFORM4FVARBPROC              , glUniform4fv              ) \
            GLEXT(PFNGLUNIFORM1IARBPROC               , glUniform1i               ) \
            GLEXT(PFNGLUNIFORMMATRIX4FVARBPROC        , glUniformMatrix4fv        ) \
            GLEXT(PFNGLUNIFORMMATRIX4FVARBPROC/*type*/, glUniformMatrix3x4fv      ) \
            GLEXT(PFNGLBINDATTRIBLOCATIONARBPROC      , glBindAttribLocation      ) \
            GLEXT(PFNGLGETACTIVEUNIFORMARBPROC        , glGetActiveUniform        ) \
            GLEXT(PFNGLGENERATEMIPMAPEXTPROC          , glGenerateMipmap          ) 

        #define GLEXT(type, name) extern type name;
            GLBASEEXTS
            GLEXTS
        #undef GLEXT
    #endif
#endif
