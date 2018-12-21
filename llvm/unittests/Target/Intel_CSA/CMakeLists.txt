#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#

include_directories(
  ${CMAKE_SOURCE_DIR}/lib/Target/Intel_CSA
  ${CMAKE_BINARY_DIR}/lib/Target/Intel_CSA
  )

set(LLVM_LINK_COMPONENTS
  CodeGen
  Core
  CSACodeGen
  CSADesc
  CSAInfo
  MC
  MIRParser
  SelectionDAG
  Support
  Target
  )

add_llvm_unittest(CSATests
  CSAOpInfo.cpp
  )
