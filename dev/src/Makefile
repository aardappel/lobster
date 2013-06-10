CXXFLAGS= -O3 -fomit-frame-pointer
override CXXFLAGS+= -m32 --std=c++11 -Wall -Wno-multichar -Wno-delete-non-virtual-dtor -DNDEBUG

INCLUDES= `freetype-config --cflags` `sdl2-config --cflags` -I.
LIBS= `freetype-config --libs` `sdl2-config --libs` -lGL

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
	sdlaudiosfxr.o \
	sdlsystem.o \
	simplex.o \
	stdafx.o \
	vmdata.o \
	../lib/stb_image.o

$(OBJS): CXXFLAGS += $(INCLUDES)

lobster: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(LIBS) $(OBJS)

install: all
	cp lobster ../../lobster/

clean:
	-$(RM) $(OBJS) lobster

all: lobster

default: all

