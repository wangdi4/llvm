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

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system library directories
include(embed.rpath)

# Add OpenMP on top TBB library
if(INCLUDE_MKL)
  message(STATUS "Including MKL library")
  if(WIN32)
    set(ICC_WIN32_PATH $ENV{ICPP_COMPILER12})
    if(NOT DEFINED ICC_WIN32_PATH)
      set(ICC_WIN32_PATH $ENV{ICPP_COMPILER13})
      if(NOT DEFINED ICC_WIN32_PATH)
        message(FATAL_ERROR "ICC 12 or 13 are not installed in the system")
      endif()
    endif()
    set(MKL_DIR "${ICC_WIN32_PATH}/mkl")
    set(ICC_DIR "${ICC_WIN32_PATH}/compiler")
  else(WIN32)
    set(ICC_DIR "/opt/intel/composerxe")
    set(MKL_DIR "${ICC_DIR}/mkl")
  endif(WIN32)
  add_definitions(-D__INCLUDE_MKL__)

  if(ADDR EQUAL 64)
    set(ICC_ARCH_DIR intel64)
    add_definitions(-DMKL_ILP64)
  else(ADDR EQUAL 64)
    set(ICC_ARCH_DIR ia32)
  endif(ADDR EQUAL 64)

  add_subdirectory(common/mkl_kernels)
endif(INCLUDE_MKL)

add_subdirectory(common/builtin_kernels)
add_subdirectory(cpu_device)
add_subdirectory(device_commands)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
endif(NOT WIN32)
