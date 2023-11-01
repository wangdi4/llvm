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

# set that name of the main output file as a target name
set(TARGET_NAME device_commands)

find_sources()
calculate_target_sources()

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCES}
  INCLUDE_DIRS
  ${CL_API_HEADERS}
  LINK_LIBS
  cl_sys_utils)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME})
