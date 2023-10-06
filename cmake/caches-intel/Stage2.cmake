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

if(NOT DEFINED XMAIN_PROJECTS)
  message(FATAL_ERROR
          "Need to include another cmake cache for debug/release/etc. settings")
endif()

if("${CMAKE_C_COMPILER}" MATCHES ".*/icx$")
  add_cflags("-mllvm -loopopt")
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(LLVM_REQUIRES_RTTI ON CACHE BOOL "")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(LLVM_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
endif()

# SDL options
set(INTEL_SDL_BUILD ON CACHE BOOL "")
set(LLVM_ENABLE_LIBCXX ON CACHE BOOL "")
set(LLVM_STATIC_LINK_CXX_STDLIB ON CACHE BOOL "")
