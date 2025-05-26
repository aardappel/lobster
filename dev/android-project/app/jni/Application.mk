
APP_SHORT_COMMANDS := true

APP_STL := c++_shared

APP_ABI := arm64-v8a # armeabi-v7a x86 x86_64

# Min runtime API level (ES3.0 == 18)
APP_PLATFORM=android-18

APP_CPPFLAGS += -fsigned-char -frtti -std=c++2a -fexceptions -Wno-switch
APP_CFLAGS += -fsigned-char
