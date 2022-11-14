# Copyright (C) 2012-2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

if(INCLUDE_MKL)
  add_definitions(-D__INCLUDE_MKL__)
endif(INCLUDE_MKL)

set(TARGET framework_test_type)

include_directories(
  ${OCL_SOURCE_DIR}/framework/export ${OCL_SOURCE_DIR}/framework/Tracing
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

set(IGNORE_FILES
    clBuildCubinProgram|clBuildProgram.cpp|clCreateCudaKernel|clCreateKernelCpuGpu|clCreateQueue
)

find_sources(${IGNORE_FILES})
calculate_target_sources()

set(TARGET_LIBS test_utils)

link_directories(${TBB_LIB_DIR})

if(WIN32)
  set(TARGET_LIBS ${TARGET_LIBS} delayimp)
endif(WIN32)

set(TARGET_LIBS ${TARGET_LIBS} intelocl${BUILD_PLATFORM})

# In test with private memory we allocate 1MB for array. But due to
# vectorization stack size depends on width of SIMD and scalar kernel, our
# compiler can works with maximum width equals to 17. So we have to allocate
# additional stack size equals to 17MB.
set_source_files_properties(cl_device_private_mem_size.cpp
                            PROPERTIES COMPILE_DEFINITIONS STACK_SIZE=17825792)

add_definitions(-DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(INPUT_FILE test.cl)
set(SPIRV_OUTPUT_FILE test.spv)
set(INV_BC_OUTPUT_FILE inv_test.bc)
set(INV_SPIRV_OUTPUT_FILE inv_test.spv)
set(TEST_SPV_FILES ${TARGET}_test_spv)
compile_spv(${INPUT_FILE} ${SPIRV_OUTPUT_FILE} EXTRA_CLANG_FLAGS
            -cl-kernel-arg-info)
compile_spv(
  ${INPUT_FILE}
  ${INV_SPIRV_OUTPUT_FILE}
  EXTRA_CLANG_FLAGS
  -cl-kernel-arg-info
  BC_OUTPUT_FILE
  ${INV_BC_OUTPUT_FILE}
  BUILD_INV)
add_custom_target(${TEST_SPV_FILES} DEPENDS ${SPIRV_OUTPUT_FILE}
                                            ${INV_SPIRV_OUTPUT_FILE})

add_ocl_unittest(${TARGET} SOURCE_FILES ${TARGET_SOURCES} LINK_LIBRARIES
                 ${TARGET_LIBS})

if(WIN32)
  set_target_properties(
    ${TARGET}
    PROPERTIES
      LINK_FLAGS_RELEASE
      "${LINK_FLAGS_RELEASE} tbb12${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12${TBB_BINARIES_POSTFIX}.dll"
      LINK_FLAGS_DEBUG
      "${LINK_FLAGS_DEBUG} tbb12_debug${TBB_BINARIES_POSTFIX}.lib /DELAYLOAD:tbb12_debug${TBB_BINARIES_POSTFIX}.dll"
  )

  # set 17 MB of stack size
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS /STACK:"17825792")
endif(WIN32)

add_ocl_unittest_artifacts(
  FILES
  data/cached_binary_current_version.bin
  data/cached_binary_gold_release_linux.bin
  data/cached_binary_gold_update1_release_linux.bin
  data/cached_binary_gold_update2_release_linux.bin
  data/cached_binary_gold_release_windows.bin
  data/cached_binary_gold_update1_release_windows.bin
  data/cached_binary_gold_update2_release_windows.bin
  data/cached_binary_gold_release_windows32.bin
  data/cached_binary_gold_update1_release_windows32.bin
  data/cached_binary_gold_update2_release_windows32.bin
  data/cached_binary_without_section.bin
  data/cached_binary_with_wrong_version.bin
  data/code_coverage.bc
  data/code_coverage_linux.bin
  data/fp-as-kernel-arg.spv
  data/replace-module-after-optimizer.ll
  data/replace-module-before-optimizer.ll
  data/reqd_num_sub_groups_32.bc
  data/reqd_num_sub_groups_64.bc
  data/spec_const.spv
  data/spirv-friendly-llvm-ir.bc)

add_dependencies(
  ${TARGET} clang_compiler${BUILD_PLATFORM} OclCpuBackEnd${BUILD_PLATFORM}
  cpu_device${BUILD_PLATFORM} ${OCL_RT_TEST_DEPS} ${TEST_SPV_FILES})
