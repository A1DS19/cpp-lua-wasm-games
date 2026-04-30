cmake_minimum_required(VERSION 3.25)

# Dynamically name the project after the folder
get_filename_component(PROJECT_NAME ${CMAKE_SOURCE_DIR} NAME)
project(${PROJECT_NAME}
    VERSION 1.0.0
    DESCRIPTION "Modern C++ Project"
    LANGUAGES C CXX
)

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
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Wunused -Wuninitialized
        -Wshadow -Wformat=2
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0 -Werror -fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wconversion -Wsign-conversion
        -Wunused -Wuninitialized
        -Wshadow -Wformat=2
    )

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
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.cxx
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/include/*.hpp
    ${CMAKE_SOURCE_DIR}/include/*.h
)

# Define the executable
add_executable(main ${SOURCES} ${HEADERS})

# Target-specific properties
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Platform-specific configurations
if(WIN32)
    target_compile_definitions(main PRIVATE WIN32_LEAN_AND_MEAN)
elseif(APPLE)
    set_target_properties(main PROPERTIES
        MACOSX_RPATH TRUE
    )
elseif(UNIX)
    target_link_libraries(main PRIVATE dl)
endif()

# Vendor libraries — auto-detected from vendor/*/src/ and vendor/*/include/
# Header-only or simple C++-source vendor libs: drop into vendor/<name>/src/ and vendor/<name>/include/
# Libraries with their own build setup (glfw, glew) are wired up explicitly below.
# -w suppresses all warnings since we don't own this code.
file(GLOB_RECURSE VENDOR_SOURCES ${CMAKE_SOURCE_DIR}/vendor/*/src/*.cpp)
# Exclude libs we build explicitly
list(FILTER VENDOR_SOURCES EXCLUDE REGEX "/vendor/(glfw-[0-9.]+|glew|glm|stb|json|cgltf|bullet3-[0-9.]+)/")
if(VENDOR_SOURCES)
    add_library(vendor STATIC ${VENDOR_SOURCES})
    file(GLOB VENDOR_DIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/vendor/*)
    foreach(VENDOR_DIR ${VENDOR_DIRS})
      if(IS_DIRECTORY ${VENDOR_DIR}
         AND NOT ${VENDOR_DIR} MATCHES "/vendor/(glfw-[0-9.]+|glew|glm|stb|json|cgltf|bullet3-[0-9.]+)$")
        target_include_directories(vendor PUBLIC ${VENDOR_DIR}/include)
      endif()
    endforeach()
    set_target_properties(vendor PROPERTIES COMPILE_FLAGS "-w")
    target_link_libraries(main PRIVATE vendor)
endif()

# ---------------------------------------------------------------------------
# GLFW — vendored as a CMake subproject (vendor/glfw-3.4)
# ---------------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glfw-3.4 EXCLUDE_FROM_ALL)
set_target_properties(glfw PROPERTIES COMPILE_FLAGS "-w")
target_link_libraries(main PRIVATE glfw)

# ---------------------------------------------------------------------------
# GLEW — manual static library (single source: vendor/glew/src/glew.c)
# ---------------------------------------------------------------------------
add_library(glew STATIC ${CMAKE_SOURCE_DIR}/vendor/glew/src/glew.c)
target_include_directories(glew PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glew/include)
target_compile_definitions(glew PUBLIC GLEW_STATIC)
set_target_properties(glew PROPERTIES COMPILE_FLAGS "-w")
target_link_libraries(main PRIVATE glew)

# ---------------------------------------------------------------------------
# GLM — header-only math library (vendor/glm)
# Headers live directly under vendor/glm/, so include path is vendor/
# (users #include <glm/glm.hpp>).
# ---------------------------------------------------------------------------
add_library(glm INTERFACE)
target_include_directories(glm SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/vendor)
target_link_libraries(main PRIVATE glm)

# ---------------------------------------------------------------------------
# stb — header-only utilities (vendor/stb)
# Single-header libraries: #include <stb_image.h>. Exactly one .cpp in the
# project must #define STB_IMAGE_IMPLEMENTATION (and similar for other stb_*).
# ---------------------------------------------------------------------------
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/vendor/stb)
target_link_libraries(main PRIVATE stb)

# ---------------------------------------------------------------------------
# nlohmann/json — single-header library (vendor/json/nlohmann/json.hpp)
# Users include as: #include <nlohmann/json.hpp>
# ---------------------------------------------------------------------------
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/vendor/json)
target_link_libraries(main PRIVATE nlohmann_json)

# ---------------------------------------------------------------------------
# cgltf — single-header glTF 2.0 loader (vendor/cgltf/cgltf.h)
# Users include as: #include <cgltf.h>
# Exactly one .cpp in the project must #define CGLTF_IMPLEMENTATION before
# the include — see src/utils/cgltf-impl.cpp.
# ---------------------------------------------------------------------------
add_library(cgltf INTERFACE)
target_include_directories(cgltf SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/vendor/cgltf)
target_link_libraries(main PRIVATE cgltf)

# ---------------------------------------------------------------------------
# Bullet3 — physics library (vendor/bullet3-3.25)
# Has its own CMakeLists.txt; we build the three core libs (BulletDynamics,
# BulletCollision, LinearMath) and skip everything else (demos, extras,
# Python bindings, OpenGL examples).
# Headers reference each other as `Bullet3Common/...` etc., so the include
# path needs to be vendor/bullet3-3.25/src/.
# ---------------------------------------------------------------------------
set(BUILD_BULLET2_DEMOS    OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS        OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS    OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS           OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS       OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET3          ON  CACHE BOOL "" FORCE)
set(BUILD_PYBULLET         OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS           OFF CACHE BOOL "" FORCE)
set(INSTALL_EXTRA_LIBS     OFF CACHE BOOL "" FORCE)
set(USE_DOUBLE_PRECISION   OFF CACHE BOOL "" FORCE)
# Bullet's top-level CMakeLists declares a too-old cmake_minimum_required for
# modern CMake. This sets the policy floor so the subproject configures.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/bullet3-3.25 EXCLUDE_FROM_ALL)
target_include_directories(main SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor/bullet3-3.25/src)
target_link_libraries(main PRIVATE BulletDynamics BulletCollision LinearMath)
# Silence Bullet's own warnings — we don't own this code.
foreach(BULLET_TARGET BulletDynamics BulletCollision LinearMath)
    if(TARGET ${BULLET_TARGET})
        target_compile_options(${BULLET_TARGET} PRIVATE -w)
    endif()
endforeach()

# ---------------------------------------------------------------------------
# OpenGL — system framework (macOS) / library (Linux/Windows)
# ---------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(main PRIVATE OpenGL::GL)

# ---------------------------------------------------------------------------
# Assets — bake the source path as a compile-time macro so runtime code can
# load files no matter what working directory the binary was launched from.
# ---------------------------------------------------------------------------
set(JPENGINE_ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
target_compile_definitions(main PRIVATE JPENGINE_ASSETS_DIR="${JPENGINE_ASSETS_DIR}")

# Copy compile_commands.json to project root for LSP
if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
    add_custom_command(
        TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
        COMMENT "Copying compile_commands.json to project root"
    )
endif()

# Optional: Enable testing
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
            ${CMAKE_SOURCE_DIR}/tests/*.cpp
        )
        add_executable(test_main ${TEST_SOURCES})
        target_include_directories(test_main PRIVATE ${CMAKE_SOURCE_DIR}/include)
        set_target_properties(test_main PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
        )
        add_test(NAME unit_tests COMMAND test_main)
    endif()
endif()

# Development target for setup
add_custom_target(dev-setup
    COMMAND ${CMAKE_COMMAND} -E echo "Setting up development environment..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Development setup complete"
)

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project Name:     ${PROJECT_NAME}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Source Directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "  Binary Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "")
