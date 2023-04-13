# INTEL_CUSTOMIZATION
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
# end INTEL_CUSTOMIZATION

set(LLVM_LINK_COMPONENTS
  Support
  TargetParser
  )

add_llvm_unittest(TargetParserTests
  CSKYTargetParserTest.cpp
  Host.cpp
  TargetParserTest.cpp
# INTEL_CUSTOMIZATION
  X86TargetParserTest.cpp
#end INTEL_CUSTOMIZATION
  TripleTest.cpp
  )

target_link_libraries(TargetParserTests PRIVATE LLVMTestingSupport)
