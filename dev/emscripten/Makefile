# Emscripten makefile

CXX= emcc
CC= emcc

# FIXME: at -O2 and above get: AssertionError: Failed to run llvm optimizations:
# https://github.com/kripken/emscripten/issues/3644

OPTLEVEL= -g4
CXXFLAGS= $(OPTLEVEL)
# apparently exception catching is slow, but we need them for the moment.
override CXXFLAGS+= -s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=0 -std=c++11 -Wall -pedantic -Wno-switch \
                    -Wno-array-bounds -Wno-gnu-anonymous-struct -Wno-nested-anon-types
CFLAGS= $(OPTLEVEL)
override CFLAGS+= -Wall

INCLUDES= -I. -I../include -I../external/freetype/include
LIBS=

CPPSRCS= \
	../src/audio.cpp \
	../src/builtins.cpp \
	../src/file.cpp \
	../src/font.cpp \
	../src/fontrenderer.cpp \
	../src/glgeom.cpp \
	../src/glloadiqm.cpp \
	../src/glshader.cpp \
	../src/glsystem.cpp \
	../src/gltexture.cpp \
	../src/graphics.cpp \
	../src/compiler.cpp \
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
    $(wildcard ../include/Box2D/Rope/*.cpp)
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

lobster: $(COBJS) $(CPPOBJS)
	emcc $(CXXFLAGS) $(COBJS) $(CPPOBJS) -o lobster.html --preload-file assets@/ -s DEMANGLE_SUPPORT=1 -s ASSERTIONS=2 --emrun

install: all
	cp lobster ../../lobster/

clean:
	-$(RM) $(COBJS) $(CPPOBJS) lobster

all: lobster

default: all

