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

# Disable libclc by default for this build.
if(DEFINED XMAIN_DISABLE_PROJECTS)
  set(XMAIN_DISABLE_PROJECTS "${XMAIN_DISABLE_PROJECTS};libclc" CACHE STRING "" FORCE)
else()
  set(XMAIN_DISABLE_PROJECTS libclc CACHE STRING "")
endif()
include(${CMAKE_CURRENT_LIST_DIR}/xmain.cmake)

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")
if(DEFINED LLVM_INTEL_FEATURES)
  set(LLVM_INTEL_FEATURES "${LLVM_INTEL_FEATURES};INTEL_INTERNAL_BUILD" CACHE STRING "" FORCE)
else()
  set(LLVM_INTEL_FEATURES "INTEL_INTERNAL_BUILD" CACHE STRING "")
endif()
set(LLVM_ENABLE_WERROR ON CACHE BOOL "")
