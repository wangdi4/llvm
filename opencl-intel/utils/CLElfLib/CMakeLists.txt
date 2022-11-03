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

set(TARGET_NAME CLElfLib)

add_opencl_library(
  ${TARGET_NAME}
  ElfReader.cpp
  ElfWriter.cpp
  INCLUDE_DIRS
  ../cl_sys_utils/export
  INSTALL_PATH
  ${OUTPUT_ARCH_SUFF})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})
