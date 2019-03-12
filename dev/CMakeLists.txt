cmake_minimum_required(VERSION 2.8)

project(Lobster)

set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library")
function(install)
endfunction(install)
add_subdirectory("external/SDL")

file(GLOB LOBSTER_SRCS
    "src/*.h"
    "src/lobster/*.h"
    "src/*.cpp"
)

file(GLOB_RECURSE EXTERNAL_SRCS
    "include/Box2D/*.cpp"
    "lib/stb_image.c"
    "external/SDLMixer/*.c"
    "external/flatbuffers/*.cpp"
    "external/imgui/*.cpp"
    external/freetype/src/autofit/autofit.c
    external/freetype/src/base/basepic.c
    external/freetype/src/base/ftapi.c
    external/freetype/src/base/ftbase.c
    external/freetype/src/base/ftbbox.c
    external/freetype/src/base/ftbitmap.c
    external/freetype/src/base/ftdbgmem.c
    external/freetype/src/base/ftdebug.c
    external/freetype/src/base/ftglyph.c
    external/freetype/src/base/ftinit.c
    external/freetype/src/base/ftpic.c
    external/freetype/src/base/ftstroke.c
    external/freetype/src/base/ftsynth.c
    external/freetype/src/base/ftsystem.c
    external/freetype/src/cff/cff.c
    external/freetype/src/pshinter/pshinter.c
    external/freetype/src/psnames/psnames.c
    external/freetype/src/raster/raster.c
    external/freetype/src/sfnt/sfnt.c
    external/freetype/src/smooth/smooth.c
    external/freetype/src/truetype/truetype.c
)

# source_group(Lobster FILES ${LOBSTER_SRCS})
# source_group(External FILES ${EXTERNAL_SRCS})

add_definitions(-DFT2_BUILD_LIBRARY=1)

set(COMMON_GCC_STYLE "-Wall -Wno-switch -Wno-unused-value") # -fno-exceptions

if(APPLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 ${COMMON_GCC_STYLE}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 ${COMMON_GCC_STYLE}")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 ${COMMON_GCC_STYLE}")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

include_directories(include
                    src
                    external/SDL/include
                    external/SDLMixer
                    external/freetype/include
                    external/imgui)

find_package(OpenGL REQUIRED)
set(SDL_LIBRARIES SDL2-static)
set(ADDITIONAL_LIBRARIES "")

if(APPLE)
  find_library(OPENGL_LIBRARIES NAMES GL GLU)
  SET(CMAKE_CXX_LINK_FLAGS "-framework OpenGL")
endif()

if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  set(OPENGL_LIBRARIES opengl32)
  set(SDL_LIBRARIES SDL2main ${SDL_LIBRARIES})
  set(ADDITIONAL_LIBRARIES "winmm.lib imm32.lib version.lib")
  link_directories("$ENV{DXSDK_DIR}/Lib/$ENV{PROCESSOR_ARCHITECTURE}" "${CMAKE_SOURCE_DIR}/lib")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lobster)

add_executable(lobster ${LOBSTER_SRCS} ${EXTERNAL_SRCS})

target_link_libraries(lobster
  ${SDL_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${OPENGL_LIBRARIES})
