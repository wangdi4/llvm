
#=============================================================================
# Copyright 2010 Kitware, Inc.
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
if(__LINUX_COMPILER_GNU)
  return()
endif()
set(__LINUX_COMPILER_GNU 1)

message(STATUS "Setting up GNU toolchain")

# GCC 4.5
set(GCC_MAJOR_VERSION 4 CACHE STRING "GCC Major version to be used.")
set(GCC_MINOR_VERSION 5 CACHE STRING "GCC minor version to be used.")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

set(SET_XCOMPILE_SYSROOT_CMD "")
set(SYSROOT "")

if (OCL_MEEGO)
  include("${OCL_CMAKE_INCLUDE_DIRECTORIES}/Linux-Meego-Cross.cmake")
  set(SET_XCOMPILE_SYSROOT_CMD "--sysroot=${SYSROOT}")
  add_definitions(-D MEEGO)
endif (OCL_MEEGO)

if (TARGET_CPU STREQUAL "Atom")
    include(CMakeForceCompiler)
    
    message(STATUS "Setting up ATOM GNU cross-compilation flags and tools")
    set(CMAKE_SYSTEM_PROCESSOR ATOM)
    
    # for gcc > 4.5.0 use "-march=atom" (no -mtune)
    # Options not supported by cross compiler: "-O3 -flto"
    set(ATOM_SPECIFIC_FLAGS_ASM          -march=core2 -mtune=generic32) # Do not quote (CREATE_ASM_RULES)!
    set(ATOM_SPECIFIC_FLAGS              "-march=core2 -mtune=generic32 -mfpmath=sse -ffast-math")

    # specify the cross compiler
    execute_process(COMMAND find /usr/lib/madde/ -name i586-meego-linux-gnu-gcc
        OUTPUT_VARIABLE C_CROSS_COMPILER OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT EXISTS ${C_CROSS_COMPILER})
        message(FATAL_ERROR "Can't find i586-meego-linux-gnu-gcc ATOM cross compiler at /usr/lib/madde/")
    endif (NOT EXISTS ${C_CROSS_COMPILER})
    CMAKE_FORCE_C_COMPILER("${C_CROSS_COMPILER}" GNU-ATOM)

    execute_process(COMMAND find /usr/lib/madde/ -name i586-meego-linux-gnu-g++
        OUTPUT_VARIABLE CXX_CROSS_COMPILER OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT EXISTS ${CXX_CROSS_COMPILER})
        message(FATAL_ERROR "Can't find i586-meego-linux-gnu-g++ ATOM cross compiler at /usr/lib/madde/")
    endif (NOT EXISTS ${CXX_CROSS_COMPILER})
    CMAKE_FORCE_CXX_COMPILER("${CXX_CROSS_COMPILER}" GNU-ATOM)
    

    execute_process(COMMAND find /usr/lib/madde/ -name i586-meego-linux-gnu-as
        OUTPUT_VARIABLE ASM_CROSS_COMPILER OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (NOT EXISTS ${ASM_CROSS_COMPILER})
        message(FATAL_ERROR "Can't find i586-meego-linux-gnu-as ATOM cross assembler at /usr/lib/madde/")
    endif (NOT EXISTS ${ASM_CROSS_COMPILER})
    set(CMAKE_ASM_COMPILER ${ASM_CROSS_COMPILER} PARENT_SCOPE)
    #CMAKE_FORCE_ASM_COMPILER("${ASM_CROSS_COMPILER}" GNU)

    execute_process(COMMAND find /usr/lib/madde/ -name i586-meego-linux-gnu-nm
        OUTPUT_VARIABLE CROSS_NM_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND dirname ${C_CROSS_COMPILER}
        OUTPUT_VARIABLE CROSS_COMPILATION_BIN_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    message(STATUS "Setting up Intel Core GNU flags and tools")

    #set(CMAKE_C_COMPILER "gcc-${OCL_GNU_VERSION}")   
    #set(CMAKE_CXX_COMPILER "g++-${OCL_GNU_VERSION}")
    #set(CMAKE_CPP_COMPILER "cpp-${OCL_GNU_VERSION}")
    set(CMAKE_C_COMPILER "gcc")   
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_CPP_COMPILER "cpp")
    set(CMAKE_ASM_COMPILER "as")
endif (TARGET_CPU STREQUAL "Atom")  


macro(__linux_compiler_gnu lang)
  # We pass this for historical reasons.  Projects may have
  # executables that use dlopen but do not set ENABLE_EXPORTS.
  set(CMAKE_SHARED_LIBRARY_LINK_${lang}_FLAGS "-rdynamic")
endmacro()

__linux_compiler_gnu(C)
__linux_compiler_gnu(CXX)

# get GCC version
execute_process( COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE GCC_VER_STR ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE )
if (GCC_VER_STR MATCHES ".*(GCC|gcc).*(${GCC_MAJOR_VERSION}\\.[0-9])\\.*")
        set(GCC_VER ${CMAKE_MATCH_2})
        message(STATUS "GCC_VER = ${GCC_VER}")
else ()
        message(FATAL_ERROR "Need GCC version ${GCC_MAJOR_VERSION}.x, and you only have '${GCC_VER_STR}' (${CMAKE_C_COMPILER})")
endif (GCC_VER_STR MATCHES ".*(GCC|gcc).*(${GCC_MAJOR_VERSION}\\.[0-9])\\.*")

if (GCC_VER LESS 4.1)
        set(SSE4_VAL)
        set(VIRTUAL_DISTRUCTOR_WARNING "-Wno-non-virtual-dtor")
else()
        set(SSE4_VAL -msse4.1)
        set(VIRTUAL_DISTRUCTOR_WARNING)
endif(GCC_VER LESS 4.1)

# gcc 4.1 complains about missing virtual dtor for interface classes. this is bad and wrong, we should supress it.
add_definitions(${VIRTUAL_DISTRUCTOR_WARNING})

