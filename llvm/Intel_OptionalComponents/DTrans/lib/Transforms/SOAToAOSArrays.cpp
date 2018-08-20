//===---------------- SOAToAOSArrays.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to array
// structures for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSArrays.h"

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/SOAToAOS.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"

// Same as in SOAToAOS.cpp.
#define DEBUG_TYPE "dtrans-soatoaos"

namespace llvm {
namespace dtrans {
namespace soatoaos {

// Offset of base pointer in DTransSOAToAOSApproxTypename.
cl::opt<unsigned> DTransSOAToAOSBasePtrOff(
    "dtrans-soatoaos-base-ptr-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Base pointer offset in dtrans-soatoaos-approx-typename"));

// Offset of memory interface in DTransSOAToAOSApproxTypename.
cl::opt<unsigned> DTransSOAToAOSMemoryInterfaceOff(
    "dtrans-soatoaos-mem-off", cl::init(-1U), cl::ReallyHidden,
    cl::desc("Memory interface offset in dtrans-soatoaos-approx-typename"));

SummaryForIdiom getParametersForSOAToAOSMethodsCheckDebug(Function &F) {
  StructType *ClassType = getStructTypeOfArray(F);

  SummaryForIdiom Failure(nullptr, nullptr, nullptr, nullptr);

  if (!ClassType)
    return Failure;

  if (ClassType->getNumElements() <= DTransSOAToAOSBasePtrOff)
    return Failure;

  auto *PBase = dyn_cast<PointerType>(
      ClassType->getTypeAtIndex(DTransSOAToAOSBasePtrOff));

  if (!PBase)
    return Failure;

  StructType *MemoryInterface = nullptr;
  if (ClassType->getNumElements() > DTransSOAToAOSMemoryInterfaceOff) {
    auto *PMemInt = dyn_cast<PointerType>(
      ClassType->getTypeAtIndex(DTransSOAToAOSMemoryInterfaceOff));
    if (!PMemInt)
      return Failure;
    MemoryInterface = dyn_cast<StructType>(PMemInt->getPointerElementType());
  }

  return SummaryForIdiom(ClassType, PBase->getPointerElementType(),
                         MemoryInterface, &F);
}
} // namespace soatoaos

using namespace soatoaos;

char SOAToAOSMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSMethodsCheckDebug::Key;

SOAToAOSMethodsCheckDebug::Ignore::Ignore(SOAToAOSMethodsCheckDebugResult *Ptr)
    : Ptr(Ptr) {}
SOAToAOSMethodsCheckDebug::Ignore::Ignore(Ignore &&Other)
    : Ptr(std::move(Other.Ptr)) {}
const SOAToAOSMethodsCheckDebugResult *
SOAToAOSMethodsCheckDebug::Ignore::get() const {
  return Ptr.get();
}
SOAToAOSMethodsCheckDebug::Ignore::~Ignore() {}

SOAToAOSMethodsCheckDebug::Ignore
SOAToAOSMethodsCheckDebug::run(Function &F, FunctionAnalysisManager &AM) {

  auto *Res = AM.getCachedResult<SOAToAOSApproximationDebug>(F);
  if (!Res)
    return Ignore(nullptr);
  const DepMap *DM = Res->get();
  // TODO: add diagnostic message.
  if (!DM)
    return Ignore(nullptr);

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);
  // TODO: add diagnostic message.
  if (!S.ArrType)
    return Ignore(nullptr);

  LLVM_DEBUG(dbgs() << "; Checking array's method " << F.getName() << "\n");

  std::unique_ptr<SOAToAOSMethodsCheckDebugResult> Result(
      new SOAToAOSMethodsCheckDebugResult());
  ComputeArrayMethodClassification MC(F.getParent()->getDataLayout(), *DM, S,
                                      *Result);
  Result->MK = MC.classify().first;
  LLVM_DEBUG(dbgs() << "; Classification: " << Result->MK << "\n");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOAARR, {
    dbgs() << "; Dump instructions needing update. Total = " << MC.getTotal();
    ComputeArrayMethodClassification::AnnotatedWriter Annotate(MC);
    F.print(dbgs(), &Annotate);
  });
  return Ignore(Result.release());
}
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
