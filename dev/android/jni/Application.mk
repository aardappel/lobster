
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
# APP_STL := stlport_static
APP_STL := gnustl_static

APP_PLATFORM := android-18
APP_ABI := armeabi-v7a
#APP_ABI := armeabi armeabi-v7a x86
APP_CFLAGS += -Wno-multichar -Wno-array-bounds -Wno-switch
#NDK_TOOLCHAIN_VERSION := 4.9
NDK_TOOLCHAIN_VERSION := clang
APP_CPPFLAGS += -std=c++14 -fexceptions -frtti
APP_STL := c++_static