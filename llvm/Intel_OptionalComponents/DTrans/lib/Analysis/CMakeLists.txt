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

add_llvm_component_library(LLVMIntel_DTransAnalysis
  DTransAllocCollector.cpp
  DTransAnnotator.cpp
  DTransBadCastingAnalyzerOP.cpp
  DTransFieldModRef.cpp
  DTransImmutableAnalysis.cpp
  DTransLibraryInfo.cpp
  DTransOPUtils.cpp
  DTransRelatedTypesUtils.cpp
  DTransSafetyAnalyzer.cpp
  DTransTypeMetadataBuilder.cpp
  DTransTypeMetadataPropagator.cpp
  DTransTypes.cpp
  DTransUtils.cpp
  MemoryBuiltinsExtras.cpp
  PtrTypeAnalyzer.cpp
  PtrTypeAnalyzerTest.cpp
  TypeMetadataReader.cpp

  ADDITIONAL_HEADER_DIRS
  ${DTRANS_MAIN_INCLUDE_DIR}/Analysis

  DEPENDS
  intrinsics_gen

  # FIXME: figure out why LINK_COMPONENTS does not link the libraries,
  #        and remove this LINK_LIBS.
  LINK_LIBS
  LLVMAnalysis
  LLVMCore
  LLVMObject
  LLVMProfileData
  LLVMSupport

  LINK_COMPONENTS
  Analysis
  Core
  Object
  ProfileData
  Support
)
