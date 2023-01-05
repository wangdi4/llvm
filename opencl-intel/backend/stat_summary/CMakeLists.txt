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

add_opencl_executable(
  SumStats
  FileList.cpp
  Stats.cpp
  SumStats.cpp
  INCLUDE_DIRS
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/include/
  ${BACKEND_ROOT_DIR}/metadata_api
  LINK_LIBS
  ${LLVM_MODULE_LIBS})
