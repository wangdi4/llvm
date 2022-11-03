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

include_directories(
  ${LLVM_INCLUDE_DIRS} ${VALIDATIONS_ROOT}/DataVersion
  ${VALIDATIONS_ROOT}/DataManager ${VALIDATIONS_ROOT}/NEAT ${CL_API_HEADERS})

# Set useful variable for all header files to manipulate them all together.
set(DV_INCLUDE_FILES DataVersion.h)

# Data Version Library
add_opencl_library(DataVersion STATIC EXCLUDE_FROM_ALL_BUILD
                   ${DV_INCLUDE_FILES} DataVersion.cpp)

if(OPENCL_INTREE_BUILD)
  add_dependencies(DataVersion intrinsics_gen)
endif()
