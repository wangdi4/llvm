# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Adding omptarget tools
#
##===----------------------------------------------------------------------===##

set(OPENMP_TOOLS_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}" CACHE PATH
    "Path for binary subdirectory (defaults to '${CMAKE_INSTALL_BINDIR}')")
mark_as_advanced(OPENMP_TOOLS_INSTALL_DIR)

# Move these macros to AddOpenMP if such a CMake module is ever created.

macro(add_openmp_tool name)
  llvm_add_tool(OPENMP ${ARGV})
endmacro()

macro(add_openmp_tool_symlink name)
  llvm_add_tool_symlink(OPENMP ${ARGV})
endmacro()

add_subdirectory(deviceinfo)
if(INTEL_CUSTOMIZATION)
else(INTEL_CUSTOMIZATION)
add_subdirectory(kernelreplay)
endif(INTEL_CUSTOMIZATION) # INTEL
