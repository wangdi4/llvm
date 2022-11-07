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

set(TARGET_NAME OclOptimizer)

set(TARGET_INCLUDE_FILES Optimizer.h OptimizerLTO.h)

set(TARGET_SOURCE_FILES Optimizer.cpp OptimizerOCL.cpp OptimizerLTO.cpp)

include_directories(BEFORE ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib/SPIRV
                    ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib/SPIRV/Mangler)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/barrier/include
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/vectorizer/Utils
  ${BACKEND_ROOT_DIR}/metadata_api
  ${CMAKE_BINARY_DIR}/include/
  ${OCL_BINARY_DIR}/include
  ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/include)

include_directories(
  ${BACKEND_ROOT_DIR}/arch_headers ${BACKEND_ROOT_DIR}/passes/PrintIRPass
  ${BACKEND_ROOT_DIR}/passes/)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_INCLUDE_FILES}
                   ${TARGET_SOURCE_FILES})

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen directives_gen LLVMSPIRVLib
                   VectorizationInfoGen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
