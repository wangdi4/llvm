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

set(TARGET_NAME ProgramConfiguration)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${VALIDATIONS_ROOT}/SATest
  ${VALIDATIONS_ROOT}/NEAT
  ${VALIDATIONS_ROOT}/Common
  ${VALIDATIONS_ROOT}/../external/tinyxml
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${VALIDATIONS_ROOT}/OCLKernelDataGenerator
  ${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  IProgramConfiguration.h
  OpenCLProgramConfiguration.h
  OpenCLProgramConfiguration.cpp
  DXProgramConfiguration.h
  DXProgramConfiguration.cpp)

target_link_libraries(${TARGET_NAME} OCLKernelDataGenerator)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Backend")
