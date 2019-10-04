#
#    Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
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

add_llvm_library(LLVMIntel_DTransOpt
  AOSToSOA.cpp
  AnnotatorCleaner.cpp
  DeleteField.cpp
  ReorderFields.cpp
  ResolveTypes.cpp
  EliminateROFieldAccess.cpp
  DynClone.cpp
  DTransOptBase.cpp
  DTransOptBaseTest.cpp
  DTransOptUtils.cpp
  DTransPaddedMalloc.cpp
  MemInitTrimDown.cpp
  PaddedPtrPropagation.cpp
  SOAToAOS.cpp
  SOAToAOSArrays.cpp
  SOAToAOSClassInfo.cpp
  SOAToAOSCommon.cpp
  SOAToAOSEffects.cpp
  SOAToAOSPrepare.cpp
  SOAToAOSStruct.cpp
  Transpose.cpp
  WeakAlign.cpp

  ADDITIONAL_HEADER_DIRS
  ${DTRANS_MAIN_INCLUDE_DIR}/Transforms

  DEPENDS
  intrinsics_gen
  directives_gen
  LLVMIntel_DTransAnalysis
  )
