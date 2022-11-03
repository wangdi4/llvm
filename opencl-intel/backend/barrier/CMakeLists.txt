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
#

set(TARGET_NAME LLVMBarrier)

file(GLOB_RECURSE TARGET_INCLUDE_FILES *.h)

include_directories(BEFORE "include")
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/utils
  ${CMAKE_BINARY_DIR}/include/)

set(TARGET_SOURCE_FILES BarrierMain.cpp)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_SOURCE_FILES}
                   ${TARGET_INCLUDE_FILES})

add_dependencies(${TARGET_NAME} LoopUtils)

set(BACKEND_PASSES_FOLDER_NAME ${BACKEND_FOLDER_NAME}/passes)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_PASSES_FOLDER_NAME})
