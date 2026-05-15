
# larecomp - ReXGlue Recompiled Project
#
# This file is yours to edit. 'rexglue migrate' will NOT overwrite it.
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(larecomp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(LARECOMP_SOURCES
    src/main.cpp
    src/spdlog_console.cpp
    src/crash_handler.cpp
    src/mc_engine/threading.cpp
    src/mc_engine/hooks.cpp
)

if(WIN32)
    add_executable(larecomp WIN32 ${LARECOMP_SOURCES})
else()
    add_executable(larecomp ${LARECOMP_SOURCES})
endif()

rexglue_setup_target(larecomp)

# rexglue 0.7.4
 target_link_libraries(larecomp
     PRIVATE
     dxgi 
 )