CXXFLAGS= -O3 -fomit-frame-pointer
override CXXFLAGS+= -m32 --std=c++0x -Wall -Wno-multichar -Wno-reorder -Wno-delete-non-virtual-dtor -DNDEBUG
CFLAGS= -O3 -fomit-frame-pointer
override CFLAGS+= -m32 -Wall -DNDEBUG

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
	lobster.o \
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

