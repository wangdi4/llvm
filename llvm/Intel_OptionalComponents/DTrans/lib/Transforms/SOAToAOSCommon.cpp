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

#include "llvm/Support/Error.h"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Offset of memory interface in structure type.
cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff(
    "dtrans-soatoaos-mem-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Memory interface offset in struct/class type"));

SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(const Function &F) {
  StructType *ClassType = getStructTypeOfMethod(F);

  if (!ClassType)
    report_fatal_error(Twine("Cannot extract struct/class type from ") +
                       F.getName() + ".");

  StructType *MemoryInterface = nullptr;
  if (ClassType->getNumElements() > DTransSOAToAOSMemoryInterfaceOff) {
    auto *PMemInt = dyn_cast<PointerType>(
        ClassType->getTypeAtIndex(DTransSOAToAOSMemoryInterfaceOff));
    if (!PMemInt)
      report_fatal_error(
          "Incorrect memory interface specification: type at offset "
          "dtrans-soatoaos-mem-off is not pointer to struct/class.");

    MemoryInterface = dyn_cast<StructType>(PMemInt->getElementType());
    if (!MemoryInterface)
      report_fatal_error(
          "Incorrect memory interface specification: type at offset "
          "dtrans-soatoaos-mem-off is not pointer to struct/class.");
  }

  return SummaryForIdiom(ClassType, MemoryInterface, &F);
}
} // namespace soatoaos
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

