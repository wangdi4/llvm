//===---------------- SOAToAOSStruct.cpp - Part of SOAToAOSPass -----------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements debug functionality related specifically to structures
// containing arrays for SOA-to-AOS: method analysis and transformations.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Compiler.h"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
#include "SOAToAOSStruct.h"

#include "Intel_DTrans/Analysis/DTransAnalysis.h"
#include "Intel_DTrans/Transforms/DTransOptBase.h"
#include "Intel_DTrans/Transforms/SOAToAOS.h"

#include "llvm/Support/CommandLine.h"

// Same as in SOAToAOS.cpp.
#define DEBUG_TYPE "dtrans-soatoaos"

namespace llvm {
namespace dtrans {
namespace soatoaos {
// Array types for structure containing arrays.
// Structure is specified in DTransSOAToAOSApproxTypename.
static cl::list<std::string> DTransSOAToAOSArrays("dtrans-soatoaos-array-type",
                                                  cl::ReallyHidden);

// Methods of arrays, which are append-like.
static cl::list<std::string>
    DTransSOAToAOSAppends("dtrans-soatoaos-array-append", cl::ReallyHidden);
// Methods of arrays, which are copy ctors.
static cl::list<std::string> DTransSOAToAOSCCtors("dtrans-soatoaos-array-cctor",
                                                  cl::ReallyHidden);
// Methods of arrays, which are regular ctors.
static cl::list<std::string> DTransSOAToAOSCtors("dtrans-soatoaos-array-ctor",
                                                 cl::ReallyHidden);
// Methods of arrays, which are dtors.
static cl::list<std::string> DTransSOAToAOSDtors("dtrans-soatoaos-array-dtor",
                                                 cl::ReallyHidden);

// CallSite comparisons to perform, requires corresponding
// dtrans-soatoaos-arrays-* to be set.
// Valid values are cctor, ctor, dtor, append.
static cl::list<std::string>
    DTransSOAToAOSComparison("dtrans-soatoaos-method-call-site-comparison",
                             cl::ReallyHidden);

static std::pair<SmallVector<StructType *, 3>, SmallVector<unsigned, 3>>
getArrayTypesForSOAToAOSStructMethodsCheckDebug(Function &F) {
  SmallVector<StructType *, 3> ArrayTypes;
  for (auto &Name : DTransSOAToAOSArrays) {
    auto *ArrayType = F.getParent()->getTypeByName(Name);
    // TODO: add diagnostic message.
    if (!ArrayType)
      return std::make_pair(SmallVector<StructType *, 3>(),
                            SmallVector<unsigned, 3>());
    ArrayTypes.push_back(ArrayType);
  }

  auto *Struct = getStructTypeOfArray(F);
  // TODO: add diagnostic message.
  if (!Struct)
    return std::make_pair(SmallVector<StructType *, 3>(),
                          SmallVector<unsigned, 3>());

  SmallVector<unsigned, 3> ArrayOffsets;
  for (unsigned I = 0, E = Struct->getNumElements(); I != E; ++I) {
    auto *PTy = dyn_cast<PointerType>(Struct->getElementType(I));
    if (!PTy)
      continue;

    auto *STy = dyn_cast<StructType>(PTy->getPointerElementType());
    if (std::find(ArrayTypes.begin(), ArrayTypes.end(), STy) !=
        ArrayTypes.end())
      ArrayOffsets.push_back(I);
  }

  if (ArrayTypes.size() != ArrayOffsets.size())
    return std::make_pair(SmallVector<StructType *, 3>(),
                          SmallVector<unsigned, 3>());

  return std::make_pair(ArrayTypes, ArrayOffsets);
}

static CallSiteComparator::FunctionSet
getSOAToAOSArrayMethods(const cl::list<std::string> &List,
                        const SmallVector<StructType *, 3> &ArrayTypes,
                        Function &F, bool &Failure) {
  CallSiteComparator::FunctionSet Methods(ArrayTypes.size(), nullptr);
  unsigned Count = 0;
  for (auto &Name : List) {
    auto *AF = F.getParent()->getFunction(Name);
    // TODO: add remark.
    if (!AF)
      continue;
    // TODO: add diagnostic message.
    if (AF->arg_size() < 1) {
      Failure = true;
      return CallSiteComparator::FunctionSet();
    }
    auto *PThis = dyn_cast<PointerType>(AF->arg_begin()->getType());
    // TODO: add diagnostic message.
    if (!PThis) {
      Failure = true;
      return CallSiteComparator::FunctionSet();
    }
    auto *ClassType = PThis->getElementType();

    auto It = std::find(ArrayTypes.begin(), ArrayTypes.end(), ClassType);
    // TODO: add diagnostic message.
    if (It == ArrayTypes.end()) {
      Failure = true;
      return CallSiteComparator::FunctionSet();
    }

    // TODO: add diagnostic message.
    if (Methods[It - ArrayTypes.begin()]) {
      Failure = true;
      return CallSiteComparator::FunctionSet();
    }
    Count++;
    Methods[It - ArrayTypes.begin()] = AF;
  }

  if (Count != 0 && Count != ArrayTypes.size()) {
    Failure = true;
    return CallSiteComparator::FunctionSet();
  }

  if (Count == 0)
    return CallSiteComparator::FunctionSet();

  return Methods;
}
} // namespace soatoaos
using namespace soatoaos;
char SOAToAOSStructMethodsCheckDebug::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey SOAToAOSStructMethodsCheckDebug::Key;

SOAToAOSStructMethodsCheckDebug::Ignore
SOAToAOSStructMethodsCheckDebug::run(Function &F, FunctionAnalysisManager &AM) {
  const ModuleAnalysisManager &MAM =
      AM.getResult<ModuleAnalysisManagerFunctionProxy>(F).getManager();
  auto *DTInfo = MAM.getCachedResult<DTransAnalysis>(*F.getParent());
  auto *TLI = MAM.getCachedResult<TargetLibraryAnalysis>(*F.getParent());
  // TODO: add diagnostic message.
  if (!DTInfo || !TLI)
    return Ignore();

  auto *Res = AM.getCachedResult<SOAToAOSApproximationDebug>(F);
  if (!Res)
    return Ignore();
  const DepMap *DM = Res->get();
  // TODO: add diagnostic message.
  if (!DM)
    return Ignore();

  SummaryForIdiom S = getParametersForSOAToAOSMethodsCheckDebug(F);
  // TODO: add diagnostic message.
  if (!S.StrType)
    return Ignore();

  LLVM_DEBUG(dbgs() << "; Checking structure's method " << F.getName() << "\n");

  auto P = getArrayTypesForSOAToAOSStructMethodsCheckDebug(F);
  if (P.first.size() <= 1)
    return Ignore();

  CallSiteComparator::CallSitesInfo CSInfo;
  for (auto &CmpKind : DTransSOAToAOSComparison)
    if (CmpKind == "append") {
      bool Failure = false;
      CSInfo.Appends =
          getSOAToAOSArrayMethods(DTransSOAToAOSAppends, P.first, F, Failure);
      // TODO: add diagnostic message.
      if (Failure)
        return Ignore();
    } else if (CmpKind == "cctor") {
      bool Failure = false;
      CSInfo.CCtors =
          getSOAToAOSArrayMethods(DTransSOAToAOSCCtors, P.first, F, Failure);
      // TODO: add diagnostic message.
      if (Failure)
        return Ignore();
    } else if (CmpKind == "ctor") {
      bool Failure = false;
      CSInfo.Ctors =
          getSOAToAOSArrayMethods(DTransSOAToAOSCtors, P.first, F, Failure);
      // TODO: add diagnostic message.
      if (Failure)
        return Ignore();
    } else if (CmpKind == "dtor") {
      bool Failure = false;
      CSInfo.Dtors =
          getSOAToAOSArrayMethods(DTransSOAToAOSDtors, P.first, F, Failure);
      // TODO: add diagnostic message.
      if (Failure)
        return Ignore();
    } else {
      // TODO: add diagnostic message.
      return Ignore();
    }

  if (DTransSOAToAOSBasePtrOff == -1U)
    // TODO: add diagnostic message.
    return Ignore();

  StructureMethodAnalysis::TransformationData TI;
  CallSiteComparator Checks(F.getParent()->getDataLayout(), *DTInfo, *TLI, *DM,
                            S, P.first, P.second, CSInfo,
                            DTransSOAToAOSBasePtrOff, TI);
  bool Result = Checks.checkStructMethod();
  (void)Result;
  LLVM_DEBUG(dbgs() << "; IR: "
                    << (Result ? "analysed completely"
                               : "has some side-effect to analyse")
                    << "\n");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOASTR, {
    dbgs() << "; Dump instructions needing update. Total = "
           << Checks.getTotal();
    StructureMethodAnalysis::AnnotatedWriter Annotate(Checks);
    F.print(dbgs(), &Annotate);
  });

  bool Comparison = Checks.canCallSitesBeMerged();
  (void)Comparison;
  LLVM_DEBUG(dbgs() << "; Array call sites analysis result: "
                    << (Comparison ? "required call sites can be merged"
                                   : "problem with call sites required to be merged")
                    << "\n");
  return Ignore();
}
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
