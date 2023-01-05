#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021-2022 Intel Corporation
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
set(TARGET_NAME OclBackendUtils)

add_subdirectory(LoopUtils)
add_subdirectory(objdump)

set(TARGET_EXTRA_SOURCE_FILES SystemInfo.cpp)
set(TARGET_EXTRA_INCLUDE_FILES
    exceptions.h ExplicitArgument.h
    ExplicitGlobalMemArgument.h ExplicitBlockLiteralArgument.h SystemInfo.h)

set(TARGET_INCLUDE_FILES
    BackendUtils.h
    BlockUtils.h
    debuggingservicetype.h
    OCLAddressSpace.h
    SerializerCompatibility.h
    TypeConversion.h
    VectorizerUtils.h
    RefcountThreadSafe.h
    Atomics.h
    Logger.h
    ${TARGET_EXTRA_INCLUDE_FILES})

set(TARGET_SOURCE_FILES
    BackendUtils.cpp
    BlockUtils.cpp
    debuggingservicetype.cpp
    SerializerCompatibility.cpp
    TypeConversion.cpp
    VectorizerUtils.cpp
    Atomics.cpp
    ${TARGET_EXTRA_SOURCE_FILES})

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/metadata_api
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}
  ${OCL_BINARY_DIR})

include_directories(${BACKEND_ROOT_DIR}/arch_headers)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_INCLUDE_FILES}
                   ${TARGET_SOURCE_FILES} ${UTILS_ASM_ADD_TO_SOURCES_LIST})

target_link_libraries(${TARGET_NAME} LLVMIntel_DPCPPKernelTransforms LLVMOrcJIT
                      LLVMSupport)

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
