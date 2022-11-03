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

set(TARGET_NAME SmartGVN)

set(TARGET_INCLUDE_FILES SmartGVN.h)

set(TARGET_SOURCE_FILES SmartGVN.cpp)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_INCLUDE_FILES}
                   ${TARGET_SOURCE_FILES})

if(OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_PASSES_FOLDER_NAME})
