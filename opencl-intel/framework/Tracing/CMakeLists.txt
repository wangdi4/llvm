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

set(TARGET_NAME Tracing)

set(TARGET_INCLUDE_FILES tracing_api.h tracing_handle.h tracing_notify.h
                         tracing_types.h)

set(TARGET_SOURCE_FILES tracing_api.cpp)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_SOURCE_FILES}
                   ${TARGET_INCLUDE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME})
