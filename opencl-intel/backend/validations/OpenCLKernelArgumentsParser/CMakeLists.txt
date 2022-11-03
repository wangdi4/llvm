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

include_directories(${LLVM_INCLUDE_DIRS} ${VALIDATIONS_ROOT}/DataManager)
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_opencl_library(
  OpenCLKernelArgumentsParser STATIC EXCLUDE_FROM_ALL_BUILD
  IOpenCLKernelArgumentsParser.h OpenCLKernelArgumentsParser.h
  OpenCLKernelArgumentsParser.cpp)

if(OPENCL_INTREE_BUILD)
  add_dependencies(OpenCLKernelArgumentsParser intrinsics_gen)
endif()

set_target_properties(OpenCLKernelArgumentsParser PROPERTIES FOLDER "Backend")
