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
# Project variables: OCL_SOURCE_DIR    - points to the top src directory
# OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root
#                     of generated files
#

# set that name of the main output file as a target name
set(TARGET_NAME MemoryAllocator)

find_sources(${IGNORE_FILES})
calculate_target_sources()

# add a target named ${TARGET_NAME}
add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCES}
  LINK_LIBS
  cl_sys_utils
  clbltpipe)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${RUNTIME_FOLDER_NAME})
