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

# Set up 32-bit build flags.
set(LLVM_BUILD_32_BITS ON CACHE BOOL "" FORCE)
set(HOST_ARCH X86 CACHE STRING "" FORCE)
set(TARGET_ARCH X86 CACHE STRING "" FORCE)

# Use MSVC instead of icx for the compiler
find_program(msvc_compiler "cl" REQUIRED)
set(CMAKE_C_COMPILER ${msvc_compiler} CACHE FILEPATH "")
set(CMAKE_CXX_COMPILER ${msvc_compiler} CACHE FILEPATH "")
set(OPENCL_ASM_COMPILER ${msvc_compiler} CACHE FILEPATH "")

include(${CMAKE_CURRENT_LIST_DIR}/opencl.cmake)
