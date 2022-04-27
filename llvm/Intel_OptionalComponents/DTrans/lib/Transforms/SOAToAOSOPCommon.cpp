//===-------------- SOAToAOSOPCommon.cpp - Part of SOAToAOSOPPass ---------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality common to both arrays structures
// and structure containing arrays for SOA-to-AOS-OP.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

#include "SOAToAOSOPCommon.h"
#include "SOAToAOSOPEffects.h"
#include "SOAToAOSOPInternal.h"

#include "llvm/Support/Error.h"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {
// Offset of memory interface in structure type.
cl::opt<unsigned> DTransSOAToAOSOPMemoryInterfaceOff(
    "dtrans-soatoaosop-mem-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Memory interface offset in struct/class type"));

SummaryForIdiom
getParametersForSOAToAOSMethodsCheckDebug(Function &F,
                                          DTransSafetyInfo *DTInfo) {
  DTransStructType *ClassType = getOPStructTypeOfMethod(&F, DTInfo);

  if (!ClassType)
    report_fatal_error(Twine("Cannot extract struct/class type from ") +
                       F.getName() + ".");

  DTransStructType *MemoryInterface = nullptr;
  if (ClassType->getNumFields() > DTransSOAToAOSOPMemoryInterfaceOff) {
    auto *PMemInt = dyn_cast<DTransPointerType>(
        ClassType->getFieldType(DTransSOAToAOSOPMemoryInterfaceOff));
    if (!PMemInt)
      report_fatal_error(
          "Incorrect memory interface specification: type at offset "
          "dtrans-soatoaosop-mem-off is not pointer to struct/class.");

    MemoryInterface =
        dyn_cast<DTransStructType>(PMemInt->getPointerElementType());
    if (!MemoryInterface)
      report_fatal_error(
          "Incorrect memory interface specification: type at offset "
          "dtrans-soatoaosop-mem-off is not pointer to struct/class.");
  }

  return SummaryForIdiom(ClassType, MemoryInterface, &F, DTInfo);
}
} // namespace soatoaosOP
} // namespace dtransOP
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
