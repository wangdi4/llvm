# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2023 Intel Corporation
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
##===----------------------------------------------------------------------===##
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
##===----------------------------------------------------------------------===##
#
# Plugin for Unified Runtime backend
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_UNIFIED_RUNTIME_BUILD)
  libomptarget_say("Building Unified Runtime offloading plugin.")

  add_library(omptarget.rtl.unified_runtime SHARED src/rtl.cpp)

  # Temporarily use SYCL-built headers/loader
  # We expect dedicated common variables for UR headers/loader in the future.
  add_dependencies(omptarget.rtl.unified_runtime
    libomptarget-libiomp-file
    UnifiedRuntimeLoader
    UnifiedRuntime-Headers)

  target_include_directories(omptarget.rtl.unified_runtime PRIVATE
    ${LIBOMPTARGET_INCLUDE_DIR}
    ${LIBOMPTARGET_LLVM_INCLUDE_DIRS}
    ${FETCHCONTENT_BASE_DIR}/unified-runtime-src/include)

  target_link_libraries(omptarget.rtl.unified_runtime PRIVATE
    elf_common
    LLVMSupport
    UnifiedRuntimeLoader
    ${LIBOMP_LIB_FILE})

  if(WIN32)
    # TODO
  else()
    target_link_libraries(omptarget.rtl.unified_runtime PRIVATE
      ${OPENMP_PTHREAD_LIB}
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")
  endif()

  install(TARGETS omptarget.rtl.unified_runtime
          LIBRARY COMPONENT llvm-unified_runtime-ompplugin
          DESTINATION lib
          RUNTIME COMPONENT llvm-unified_runtime-ompplugin
          DESTINATION bin)

  # Report to the parent scope that we are building a plugin for Unified Runtime
  set(LIBOMPTARGET_SYSTEM_TARGETS
    "${LIBOMPTARGET_SYSTEM_TARGETS} spir64-unknown-unknown" PARENT_SCOPE)
endif(LIBOMPTARGET_UNIFIED_RUNTIME_BUILD)
# end INTEL_CUSTOMIZATION
