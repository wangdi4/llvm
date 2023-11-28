# Copyright (C) 2012 Intel Corporation
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

# Add opencl cpp header
include_directories(${opencl-cpp-header_SOURCE_DIR}/include)

set(nowarning_source_files
  clBuildOptions.cpp
  opencl_printf_floating_point_test.cpp
  opencl_printf_test.cpp
  TbbSetMaxThreads.cpp)
set(nowarning_flags "-Wno-suggest-override -Wno-cast-qual -Wno-missing-field-initializers")
check_cxx_compiler_flag("${nowarning_flags}" CXX_SUPPORTS_FLAGS)
if(CXX_SUPPORTS_FLAGS)
  set_source_files_properties(${nowarning_source_files} PROPERTIES COMPILE_FLAGS "${nowarning_flags}")
endif()

set_source_files_properties(cpu_bind_test.cpp PROPERTIES
                            INCLUDE_DIRECTORIES ${HWLOC_INCLUDE_DIR})

include_directories(
  ${OCL_SOURCE_DIR}/framework/export ${OCL_SOURCE_DIR}/framework/Tracing
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

set(LLVM_OPTIONAL_SOURCES
  clBuildCubinProgram.cpp
  clBuildProgram.cpp
  clCreateCudaKernel.cpp
  clCreateKernelCpuGpu.cpp
  clCreateQueue.cpp
)

find_sources()
calculate_target_sources()

set(TARGET_LIBS
  LLVMDebugInfoTraceBack
  LLVMMC
  LLVMObject
  LLVMSupport
  LLVMTargetParser
  intelocl${BUILD_PLATFORM}
  cl_hw_utils
  cl_sys_utils
  test_utils
  )

if(WIN32)
  list(APPEND TARGET_LIBS delayimp)
endif(WIN32)

# In test with private memory we allocate 1MB for array. But due to
# vectorization stack size depends on width of SIMD and scalar kernel, our
# compiler can works with maximum width equals to 17. So we have to allocate
# additional stack size equals to 17MB.
set_source_files_properties(cl_device_private_mem_size.cpp
                            PROPERTIES COMPILE_DEFINITIONS STACK_SIZE=17825792)

add_definitions(-DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  -DCXX_COMPILER="${CMAKE_CXX_COMPILER}"
)

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
  # set 17 MB of stack size
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS /STACK:"17825792")
endif(WIN32)

link_target_with_tbb_library(${TARGET})

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
  data/device_global.bc
  data/fp-as-kernel-arg.spv
  data/jit-saveload-cached-binary.bin
  data/replace-module-after-optimizer-1.ll
  data/replace-module-after-optimizer-2.ll
  data/replace-module-before-optimizer-1.ll
  data/replace-module-before-optimizer-2.ll
  data/reqd_num_sub_groups_32.bc
  data/reqd_num_sub_groups_64.bc
  data/spec_const.spv
  data/spirv-friendly-llvm-ir.bc)

add_dependencies(
  ${TARGET} clang_compiler OclCpuBackEnd cpu_device
  ${OCL_RT_TEST_DEPS} ${TEST_SPV_FILES})
