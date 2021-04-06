# *********************** WARNING *********************
# This is a prototype implementation that tries to reduce the coupling with ICS.
# It's not formally supported by the T&I team and might not be up-to-date. Use
# at your own risk!

set(CMAKE_COLOR_MAKEFILE OFF CACHE BOOL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "")

set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "")
set(LLVM_ENABLE_LIBPFM OFF CACHE BOOL "")
set(LLVM_ENABLE_WERROR ON CACHE BOOL "")
set(LLVM_ENABLE_DUMP ON CACHE BOOL "")
set(LLVM_ENABLE_ASSERTIONS ON CACHE BOOL "")

set(CMAKE_BUILD_TYPE Release CACHE STRING "")

set(LLVM_TARGETS_TO_BUILD X86 CACHE STRING "")
SET(TARGET_ARCH EFI2 CACHE STRING "")
set(TARGET_OS LINUX CACHE STRING "")
set(TARGET_LINUX ON CACHE BOOL "")
SET(HOST_ARCH EFI2 CACHE STRING "")
set(HOST_OS LINUX CACHE STRING "")
set(HOST_LINUX ON CACHE BOOL "")

set(LLVM_USE_INTEL_JITEVENTS ON CACHE BOOL "")
set(INTEL_INCLUDE_DTRANS ON CACHE BOOL "")
set(BUILD_FORT ON CACHE BOOL "")
set(LRB_SUPPORT DISABLED CACHE STRING "")
set(BUILD_SV NO CACHE STRING "")
set(SYCL_ENABLE_XPTI_TRACING ON CACHE STRING "")
set(UWD_BUILD ON CACHE BOOL "")
set(INTEL_CMAKE_DEPLOY ON CACHE STRING "")

set(CMAKE_SYCL_USE_LEVEL0 ON CACHE BOOL "")
set(INTEL_CMAKE_DEPLOY ON CACHE BOOL "")

set(LLVM_BUILD_RUNTIME OFF CACHE BOOL "")
set(LLVM_BUILD_OPENMP ON CACHE BOOL "")
set(LLVM_ENABLE_PROJECTS "clang;lld;libdevice;openmp" CACHE STRING "")
set(LLVM_INTEL_FEATURES "INTEL_INTERNAL_BUILD" CACHE STRING "")

# Is it really needed?
set(USER $ENV{USER} CACHE STRING "")
cmake_host_system_information(RESULT SYSTEM QUERY HOSTNAME)

if(NOT DEFINED ENV{ICS_WSDIR})
  message(FATAL_ERROR "ICS_WSDIR not defined, cannot setup all required paths!")
endif()

set(DIR $ENV{ICS_WSDIR} CACHE PATH "")

set(LLVM_EXTERNAL_XDEV_SOURCE_DIR $ENV{ICS_WSDIR}/xdev CACHE PATH "")
set(LLVM_EXTERNAL_SYCL_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/sycl CACHE PATH "")
set(LLVM_EXTERNAL_OPENCL_AOT_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/opencl-aot CACHE PATH "")
set(LLVM_EXTERNAL_AOCL_IOC64_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/aocl-ioc64 CACHE PATH "")
set(LLVM_EXTERNAL_LIBDEVICE_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/libdevice CACHE PATH "")
set(LLVM_EXTERNAL_XPTI_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/xpti CACHE PATH "")
set(LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/llvm-spirv CACHE PATH "")

# Variables to enable OpenCL.
# TODO: Investigate how that grouping is done in the ICS and port directly. I'd
# rather have a higher-level options for this cache to enable things. On the
# othe hand, it might be fine to always setup these variables and only rule by
# the ENABLE_*_PROJECTS.
set(LLVM_EXTERNAL_OPENCL_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/opencl CACHE PATH "")
set(LLVM_EXTERNAL_OCL_CCLANG_SOURCE_DIR $ENV{ICS_WSDIR}/llvm/ocl-cclang CACHE PATH "")
set(LLVM_PATH_FE ${CMAKE_BINARY_DIR} CACHE PATH "")
set(LLVM_PATH_BE ${CMAKE_BINARY_DIR} CACHE PATH "")

set(OpenCL_INCLUDE_DIR $ENV{ICS_WSDIR}/llvm/external/opencl-headers CACHE PATH "")

