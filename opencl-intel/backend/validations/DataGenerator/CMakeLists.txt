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

set(DG_INCLUDE_FILES DataGenerator.h DGHelper.h)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${VALIDATIONS_ROOT}/DataManager)
# NEAT library to (de)serialize NEAT data.
include_directories(${VALIDATIONS_ROOT}/NEAT)

add_opencl_library(DataGenerator STATIC EXCLUDE_FROM_ALL_BUILD
                   ${DG_INCLUDE_FILES} DataGenerator.cpp DGHelper.cpp)
