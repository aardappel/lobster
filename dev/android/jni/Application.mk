
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
# APP_STL := stlport_static
APP_STL := gnustl_static

APP_ABI := armeabi-v7a
#APP_ABI := armeabi armeabi-v7a x86
APP_CFLAGS += -Wno-multichar
NDK_TOOLCHAIN_VERSION := 4.8
#NDK_TOOLCHAIN_VERSION := clang
APP_CPPFLAGS += -std=c++11 -fexceptions
