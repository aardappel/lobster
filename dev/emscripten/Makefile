# Emscripten makefile

CXX= emcc
CC= emcc

# LTO only has an effect for the cppout target, where it gives a huge speedup.
LTOFLAGS= --llvm-opts 3 --llvm-lto 1 -s WASM_OBJECT_FILES=0

# Pure debug mode.
#OPTLEVEL= -g4 -s ASSERTIONS=2 -s DISABLE_EXCEPTION_CATCHING=0
# Fast debug mode.
#OPTLEVEL= -g4 -O2 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0
# Release/Shipping mode
OPTLEVEL= -O3 -DNDEBUG -s ASSERTIONS=0 $(LTOFLAGS) -s DISABLE_EXCEPTION_CATCHING=0
# Benchmark mode: does not support graphics/exceptions.
#OPTLEVEL= -O3 -DNDEBUG -s ASSERTIONS=0 $(LTOFLAGS) -fno-exceptions -s DISABLE_EXCEPTION_CATCHING=1 -DDISABLE_EXCEPTION_HANDLING

CXXFLAGS= $(OPTLEVEL)
# apparently exception catching is slow, but we need them for the moment.
override CXXFLAGS+= -s USE_SDL=2 -s NO_EXIT_RUNTIME=1 \
                    -std=c++17 -Wall -pedantic -Wno-switch \
                    -Wno-array-bounds -Wno-gnu-anonymous-struct -Wno-nested-anon-types
CFLAGS= $(OPTLEVEL)
override CFLAGS+= -Wall

INCLUDES=   -I. \
			-I../include \
			-I../src \
			-I../external/freetype/include \
			-I../external/SDLMixer \
			-I../external/imgui
LIBS=

HEADERS= $(wildcard ../src/lobster/*.h)

CPPSRCS= \
	../src/audio.cpp \
	../src/builtins.cpp \
	../src/compiler.cpp \
	../src/cubegen.cpp \
	../src/disasm.cpp \
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
	../src/graphics.cpp \
	../src/imbind.cpp \
	../src/lobsterreader.cpp \
	../src/meshgen.cpp \
	../src/octree.cpp \
	../src/platform.cpp \
	../src/physics.cpp \
	../src/sdlaudiosfxr.cpp \
	../src/sdlsystem.cpp \
	../src/simplex.cpp \
	../src/steamworks.cpp \
	../src/stdafx.cpp \
	../src/tocpp.cpp \
	../src/tonative.cpp \
	../src/towasm.cpp \
    ../src/vm.cpp \
	../src/vmdata.cpp \
	../src/vmlog.cpp \
    $(wildcard ../include/Box2D/Collision/*.cpp) \
    $(wildcard ../include/Box2D/Collision/Shapes/*.cpp) \
    $(wildcard ../include/Box2D/Common/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Contacts/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Joints/*.cpp) \
    $(wildcard ../include/Box2D/Particle/*.cpp) \
    $(wildcard ../include/Box2D/Rope/*.cpp) \
    $(wildcard ../external/flatbuffers/src/*.cpp) \
    $(wildcard ../external/imgui/*.cpp)
MAINSRC= ../src/main.cpp
MAINCOMPSRC= ../compiled_lobster/src/compiled_lobster.cpp
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
MAINOBJ := $(patsubst %.cpp,%.o,$(MAINSRC))
MAINCOMPOBJ := $(patsubst %.cpp,%.o,$(MAINCOMPSRC))

$(CPPOBJS): CXXFLAGS += $(INCLUDES)
$(MAINOBJ): CXXFLAGS += $(INCLUDES)
$(MAINCOMPOBJ): CXXFLAGS += $(INCLUDES)

#$(CPPOBJS): $(CPPSRCS) $(HEADERS)
#$(MAINOBJ): $(MAINSRC) $(HEADERS)
#$(MAINCOMPOBJ): $(MAINCOMPSRC) $(HEADERS)

$(COBJS): CFLAGS += $(INCLUDES) -DFT2_BUILD_LIBRARY=1

EFLAGS= -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 \
		--source-map-base http://localhost:6931/ \
		-s ALLOW_MEMORY_GROWTH=1 -o lobster.html --preload-file assets@/ \
		-s DEMANGLE_SUPPORT=1 --emrun --profiling-funcs \
		-s BINARYEN_IGNORE_IMPLICIT_TRAPS=1
		#-s BINARYEN_EXTRA_PASSES='"--always-inline-max-function-size=5,--flexible-inline-max-function-size=50,--one-caller-inline-max-function-size=200"'

#EMCC_DEBUG=1
#EMCC_WASM_BACKEND=1
lobster: $(COBJS) $(CPPOBJS) $(MAINOBJ)
	emcc $(CXXFLAGS) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(EFLAGS)

lobster_cppout: CXXFLAGS += -DVM_COMPILED_CODE_MODE
lobster_cppout: $(COBJS) $(CPPOBJS) $(MAINCOMPOBJ)
	emcc $(CXXFLAGS) $(COBJS) $(CPPOBJS) $(MAINCOMPOBJ) $(EFLAGS)

WASMOUT= compiled_lobster_wasm.o
lobster_wasmout: CXXFLAGS += -DVM_COMPILED_CODE_MODE
lobster_wasmout: $(COBJS) $(CPPOBJS) $(WASMOUT)
	emcc -v $(CXXFLAGS) $(WASMOUT) $(COBJS) $(CPPOBJS) $(EFLAGS)

clean:
	-$(RM) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(MAINCOMPOBJ) lobster lobster_cppout lobster_wasmout

all: lobster

default: all

