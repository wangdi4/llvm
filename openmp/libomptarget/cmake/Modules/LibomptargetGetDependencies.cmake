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
#
#//===----------------------------------------------------------------------===//
#//
#// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
#// See https://llvm.org/LICENSE.txt for license information.
#// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#//
#//===----------------------------------------------------------------------===//
#

# Try to detect in the system several dependencies required by the different
# components of libomptarget. These are the dependencies we have:
#
# libffi : required to launch target kernels given function and argument
#          pointers.
# CUDA : required to control offloading to NVIDIA GPUs.

include (FindPackageHandleStandardArgs)

################################################################################
# Looking for LLVM...
################################################################################

if (OPENMP_STANDALONE_BUILD)
  # Complete LLVM package is required for building libomptarget
  # in an out-of-tree mode.
  find_package(LLVM REQUIRED)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVM in: ${LLVM_DIR}")
  list(APPEND LIBOMPTARGET_LLVM_INCLUDE_DIRS ${LLVM_INCLUDE_DIRS})
  list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
  include(AddLLVM)
  if(TARGET omptarget)
    message(FATAL_ERROR "CMake target 'omptarget' already exists. "
                        "Use an LLVM installation that doesn't expose its 'omptarget' target.")
  endif()
else()
  # Note that OPENMP_STANDALONE_BUILD is FALSE, when
  # openmp is built with -DLLVM_ENABLE_RUNTIMES="openmp" vs
  # -DLLVM_ENABLE_PROJECTS="openmp", but openmp build
  # is actually done as a standalone project build with many
  # LLVM CMake variables propagated to it.
  list(APPEND LIBOMPTARGET_LLVM_INCLUDE_DIRS
    ${LLVM_MAIN_INCLUDE_DIR} ${LLVM_BINARY_DIR}/include
    )
  message(STATUS
    "Using LLVM include directories: ${LIBOMPTARGET_LLVM_INCLUDE_DIRS}")
endif()

# INTEL_CUSTOMIZATION
################################################################################
# Looking for libelf...
################################################################################

find_path (
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR
  NAMES
    libelf.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH
  PATH_SUFFIXES
    libelf)

