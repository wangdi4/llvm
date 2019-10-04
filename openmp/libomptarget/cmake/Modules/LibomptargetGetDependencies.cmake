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
# libelf : required by some targets to handle the ELF files at runtime.
# libffi : required to launch target kernels given function and argument 
#          pointers.
# CUDA : required to control offloading to NVIDIA GPUs.

include (FindPackageHandleStandardArgs)

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
if (CUDA_TOOLKIT_ROOT_DIR)
  set(LIBOMPTARGET_CUDA_TOOLKIT_ROOT_DIR_PRESET TRUE)
endif()
find_package(CUDA QUIET)

set(LIBOMPTARGET_DEP_CUDA_FOUND ${CUDA_FOUND})
set(LIBOMPTARGET_DEP_CUDA_INCLUDE_DIRS ${CUDA_INCLUDE_DIRS})

mark_as_advanced(
  LIBOMPTARGET_DEP_CUDA_FOUND 
  LIBOMPTARGET_DEP_CUDA_INCLUDE_DIRS)

################################################################################
# Looking for CUDA Driver API... (needed for CUDA plugin)
################################################################################

find_library (
    LIBOMPTARGET_DEP_CUDA_DRIVER_LIBRARIES
  NAMES
    cuda
  PATHS
    /lib64)

# There is a libcuda.so in lib64/stubs that can be used for linking.
if (NOT LIBOMPTARGET_DEP_CUDA_DRIVER_LIBRARIES AND CUDA_FOUND)
  # Since CMake 3.3 FindCUDA.cmake defaults to using static libraries. In this
  # case CUDA_LIBRARIES contains additional linker arguments which breaks
  # get_filename_component below. Fortunately, since that change the module
  # exports CUDA_cudart_static_LIBRARY which points to a single file in the
  # right directory.
  set(cuda_library ${CUDA_LIBRARIES})
  if (DEFINED CUDA_cudart_static_LIBRARY)
    set(cuda_library ${CUDA_cudart_static_LIBRARY})
  endif()
  get_filename_component(CUDA_LIBDIR ${cuda_library} DIRECTORY)
  find_library (
      LIBOMPTARGET_DEP_CUDA_DRIVER_LIBRARIES
    NAMES
      cuda
    HINTS
      "${CUDA_LIBDIR}/stubs")
endif()

find_package_handle_standard_args(
  LIBOMPTARGET_DEP_CUDA_DRIVER
  DEFAULT_MSG
  LIBOMPTARGET_DEP_CUDA_DRIVER_LIBRARIES)

mark_as_advanced(LIBOMPTARGET_DEP_CUDA_DRIVER_LIBRARIES)

################################################################################
# Looking for CUDA libdevice subdirectory
#
# Special case for Debian/Ubuntu to have nvidia-cuda-toolkit work
# out of the box. More info on http://bugs.debian.org/882505
################################################################################

set(LIBOMPTARGET_CUDA_LIBDEVICE_SUBDIR nvvm/libdevice)

# Don't alter CUDA_TOOLKIT_ROOT_DIR if the user specified it, if a value was
# already cached for it, or if it already has libdevice.  Otherwise, on
# Debian/Ubuntu, look where the nvidia-cuda-toolkit package normally installs
# libdevice.
if (NOT LIBOMPTARGET_CUDA_TOOLKIT_ROOT_DIR_PRESET AND
    NOT EXISTS
      "${CUDA_TOOLKIT_ROOT_DIR}/${LIBOMPTARGET_CUDA_LIBDEVICE_SUBDIR}")
  find_program(LSB_RELEASE lsb_release)
  if (LSB_RELEASE)
    execute_process(COMMAND ${LSB_RELEASE} -is
      OUTPUT_VARIABLE LSB_RELEASE_ID
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(candidate_dir /usr/lib/cuda)
    if ((LSB_RELEASE_ID STREQUAL "Debian" OR LSB_RELEASE_ID STREQUAL "Ubuntu")
        AND EXISTS "${candidate_dir}/${LIBOMPTARGET_CUDA_LIBDEVICE_SUBDIR}")
      set(CUDA_TOOLKIT_ROOT_DIR "${candidate_dir}" CACHE PATH
          "Toolkit location." FORCE)
    endif()
  endif()
endif()
# INTEL_COLLAB

################################################################################
# Looking for OpenCL...
################################################################################

# Cmake 3.4.3 cannot find OpenCL library unless an OpenCL SDK is
# installed as root. This workaround is a fallback to let us find the library as
# well as includes for now.
message(STATUS "Looking for OpenCL includes.")
if (INTEL_CUSTOMIZATION)
  # If this command does not find the header files, then next
  # find_path (see below) will look again.
  find_path(LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS
    NAMES
      CL/cl.h OpenCL/cl.h
    PATHS ${OpenCL_INCLUDE_DIR})
endif(INTEL_CUSTOMIZATION)

find_path(LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS
  NAMES
    CL/cl.h OpenCL/cl.h
  PATHS
    ENV LIBOMPTARGET_OCL_ROOT
    ENV CPATH
  PATH_SUFFIXES
    include)
message(STATUS "OpenCL include DIR: ${LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS}")

if (NOT LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS)
  set(LIBOMPTARGET_DEP_OPENCL_FOUND FALSE)
  message(STATUS "Could NOT find OpenCL. Missing includes.")
else()

  message(STATUS "Looking for OpenCL library.")
  if (INTEL_CUSTOMIZATION)
    if (OpenCL_LIBRARY)
      if (NOT EXISTS ${OpenCL_LIBRARY})
        message(FATAL_ERROR
          "OpenCL library specified with OpenCL_LIBRARY variable \
           (${OpenCL_LIBRARY}) does not exist.")
      endif()
      set(LIBOMPTARGET_DEP_OPENCL_LIBRARIES "${OpenCL_LIBRARY}")
      # find_library() below will not run, if the output variable
      # is already set.
    endif()
  endif(INTEL_CUSTOMIZATION)

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

if (INTEL_CUSTOMIZATION)
  # FIXME: for some reason CMake is able to find locally installed
  # OpenCL package (see find_package below), but OPENCL_LIBRARIES
  # is left NOTFOUND. Figure out how to deal with that.
  set(CMAKE_DISABLE_FIND_PACKAGE_OpenCL TRUE)
endif(INTEL_CUSTOMIZATION)

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

# INTEL_CUSTOMIZATION
################################################################################
# Looking for Level0
################################################################################

message(STATUS "Looking for Level0 includes.")

# It depends on OpenCL headers
if(NOT LIBOMPTARGET_DEP_OPENCL_INCLUDE_DIRS)
  set(LIBOMPTARGET_DEP_LEVEL0_FOUND FALSE)
  message(STATUS "Could NOT find OpenCL for Level0. Missing includes.")
else()
  find_path(LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS
    NAMES
      ze_api.h
    PATHS
      ENV LIBOMPTARGET_LEVEL0_ROOT
      ENV CPATH
    PATH_SUFFIXES
      include/level_zero)

  if(NOT LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS)
    set(LIBOMPTARGET_DEP_LEVEL0_FOUND FALSE)
    message(STATUS "Could NOT find Level0. Missing includes.")
  else()
    message(STATUS "Level0 include DIR: ${LIBOMPTARGET_DEP_LEVEL0_INCLUDE_DIRS}")
    message(STATUS "Looking for Level0 library.")

    find_library(LIBOMPTARGET_DEP_LEVEL0_LIBRARIES
      NAMES level_zero
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

# end INTEL_COLLAB
