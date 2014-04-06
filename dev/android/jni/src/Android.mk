LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../../../../external/sdl
LOBSTER_PATH := ../../..

LOCAL_C_INCLUDES := $(SDL_PATH)/include $(LOBSTER_PATH)/src $(LOCAL_PATH)/$(LOBSTER_PATH)/include

LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(LOBSTER_PATH)/src/lobster.cpp \
	$(LOBSTER_PATH)/src/audio.cpp \
	$(LOBSTER_PATH)/src/builtins.cpp \
	$(LOBSTER_PATH)/src/file.cpp \
	$(LOBSTER_PATH)/src/font.cpp \
	$(LOBSTER_PATH)/src/ftsystem.cpp \
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
	$(LOBSTER_PATH)/lib/stb_image.c

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_STATIC_LIBRARIES := freetype2-static

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
