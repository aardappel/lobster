cmake_minimum_required(VERSION 2.8)

project(Lobster)

file(GLOB LOBSTER_SRCS
    "src/*.h"
    "src/*.cpp"
)

file(GLOB_RECURSE EXTERNAL_SRCS
    "include/Box2D/*.cpp"
    "lib/stb_image.c"
)

# source_group(Lobster FILES ${LOBSTER_SRCS})
# source_group(External FILES ${EXTERNAL_SRCS})

if(APPLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -pedantic -Wno-switch")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -Wno-switch")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++ -Wall -pedantic -Wno-switch")
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

include_directories(include)

find_package(OpenGL REQUIRED)
set(SDL_LIBRARIES SDL2)
set(FREETYPE_LIBRARIES freetype)
set(ADDITIONAL_LIBRARIES "")

if(APPLE)
  find_library(OPENGL_LIBRARIES NAMES GL GLU)
  SET(CMAKE_CXX_LINK_FLAGS "-framework OpenGL")
endif()

if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  set(OPENGL_LIBRARIES opengl32)
  set(SDL_LIBRARIES SDL2main ${SDL_LIBRARIES})
  set(FREETYPE_LIBRARIES freetype246)
  set(ADDITIONAL_LIBRARIES "winmm.lib imm32.lib version.lib")
  link_directories("$ENV{DXSDK_DIR}/Lib/$ENV{PROCESSOR_ARCHITECTURE}" "${CMAKE_SOURCE_DIR}/lib")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lobster)

add_executable(lobster_cmake ${LOBSTER_SRCS} ${EXTERNAL_SRCS})

target_link_libraries(lobster_cmake
  ${SDL_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${ADDITIONAL_LIBRARIES}
  ${OPENGL_LIBRARIES})
