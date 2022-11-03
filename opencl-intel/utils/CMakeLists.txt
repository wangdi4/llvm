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

if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED LLVM_PATH_FE)
    message(
      FATAL_ERROR
        "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE"
    )
  endif()
  set(LLVM_PATH ${LLVM_PATH_FE})
  find_package(LLVM REQUIRED)
endif()

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
endif(NOT WIN32)

add_definitions(-DCLANG_VERSION_STRING="${CLANG_VERSION}")

add_subdirectory(cl_hw_utils)
add_subdirectory(cl_logger)
add_subdirectory(cl_sys_utils)
add_subdirectory(CLElfLib)
add_subdirectory(cache_binary_handler)

if(WIN32)
  add_subdirectory(dyn_cl_sys_utils)
  add_subdirectory(dyn_cl_hw_utils)
endif(WIN32)

add_subdirectory(task_executor)
