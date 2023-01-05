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

use_eh(TRUE)

set(TARGET_NAME OCLKernelDataGenerator)

# TinyXML library is used for serialization to XML.
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml)
include_directories(${VALIDATIONS_ROOT}/DataManager)
include_directories(${VALIDATIONS_ROOT}/OpenCLKernelArgumentsParser)

# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  OCLKernelDataGenerator.h
  OCLKernelDataGenerator.cpp
  RandomUniformProvider.h
  RandomUniformProvider.cpp
  Generators.cpp
  Generators.h
  GeneratorConfig.cpp
  GeneratorConfig.h
  OCLKernelBufferContainerListAllocator.h
  OCLKernelBufferContainerListAllocator.cpp)

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME}
                      PROPERTIES FOLDER ${BACKEND_VALIDATION_FOLDER_NAME})
