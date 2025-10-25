# Emscripten makefile

CXX= emcc
CC= emcc

# LTO only has an effect for the cppout target, where it gives a huge speedup.
LTOFLAGS= --llvm-opts 3 --llvm-lto 1 -sWASM_OBJECT_FILES=0

# Pure debug mode.
# use a minimal amount of optimisation to reduce locals, but hopefully avoid
# inlining for good stack traces.
#OPTLEVEL= -g4 -O -sSAFE_HEAP=1
#OPTLEVELLINK= -sASSERTIONS=2
# Fast debug mode.
#OPTLEVEL= -g4 -O2
#OPTLEVELLINK= -sASSERTIONS=1
# Release/Shipping mode regular
OPTLEVEL= -O3 -DNDEBUG
OPTLEVELLINK= -sASSERTIONS=0
# Release/Shipping/benchmark mode with LTO
#OPTLEVEL= -O3 -DNDEBUG $(LTOFLAGS)
#OPTLEVELLINK= -sASSERTIONS=0

CFLAGS= $(OPTLEVEL)
CXXFLAGS= $(OPTLEVEL)
SHAREDFLAGS= -sUSE_SDL=2 -sUSE_SDL_MIXER=0 -sUSE_FREETYPE=1 \
         -Wall -pedantic -Wno-switch \
         -Wno-array-bounds -Wno-gnu-anonymous-struct -Wno-nested-anon-types \
		 -Wno-nontrivial-memcall -Wno-strict-prototypes -Wno-empty-translation-unit
override CFLAGS+= $(SHAREDFLAGS)
# We always build without exceptions, which is ok, because we don't rely on exception handling
# to exit the evaluation loop anymore, and we don't support the interpreter mode in Wasm.
# Also, asyncify relies on there not being any call_indirects in the path to the yield,
# which exception handling currently inserts.
override CXXFLAGS+= $(SHAREDFLAGS) -fno-exceptions -sDISABLE_EXCEPTION_CATCHING=1 -DDISABLE_EXCEPTION_HANDLING \
                    -std=c++20

INCLUDES=   -I. \
			-I../include \
			-I../src \
			-I../external/freetype/include \
			-I../external/imgui \
			-I../external/SDLMixer \
			-I../external/SDLMixer/codecs
LIBS=

HEADERS= $(wildcard ../src/lobster/*.h)

CPPSRCS= \
	../compiled_lobster/src/compiled_lobster.cpp \
	../src/audio.cpp \
	../src/builtins.cpp \
	../src/compiler.cpp \
	../src/cubegen.cpp \
	../src/engine.cpp \
	../src/file.cpp \
	../src/font.cpp \
	../src/fontrenderer.cpp \
	../src/glgeom.cpp \
	../src/glloadiqm.cpp \
	../src/glshader.cpp \
	../src/glsystem.cpp \
	../src/gltexture.cpp \
	../src/gltimequery.cpp \
	../src/glvr.cpp \
	../src/graphics.cpp \
	../src/imbind.cpp \
	../src/lobsterreader.cpp \
	../src/meshgen.cpp \
	../src/platform.cpp \
	../src/physics.cpp \
	../src/sdlaudiosfxr.cpp \
	../src/sdlsystem.cpp \
	../src/simplex.cpp \
	../src/steamworks.cpp \
	../src/stdafx.cpp \
	../src/tonative.cpp \
    ../src/vm.cpp \
	../src/vmdata.cpp \
    $(wildcard ../include/Box2D/Collision/*.cpp) \
    $(wildcard ../include/Box2D/Collision/Shapes/*.cpp) \
    $(wildcard ../include/Box2D/Common/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Contacts/*.cpp) \
    $(wildcard ../include/Box2D/Dynamics/Joints/*.cpp) \
    $(wildcard ../include/Box2D/Particle/*.cpp) \
    $(wildcard ../include/Box2D/Rope/*.cpp) \
    $(wildcard ../external/flatbuffers/src/*.cpp) \
    $(wildcard ../external/imgui/*.cpp) \

CSRCS= \
    $(wildcard ../external/SDLMixer/*.c) \
    $(wildcard ../external/SDLMixer/codecs/*.c)

CPPOBJS := $(patsubst %.cpp,%.o,$(CPPSRCS))
COBJS := $(patsubst %.c,%.o,$(CSRCS))

$(CPPOBJS): CXXFLAGS += $(INCLUDES)
$(COBJS): CFLAGS += $(INCLUDES)

#$(CPPOBJS): $(CPPSRCS) $(HEADERS)

EFLAGS= -sWASM=1 -sUSE_WEBGL2=1 -sFULL_ES3=1 \
		--source-map-base http://localhost:6931/ \
		-sALLOW_MEMORY_GROWTH=1 -o lobster.html --preload-file assets@/ \
		--emrun --profiling-funcs \
		-sBINARYEN_IGNORE_IMPLICIT_TRAPS=1 \
		-sASYNCIFY -sASYNCIFY_IGNORE_INDIRECT
		#-sBINARYEN_EXTRA_PASSES='"--always-inline-max-function-size=5,--flexible-inline-max-function-size=50,--one-caller-inline-max-function-size=200"'

#EMCC_DEBUG=1
#EMCC_WASM_BACKEND=1

# We only support compiling with the C++ backend, no JIT mode.

cpp: CXXFLAGS += -DVM_COMPILED_CODE_MODE -DIMGUI_IMPL_OPENGL_ES3
cpp: $(CPPOBJS) $(COBJS)
	emcc $(OPTLEVELLINK) $(CFLAGS) $(COBJS) $(CXXFLAGS) $(CPPOBJS) $(EFLAGS)

clean:
	-$(RM) $(CPPOBJS) $(COBJS)

cleanw:
	del $(addprefix ",$(addsuffix ",$(CPPOBJS)))
	del $(addprefix ",$(addsuffix ",$(COBJS)))

all: cpp

default: all

