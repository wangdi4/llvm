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
set(LLVM_LINK_COMPONENTS
    DebugInfoDWARF
    MC
    MCParser
    Object
    Support
    Symbolize
    )

add_llvm_library(LLVMCFIVerify
  STATIC
  FileAnalysis.cpp
  FileAnalysis.h
  GraphBuilder.cpp
  GraphBuilder.h
  )

# INTEL_CUSTOMIZATION
set_msvc_crt_flags(LLVMCFIVerify)
# end INTEL_CUSTOMIZATION