# TODO: Remove projects from "-bldocl"?
set(OPENCL_INTREE_BUILD ON CACHE BOOL "")
set(BUILD_X64 ON CACHE BOOL "")
set(LLVM_EXTERNAL_PROJECTS "llvm-spirv;xdev;ocl-cclang;opencl;sycl;opencl-aot;aocl-ioc64;libdevice;xpti" CACHE STRING "")

# Note, this might not be guaranteed to work, but seems like the best thing we
# can do. See also https://cmake.org/pipermail/cmake/2010-December/041134.html.
set(CMAKE_INSTALL_PREFIX $ENV{ICS_WSDIR}/deploy/linux_prod CACHE PATH "")
set(IL0_PREBUILT_COMPONENTS_STRUCTURE install CACHE STRING "")

# FIXME: This won't trigger cmake reconfiguration if this file is changed.
# Probably should be located somewhere else (like one of CMakeLists.txt files)
# and not working on CACHE.

# The issue being addressed here is that if icsconfig files are changed in the
# workspace but the only commands executed in the shell are "ics update" and
# "ics build", then such changes won't be propagated to the environment. As
# such, we try to check for such a situation and detect if any mismatches exist
# between the current ENV and the real latest values.
#
# So, two scenarios possible:
#   - icsconfig was updated and there happened a command updating the environment.
#     In this case will emit an error to ensure this file is updated as well.
#   - This file has been updated as per previous bullet, but the environment
#   - still contains the old data. We'll be able to notify the user to update
#   - it.
if($ENV{L0LOADERVER} MATCHES "1.2.3")
else()
  message(FATAL_ERROR "Unexpected value of L0LOADERVER environment variable!")
endif()

if($ENV{L0LOADERROOT} MATCHES "/rdrive/ref/opencl/runtime/linux/level_zero_loader")
else()
  message(FATAL_ERROR "Unexpected value of L0LOADERROOT environment variable!")
endif()

set(L0_INCLUDE_DIR "$ENV{L0LOADERROOT}/$ENV{L0LOADERVER}/include" CACHE PATH "" FORCE)
set(L0_LIBRARY "$ENV{L0LOADERROOT}/$ENV{L0LOADERVER}/libze_loader.so" CACHE FILEPATH "" FORCE)

if(${HACKY_OPENCL_FOR_OMP})
  # LibOmptargetGetDependencies needs this on the CMake configuration stage to
  # exist, but the real read happens only during the build.
  #
  # Use this path to create the fake file to be able to do CMake configuration
  # without providing existing library.
  #
  # WARNING: The user must ensure that "ocl-icd" target is built before the
  # actual access happens inside the omptarget build because we're tricking the
  # build system here! Also, once the target is built the library needs to be
  # *manually* copied into tools/opencl-aot/libOpenCL.so and
  # tools/aocl-ioc64/libOpenCL.so.
  #
  # That is a lesser problem for the Ninja generator because some dependencies
  # aren't specified properly for it anyway - one has to explicitly build
  # "libomptarget-libiomp-file ocl-icd" targets first which partially performs
  # the action described above.
  #
  # To summarize, the usage scenario is like this:
  #
  #   $ cmake -DHACKY_OPENCL_FOR_OMP=1 <path-to-this-file> -G Ninja \
  #        -DLLVM_BINUTILS_INCDIR=<...> -DIL0_PREBUILT_COMPONENTS_PREFIX=<...> \
  #        <path-to-llvm>
  #   $ ninja libomptarget-libiomp-file ocl-icd
  #   $ cp lib/libOpenCL.so tools/opencl-aot/libOpenCL.so
  #   $ cp lib/libOpenCL.so tools/aocl-ioc64/libOpenCL.so
  #

  get_filename_component(OpenCL_LIBRARY lib/libOpenCL.so ABSOLUTE BASE_DIR "${CMAKE_BINARY_DIR}" CACHE)
  get_filename_component(OPENCL_LIBRARY_DIR ${OpenCL_LIBRARY} DIRECTORY CACHE)
  if(NOT EXISTS ${OpenCL_LIBRARY})
    file(WRITE ${OpenCL_LIBRARY} "fake")
  endif()
endif()
