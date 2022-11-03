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

if(NOT OPENCL_INTREE_BUILD)
  if(NOT DEFINED LLVM_PATH_FE)
    message(
      FATAL_ERROR
        "LLVM_PATH_FE is not specified. Please specify LLVM library location for backend using LLVM_PATH_FE parameter to CMAKE"
    )
  endif()
  set(LLVM_FE_INCLUDE_DIRS ${LLVM_PATH_FE}/include)
endif()

#
# Settings for Linux compilation
#
set(TARGET_NAME SATest)
use_rtti(FALSE)

if(WIN32)
  set(ICC_WIN32_PATH $ENV{ICPP_COMPILER16})

  if("${ICC_WIN32_PATH}" STREQUAL "")
    message(
      FATAL_ERROR "ICC Compiler env var is not exported! SATest won't link.")
  endif()

  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ICC_PLATFORM ia32)
  else()
    set(ICC_PLATFORM intel64)
  endif()

  link_directories("${ICC_WIN32_PATH}/compiler/lib/${ICC_PLATFORM}")
endif(WIN32)

link_directories(${OCL_OUTPUT_LIBRARY_DIR})

if(NOT OPENCL_INTREE_BUILD)
  link_directories(${LLVM_LIBRARY_DIRS})
endif()

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${LLVM_ROOT}/lib/ExecutionEngine/Interpreter
  ${BACKEND_ROOT_DIR}/arch_headers
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/external/tinyxml
  ${BACKEND_ROOT_DIR}/name_mangling
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${VALIDATIONS_ROOT}/SATest
  ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/Comparator
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/OCLBuiltins
  ${VALIDATIONS_ROOT}/InterpreterPluggable
  ${VALIDATIONS_ROOT}/PlugInNEAT
  ${VALIDATIONS_ROOT}/OCLBuilder
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${VALIDATIONS_ROOT}/ProgramConfiguration)

if(OPENCL_INTREE_BUILD)
  include_directories(${CCLANG_DEV_INCLUDE_DIRS})
else()
  include_directories(${LLVM_FE_INCLUDE_DIRS}/cclang)
endif()

add_opencl_executable(
  ${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  mem_utils.cpp
  dbg_utils.cpp
  main.cpp
  RunResult.cpp
  Performance.cpp
  PerformancePrinter.cpp
  RunnerFactoryGenerator.cpp
  SATest.cpp
  OpenCLProgram.cpp
  OpenCLReferenceRunner.cpp
  OpenCLRunConfiguration.cpp
  OpenCLBackendWrapper.cpp
  OpenCLComparator.cpp
  OpenCLArgsBuffer.cpp
  OpenCLCPUBackendRunner.cpp
  OpenCLFactory.cpp
  OpenCLBackendRunner.cpp
  ${VALIDATIONS_ROOT}/Common/md5.cpp
  OpenCLStamp.cpp)

target_link_libraries(
  ${TARGET_NAME}
  OpenCLKernelArgumentsParser
  OCLKernelDataGenerator
  OCLBuilder
  PlugInNEAT
  DataManager
  DataVersion
  Comparator
  OclBackendUtils
  OCLBuiltinsRef
  NEAT
  REFALU
  ProgramConfiguration
  dynamic_load
  tinyxml_STL
  ImathLibd
  cl_sys_utils
  ${LLVM_MODULE_LIBS}
  LLVMAsmParser
  LLVMBitReader
  LLVMInterpreter
  LLVMInterpreterPluggable
  LLVMTransformUtils)

if(NOT WIN32)
  add_dependencies(${TARGET_NAME} ImathLibd)
endif(NOT WIN32)
