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
# VEOS : required to control offloading to NEC Aurora.

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

# Try to get the highest Nvidia GPU architecture the system supports
set(LIBOMPTARGET_NVPTX_AUTODETECT_COMPUTE_CAPABILITY TRUE CACHE BOOL
  "Auto detect CUDA Compute Capability if CUDA is detected.")
if (CUDA_FOUND AND LIBOMPTARGET_NVPTX_AUTODETECT_COMPUTE_CAPABILITY)
  cuda_select_nvcc_arch_flags(CUDA_ARCH_FLAGS)
  string(REGEX MATCH "sm_([0-9]+)" CUDA_ARCH_MATCH_OUTPUT ${CUDA_ARCH_FLAGS})
  if (NOT DEFINED CUDA_ARCH_MATCH_OUTPUT OR "${CMAKE_MATCH_1}" LESS 35)
    libomptarget_warning_say("Setting Nvidia GPU architecture support for OpenMP target runtime library to sm_35 by default")
    set(LIBOMPTARGET_DEP_CUDA_ARCH "35")
  else()
    set(LIBOMPTARGET_DEP_CUDA_ARCH "${CMAKE_MATCH_1}")
  endif()
endif()

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
  get_filename_component(CUDA_LIBDIR "${CUDA_cudart_static_LIBRARY}" DIRECTORY)
  find_library(
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
# Looking for VEO...
################################################################################

find_path (
  LIBOMPTARGET_DEP_VEO_INCLUDE_DIR
  NAMES
    ve_offload.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/local/include
    /sw/include
    /opt/nec/ve/veos/include
    ENV CPATH
  PATH_SUFFIXES
    libveo)

find_library (
  LIBOMPTARGET_DEP_VEO_LIBRARIES
  NAMES
    veo
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    /opt/nec/ve/veos/lib64
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

find_library(
  LIBOMPTARGET_DEP_VEOSINFO_LIBRARIES
  NAMES
    veosinfo
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    /opt/nec/ve/veos/lib64
    ENV LIBRARY_PATH
    ENV LD_LIBRARY_PATH)

set(LIBOMPTARGET_DEP_VEO_INCLUDE_DIRS ${LIBOMPTARGET_DEP_VEO_INCLUDE_DIR})
find_package_handle_standard_args(
  LIBOMPTARGET_DEP_VEO
  DEFAULT_MSG
  LIBOMPTARGET_DEP_VEO_LIBRARIES
  LIBOMPTARGET_DEP_VEOSINFO_LIBRARIES
  LIBOMPTARGET_DEP_VEO_INCLUDE_DIRS)

mark_as_advanced(
  LIBOMPTARGET_DEP_VEO_FOUND
  LIBOMPTARGET_DEP_VEO_INCLUDE_DIRS)

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

if (OPENMP_STANDALONE_BUILD)
  # This duplicates code from llvm/cmake/config-ix.cmake
  if( WIN32 AND NOT CYGWIN )
    # We consider Cygwin as another Unix
    set(PURE_WINDOWS 1)
  endif()

  # library checks
  if( NOT PURE_WINDOWS )
    check_library_exists(pthread pthread_create "" HAVE_LIBPTHREAD)
  endif()

  if(HAVE_LIBPTHREAD)
    # We want to find pthreads library and at the moment we do want to
    # have it reported as '-l<lib>' instead of '-pthread'.
    # TODO: switch to -pthread once the rest of the build system can deal with it.
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_HAVE_PTHREAD_ARG Off)
    find_package(Threads REQUIRED)
    set(OPENMP_PTHREAD_LIB ${CMAKE_THREAD_LIBS_INIT})
  endif()
else()
  set(OPENMP_PTHREAD_LIB ${LLVM_PTHREAD_LIB})
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
    PATHS
      ${OpenCL_INCLUDE_DIR}
    NO_DEFAULT_PATH)
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

# end INTEL_COLLAB
