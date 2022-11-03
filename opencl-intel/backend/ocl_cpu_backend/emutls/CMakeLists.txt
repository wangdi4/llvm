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

set(TARGET_NAME emutls)

set(TARGET_SOURCE_FILES emutls.c)

# Compile emutls built-in without debug information unconditionally The use of
# the built-in is always implicit and should be hidden from the user The -g0
# option is only apply for Linux build since there is no equivalent option in
# MSVC compiler on Windows (It's not supported by clang-cl driver neither).
if(NOT WIN32)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g0")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g0")
endif(NOT WIN32)

add_opencl_library(${TARGET_NAME} STATIC ${TARGET_SOURCE_FILES})
