cmake_minimum_required(VERSION 3.16)

project(nle
    VERSION 0.1.0
    DESCRIPTION "nice little engine - experimental graphics engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Everything built lands under the build directory this was configured with.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Where the engine lives. A project that pulls this file in through
# add_subdirectory() gets a CMAKE_SOURCE_DIR pointing at itself rather than at
# the engine, so paths into the engine tree use this instead.
set(NLE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "nle source root")

option(NLE_BUILD_TESTS "Build the test executables" ON)
option(NLE_ENABLE_DEBUG_OUTPUT "Define DEBUG for verbose engine logging" ON)

# ---------------------------------------------------------------------------
# dependencies
# ---------------------------------------------------------------------------

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 QUIET)

find_package(PkgConfig REQUIRED)

if(NOT glfw3_FOUND)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
endif()

# imgui and stb come from the system. The headers come from there too, which
# they did not use to: vendor/imgui is pinned at 1.86 while the system library
# here is 1.90.1, and ImGuiIO does not have the same layout in both. Compiling
# against one and linking the other is undefined behaviour that happens to
# start by silently reading the wrong fields. vendor/imgui is left in place as
# a submodule but is no longer on the include path.
pkg_check_modules(IMGUI IMPORTED_TARGET imgui)

# Sound is optional. A machine without the mixer still builds and still
# runs; nle::audio simply reports that it is not ready and everything it is
# asked to play is silence. Refusing to build a game because the speakers are
# missing would be the wrong trade.
pkg_check_modules(SDL2MIXER IMPORTED_TARGET SDL2_mixer sdl2)

find_library(STB_LIBRARY NAMES stb)
if(NOT STB_LIBRARY)
    message(FATAL_ERROR "libstb not found; it provides the stb_image implementation")
endif()

# ---------------------------------------------------------------------------
# libnle
# ---------------------------------------------------------------------------

file(GLOB_RECURSE NLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/nle/*.cpp")

add_library(nle STATIC ${NLE_SOURCES})
add_library(nle::nle ALIAS nle)

# Two include roots, not one per directory as the old Makefile had: "src" for
# fully qualified includes such as <nle/nle.h>, and "src/nle" for the short
# forms the engine sources already use.
target_include_directories(nle PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nle"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/nlohmann")

target_compile_options(nle PRIVATE -Wall -Wextra)

if(NLE_ENABLE_DEBUG_OUTPUT)
    target_compile_definitions(nle PUBLIC DEBUG)
endif()

target_link_libraries(nle PUBLIC
    OpenGL::GL
    OpenGL::GLU
    GLEW::GLEW
    "${STB_LIBRARY}")

if(glfw3_FOUND)
    target_link_libraries(nle PUBLIC glfw)
else()
    target_link_libraries(nle PUBLIC PkgConfig::GLFW)
endif()

if(IMGUI_FOUND)
    target_link_libraries(nle PUBLIC PkgConfig::IMGUI)
else()
    find_library(IMGUI_LIBRARY NAMES imgui REQUIRED)
    target_link_libraries(nle PUBLIC "${IMGUI_LIBRARY}")
endif()

if(SDL2MIXER_FOUND)
    target_link_libraries(nle PUBLIC PkgConfig::SDL2MIXER)
    target_compile_definitions(nle PUBLIC NLE_HAS_AUDIO)
else()
    message(STATUS "SDL2_mixer not found; nle is being built without sound")
endif()

# Shaders are read at runtime, so executables need to know where they live.
set(NLE_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shader" CACHE PATH "runtime shader directory")

# And the engine's own handful of pictures, of which there is currently one.
set(NLE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" CACHE PATH "runtime engine asset directory")

target_compile_definitions(nle PUBLIC
    NLE_SHADER_DIR="${NLE_SHADER_DIR}"
    NLE_ASSET_DIR="${NLE_ASSET_DIR}")

# ---------------------------------------------------------------------------
# tests
# ---------------------------------------------------------------------------

if(NLE_BUILD_TESTS)
    enable_testing()

    add_executable(nle_gltf_animation_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/gltf_animation_test.cpp")
    target_link_libraries(nle_gltf_animation_test PRIVATE nle)
    target_compile_options(nle_gltf_animation_test PRIVATE -Wall -Wextra)

    add_test(NAME gltf_animation
             COMMAND nle_gltf_animation_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/assets")

    # The test needs a GL context; without a display it reports 77 rather than
    # failing, and ctest treats that as skipped.
    set_tests_properties(gltf_animation PROPERTIES SKIP_RETURN_CODE 77)

    # This one does not: the camera is a transform and some arithmetic, and
    # what it collides with arrives through a callback a stub can supply.
    # Six planes and some arithmetic; no context needed.
    add_executable(nle_frustum_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/frustum_test.cpp")
    target_link_libraries(nle_frustum_test PRIVATE nle)
    target_compile_options(nle_frustum_test PRIVATE -Wall -Wextra)

    add_test(NAME frustum COMMAND nle_frustum_test)

    add_executable(nle_third_person_camera_test
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/third_person_camera_test.cpp")
    target_link_libraries(nle_third_person_camera_test PRIVATE nle)
    target_compile_options(nle_third_person_camera_test PRIVATE -Wall -Wextra)

    add_test(NAME third_person_camera COMMAND nle_third_person_camera_test)

    # Also headless: input_handler is a template over the window handle, so the
    # test instantiates it over an int and drives the setters a poll would.
    add_executable(nle_input_handler_test
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/input_handler_test.cpp")
    target_link_libraries(nle_input_handler_test PRIVATE nle)
    target_compile_options(nle_input_handler_test PRIVATE -Wall -Wextra)

    add_test(NAME input_handler COMMAND nle_input_handler_test)

    # A shader that will not compile draws nothing and says nothing about it,
    # so compiling and drawing it is the test. Needs a context.
    add_executable(nle_particle_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/particle_test.cpp")
    target_link_libraries(nle_particle_test PRIVATE nle)
    target_compile_options(nle_particle_test PRIVATE -Wall -Wextra)

    add_test(NAME particles COMMAND nle_particle_test)
    set_tests_properties(particles PROPERTIES SKIP_RETURN_CODE 77)

    # Opens a real device and really plays, very quietly. Passes without one
    # too, which is the case a build server actually runs.
    add_executable(nle_audio_test "${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_test.cpp")
    target_link_libraries(nle_audio_test PRIVATE nle)
    target_compile_options(nle_audio_test PRIVATE -Wall -Wextra)

    add_test(NAME audio COMMAND nle_audio_test)

    # Building a terrain uploads a mesh, so this one needs a context too.
    add_executable(nle_terrain_sculpt_test
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/terrain_sculpt_test.cpp")
    target_link_libraries(nle_terrain_sculpt_test PRIVATE nle)
    target_compile_options(nle_terrain_sculpt_test PRIVATE -Wall -Wextra)

    add_test(NAME terrain_sculpt COMMAND nle_terrain_sculpt_test)
    set_tests_properties(terrain_sculpt PROPERTIES SKIP_RETURN_CODE 77)
endif()
