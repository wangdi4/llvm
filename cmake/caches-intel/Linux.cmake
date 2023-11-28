# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
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

# We need gcc for libdev and sycl.
find_program(gcc_exe "gcc" CACHE REQUIRED)
if(libdev IN_LIST enabled_projects)
  execute_process(COMMAND ${gcc_exe} -print-file-name=libgcc.a
    OUTPUT_VARIABLE libgcc_path OUTPUT_STRIP_TRAILING_WHITESPACE)
  cmake_path(NORMAL_PATH libgcc_path)
  cmake_path(GET libgcc_path PARENT_PATH libgcc_path)
  if(NOT EXISTS ${libgcc_path})
    message(FATAL_ERROR "Cannot find libgcc location for ${gcc_exe}")
  endif()
  set(GCC_LIBS "${libgcc_path}" CACHE PATH "")
  set(USRLIB "/usr/lib64" CACHE PATH "")
endif()

if(sycl IN_LIST enabled_projects)
  cmake_path(GET gcc_exe PARENT_PATH gcc_toolchain)
  cmake_path(GET gcc_toolchain PARENT_PATH gcc_toolchain)
  set(SYCL_LIBDEVICE_GCC_TOOLCHAIN ${gcc_toolchain} CACHE PATH "")
endif()

# Compiler options.
if(NOT DEFINED CMAKE_C_COMPILER OR NOT DEFINED CMAKE_CXX_COMPILER)
  find_program(cc_path "icx" REQUIRED)
  set(CMAKE_C_COMPILER ${cc_path} CACHE FILEPATH "")
  find_program(cxx_path "icpx" REQUIRED)
  set(CMAKE_CXX_COMPILER ${cxx_path} CACHE FILEPATH "")
  add_cflags("-fp-model=precise")
endif()

# Linux xdev options
set(TARGET_OS "LINUX" CACHE STRING "")
set(HOST_OS "LINUX" CACHE STRING "")
set(TARGET_LINUX ON CACHE BOOL "")
set(HOST_LINUX ON CACHE BOOL "")
xarch(IL0_PREBUILT_COMPONENTS_PREFIX ${ARCHPROJ} efi2linux ${ARCHVER} linux_qa_release)

# Include the extra libraries for an xisa build (the test is weird to avoid
# triggering the feature filtering tool).
if("${LLVM_INTEL_FEATURES}" MATCHES ".*XISA.*")
  xarch(IL0_PREBUILT_LIBIRC_COMPONENTS_PREFIX ${ARCHPROJ} efi2linux_avx3 ${ARCHVER} linux_qa_release)
endif()

# Misc options
set(LLVM_USE_INTEL_JITEVENTS ON CACHE BOOL "")
set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "")

