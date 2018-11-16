# Emscripten makefile

CXX= emcc
CC= emcc

# use -g4 for maximum debug info.
OPTLEVEL= -g4 -O2
CXXFLAGS= $(OPTLEVEL)
# apparently exception catching is slow, but we need them for the moment.
# Note: exceptions disabled because of: https://github.com/kripken/emscripten/issues/7399
override CXXFLAGS+= -s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1 -std=c++17 -Wall -pedantic -Wno-switch \
                    -Wno-array-bounds -Wno-gnu-anonymous-struct -Wno-nested-anon-types
CFLAGS= $(OPTLEVEL)
override CFLAGS+= -Wall

INCLUDES= -I. -I../include -I../src -I../external/freetype/include -I../external/SDLMixer
LIBS=

CPPSRCS= \
	../src/audio.cpp \
	../src/builtins.cpp \
	../src/engine.cpp \
	../src/file.cpp \
	../src/font.cpp \
	../src/fontrenderer.cpp \
	../src/glgeom.cpp \
	../src/glloadiqm.cpp \
	../src/glshader.cpp \
	../src/glsystem.cpp \
	../src/gltexture.cpp \
	../src/glvr.cpp \
	../src/steamworks.cpp \
	../src/cubegen.cpp \
	../src/graphics.cpp \
	../src/compiler.cpp \
	../src/vmlog.cpp \
    ../src/vm.cpp \
    ../src/main.cpp \
	../src/lobsterreader.cpp \
	../src/meshgen.cpp \
	../src/platform.cpp \
	../src/physics.cpp \
	../src/sdlaudiosfxr.cpp \
	../src/sdlsystem.cpp \
	../src/simplex.cpp \
	../src/stdafx.cpp \
	../src/vmdata.cpp \
    $(wildcard ../include/Box2D/Collision/*.cpp) \
    $(wildcard ../include/Box2D/Collision/Shapes/*.cpp) \
    $(wildcard ../include/Box2D/Common/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Contacts/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Joints/*.cpp) \
    $(wildcard ../include/Box2D/Particle/*.cpp) \
    $(wildcard ../include/Box2D/Rope/*.cpp) \
    $(wildcard ../external/flatbuffers/src/*.cpp)
CSRCS= \
    ../external/freetype/src/autofit/autofit.c \
	../external/freetype/src/base/basepic.c \
	../external/freetype/src/base/ftapi.c \
	../external/freetype/src/base/ftbase.c \
	../external/freetype/src/base/ftbbox.c \
	../external/freetype/src/base/ftbitmap.c \
	../external/freetype/src/base/ftdbgmem.c \
	../external/freetype/src/base/ftdebug.c \
	../external/freetype/src/base/ftglyph.c \
	../external/freetype/src/base/ftinit.c \
	../external/freetype/src/base/ftpic.c \
	../external/freetype/src/base/ftstroke.c \
	../external/freetype/src/base/ftsynth.c \
	../external/freetype/src/base/ftsystem.c \
	../external/freetype/src/cff/cff.c \
	../external/freetype/src/pshinter/pshinter.c \
	../external/freetype/src/psnames/psnames.c \
	../external/freetype/src/raster/raster.c \
	../external/freetype/src/sfnt/sfnt.c \
	../external/freetype/src/smooth/smooth.c \
	../external/freetype/src/truetype/truetype.c

COBJS := $(patsubst %.c,%.o,$(CSRCS))
CPPOBJS := $(patsubst %.cpp,%.o,$(CPPSRCS))

$(CPPOBJS): CXXFLAGS += $(INCLUDES)

$(COBJS): CFLAGS += $(INCLUDES) -DFT2_BUILD_LIBRARY=1

# add -s ASSERTIONS=2 when troubleshooting.
#EMCC_WASM_BACKEND=1
lobster: $(COBJS) $(CPPOBJS)
	emcc $(CXXFLAGS) $(COBJS) $(CPPOBJS) -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 \
	  -s ALLOW_MEMORY_GROWTH=1 -o lobster.html --preload-file assets@/ \
	  -s DEMANGLE_SUPPORT=1 --emrun

install: all
	cp lobster ../../lobster/

clean:
	-$(RM) $(COBJS) $(CPPOBJS) lobster

all: lobster

default: all

