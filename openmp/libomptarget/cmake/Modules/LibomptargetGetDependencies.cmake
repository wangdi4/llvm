#
#//===----------------------------------------------------------------------===//
#//
#//                     The LLVM Compiler Infrastructure
#//
#// This file is dual licensed under the MIT and the University of Illinois Open
#// Source Licenses. See LICENSE.txt for details.
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
# INTEL_COLLAB

################################################################################
# Looking for OpenCL...
################################################################################

# Cmake 3.4.3 cannot find OpenCL library unless an OpenCL SDK is
# installed as root. This workaround is a fallback to let us find the library as
# well as includes for now.
message(STATUS "Looking for OpenCL includes.")
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
  find_library(LIBOMPTARGET_DEP_OPENCL_LIBRARIES
    NAMES OpenCL
    PATHS
      ENV LIBOMPTARGET_OCL_ROOT
      ENV LIBRARY_PATH
      ENV LD_LIBRARY_PATH
    PATH_SUFFIXES
      lib lib64)
  message(STATUS "OpenCL lib: ${LIBOMPTARGET_DEP_OPENCL_LIBRARIES}")

  if (NOT LIBOMPTARGET_DEP_OPENCL_LIBRARIES)
    set(LIBOMPTARGET_DEP_OPENCL_FOUND FALSE)
    message(STATUS "Could NOT find OpenCL. Missing libs.")
  else()
    set(LIBOMPTARGET_DEP_OPENCL_FOUND TRUE)
  endif()

endif()

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
# end INTEL_COLLAB
