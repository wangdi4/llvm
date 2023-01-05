# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2022 Intel Corporation
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
set(LLVM_LINK_COMPONENTS
  Object
  Support
  )
add_llvm_tool(llvm-debuginfod-find
  llvm-debuginfod-find.cpp
  )
# INTEL_CUSTOMIZATION
target_link_libraries(llvm-debuginfod-find PRIVATE LLVMDebuginfod LLVMSymbolize)
# end INTEL_CUSTOMIZATION
if(LLVM_INSTALL_BINUTILS_SYMLINKS)
  add_llvm_tool_symlink(debuginfod-find llvm-debuginfod-find)
endif()
