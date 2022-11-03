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

set(CMP_INCLUDE_FILES
    Comparator.h StatisticsCollector.h IComparisonResults.h ComparisonResults.h
    ../Common/dxfloat.h ../Common/Exception.h)

include_directories(
  ${LLVM_INCLUDE_DIRS} ${VALIDATIONS_ROOT}/DataManager ${VALIDATIONS_ROOT}/NEAT
  ${CL_API_HEADERS} ${BACKEND_ROOT_DIR} ${BACKEND_ROOT_DIR}/export)
add_opencl_library(
  Comparator
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  ${CMP_INCLUDE_FILES}
  Comparator.cpp
  ComparisonResults.cpp
  StatisticsCollector.cpp)

add_dependencies(Comparator DataManager NEAT)
