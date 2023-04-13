#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2023 Intel Corporation
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
set(INTEL_SOURCE_FILES_TO_BUILD)
intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
  COMPLEMENT LLVM_OPTIONAL_SOURCES
  Intel_Debug.cpp
  IntelDebugRemoveXDeref.cpp
  )
add_llvm_component_library(LLVMIntel_Debug
  ${INTEL_SOURCE_FILES_TO_BUILD}

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Core
  Support
  )

