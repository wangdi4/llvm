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

use_eh(TRUE)

set(TARGET_NAME NEAT)

# To be sure that ICC compiler uses the correct presentation of 80-bit long
# doubles we have to set proper compiler options, for win32 only, for linux
# gcc(g++) is used and we don't need to set such options
if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICC_AUXILIARY_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}   ${ICC_AUXILIARY_C_FLAGS}")
endif(WIN32)

include_directories(
  ${LLVM_INCLUDE_DIRS} ${CL_API_HEADERS} ${VALIDATIONS_ROOT}/REFALU
  ${VALIDATIONS_ROOT}/ImathLibd ${VALIDATIONS_ROOT}/Common)

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  EXCLUDE_FROM_ALL_BUILD
  IntervalError.cpp
  NEATALU.cpp
  NEATValue.cpp
  NEATVector.cpp
  NEAT_WRAP.cpp)

set_target_properties(${TARGET_NAME}
                      PROPERTIES FOLDER ${BACKEND_VALIDATION_FOLDER_NAME})
