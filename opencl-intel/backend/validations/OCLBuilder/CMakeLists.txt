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

set(TARGET_NAME OCLBuilder)

project(${TARGET_NAME})

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  OCLBuilder.h
  OCLBuilder.cpp
  INCLUDE_DIRS
  ${BACKEND_ROOT_DIR}/dynamic_lib
  ${BACKEND_ROOT_DIR}/ocl_cpu_backend/export
  ${BACKEND_ROOT_DIR}/plugin_manager
  ${CL_API_HEADERS}
  ${OCL_SOURCE_DIR}/cl_api
  ${OCL_SOURCE_DIR}/FrontendDriver
  ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER
                                                ${BACKEND_FOLDER_NAME}/plugins)
