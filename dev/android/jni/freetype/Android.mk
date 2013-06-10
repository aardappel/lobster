LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype-prebuilt
LOCAL_SRC_FILES := libfreetype2-static.so
include $(PREBUILT_SHARED_LIBRARY)
