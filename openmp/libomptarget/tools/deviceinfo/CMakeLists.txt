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
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Build llvm-omp-device-info tool
#
##===----------------------------------------------------------------------===##

libomptarget_say("Building the llvm-omp-device-info tool")

add_openmp_tool(llvm-omp-device-info llvm-omp-device-info.cpp)

llvm_update_compile_flags(llvm-omp-device-info)

target_include_directories(llvm-omp-device-info PRIVATE
  ${LIBOMPTARGET_INCLUDE_DIR}
)

# INTEL_CUSTOMIZATION
# Do not add omp as a dependency since Intel specific OpenMP runtime is being
# used.
target_link_libraries(llvm-omp-device-info PRIVATE
  omptarget
)
# end INTEL_CUSTOMIZATION
