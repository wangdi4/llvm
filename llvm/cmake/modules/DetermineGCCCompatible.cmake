# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
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
#
# end INTEL_CUSTOMIZATION
# Determine if the compiler has GCC-compatible command-line syntax.

if(NOT DEFINED LLVM_COMPILER_IS_GCC_COMPATIBLE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(LLVM_COMPILER_IS_GCC_COMPATIBLE ON)
  elseif( MSVC )
    set(LLVM_COMPILER_IS_GCC_COMPATIBLE OFF)
  elseif( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
    set(LLVM_COMPILER_IS_GCC_COMPATIBLE ON)
# start INTEL_CUSTOMIZATION
  elseif( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel" AND NOT WIN32 )
    set(LLVM_COMPILER_IS_GCC_COMPATIBLE ON)
# end INTEL_CUSTOMIZATION
  endif()
endif()
