#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

# ##############################################################################
# This file adds ITT (former GPA) definitions to your cmake project.
#

# ITT/VTune integration
add_definitions(-DUSE_ITT)

# From CMake documentation: If the SYSTEM option is given, the compiler will be
# told the directories are meant as system include directories on some
# platforms. Signalling this setting might achieve effects such as the compiler
# skipping warnings, or these fixed-install system files not being considered in
# dependency calculations - see compiler docs.
set(EXTERNALS_DIR ${CMAKE_SOURCE_DIR}/externals)
if(OPENCL_INTREE_BUILD)
  set(EXTERNALS_DIR ${PROJECT_SOURCE_DIR}/externals)
endif()

# disable "implicit-fallthrough", "self-assign" and "static-in-inline"(windows
# only) warnings in itt/ittnotify/ittnotify_static.c
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|IntelLLVM")
  set_source_files_properties(
    ${EXTERNALS_DIR}/itt/ittnotify/ittnotify_static.c
    PROPERTIES
      COMPILE_FLAGS
      "-Wno-implicit-fallthrough -Wno-self-assign -Wno-static-in-inline")
endif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|IntelLLVM")

include_directories(SYSTEM ${EXTERNALS_DIR}/itt/include
                    ${EXTERNALS_DIR}/itt/ittnotify/)

# ITT libraries directory suffix
if(USE_GPA)
  if(BUILD_X64)
    set(GPA_LIB_DIR_SUFFIX x64)
  else()
    set(GPA_LIB_DIR_SUFFIX x86)
  endif()

  add_library(imp_gpasdk STATIC IMPORTED)
  set_property(
    TARGET imp_gpasdk
    PROPERTY IMPORTED_LOCATION
             ${EXTERNALS_DIR}/gpa/libs/${GPA_LIB_DIR_SUFFIX}/gpasdk_s.lib)

  list(APPEND LINK_LIBS imp_gpasdk)
else()
  list(APPEND TARGET_SOURCES ${EXTERNALS_DIR}/itt/ittnotify/ittnotify_static.c)
endif()
