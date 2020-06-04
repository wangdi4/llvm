#
#    Copyright (C) 2018-2020 Intel Corporation. All rights reserved.
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

add_llvm_component_library(LLVMIntel_DTransAnalysis
  DTransUtils.cpp
  DTransAllocAnalyzer.cpp
  DTransAnalysis.cpp
  DTransAnnotator.cpp
  DTransFieldModRef.cpp
  DTransImmutableAnalysis.cpp
  DTransSafetyAnalyzer.cpp
  DTransTypes.cpp
  PtrTypeAnalyzer.cpp
  PtrTypeAnalyzerTest.cpp
  TypeMetadataReader.cpp

  ADDITIONAL_HEADER_DIRS
  ${DTRANS_MAIN_INCLUDE_DIR}/Analysis

  DEPENDS
  intrinsics_gen
)
