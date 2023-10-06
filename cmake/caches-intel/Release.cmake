# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

include(${CMAKE_CURRENT_LIST_DIR}/xmain.cmake)

add_cflags("-DINTEL_PRODUCT_RELEASE=1 -DHIDE_BUILD_STAMP=1")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
set(INTEL_SDL_BUILD ON CACHE BOOL "")

if("$ENV{ICS_USAGE_TYPE}" STREQUAL qa)
  add_cflags("-DBUILD_DATE_STAMP=$ENV{BUILD_DATE}")
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(LLVM_ENABLE_WERROR ON CACHE BOOL "")
endif()
