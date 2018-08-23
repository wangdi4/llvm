//===---------------- SOAToAOSCommon.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality common to both arrays structures
// and structure containing arrays for SOA-to-AOS.
//
//===----------------------------------------------------------------------===//
#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSCommon.h"
#include "SOAToAOSEffects.h"

namespace llvm {
namespace dtrans {
namespace soatoaos {
SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F) {
  StructType *ClassType = getStructTypeOfArray(F);
  SummaryForIdiom Failure(nullptr, nullptr, nullptr);
  if (!ClassType)
    return Failure;

  StructType *MemoryInterface = nullptr;
  if (ClassType->getNumElements() > DTransSOAToAOSMemoryInterfaceOff) {
    auto *PMemInt = dyn_cast<PointerType>(
        ClassType->getTypeAtIndex(DTransSOAToAOSMemoryInterfaceOff));
    if (!PMemInt)
      return Failure;
    MemoryInterface = dyn_cast<StructType>(PMemInt->getPointerElementType());
  }

  return SummaryForIdiom(ClassType, MemoryInterface, &F);
}
} // namespace soatoaos
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

