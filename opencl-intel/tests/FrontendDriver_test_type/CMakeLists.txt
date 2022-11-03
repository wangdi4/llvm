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

set(TARGET FrontendDriver_test_type)

add_definitions(${LLVM_DEFINITIONS})

# Add include directories
include_directories(
  ${LLVM_INCLUDE_DIRS}
  # common_clang.h is in CCLANG_DEV_INCLUDE_DIRS
  ${CCLANG_DEV_INCLUDE_DIRS}
  ${OCL_SOURCE_DIR}/FrontendDriver
  ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib)

# Generate SPIR-V input for tests
set(INPUT_FILE data/test.cl)
set(SPIRV_OUTPUT_FILE test.spv)
set(TEST_SPV_FILE ${TARGET}_test_spv)
compile_spv(${INPUT_FILE} ${SPIRV_OUTPUT_FILE} EXTRA_CLANG_FLAGS -cl-std=CL2.0
            -O0)
add_custom_target(${TEST_SPV_FILE} DEPENDS ${SPIRV_OUTPUT_FILE})
add_definitions(-DSPIRV_TEST_FILE="${SPIRV_OUTPUT_FILE}")

# TODO: remove from runtime unittest
add_ocl_unittest(
  ${TARGET}
  SOURCE_FILES
  FrontendDriverFixture.cpp
  FrontendDriver_test_type.cpp
  LINK_LIBRARIES
  clang_compiler${BUILD_PLATFORM}
  test_utils
  LLVMCore
  LLVMSupport)

add_dependencies(${TARGET} ${TEST_SPV_FILE})

add_ocl_unittest_artifacts(
  FILES data/opencl.cts.spir.test_atomic_fn.atomic_and_local_int.bc64)
