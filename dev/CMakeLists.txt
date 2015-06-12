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

include_directories(include)

find_package(OpenGL REQUIRED)

if(APPLE)
  find_library(OPENGL_LIBRARIES NAMES GL GLU)
  SET(CMAKE_CXX_LINK_FLAGS "-framework OpenGL")
endif()

set (SDL_LIBRARIES SDL2)
if(WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
  set(OPENGL_LIBRARIES opengl32)
  set(SDL_LIBRARIES SDL2main ${SDL_LIBRARIES})
  link_directories("$ENV{DXSDK_DIR}/Lib/$ENV{PROCESSOR_ARCHITECTURE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lobster)

add_executable(lobster_linux ${LOBSTER_SRCS} ${EXTERNAL_SRCS})

target_link_libraries(lobster_linux
  ${SDL_LIBRARIES}
  freetype
  ${OPENGL_LIBRARIES})
