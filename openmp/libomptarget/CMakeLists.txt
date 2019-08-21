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

if(OPENMP_STANDALONE_BUILD)
  # Build all libraries into a common place so that tests can find them.
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Message utilities.
include(LibomptargetUtils)

# Get dependencies for the different components of the project.
include(LibomptargetGetDependencies)

# This is a list of all the targets that are supported/tested right now.
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} aarch64-unknown-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64le-ibm-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} powerpc64-ibm-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} x86_64-pc-linux-gnu")
set (LIBOMPTARGET_ALL_TARGETS "${LIBOMPTARGET_ALL_TARGETS} nvptx64-nvidia-cuda")

# Once the plugins for the different targets are validated, they will be added to
# the list of supported targets in the current system.
set (LIBOMPTARGET_SYSTEM_TARGETS "")

# Check whether using debug mode. In debug mode, allow dumping progress
# messages at runtime by default. Otherwise, it can be enabled
# independently using the LIBOMPTARGET_ENABLE_DEBUG option.
string( TOLOWER "${CMAKE_BUILD_TYPE}" LIBOMPTARGET_CMAKE_BUILD_TYPE)
if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
  option(LIBOMPTARGET_ENABLE_DEBUG "Allow debug output with the environment variable LIBOMPTARGET_DEBUG=1" ON)
else()
  option(LIBOMPTARGET_ENABLE_DEBUG "Allow debug output with the environment variable LIBOMPTARGET_DEBUG=1" OFF)
endif()
if(LIBOMPTARGET_ENABLE_DEBUG)
  add_definitions(-DOMPTARGET_DEBUG)
endif()
# INTEL_CUSTOMIZATION
add_definitions(-DOMPTARGET_DEBUG)
# end INTEL_CUSTOMIZATION

include_directories(include)

# INTEL_COLLAB
  if (OPENMP_STANDALONE_BUILD)
    message(FATAL_ERROR "Standalone OpenMP build is unsupported.")
  else()
    if (WIN32)
      set(LIBOMP_LIB_NAME iomp5md libiomp5md)
    else()
      set(LIBOMP_LIB_NAME iomp5 libiomp5)
    endif()
    set(LIBOMP_LIB_LOC ${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER})

    if (INTEL_CUSTOMIZATION)
      if ((NOT LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER) OR
          (NOT EXISTS ${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}))
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
          if (WIN32)
            set(LIBOMP_LIB_LOC
              "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/ia32_win")
          else()
            set(LIBOMP_LIB_LOC
              "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/ia32_lin")
          endif()
        else()
          if (WIN32)
            set(LIBOMP_LIB_LOC
              "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/intel64_win")
          else()
            set(LIBOMP_LIB_LOC
              "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/lib/intel64_lin")
          endif()
        endif()
      endif()
    endif(INTEL_CUSTOMIZATION)

    set(LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER ${LIBOMP_LIB_LOC} CACHE STRING
        "Path to folder containing libiomp5 library" FORCE)

    message(STATUS "Looking for OMP library (${LIBOMP_LIB_NAME}) in \
                    ${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}")
    find_library(LIBOMP_LIB NAMES ${LIBOMP_LIB_NAME}
                 PATHS "${LIBOMPTARGET_OPENMP_TARGET_RTL_FOLDER}"
                 NO_DEFAULT_PATH)

    if (NOT LIBOMP_LIB)
      message(FATAL_ERROR "OMP library required for libomptarget is not found.")
    endif()

    message(STATUS "OMP library found: ${LIBOMP_LIB}")

    if (INTEL_CUSTOMIZATION)
      # We cannot use UNC paths with forward slashes with MS linker,
      # so we have to copy the OMP library to the current binary directory.
      get_filename_component(LIBOMP_LIB_BASENAME "${LIBOMP_LIB}" NAME)
      set(LIBOMP_LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${LIBOMP_LIB_BASENAME})
      add_custom_target(libomptarget-libiomp-file
        DEPENDS ${LIBOMP_LIB}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${LIBOMP_LIB}" ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Copying ${LIBOMP_LIB} to ${CMAKE_CURRENT_BINARY_DIR}")
    else(INTEL_CUSTOMIZATION)
      set(LIBOMP_LIB_FILE "${LIBOMP_LIB}")
      add_custom_target(libomptarget-libiomp-file
                        DEPENDS ${LIBOMP_LIB})
    endif(INTEL_CUSTOMIZATION)

    set(LIBOMP_HEADER_LOC ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER})

    if (INTEL_CUSTOMIZATION)
      if ((NOT LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER) OR
          (NOT EXISTS ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}))
        set(LIBOMP_HEADER_LOC
          "${IL0_PREBUILT_COMPONENTS_PREFIX}/compiler/include")
      endif()
    endif(INTEL_CUSTOMIZATION)

    set(LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER
        ${LIBOMP_HEADER_LOC} CACHE STRING
        "Path to folder containing omp.h" FORCE)

    message(STATUS "Looking for omp.h in \
                    ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}")
    find_file(LIBOMP_OMP_H_FILE "omp.h"
              PATHS ${LIBOMPTARGET_OPENMP_TARGET_HEADER_FOLDER}
              NO_DEFAULT_PATH)
    if (NOT LIBOMP_OMP_H_FILE)
      message(FATAL_ERROR "omp.h requires for libomptarget is not found.")
    endif()

    message(STATUS "omp.h found: ${LIBOMP_OMP_H_FILE}")
  endif()
# end INTEL_COLLAB

# Build target agnostic offloading library.
add_subdirectory(src)

# Retrieve the path to the resulting library so that it can be used for 
# testing.
get_target_property(LIBOMPTARGET_LIBRARY_DIR omptarget LIBRARY_OUTPUT_DIRECTORY)
if(NOT LIBOMPTARGET_LIBRARY_DIR)
  set(LIBOMPTARGET_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

# Definitions for testing, for reuse when testing libomptarget-nvptx.
if(OPENMP_STANDALONE_BUILD)
  set(LIBOMPTARGET_OPENMP_HEADER_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/../runtime/src" CACHE STRING
    "Path to folder containing omp.h")
  set(LIBOMPTARGET_OPENMP_HOST_RTL_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/../runtime/src" CACHE STRING
    "Path to folder containing libomp.so")
else()
  set(LIBOMPTARGET_OPENMP_HEADER_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/../runtime/src")
endif()


# Build offloading plugins and device RTLs if they are available.
add_subdirectory(plugins)
add_subdirectory(deviceRTLs)

# Add tests.
add_subdirectory(test)
