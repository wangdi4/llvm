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

# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
# of generated files

use_eh(TRUE)
use_rtti(FALSE)

# set that name of the main output file as a target name
set(TARGET_NAME clang_compiler)

# Embed RPATH/RUNPATH for libraries which dependencies aren't in the standard
# system library directories
include(embed.rpath)

set(LLVM_TARGET_DEFINITIONS OpenCLLinkOptions.td)
tablegen(LLVM OpenCLLinkOptions.inc -gen-opt-parser-defs)
add_public_tablegen_target(OclLinkOptionsTableGen)

#
# Compilation flags
#
set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -D OCL_DEV_BACKEND_PLUGINS -D OCLFRONTEND_PLUGINS"
)
add_definitions(${LLVM_DEFINITIONS})

#
# Include directories
#
set(INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${CCLANG_DEV_INCLUDE_DIRS}
    ${CL_API_HEADERS}
    ${OCL_SOURCE_DIR}/backend/name_mangling
    ${OCL_SOURCE_DIR}/backend/reflection_module
    ${OCL_SOURCE_DIR}/backend/plugin_manager
    ${OCL_SOURCE_DIR}/backend/ocl_cpu_backend/export
    ${OCL_SOURCE_DIR}/backend/dynamic_lib
    ${OCL_SOURCE_DIR}/backend/utils
    ${OCL_SOURCE_DIR}/externals/khronos/spirv/headers/include
    ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
    ${OCL_SOURCE_DIR}/utils/cache_binary_handler
    ${OCL_SOURCE_DIR}/utils/CLElfLib
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}
    ${OCL_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${OCL_BINARY_DIR}/../llvm-spirv/SPIRV-Headers/include
    ${CCLANG_DEV_INCLUDE_DIRS}/../../../../include)

if(OPENCL_INTREE_BUILD)
  if(LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR)
    list(APPEND INCLUDE_DIRS ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/include
         ${LLVM_EXTERNAL_LLVM_SPIRV_SOURCE_DIR}/lib)
    add_definitions(-DOPENCL_INTREE_BUILD=1)
  endif()
endif()

add_subdirectory(header_linker)

set(TARGET_SOURCES
    clang_device_info.h
    Compile.cpp
    Compile.h
    FrontendDriver.cpp
    FrontendDriver.h
    FrontendResultImpl.h
    GetKernelArgInfo.cpp
    GetKernelArgInfo.h
    Link.cpp
    Link.h
    ParseSPIRV.cpp
    ParseSPIRV.h
    SPIRMaterializer.cpp
    SPIRMaterializer.h
    SPIRVMaterializer.cpp
    SPIRVMaterializer.h)

add_subdirectory(pre_release_header)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCES}
  $<TARGET_OBJECTS:pre_release_header>
  INCLUDE_DIRS
  ${INCLUDE_DIRS}
  COMPONENTS
  CacheBinaryHandler
  cl_logger
  cl_sys_utils
  reflection_module
  LINK_LIBS
  LLVMSPIRVLib)

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} ${COMMON_CLANG})
endif()
add_dependencies(${TARGET_NAME} OclLinkOptionsTableGen)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${FRONTEND_FOLDER_NAME})

if(WIN32)
  add_definitions(-DCCLANG_LIB_NAME="${COMMON_CLANG}.dll"
                  -DDEFAULT_OCL_LIBRARY_DIR="${COMMON_CLANG_BIN_DIR}")
endif(WIN32)
