#
#    Copyright (C) 2018-2022 Intel Corporation. All rights reserved.
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

add_llvm_component_library(LLVMIntel_DTransOpt
  AOSToSOA.cpp
  AOSToSOAOP.cpp
  AnnotatorCleaner.cpp
  ClassInfoOPUtils.cpp
  CodeAlign.cpp
  CodeAlignOP.cpp
  CommuteCond.cpp
  ReuseField.cpp
  ReuseFieldOP.cpp
  DeleteField.cpp
  DeleteFieldOP.cpp
  ReorderFields.cpp
  ReorderFieldsOP.cpp
  ResolveTypes.cpp
  EliminateROFieldAccess.cpp
  DynClone.cpp
  DTransForceInline.cpp
  DTransForceInlineOP.cpp
  DTransNormalizeOP.cpp
  DTransOPOptBase.cpp
  DTransOPOptBaseTest.cpp
  DTransOptBase.cpp
  DTransOptBaseTest.cpp
  DTransOptUtils.cpp
  DTransPaddedMalloc.cpp
  MemInitTrimDown.cpp
  MemInitTrimDownOP.cpp
  MemManageTrans.cpp
  MemManageTransOP.cpp
  PaddedPtrPropagation.cpp
  RemoveDTransTypeMetadata.cpp
  SOAToAOS.cpp
  SOAToAOSArrays.cpp
  SOAToAOSClassInfo.cpp
  SOAToAOSCommon.cpp
  SOAToAOSEffects.cpp
  SOAToAOSInternal.cpp
  SOAToAOSOP.cpp
  SOAToAOSOPArrays.cpp
  SOAToAOSOPClassInfo.cpp
  SOAToAOSOPCommon.cpp
  SOAToAOSOPEffects.cpp
  SOAToAOSOPInternal.cpp
  SOAToAOSOPStruct.cpp
  SOAToAOSPrepare.cpp
  SOAToAOSOPPrepare.cpp
  SOAToAOSStruct.cpp
  Transpose.cpp
  WeakAlign.cpp

  ADDITIONAL_HEADER_DIRS
  ${DTRANS_MAIN_INCLUDE_DIR}/Transforms

  DEPENDS
  intrinsics_gen
  directives_gen
  LLVMIntel_DTransAnalysis

  # FIXME: figure out why LINK_COMPONENTS does not link the libraries,
  #        and remove this LINK_LIBS.
  LINK_LIBS
  LLVMAnalysis
  LLVMCore
  LLVMIntel_DTransAnalysis
  LLVMSupport
  LLVMTransformUtils

  LINK_COMPONENTS
  Analysis
  Core
  Intel_DTransAnalysis
  Support
  TransformUtils
  )
