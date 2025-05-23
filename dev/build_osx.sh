mkdir -p xcode-cmake
cd xcode-cmake
# FIXME make joystick stuff compile?
usex86="${1:-OFF}"
cmake .. -DLOBSTER_OSX_X86=$usex86 -G Xcode -DSDL_HIDAPI_JOYSTICK=OFF -DSDL_JOYSTICK=OFF -DSDL_HAPTIC=OFF
cd ..
