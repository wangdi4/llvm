#
#    Copyright (C) 2018 Intel Corporation. All rights reserved.
#
#    The information and source code contained herein is the exclusive
#    property of Intel Corporation. and may not be disclosed, examined
#    or reproduced in whole or in part without explicit written authorization
#    from the company.
#
#    Source file: CMakeLists.txt
#    ------------
#    Configures build for project.
#

add_llvm_component_library(LLVMIntel_CSAScalarOpt
  Scalar.cpp
  LoopSPMDization.cpp
  CSALowerParallelIntrinsics.cpp
  CSAGraphSplitter.cpp

  ADDITIONAL_HEADER_DIRS
  ${CSA_MAIN_INCLUDE_DIR}/Transforms/Scalar

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  Core
  Support
  TransformUtils
  )
