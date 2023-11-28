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

add_llvm_component_library(LLVMIntel_DTrans
  DTransCommon.cpp

  ADDITIONAL_HEADER_DIRS
  ${DTRANS_MAIN_INCLUDE_DIR}/Transforms

  DEPENDS
  intrinsics_gen
  LLVMIntel_DTransOpt
  LLVMIntel_DTransAnalysis

  # FIXME: figure out why LINK_COMPONENTS does not link the libraries,
  #        and remove this LINK_LIBS.
  LINK_LIBS
  LLVMCore
  LLVMIntel_DTransAnalysis
  LLVMIntel_DTransOpt
  LLVMIRPrinter
  LLVMSupport

  LINK_COMPONENTS
  Core
  Intel_DTransAnalysis
  Intel_DTransOpt
  Support
  )

add_subdirectory(Analysis)
add_subdirectory(Transforms)
