#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2021 Intel Corporation
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

set(TARGET_NAME BuiltinLibInfo)

include_directories(BEFORE ".")
include_directories(BEFORE "Driver")
include_directories(BEFORE "Driver/OpenCL")
include_directories(BEFORE "Driver/Shared")

include_directories(${LLVM_INCLUDE_DIRS} ${BACKEND_ROOT_DIR}/reflection_module
                    ${BACKEND_ROOT_DIR}/name_mangling)

set(TARGET_EXTRA_SOURCE_FILES Driver/Volcano/VolcanoOpenclRuntime.cpp
                              Driver/Functions.cpp)
set(TARGET_EXTRA_INCLUDE_FILES Driver/Volcano/VolcanoOpenclRuntime.h
                               Driver/Functions.h)

set(TARGET_INCLUDE_FILES
    RuntimeServices.h VectorizerFunction.h
    Driver/OpenCL/OpenclRuntime.h Driver/Shared/DriverVectorizerFunction.h
    ${TARGET_EXTRA_INCLUDE_FILES})

set(TARGET_SOURCE_FILES
    Driver/OpenCL/OpenclRuntime.cpp
    Driver/Shared/DriverVectorizerFunction.cpp ${TARGET_EXTRA_SOURCE_FILES})

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
  COMPONENTS
  reflection_module)

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_PASSES_FOLDER_NAME})
