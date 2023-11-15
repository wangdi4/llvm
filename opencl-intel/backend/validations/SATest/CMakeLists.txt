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

  if(BUILD_X64)
    set(ICC_PLATFORM intel64)
  else()
    set(ICC_PLATFORM ia32)
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

set(LINK_LIBRARIES "")
if(WIN32)
  list(APPEND LINK_LIBRARIES lldCOFF lldCommon)
endif()

if(HAVE_LIBPTHREAD)
  list(APPEND LINK_LIBRARIES pthread)
endif()

add_opencl_executable(
  ${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  OpenCLArgsBuffer.cpp
  OpenCLBackendRunner.cpp
  OpenCLBackendWrapper.cpp
  OpenCLCPUBackendRunner.cpp
  OpenCLComparator.cpp
  OpenCLFactory.cpp
  OpenCLProgram.cpp
  OpenCLReferenceRunner.cpp
  OpenCLRunConfiguration.cpp
  OpenCLStamp.cpp
  Performance.cpp
  PerformancePrinter.cpp
  RunResult.cpp
  RunnerFactoryGenerator.cpp
  SATest.cpp
  dbg_utils.cpp
  main.cpp
  mem_utils.cpp
  LINK_LIBS
  ${COMMON_CLANG_LIB}
  ${LLVM_MODULE_LIBS}
  BuiltinLibInfo
  CLElfLib
  CacheBinaryHandler
  Comparator
  DataManager
  DataVersion
  ImathLibd
  LLVMAsmParser
  LLVMBinaryFormat
  LLVMDebugInfoBTF
  LLVMDebugInfoDWARF
  LLVMDebugInfoTraceBack
  LLVMInstrumentation
  LLVMInterpreterPluggable
  LLVMIntel_Debug
  LLVMMC
  LLVMMCDisassembler
  LLVMMCParser
  LLVMObject
  LLVMOrcTargetProcess
  LLVMTargetParser
  LLVMVPOTransforms
  LLVMVectorize
  NEAT
  OCLBuilder
  OCLBuiltinsRef
  OCLKernelDataGenerator
  ObjDump
  OclBackendUtils
  OclCpuBackEnd
  OclOptimizer
  OclPluginManager
  OpenCLKernelArgumentsParser
  PlugInNEAT
  ProgramConfiguration
  REFALU
  cl_hw_utils
  cl_logger
  cl_sys_utils
  clang_compiler
  dynamic_load
  emutls
  reflection_module
  soft_math
  tinyxml_STL
  ${LINK_LIBRARIES})

if(NOT WIN32)
  add_dependencies(${TARGET_NAME} ImathLibd)
endif()
