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

set(TARGET bi_test_type)

set(LLVM_OPTIONAL_SOURCES pipe_tests.cpp)

find_sources()
calculate_target_sources()

set(TARGET_LIBS
  LLVMSupport
  LLVMTargetParser
  test_utils
  intelocl${BUILD_PLATFORM}
  cl_hw_utils
  cl_sys_utils
  )

include_directories(${OCL_SOURCE_DIR}/tests/framework_test_type
                    ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins)

# TODO: remove from runtime unittests
add_ocl_unittest(
  ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
  ${OCL_SOURCE_DIR}/tests/framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES ${TARGET_LIBS})

add_ocl_unittest_artifacts(FILES sycl_amx_builtin.spv)
