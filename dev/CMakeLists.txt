cmake_minimum_required(VERSION 2.8)

project(Lobster)

option(LOBSTER_ENGINE "Build a Lobster with the default engine included" ON)
option(LOBSTER_TOCPP "Build Lobster with the output of --to-cpp" OFF)
option(LOBSTER_WERROR "Treat warnings as errors" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/..)
endif()

# git commit date and commit hash into a definition
execute_process(
  COMMAND git log -1 --pretty="%cI|%h"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_INFO
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT MINGW)
    add_definitions("-DGIT_COMMIT_INFOSTR=${GIT_COMMIT_INFO}") # this currently breaks MingW compilation...
endif()

if(LOBSTER_ENGINE)
  file(GLOB LOBSTER_SRCS
    "src/lobster/*.h"
    "src/*.cpp"
  )
  add_definitions(-DLOBSTER_ENGINE=1)
else()
  # FIXME: move into separate dirs so these can be globbed.
  set(LOBSTER_SRCS
     src/lobster/bytecode_generated.h
     src/lobster/codegen.h
     src/lobster/compiler.h
     src/lobster/disasm.h
     src/lobster/geom.h
     src/lobster/idents.h
     src/lobster/il.h
     src/lobster/lex.h
     src/lobster/natreg.h
     src/lobster/node.h
     src/lobster/optimizer.h
     src/lobster/parser.h
     src/lobster/platform.h
     src/lobster/slaballoc.h
     src/lobster/stdafx.h
     src/lobster/tonative.h
     src/lobster/tools.h
     src/lobster/ttypes.h
     src/lobster/typecheck.h
     src/lobster/unicode.h
     src/lobster/vmdata.h
     src/lobster/wasm_binary_writer.h
     src/lobster/wasm_binary_writer_test.h
     src/lobster/wentropy.h
     src/lobster/wfc.h
     src/builtins.cpp
     src/compiler.cpp
     src/disasm.cpp
     src/file.cpp
     src/lobsterreader.cpp
     src/main.cpp
     src/platform.cpp
     src/stdafx.cpp
     src/tocpp.cpp
     src/tonative.cpp
     src/towasm.cpp
     src/vm.cpp
     src/vmdata.cpp
  )
  add_definitions(-DLOBSTER_ENGINE=0)
endif()

file(GLOB_RECURSE EXTERNAL_SRCS
    "external/flatbuffers/*.cpp"
)

if(LOBSTER_ENGINE)
  set(SDL_SHARED OFF CACHE INTERNAL "")
  set(SDL_STATIC ON CACHE INTERNAL "")
  set(VIDEO_VULKAN OFF CACHE INTERNAL "")
  set(VIDEO_WAYLAND OFF CACHE INTERNAL "")
  add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
  file(GLOB_RECURSE EXTERNAL_ENGINE_SRCS
    "include/Box2D/*.cpp"
    "lib/stb_image.c"
    "external/SDLMixer/*.c"
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
  add_definitions(-DFT2_BUILD_LIBRARY=1)
  list (APPEND EXTERNAL_SRCS ${EXTERNAL_ENGINE_SRCS})
endif()

set(COMMON_GCC_STYLE "-Wall -Wno-switch -Wno-unused-value -Wno-multichar") # -fno-exceptions
if(LOBSTER_WERROR)
  set(COMMON_GCC_STYLE "${COMMON_GCC_STYLE} -Werror")
endif()

if(APPLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 ${COMMON_GCC_STYLE}")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libstdc++ ${COMMON_GCC_STYLE}")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++17 ${COMMON_GCC_STYLE}")
endif()
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FS")
  #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  add_compile_options("/std:c++latest")
endif()
if(WIN32)
    # Valve SDKs only used in VS project for now.
    add_compile_definitions(SKIP_SDKS)
endif()

find_package(Threads)

include_directories(include
                    src
                    external/SDL/include
                    external/SDLMixer
                    external/freetype/include
                    external/imgui)

if(LOBSTER_ENGINE)
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
    if(MSVC)
        set(ADDITIONAL_LIBRARIES "winmm.lib imm32.lib version.lib")
        link_directories("$ENV{DXSDK_DIR}/Lib/$ENV{PROCESSOR_ARCHITECTURE}" "${CMAKE_SOURCE_DIR}/lib")
    endif()
    if(MINGW)
        SET(CMAKE_CXX_LINK_FLAGS "-mconsole -lmingw32")
    endif()
  endif()
endif()

if(UNIX)
   include(GNUInstallDirs)
   set(BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
   set(DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
   set(DOCDIR ${CMAKE_INSTALL_FULL_DOCDIR})
   set_source_files_properties(src/platform.cpp PROPERTIES COMPILE_FLAGS -DPLATFORM_DATADIR="\\\"${DATADIR}/\\\"")
   if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
     set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
   endif()
else()
   set(BINDIR ${CMAKE_INSTALL_PREFIX})
   set(DATADIR ${CMAKE_INSTALL_PREFIX})
   set(DOCDIR ${CMAKE_INSTALL_PREFIX}/docs)
   if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
     set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
   endif()
endif()

# Turn off warnings for files we don't control.
set_source_files_properties(${EXTERNAL_SRCS} PROPERTIES COMPILE_FLAGS -w)

set(EXE_NAME lobster)
if(LOBSTER_TOCPP)
  list(REMOVE_ITEM LOBSTER_SRCS "src/main.cpp")
  list(REMOVE_ITEM LOBSTER_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")  #FIXME
  list(APPEND LOBSTER_SRCS "compiled_lobster/src/compiled_lobster.cpp")
  add_definitions(-DVM_COMPILED_CODE_MODE=1)
  add_definitions(-flto)
  set(EXE_NAME compiled_lobster)
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
  endif()
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()
add_executable(${EXE_NAME} ${LOBSTER_SRCS} ${EXTERNAL_SRCS})
target_link_libraries(${EXE_NAME} ${CMAKE_THREAD_LIBS_INIT})
if(LOBSTER_ENGINE)
    target_link_libraries(${EXE_NAME}
      ${SDL_LIBRARIES}
      ${ADDITIONAL_LIBRARIES}
      ${OPENGL_LIBRARIES})
endif()

install(TARGETS ${EXE_NAME} DESTINATION ${BINDIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/../data" DESTINATION ${DATADIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/../modules" DESTINATION ${DATADIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/../samples" DESTINATION ${DATADIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/../tests" DESTINATION ${DATADIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/../docs/" DESTINATION ${DOCDIR})

enable_testing()
add_test(unittest ${EXE_NAME} ${CMAKE_SOURCE_DIR}/../tests/unittest.lobster)
add_test(speedtest ${EXE_NAME} ${CMAKE_SOURCE_DIR}/../tests/speedtest.lobster)
