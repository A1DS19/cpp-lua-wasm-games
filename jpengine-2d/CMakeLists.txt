cmake_minimum_required(VERSION 3.25)

# Dynamically name the project after the folder
get_filename_component(PROJECT_NAME ${CMAKE_SOURCE_DIR} NAME)
project(
  ${PROJECT_NAME}
  VERSION 1.0.0
  DESCRIPTION "Modern C++ Project"
  LANGUAGES CXX C)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable compile_commands.json for LSP support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Modern C++ standard (change to 20 if C++23 is not available)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wunused
    -Wuninitialized
    -Wshadow
    -Wformat=2)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Werror -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  else()
    add_compile_options(-O3 -DNDEBUG)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wunused
    -Wuninitialized
    -Wshadow
    -Wformat=2)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Werror -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
  else()
    add_compile_options(-O3 -DNDEBUG)
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4 /permissive-)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/Od /RTC1 /WX)
  else()
    add_compile_options(/O2 /DNDEBUG)
  endif()
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Gather all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp
     ${CMAKE_SOURCE_DIR}/src/*.cxx)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/*.hpp
     ${CMAKE_SOURCE_DIR}/include/*.h)

# Define the executable
add_executable(main ${SOURCES} ${HEADERS})

# Target-specific properties
set_target_properties(
  main
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
             CXX_STANDARD 23
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

# SDL2 + OpenGL ES 3
if(EMSCRIPTEN)
  # Emscripten bundles SDL2 as a port — WebGL2 = GLES3
  set_target_properties(main PROPERTIES SUFFIX ".html")
  target_compile_options(main PRIVATE -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_MIXER=2
                                      -fwasm-exceptions)
  target_link_options(
    main
    PRIVATE
    -sUSE_SDL=2
    -sUSE_SDL_IMAGE=2
    -sUSE_SDL_MIXER=2
    -sSDL2_MIXER_FORMATS=["ogg","mp3"]
    -sSDL2_IMAGE_FORMATS=["png","jpg"]
    -sALLOW_MEMORY_GROWTH=1
    -sUSE_WEBGL2=1
    -sFULL_ES3=1
    -fwasm-exceptions
    --preload-file
    ${CMAKE_SOURCE_DIR}/assets@/assets)
else()
  find_package(SDL2 REQUIRED)
  target_include_directories(main PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(main PRIVATE ${SDL2_LIBRARIES})

  find_package(SDL2_image REQUIRED)
  target_include_directories(main PRIVATE ${CMAKE_PREFIX_PATH}/include)
  target_link_libraries(main PRIVATE SDL2_image::SDL2_image)

  find_package(SDL2_mixer QUIET)
  if(SDL2_mixer_FOUND)
    target_link_libraries(main PRIVATE SDL2_mixer::SDL2_mixer)
  else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(SDL2_MIXER QUIET IMPORTED_TARGET SDL2_mixer)
    endif()

    if(TARGET PkgConfig::SDL2_MIXER)
      target_link_libraries(main PRIVATE PkgConfig::SDL2_MIXER)
    else()
      message(WARNING "SDL2_mixer not found on native build; SDL_mixer features will be unavailable.")
    endif()
  endif()

  # OpenGL / OpenGL ES 3
  if(APPLE)
    # macOS has no native GLESv2 — use the OpenGL framework
    find_package(OpenGL REQUIRED)
    target_link_libraries(main PRIVATE OpenGL::GL)
  else()
    # Linux/Windows: GLESv2 library exposes the GLES3 API
    find_library(GLES3_LIB GLESv2 REQUIRED)
    target_link_libraries(main PRIVATE ${GLES3_LIB})
  endif()
endif()

# Platform-specific configurations
if(WIN32)
  target_compile_definitions(main PRIVATE WIN32_LEAN_AND_MEAN)
elseif(APPLE)
  set_target_properties(main PROPERTIES MACOSX_RPATH TRUE)
elseif(UNIX)
  target_link_libraries(main PRIVATE dl)
endif()

# Vendor libraries — auto-detected from vendor/*/src/ and vendor/*/include/ To
# add a new library: drop it into vendor/<name>/src/ and vendor/<name>/include/
# All vendor sources are compiled into a single static library called "vendor".
# Header-only libraries (no src/) are included via target_include_directories on
# main. -w suppresses all warnings since we don't own this code.

# Always add vendor include dirs to main (handles header-only libs like sol2)
file(
  GLOB VENDOR_DIRS
  LIST_DIRECTORIES true
  ${CMAKE_SOURCE_DIR}/vendor/*)
foreach(VENDOR_DIR ${VENDOR_DIRS})
  if(IS_DIRECTORY ${VENDOR_DIR})
    target_include_directories(main SYSTEM PRIVATE ${VENDOR_DIR}/include)
  endif()
endforeach()

file(GLOB_RECURSE VENDOR_SOURCES ${CMAKE_SOURCE_DIR}/vendor/*/src/*.cpp)
if(VENDOR_SOURCES)
  add_library(vendor STATIC ${VENDOR_SOURCES})
  file(
    GLOB VENDOR_DIRS
    LIST_DIRECTORIES true
    ${CMAKE_SOURCE_DIR}/vendor/*)
  foreach(VENDOR_DIR ${VENDOR_DIRS})
    if(IS_DIRECTORY ${VENDOR_DIR})
      target_include_directories(vendor PUBLIC ${VENDOR_DIR}/include)
    endif()
  endforeach()
  set_target_properties(vendor PROPERTIES COMPILE_FLAGS "-w")
  target_link_libraries(main PRIVATE vendor)
endif()

# stb — flat header layout, implementation compiled via stb_impl.cpp
target_include_directories(main SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb)

# glm — header-only, lives in vendor/glm/ so add vendor/ as the include root
# usage: #include <glm/glm.hpp>
target_include_directories(main SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

# Lua — flat C source layout in vendor/lua/
file(GLOB LUA_SOURCES ${CMAKE_SOURCE_DIR}/vendor/lua/*.c)
# Exclude standalone interpreter, amalgamation, and test runner (each has
# main())
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*(lua|luac|onelua|ltests)\\.c$")
add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC ${CMAKE_SOURCE_DIR}/vendor/lua)
set_target_properties(lua PROPERTIES COMPILE_FLAGS "-w")
if(EMSCRIPTEN)
  target_compile_options(lua PRIVATE -fwasm-exceptions)
endif()
target_link_libraries(main PRIVATE lua)

# Copy compile_commands.json to project root for LSP
if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
  add_custom_command(
    TARGET main
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to project root")
endif()

# Optional: Enable testing
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
         ${CMAKE_SOURCE_DIR}/tests/*.cpp)
    add_executable(test_main ${TEST_SOURCES})
    target_include_directories(test_main PRIVATE ${CMAKE_SOURCE_DIR}/include)
    set_target_properties(test_main PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                               ${CMAKE_SOURCE_DIR}/bin)
    add_test(NAME unit_tests COMMAND test_main)
  endif()
endif()

# Development target for setup
add_custom_target(
  dev-setup
  COMMAND ${CMAKE_COMMAND} -E echo "Setting up development environment..."
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMENT "Development setup complete")

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project Name:     ${PROJECT_NAME}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(
  STATUS
    "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
)
message(STATUS "  Source Directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "  Binary Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "")
