# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

use_rtti( FALSE )
set( TARGET_NAME LLVMVectorizer)

set(VECTORIZATION_INFO_GEN ${OCL_BINARY_DIR}/include/VectInfo.gen)
file(MAKE_DIRECTORY ${OCL_BINARY_DIR}/include)
add_custom_command(
  OUTPUT ${VECTORIZATION_INFO_GEN}
  COMMAND ocl-tblgen -gen-vectorization-info
  -I ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins
  ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/source/misc_main.td
  -o ${OCL_BINARY_DIR}/include/VectInfo.gen
  DEPENDS ocl-tblgen ${OCL_ALL_TD_FILES}
  COMMENT "Generating built-in vectorization info ..."
  VERBATIM
)

add_custom_target(VectorizationInfoGen ALL
  DEPENDS ${VECTORIZATION_INFO_GEN}
  VERBATIM
)

include_directories(BEFORE "Utils")

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/reflection_module
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/utils/LoopUtils
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/passes
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo
  ${BACKEND_ROOT_DIR}/passes/BuiltinLibInfo/Driver/OpenCL
  ${BACKEND_ROOT_DIR}/metadata_api
  ${CMAKE_BINARY_DIR}/include
  ${OCL_BINARY_DIR}/include
)

set(TARGET_INCLUDE_FILES
  Utils/VecConfig.h
  Utils/VectorizerCommon.h
)

set(TARGET_SOURCE_FILES
  Utils/VecConfig.cpp
  Utils/VectorizerCommon.cpp
)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
)

add_dependencies(${TARGET_NAME} LoopUtils OclBackendUtils VectorizationInfoGen)

set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

if (NOT OPENCL_INTREE_BUILD)
  set_property( GLOBAL APPEND PROPERTY LLVM_LIBS ${TARGET_NAME} )
endif()
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
