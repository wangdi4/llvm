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


find_sources()
calculate_target_sources()

include_directories(include ${OCL_SOURCE_DIR}/tests/framework_test_type)

set(FPGA_TEST_TYPE_LINK_LIBS
  LLVMDebugInfoBTF
  LLVMSupport
  LLVMTargetParser
  cl_hw_utils
  cl_sys_utils
  test_utils
  intelocl${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF})

add_ocl_unittest(
  fpga_test_type
  SOURCE_FILES
  ${TARGET_SOURCES}
  ${OCL_SOURCE_DIR}/tests/framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES
  ${FPGA_TEST_TYPE_LINK_LIBS})

link_target_with_tbb_library(fpga_test_type)

add_ocl_unittest_artifacts(
  FILES
  data/atomics64.spv
  data/device_global_constant_initializer.bc
  data/device_global_image1.bc
  data/device_global_image2.bc
  data/internal-ctor-kernel.ioc.obj
  data/internal-ctor-lib.cpp.ioc.obj)

add_dependencies(
  fpga_test_type clang_compiler OclCpuBackEnd cpu_device ${OCL_RT_TEST_DEPS})
