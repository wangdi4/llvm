# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Build offloading library and related plugins.
#
##===----------------------------------------------------------------------===##

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  message(FATAL_ERROR "Direct configuration not supported, please use parent directory!")
endif()

# Add cmake directory to search for custom cmake functions.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})

# Set the path of all resulting libraries to a unified location so that it can
# be used for testing.
set(LIBOMPTARGET_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
# INTEL_CUSTOMIZATION
# These settings override CMAKE defaults. These are producing problems
# for any dependencies libomptarget may depend on, e.g. libOpenCL.so. It leads
# to non-determenistic placement of libOpenCL.so in the build directory.
# Disabling for now to test more.
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIBOMPTARGET_LIBRARY_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBOMPTARGET_LIBRARY_DIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LIBOMPTARGET_LIBRARY_DIR})
# end INTEL_CUSTOMIZATION

# Message utilities.
include(LibomptargetUtils)

# Get dependencies for the different components of the project.
include(LibomptargetGetDependencies)

# LLVM source tree is required at build time for libomptarget
if (NOT LIBOMPTARGET_LLVM_INCLUDE_DIRS)
  message(FATAL_ERROR "Missing definition for LIBOMPTARGET_LLVM_INCLUDE_DIRS")
endif()

include_directories(${LIBOMPTARGET_LLVM_INCLUDE_DIRS})

# This is a list of all the targets that are supported/tested right now.
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} aarch64-unknown-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} aarch64-unknown-linux-gnu-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} aarch64-unknown-linux-gnu-LTO")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} amdgcn-amd-amdhsa")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} amdgcn-amd-amdhsa-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} amdgcn-amd-amdhsa-LTO")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64le-ibm-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64le-ibm-linux-gnu-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64le-ibm-linux-gnu-LTO")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64-ibm-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64-ibm-linux-gnu-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64-ibm-linux-gnu-LTO")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} x86_64-pc-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} x86_64-pc-linux-gnu-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} x86_64-pc-linux-gnu-LTO")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} nvptx64-nvidia-cuda")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} nvptx64-nvidia-cuda-oldDriver")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} nvptx64-nvidia-cuda-LTO")

# Once the plugins for the different targets are validated, they will be added to
# the list of supported targets in the current system.
set (LIBOMPTARGET_SYSTEM_TARGETS "")
set (LIBOMPTARGET_TESTED_PLUGINS "")

# Check whether using debug mode. In debug mode, allow dumping progress
# messages at runtime by default. Otherwise, it can be enabled
# independently using the LIBOMPTARGET_ENABLE_DEBUG option.
# INTEL_CUSTOMIZATION
# Always build debug version for Intel compiler product.
option(LIBOMPTARGET_ENABLE_DEBUG "Allow debug output with the environment variable LIBOMPTARGET_DEBUG=1" ON)
# end INTEL_CUSTOMIZATION
string( TOLOWER "${CMAKE_BUILD_TYPE}" LIBOMPTARGET_CMAKE_BUILD_TYPE)
if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
  option(LIBOMPTARGET_ENABLE_DEBUG "Allow debug output with the environment variable LIBOMPTARGET_DEBUG=1" ON)
else()
  option(LIBOMPTARGET_ENABLE_DEBUG "Allow debug output with the environment variable LIBOMPTARGET_DEBUG=1" OFF)
endif()
if(LIBOMPTARGET_ENABLE_DEBUG)
  add_definitions(-DOMPTARGET_DEBUG)
endif()
if(INTEL_CUSTOMIZATION)
  option(LIBOMPTARGET_VERIFY_DLL_SIGNATURE "Verify signature of the loaded plugins" OFF)
  if(LIBOMPTARGET_VERIFY_DLL_SIGNATURE)
    add_definitions(-DOMPTARGET_VERIFY_DLL_SIGNATURE=1)
  endif()
endif(INTEL_CUSTOMIZATION)

