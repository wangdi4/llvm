
#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

# This module is shared by multiple languages; use include blocker.
if(__LINUX_COMPILER_INTEL)
  return()
endif()
set(__LINUX_COMPILER_INTEL 1)

message(STATUS "Setting up Intel toolchain")

set(SET_XCOMPILE_SYSROOT_CMD "")
set(SYSROOT "")

if (OCL_MEEGO)
  include("${OCL_CMAKE_INCLUDE_DIRECTORIES}/Linux-Meego-Cross.cmake")
  set(SET_XCOMPILE_SYSROOT_CMD "-isystem=${SYSROOT}")
  add_definitions(-D MEEGO)
endif (OCL_MEEGO)

set(INTEL_COMPILER_VERSION 11.1)
set(INTEL_COMPILER_ROOT "/opt/intel/Compiler/${INTEL_COMPILER_VERSION}")


execute_process(COMMAND find ${INTEL_COMPILER_ROOT} -regex ".*/[0-9]*/include" -type d
    OUTPUT_VARIABLE INTEL_INCLUDE_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(SYSTEM "${INTEL_INCLUDE_PATH}")

if (BUILD_X64)
    execute_process(COMMAND find ${INTEL_COMPILER_ROOT} -wholename "*/bin/intel64" -type d
        OUTPUT_VARIABLE INTEL_BIN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    execute_process(COMMAND find ${INTEL_COMPILER_ROOT} -wholename "*/bin/ia32" -type d
        OUTPUT_VARIABLE INTEL_BIN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

if (NOT IS_DIRECTORY ${INTEL_BIN_PATH})
    message(FATAL_ERROR "Can't find Intel compiler at ${INTEL_COMPILER_ROOT}")
endif (NOT IS_DIRECTORY ${INTEL_BIN_PATH})

set(CMAKE_C_COMPILER "${INTEL_BIN_PATH}/icc")
set(CMAKE_CXX_COMPILER "${INTEL_BIN_PATH}/icpc")


if(NOT XIAR)
  set(_intel_xiar_hints)
  foreach(lang C CXX Fortran)
    if(IS_ABSOLUTE "${CMAKE_${lang}_COMPILER}")
      get_filename_component(_hint "${CMAKE_${lang}_COMPILER}" PATH)
      list(APPEND _intel_xiar_hints ${_hint})
    endif()
  endforeach()
  find_program(XIAR NAMES xiar HINTS ${_intel_xiar_hints})
  mark_as_advanced(XIAR)
endif(NOT XIAR)

macro(__linux_compiler_intel lang)
  set(CMAKE_SHARED_LIBRARY_${lang}_FLAGS "-fPIC")
  set(CMAKE_SHARED_LIBRARY_CREATE_${lang}_FLAGS "-shared")

  # We pass this for historical reasons.  Projects may have
  # executables that use dlopen but do not set ENABLE_EXPORTS.
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-rdynamic")

  if(XIAR)
    # INTERPROCEDURAL_OPTIMIZATION
    set(CMAKE_${lang}_COMPILE_OPTIONS_IPO -ipo)
    set(CMAKE_${lang}_CREATE_STATIC_LIBRARY_IPO
      "${XIAR} cr <TARGET> <LINK_FLAGS> <OBJECTS> "
      "${XIAR} -s <TARGET> ")
  endif()
endmacro()

__linux_compiler_intel(C)
__linux_compiler_intel(CXX)

set( CMAKE_ASM_COMPILER  gas PARENT_SCOPE )

if (TARGET_CPU STREQUAL "Atom")
    message(STATUS "Setting up ATOM Intel cross-compilation flags and tools")

    set(CMAKE_SYSTEM_PROCESSOR ATOM)
    # for gcc > 4.5.0 use "-march=atom" (no -mtune)
    # Options not supported by cross compiler: "-O3 -flto"
    set(ATOM_SPECIFIC_FLAGS           " -xSSE3_ATOM --ipo  ")
endif(TARGET_CPU STREQUAL "Atom")

