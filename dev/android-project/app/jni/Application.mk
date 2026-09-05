
APP_SHORT_COMMANDS := true

APP_STL := c++_shared

APP_ABI := arm64-v8a # armeabi-v7a x86 x86_64

# Min runtime API level (ES3.0 == 18)
APP_PLATFORM=android-18

# -Wno-array-bounds because the generated C++ indexes VM::fvars, which is declared as a
# 1-element array that the VM allocates the real size for.
APP_CPPFLAGS += -fsigned-char -frtti -std=c++2a -fexceptions -Wno-switch -Wno-array-bounds -fwrapv
APP_CFLAGS += -fsigned-char
