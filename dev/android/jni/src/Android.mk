LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOBSTER_PATH := ../../..

SDL_PATH := ../../../../external/sdl
FREETYPE_PATH := $(LOBSTER_PATH)/external/freetype

LOCAL_C_INCLUDES := \
    $(SDL_PATH)/include \
    $(LOCAL_PATH)/$(FREETYPE_PATH)/include \
    $(LOBSTER_PATH)/src \
    $(LOCAL_PATH)/$(LOBSTER_PATH)/include

LOCAL_SRC_FILES := \
	$(LOBSTER_PATH)/src/main.cpp \
    $(LOBSTER_PATH)/src/compiler.cpp \
	$(LOBSTER_PATH)/src/audio.cpp \
	$(LOBSTER_PATH)/src/builtins.cpp \
	$(LOBSTER_PATH)/src/file.cpp \
	$(LOBSTER_PATH)/src/font.cpp \
	$(LOBSTER_PATH)/src/fontrenderer.cpp \
	$(LOBSTER_PATH)/src/glgeom.cpp \
	$(LOBSTER_PATH)/src/glloadiqm.cpp \
	$(LOBSTER_PATH)/src/glshader.cpp \
	$(LOBSTER_PATH)/src/glsystem.cpp \
	$(LOBSTER_PATH)/src/gltexture.cpp \
	$(LOBSTER_PATH)/src/graphics.cpp \
	$(LOBSTER_PATH)/src/lobsterreader.cpp \
	$(LOBSTER_PATH)/src/meshgen.cpp \
	$(LOBSTER_PATH)/src/platform.cpp \
	$(LOBSTER_PATH)/src/sdlaudiosfxr.cpp \
	$(LOBSTER_PATH)/src/sdlsystem.cpp \
	$(LOBSTER_PATH)/src/simplex.cpp \
	$(LOBSTER_PATH)/src/stdafx.cpp \
	$(LOBSTER_PATH)/src/vmdata.cpp \
    $(FREETYPE_PATH)/src/autofit/autofit.c \
	$(FREETYPE_PATH)/src/base/basepic.c \
	$(FREETYPE_PATH)/src/base/ftapi.c \
	$(FREETYPE_PATH)/src/base/ftbase.c \
	$(FREETYPE_PATH)/src/base/ftbbox.c \
	$(FREETYPE_PATH)/src/base/ftbitmap.c \
	$(FREETYPE_PATH)/src/base/ftdbgmem.c \
	$(FREETYPE_PATH)/src/base/ftdebug.c \
	$(FREETYPE_PATH)/src/base/ftglyph.c \
	$(FREETYPE_PATH)/src/base/ftinit.c \
	$(FREETYPE_PATH)/src/base/ftpic.c \
	$(FREETYPE_PATH)/src/base/ftstroke.c \
	$(FREETYPE_PATH)/src/base/ftsynth.c \
	$(FREETYPE_PATH)/src/base/ftsystem.c \
	$(FREETYPE_PATH)/src/cff/cff.c \
	$(FREETYPE_PATH)/src/pshinter/pshinter.c \
	$(FREETYPE_PATH)/src/psnames/psnames.c \
	$(FREETYPE_PATH)/src/raster/raster.c \
	$(FREETYPE_PATH)/src/sfnt/sfnt.c \
	$(FREETYPE_PATH)/src/smooth/smooth.c \
	$(FREETYPE_PATH)/src/truetype/truetype.c \
    $(SDL_PATH)/src/main/android/SDL_android_main.c

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_STATIC_LIBRARIES :=

LOCAL_CPPFLAGS := -std=c++11
LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY=1

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
