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

set(TARGET_NAME NEATChecker)

include_directories(
  ${LLVM_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../../external/tinyxml
  ${VALIDATIONS_ROOT}/DataManager ${VALIDATIONS_ROOT}/NEAT)

link_directories(${OCL_OUTPUT_LIBRARY_DIR})

add_opencl_executable(${TARGET_NAME}
  EXCLUDE_FROM_ALL_BUILD
  main.cpp
  LINK_LIBS
  DataManager
  REFALU
  NEAT
  ImathLibd
  tinyxml_STL
  ${LLVM_MODULE_LIBS})

if(NOT WIN32)
  add_dependencies(${TARGET_NAME} ImathLibd)
endif(NOT WIN32)

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION tests/LITTests)
