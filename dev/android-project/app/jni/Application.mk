
APP_SHORT_COMMANDS := true

APP_STL := c++_shared

APP_ABI := arm64-v8a # armeabi-v7a x86 x86_64

# Min runtime API level (ES3.0 == 18)
APP_PLATFORM=android-18

APP_CPPFLAGS += -frtti -std=c++17 -Wno-c++17-extensions -fexceptions -Wno-switch