set(LIBOMPTARGET_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (INTEL_CUSTOMIZATION)
  if (OPENMP_STANDALONE_BUILD)
    message(FATAL_ERROR "Standalone OpenMP build is unsupported.")
  else()
    if (WIN32)
      set(LIBOMP_LIB_NAME iomp5md libiomp5md)
    else()
      set(LIBOMP_LIB_NAME iomp5 libiomp5)
    endif()

    if ((NOT LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER) OR
        (NOT EXISTS ${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}))
      if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        if (WIN32)
          set(LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER
            "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/ia32_win")
        else()
          set(LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER
            "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/ia32_lin")
        endif()
      else()
        if (WIN32)
          set(LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER
            "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/intel64_win")
        else()
          set(LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER
            "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/intel64_lin")
        endif()
      endif()
    endif()

    message(STATUS "Looking for OMP library (${LIBOMP_LIB_NAME}) in "
      "${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}")

    if (NOT LIBOMP_LIB OR NOT EXISTS ${LIBOMP_LIB})
      # Force cache reset, if the search path has been updated.
      # This will let find_library() try a new path instead of
      # taking non-existent LIBOMP_LIB path from the cache.
      set(LIBOMP_LIB
        "LIBOMP_LIB-NOTFOUND" CACHE FILEPATH "Path to libiomp5 library" FORCE)
    endif()
    find_library(LIBOMP_LIB NAMES ${LIBOMP_LIB_NAME}
      PATHS "${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}"
      NO_DEFAULT_PATH)

    if (NOT LIBOMP_LIB)
      message(FATAL_ERROR "OMP library required for libomptarget is not found.")
    endif()

    message(STATUS "OMP library found: ${LIBOMP_LIB}")

    # We cannot use UNC paths with forward slashes with MS linker,
    # so we have to copy the OMP library to the current binary directory.
    get_filename_component(LIBOMP_LIB_BASENAME "${LIBOMP_LIB}" NAME)
    set(LIBOMP_LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${LIBOMP_LIB_BASENAME})
    add_custom_target(libomptarget-libiomp-file
      BYPRODUCTS ${LIBOMP_LIB_FILE}
      DEPENDS ${LIBOMP_LIB}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${LIBOMP_LIB}" ${LIBOMP_LIB_FILE}
      COMMENT "Copying ${LIBOMP_LIB} to ${LIBOMP_LIB_FILE}")

    if ((NOT LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER) OR
        (NOT EXISTS ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}))
      set(LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER
        "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/include")
    endif()

    message(STATUS "Looking for omp.h in "
      "${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")
    if (NOT LIBOMP_OMP_H_FILE OR NOT EXISTS ${LIBOMP_OMP_H_FILE})
      # Force cache reset, if the search path has been updated.
      # This will let find_file() try a new path instead of
      # taking non-existent LIBOMP_OMP_H_FILE path from the cache.
      set(LIBOMP_OMP_H_FILE
        "LIBOMP_OMP_H_FILE-NOTFOUND" CACHE FILEPATH "Path to omp.h" FORCE)
    endif()
    find_file(LIBOMP_OMP_H_FILE "omp.h"
      PATHS ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}
      NO_DEFAULT_PATH)
    if (NOT LIBOMP_OMP_H_FILE)
      message(FATAL_ERROR "omp.h required for libomptarget is not found.")
    endif()

    message(STATUS "omp.h found: ${LIBOMP_OMP_H_FILE}")
  endif()
endif(INTEL_CUSTOMIZATION)

# Build target agnostic offloading library.
set(LIBOMPTARGET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${LIBOMPTARGET_SRC_DIR})

# Definitions for testing, for reuse when testing libomptarget-nvptx.
if (INTEL_CUSTOMIZATION)
  if (OPENMP_STANDALONE_BUILD)
    message(FATAL_ERROR
      "Standalone OpenMP build is not supported in xmain yet.")
  endif()
  set(LIBOMP_INCLUDE_DIR ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER})
  set(LIBOMP_LIBRARY_DIR ${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER})
endif(INTEL_CUSTOMIZATION)
set(LIBOMPTARGET_OPENMP_HEADER_FOLDER "${LIBOMP_INCLUDE_DIR}" CACHE STRING
  "Path to folder containing omp.h")
set(LIBOMPTARGET_OPENMP_HOST_RTL_FOLDER "${LIBOMP_LIBRARY_DIR}" CACHE STRING
  "Path to folder containing libomp.so, and libLLVMSupport.so with profiling enabled")
set(LIBOMPTARGET_LLVM_LIBRARY_DIR "${LLVM_LIBRARY_DIR}" CACHE STRING
  "Path to folder containing llvm library libomptarget.so")

# Build offloading plugins and device RTLs if they are available.
add_subdirectory(plugins)
if(INTEL_CUSTOMIZATION)
add_subdirectory(deviceRTLs)
else(INTEL_CUSTOMIZATION)
add_subdirectory(DeviceRTL)
endif(INTEL_CUSTOMIZATION)
add_subdirectory(tools)

# Add tests.
add_subdirectory(test)
