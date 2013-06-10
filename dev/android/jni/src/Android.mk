LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/../../../include

LOBSTERSRC := ../../../src
SLMATH := ../../../lib/slmath

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
	$(LOBSTERSRC)/lobster.cpp \
	$(LOBSTERSRC)/builtins.cpp \
	$(LOBSTERSRC)/file.cpp \
	$(LOBSTERSRC)/graphics.cpp \
	$(LOBSTERSRC)/lobsterreader.cpp \
	$(LOBSTERSRC)/meshgen.cpp \
	$(LOBSTERSRC)/openglsup.cpp \
	$(LOBSTERSRC)/sfxr.cpp \
	$(LOBSTERSRC)/simplex.cpp \
	$(LOBSTERSRC)/vmdata.cpp \
	$(SLMATH)/vec2.cpp \
	$(SLMATH)/vec3.cpp \
	$(SLMATH)/vec4.cpp \
	$(SLMATH)/mat4.cpp \
	$(SLMATH)/runtime_checks.cpp \
	$(SLMATH)/random.cpp \
	$(SLMATH)/random_util.cpp \
	$(SLMATH)/quat.cpp \
	$(SLMATH)/intersect_util.cpp \
	$(SLMATH)/float_util.cpp \
	../../../lib/stb_image.c


LOCAL_SHARED_LIBRARIES := SDL2 gnustl_shared freetype-prebuilt

LOCAL_CFLAGS += -Wno-multichar -fpermissive -std=gnu++0x -Wno-write-strings -fexceptions

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog $(LOCAL_PATH)/../freetype/libfreetype2-static.so

include $(BUILD_SHARED_LIBRARY)
