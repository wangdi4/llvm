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

set(TARGET_NAME Platform)

set(TARGET_INCLUDE_FILES Device.h fe_compiler.h iplatform.h observer.h
                         platform_module.h)

set(TARGET_SOURCE_FILES Device.cpp fe_compiler.cpp platform_module.cpp
                        ${TARGET_SOURCES})

include_directories(${CCLANG_DEV_INCLUDE_DIRS})

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
  ${TARGET_INCLUDE_FILES}
  LINK_LIBS
  cl_sys_utils
  cpu_device)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME})
