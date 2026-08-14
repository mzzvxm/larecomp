
# larecomp - ReXGlue Recompiled Project
#
# This file is yours to edit. 'rexglue migrate' will NOT overwrite it.
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(larecomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(REXGLUE_REQUIRED_GRAPHICS_BACKENDS "d3d12;vulkan" CACHE STRING "Required ReXGlue graphics backends")
else()
    set(REXGLUE_REQUIRED_GRAPHICS_BACKENDS "vulkan" CACHE STRING "Required ReXGlue graphics backends")
endif()

include(generated/rexglue.cmake)

# Sources
set(LARECOMP_SOURCES
    src/main.cpp
    src/spdlog_console.cpp
    src/crash_handler.cpp

    src/mc_engine/threading.cpp
    src/mc_engine/hooks.cpp
    src/mc_engine/graphics_button.cpp

    src/discord_rpc/discord_rpc.cpp
    src/discord_rpc/discord_hooks.cpp

    src/isoinstaller/larecomp_iso_installer.cpp
)
if(WIN32)
    add_executable(larecomp WIN32 ${LARECOMP_SOURCES})
else()
    add_executable(larecomp ${LARECOMP_SOURCES})
endif()

# GPU_PLUGINS stages rexgpu-xenos next to the executable: the Xenos GPU
# emulation is a runtime-loaded plugin now, so it is never linked and
# TARGET_RUNTIME_DLLS does not see it.
rexglue_setup_target(larecomp GPU_PLUGINS xenos)

# rexglue 0.7.4
 target_link_libraries(larecomp
     PRIVATE
     dxgi
 )
