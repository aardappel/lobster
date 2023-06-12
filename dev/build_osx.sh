mkdir -p xcode-cmake
cd xcode-cmake
# FIXME make joystick stuff compile?
cmake .. -G Xcode -DSDL_HIDAPI_JOYSTICK=OFF -DSDL_JOYSTICK=OFF -DSDL_HAPTIC=OFF
cd ..
