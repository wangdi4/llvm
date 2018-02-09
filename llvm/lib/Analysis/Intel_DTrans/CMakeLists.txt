#
#    Copyright (C) 2017 Intel Corporation. All rights reserved.
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

add_llvm_library(LLVMIntel_DTransAnalysis
  DTransUtils.cpp
  DTransAnalysis.cpp
)

add_dependencies(LLVMIntel_DTransAnalysis intrinsics_gen)