find_library (
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES
  NAMES
    elf
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

set(LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBELF
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBELF_LIBRARIES)
# end INTEL_CUSTOMIZATION

################################################################################
# Looking for libffi...
################################################################################
find_package(PkgConfig)

pkg_check_modules(LIBOMPTARGET_SEARCH_LIBFFI QUIET libffi)

find_path (
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR
  NAMES
    ffi.h
  HINTS
    ${LIBOMPTARGET_SEARCH_LIBFFI_INCLUDEDIR}
    ${LIBOMPTARGET_SEARCH_LIBFFI_INCLUDE_DIRS}
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    ENV CPATH)

# Don't bother look for the library if the header files were not found.
if (LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR)
  find_library (
      LIBOMPTARGET_DEP_LIBFFI_LIBRARIES
    NAMES
      ffi
    HINTS
      ${LIBOMPTARGET_SEARCH_LIBFFI_LIBDIR}
      ${LIBOMPTARGET_SEARCH_LIBFFI_LIBRARY_DIRS}
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH)
endif()

set(LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS ${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_LIBFFI
  DEFAULT_MSG
  LIBOMPTARGET_DEP_LIBFFI_LIBRARIES
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS
  LIBOMPTARGET_DEP_LIBFFI_LIBRARIES)

################################################################################
# Looking for CUDA...
################################################################################

find_package(CUDAToolkit QUIET)
set(LIBOMPTARGET_DEP_CUDA_FOUND ${CUDAToolkit_FOUND})

################################################################################
# Looking for NVIDIA GPUs...
################################################################################
set(LIBOMPTARGET_DEP_CUDA_ARCH "sm_35")

if(TARGET nvptx-arch)
  get_property(LIBOMPTARGET_NVPTX_ARCH TARGET nvptx-arch PROPERTY LOCATION)
else()
  find_program(LIBOMPTARGET_NVPTX_ARCH NAMES nvptx-arch
               PATHS ${LLVM_TOOLS_BINARY_DIR}/bin)
endif()

if(LIBOMPTARGET_NVPTX_ARCH)
  execute_process(COMMAND ${LIBOMPTARGET_NVPTX_ARCH}
                  OUTPUT_VARIABLE LIBOMPTARGET_NVPTX_ARCH_OUTPUT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" nvptx_arch_list "${LIBOMPTARGET_NVPTX_ARCH_OUTPUT}")
  if(nvptx_arch_list)
    set(LIBOMPTARGET_FOUND_NVIDIA_GPU TRUE)
    set(LIBOMPTARGET_NVPTX_DETECTED_ARCH_LIST "${nvptx_arch_list}")
    list(GET nvptx_arch_list 0 LIBOMPTARGET_DEP_CUDA_ARCH)
  endif()
endif()


################################################################################
# Looking for AMD GPUs...
################################################################################

if(TARGET amdgpu-arch)
  get_property(LIBOMPTARGET_AMDGPU_ARCH TARGET amdgpu-arch PROPERTY LOCATION)
else()
  find_program(LIBOMPTARGET_AMDGPU_ARCH NAMES amdgpu-arch
               PATHS ${LLVM_TOOLS_BINARY_DIR}/bin)
endif()

if(LIBOMPTARGET_AMDGPU_ARCH)
  execute_process(COMMAND ${LIBOMPTARGET_AMDGPU_ARCH}
                  OUTPUT_VARIABLE LIBOMPTARGET_AMDGPU_ARCH_OUTPUT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "\n" ";" amdgpu_arch_list "${LIBOMPTARGET_AMDGPU_ARCH_OUTPUT}")
  if(amdgpu_arch_list)
    set(LIBOMPTARGET_FOUND_AMDGPU_GPU TRUE)
    set(LIBOMPTARGET_AMDGPU_DETECTED_ARCH_LIST "${amdgpu_arch_list}")
  endif()
endif()

set(OPENMP_PTHREAD_LIB ${LLVM_PTHREAD_LIB})
# INTEL_CUSTOMIZATION

################################################################################
# Looking for OpenCL...
################################################################################

# Cmake 3.4.3 cannot find OpenCL library unless an OpenCL SDK is
# installed as root. This workaround is a fallback to let us find the library as
# well as includes for now.
message(STATUS "Looking for OpenCL includes.")
# If this command does not find the header files, then next
# find_path (see below) will look again.
add_llvm_external_project(opencl)
set(LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
set(LIBOMPTARGET_DEP_OPENCL_LIBRARIES OpenCL-ICD)

find_path(LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS
  NAMES
    CL/cl.h OpenCL/cl.h
  PATHS
    ENV LIBOMPTARGET_OCL_ROOT
    ENV CPATH
  PATH_SUFFIXES
    include include/sycl)
message(STATUS "OpenCL include DIR: ${LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS}")

if (NOT LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS)
  set(LIBOMPTARGET_DEP_OPENCL_FOUND FALSE)
  message(STATUS "Could NOT find OpenCL. Missing includes.")
else()

  message(STATUS "Looking for OpenCL library.")

  find_library(LIBOMPTARGET_DEP_OPENCL_LIBRARIES
    NAMES OpenCL
    PATHS
      ENV LIBOMPTARGET_OCL_ROOT
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH
    PATH_SUFFIXES
      lib lib64 lib/intel64_lin)
  message(STATUS "OpenCL lib: ${LIBOMPTARGET_DEP_OPENCL_LIBRARIES}")

  if (NOT LIBOMPTARGET_DEP_OPENCL_LIBRARIES)
    set(LIBOMPTARGET_DEP_OPENCL_FOUND FALSE)
    message(STATUS "Could NOT find OpenCL. Missing libs.")
  else()
    set(LIBOMPTARGET_DEP_OPENCL_FOUND TRUE)
  endif()

endif()

# FIXME: for some reason CMake is able to find locally installed
# OpenCL package (see find_package below), but OPENCL_LIBRARIES
# is left NOTFOUND. Figure out how to deal with that.
set(CMAKE_DISABLE_FIND_PACKAGE_OpenCL TRUE)

if (NOT LIBOMPTARGET_DEP_OPENCL_FOUND)
  message(STATUS "Looking for OpenCL again.")
  find_package(OpenCL)
  set(LIBOMPTARGET_DEP_OPENCL_FOUND ${OPENCL_FOUND})
  set(LIBOMPTARGET_DEP_OPENCL_LIBRARIES ${OPENCL_LIBRARIES})
  set(LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS ${OPENCL_INCLUDE_DIRS})
endif()

mark_as_advanced(
        LIBOMPTARGET_DEP_OPENCL_FOUND
        LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS
        LIBOMPTARGET_DEP_OPENCL_LIBRARIES)

################################################################################
# Looking for Level0
################################################################################

message(STATUS "Looking for Level0 includes.")

# It depends on OpenCL headers
if(NOT LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS)
  set(LIBOMPTARGET_DEP_LEVEL0_FOUND FALSE)
  message(STATUS "Could NOT find OpenCL for Level0. Missing includes.")
else()
  if(L0_INCLUDE_DIR)
    if(NOT EXISTS ${L0_INCLUDE_DIR})
      message(FATAL_ERROR
        "Level0 include path specified with L0_INCLUDE_DIR variable \
         (${L0_INCLUDE_DIR}) does not exist.")
    endif()
    set(LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS "${L0_INCLUDE_DIR}/level_zero")
  endif()
  find_path(LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS
    NAMES
      ze_api.h
    PATHS
      ENV LIBOMPTARGET_LEVEL0_ROOT
      ENV CPATH
    PATH_SUFFIXES
      level_zero
      include/level_zero)

  if(NOT LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS)
    set(LIBOMPTARGET_DEP_LEVEL0_FOUND FALSE)
    message(STATUS "Could NOT find Level0. Missing includes.")
  else()
    message(STATUS "Level0 include DIR: ${LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS}")
    message(STATUS "Looking for Level0 library.")

    # Check cmake variable first
    if(L0_LIBRARY)
      if(NOT EXISTS ${L0_LIBRARY})
        message(FATAL_ERROR
          "Level0 library specified with L0_LIBRARY variable \
           (${L0_LIBRARY}) does not exist.")
      endif()
      set(LIBOMPTARGET_DEP_LEVEL0_LIBRARIES "${L0_LIBRARY}")
    endif()
    # Search L0 library
    if(WIN32)
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LEVEL0_LIBRARY_NAME level_zero64)
      else()
        set(LEVEL0_LIBRARY_NAME level_zero32)
      endif()
    else()
      set(LEVEL0_LIBRARY_NAME level_zero ze_loader)
    endif()
    find_library(LIBOMPTARGET_DEP_LEVEL0_LIBRARIES
      NAMES
        ${LEVEL0_LIBRARY_NAME}
      PATHS
        ENV LIBOMPTARGET_LEVEL0_ROOT
        ENV LIBRARY_PATH
        ENV LD_LIBRARY_PATH
      PATH_SUFFIXES
        lib/ubuntu_18.04 lib) # TODO: follow up with path changes

    if(NOT LIBOMPTARGET_DEP_LEVEL0_LIBRARIES)
      set(LIBOMPTARGET_DEP_LEVEL0_FOUND FALSE)
      message(STATUS "Could NOT find Level0. Missing library.")
    else()
      message(STATUS "Level0 library: ${LIBOMPTARGET_DEP_LEVEL0_LIBRARIES}")
      set(LIBOMPTARGET_DEP_LEVEL0_FOUND TRUE)
    endif()
  endif()
endif()

mark_as_advanced(
    LIBOMPTARGET_DEP_LEVEL0_FOUND
    LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS
    LIBOMPTARGET_DEP_LEVEL0_LIBRARIES)

# end INTEL_CUSTOMIZATION
