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

set(TARGET_NAME PlugInNEAT)
#
# Settings for Linux compilation
#
if(NOT WIN32)
  # Compiler switches that CANNOT be modified during makefile generation
  set(ADD_COMMON_C_FLAGS "-msse3 -mssse3 -fPIC -fdiagnostics-show-option")

  set(ADD_C_FLAGS "${ADD_COMMON_C_FLAGS} -std=gnu99")
  set(ADD_CXX_FLAGS "${ADD_COMMON_C_FLAGS} ")

  set(ADD_C_FLAGS_DEBUG "-O0 -ggdb -D _DEBUG")
  set(ADD_C_FLAGS_RELEASE "-O2 -g0 ")

  # C switches
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}         ${ADD_C_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")

  # C++ switches
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}         ${ADD_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}   ${ADD_C_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE
      "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_C_FLAGS_RELEASE}")
endif(NOT WIN32)

add_opencl_library(${TARGET_NAME} STATIC EXCLUDE_FROM_ALL_BUILD PlugInNEAT.h
                   PlugInNEAT.cpp)
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/name_mangling
  ${VALIDATIONS_ROOT}/Interpreter
  ${CL_API_HEADERS}
  ${VALIDATIONS_ROOT}/InterpreterPluggable
  ${VALIDATIONS_ROOT}/OCLBuiltins
  ${VALIDATIONS_ROOT}/DataManager
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/ImathLibd)

# During the compilation of the LLVMInterpreter the Interpreter.h file is
# copying to the build directory (and then to the install one - but this is
# irrelevant for current problem). This file is then used by IntepreterPluggable
# build. So we need to ensure the LLVMInterpreter is running first

add_dependencies(${TARGET_NAME} LLVMInterpreter)
