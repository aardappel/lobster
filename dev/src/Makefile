# note: the way the VM works, Lobster should really be built as a 32bit binary
# even on a 64bit system. It will work as 64bit binary, but will be inefficient
# (use twice as much memory for Lobster runtime data).
# Change the flag below to override this:
#ARCHFLAGS= -m32
ARCHFLAGS=

CXXFLAGS= -O3 -fomit-frame-pointer
override CXXFLAGS+= $(ARCHFLAGS) --std=c++0x -Wall -Wno-multichar -Wno-reorder \
    -Wno-delete-non-virtual-dtor -Wno-switch -DNDEBUG
CFLAGS= -O3 -fomit-frame-pointer
override CFLAGS+= $(ARCHFLAGS) -Wall -DNDEBUG

INCLUDES= `freetype-config --cflags` `sdl2-config --cflags` -I. -I../include
LIBS= -L/usr/lib32 `freetype-config --libs` `sdl2-config --libs` -lGL

OBJS= \
	audio.o \
	builtins.o \
	file.o \
	font.o \
	ftsystem.o \
	glgeom.o \
	glloadiqm.o \
	glshader.o \
	glsystem.o \
	gltexture.o \
	graphics.o \
	compiler.o \
        vm.o \
        main.o \
	lobsterreader.o \
	meshgen.o \
	platform.o \
	physics.o \
	sdlaudiosfxr.o \
	sdlsystem.o \
	simplex.o \
	stdafx.o \
	vmdata.o \
	../lib/stb_image.o

OBJS += $(patsubst %.cpp,%.o,$(shell find ../include/Box2D -name "*.cpp"))

$(OBJS): CXXFLAGS += $(INCLUDES)

lobster: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

install: all
	cp lobster ../../lobster/

clean:
	-$(RM) $(OBJS) lobster

all: lobster

default: all

