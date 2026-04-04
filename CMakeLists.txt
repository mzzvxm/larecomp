
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
)

if(WIN32)
    add_executable(larecomp WIN32 ${LARECOMP_SOURCES})
else()
    add_executable(larecomp ${LARECOMP_SOURCES})
endif()

rexglue_setup_target(larecomp)
