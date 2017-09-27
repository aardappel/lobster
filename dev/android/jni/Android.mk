LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

LOBSTER_PATH := ../..

SDL_PATH := $(LOBSTER_PATH)/external/SDL
SDL_MIXER_PATH := $(LOBSTER_PATH)/external/SDLMixer
FREETYPE_PATH := $(LOBSTER_PATH)/external/freetype
BOX2D_PATH := $(LOBSTER_PATH)/include/Box2D

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/$(SDL_PATH)/include \
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
	$(LOBSTER_PATH)/src/glvr.cpp \
	$(LOBSTER_PATH)/src/graphics.cpp \
	$(LOBSTER_PATH)/src/physics.cpp \
	$(LOBSTER_PATH)/src/lobsterreader.cpp \
	$(LOBSTER_PATH)/src/meshgen.cpp \
	$(LOBSTER_PATH)/src/cubegen.cpp \
	$(LOBSTER_PATH)/src/platform.cpp \
	$(LOBSTER_PATH)/src/sdlaudiosfxr.cpp \
	$(LOBSTER_PATH)/src/sdlsystem.cpp \
	$(LOBSTER_PATH)/src/simplex.cpp \
	$(LOBSTER_PATH)/src/stdafx.cpp \
	$(LOBSTER_PATH)/src/vmdata.cpp \
	$(LOBSTER_PATH)/src/vmlog.cpp \
	$(LOBSTER_PATH)/src/vm.cpp \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Collision/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Collision/Shapes/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Common/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Dynamics/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Dynamics/Joints/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Dynamics/Contacts/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Particle/*.cpp) \
	$(wildcard $(LOCAL_PATH)/$(BOX2D_PATH)/Rope/*.cpp) \
	) \
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
    $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(SDL_PATH)/src/atomic/SDL_atomic.c \
	$(SDL_PATH)/src/atomic/SDL_spinlock.c.arm \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/$(SDL_MIXER_PATH)/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/audio/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/audio/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/audio/dummy/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/core/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/cpuinfo/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/dynapi/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/events/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/file/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/haptic/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/haptic/dummy/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/joystick/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/joystick/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/loadso/dlopen/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/power/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/power/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/filesystem/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/render/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/render/*/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/stdlib/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/thread/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/thread/pthread/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/timer/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/timer/unix/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/video/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/video/android/*.c) \
	$(wildcard $(LOCAL_PATH)/$(SDL_PATH)/src/test/*.c) \
	)

LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES :=

LOCAL_CPPFLAGS := -std=c++11
LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY=1 -DGL_GLEXT_PROTOTYPES

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -ldl -landroid
include $(BUILD_SHARED_LIBRARY)
