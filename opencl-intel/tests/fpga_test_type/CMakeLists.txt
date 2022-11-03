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

link_directories(${TBB_LIB_DIR})

add_ocl_unittest(
  fpga_test_type
  SOURCE_FILES
  ${TARGET_SOURCES}
  ${OCL_SOURCE_DIR}/tests/framework_test_type/TestsHelpClasses.cpp
  LINK_LIBRARIES
  test_utils
  intelocl${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF})

if(WIN32)
  set_target_properties(
    fpga_test_type
    PROPERTIES
      LINK_FLAGS_RELEASE
      "${LINK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
      LINK_FLAGS_DEBUG
      "${LINK_FLAGS_DEBUG} tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
  )
endif()

add_ocl_unittest_artifacts(
  FILES data/internal-ctor-kernel.ioc.obj data/internal-ctor-lib.cpp.ioc.obj
  data/atomics64.spv)

add_dependencies(
  fpga_test_type clang_compiler${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
  OclCpuBackEnd${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF}
  cpu_device${BUILD_PLATFORM}_${OUTPUT_EMU_SUFF} ${OCL_RT_TEST_DEPS})
