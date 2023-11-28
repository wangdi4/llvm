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

use_eh(TRUE)

set(TARGET_NAME ValidationTests)

# to be shure that ICC compiler uses the correct presentation of 80-bit long
# doubles we have to set proper compiler options, for win32 only, for linux
# gcc(g++) is used and we dont need to set such options
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICC_AUXILIARY_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${ICC_AUXILIARY_C_FLAGS}")
endif(WIN32)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CL_API_HEADERS}
  ${BACKEND_ROOT_DIR}/external/tinyxml
  ${BACKEND_ROOT_DIR}/plugin_manager
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/Comparator
  ${VALIDATIONS_ROOT}/ImathLibd
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/DataGenerator
  ${VALIDATIONS_ROOT}/OCLBuiltins
  ${VALIDATIONS_ROOT}/PlugInNEAT
  ${VALIDATIONS_ROOT}/OCLBuilder
  ${VALIDATIONS_ROOT}/OCLRecorder
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser
  ${CMAKE_CURRENT_SOURCE_DIR}/Common
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export
  ${OCL_SOURCE_DIR}/tests/test_utils/export)

if(OPENCL_INTREE_BUILD)
  include_directories(${CCLANG_DEV_INCLUDE_DIRS})
else()
  include_directories(${LLVM_FE_INCLUDE_DIRS}/cclang)
endif()

link_directories(${OCL_OUTPUT_LIBRARY_DIR})

if(HAVE_LIBPTHREAD)
  set(LINK_LIBRARIES pthread)
endif()

add_opencl_executable(
  ${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  DVTest.cpp
  DMTest.cpp
  DMImageTest.cpp
  CMPTest.cpp
  OpenCLKernelArgumentsParserTest.cpp
  DGTest.cpp
  Float16Test.cpp
  NEATALUUtils.cpp
  NEATALUTest.cpp
  NEATALUCommonTest.cpp
  NEATALUConversionTest.cpp
  NEATALUVLoadStoreTest.cpp
  NEATValueTest.cpp
  NEATMathTest.cpp
  OCLBuiltinParserTest.cpp
  NEATALUImageTest.cpp
  SourceRecorder.cpp
  ClangSA.cpp
  OCLKernelDataGeneratorTest.cpp
  LINK_LIBS
  ${COMMON_CLANG_LIB}
  ${LLVM_MODULE_LIBS}
  CLElfLib
  CacheBinaryHandler
  Comparator
  DataGenerator
  DataManager
  ImathLibd
  LLVMAsmParser
  LLVMInterpreterPluggable
  LLVMTargetParser
  NEAT
  OCLBuilder
  OCLBuiltinsRef
  OCLKernelDataGenerator
  OCLRecorderStat
  OclPluginManager
  OpenCLKernelArgumentsParser
  PlugInNEAT
  REFALU
  cl_hw_utils
  cl_logger
  cl_sys_utils
  clang_compiler
  dynamic_load
  llvm_gtest
  reflection_module
  tinyxml_STL
  ${LINK_LIBRARIES}
  )

add_dependencies(${TARGET_NAME} cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

add_subdirectory(fe_plugin_mock)
