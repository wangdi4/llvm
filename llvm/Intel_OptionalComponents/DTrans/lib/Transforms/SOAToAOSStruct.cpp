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
// This file implements debug functionality related specifically structures
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
cl::list<std::string> DTransSOAToAOSArrays("dtrans-soatoaos-array-type",
                                           cl::ReallyHidden);
std::pair<SmallVector<StructType *, 3>, SmallVector<unsigned, 3>>
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

  StructureMethodAnalysis::TransformationData TI;
  StructureMethodAnalysis MC(F.getParent()->getDataLayout(), *DTInfo, *TLI, *DM,
                             S, P.first, P.second, TI);
  bool Result = MC.checkStructMethod();
  (void)Result;
  LLVM_DEBUG(dbgs() << "; IR: "
                    << (Result ? "analysed completely"
                               : "has some side-effect to analyse")
                    << "\n");

  // Dump results of analysis.
  DEBUG_WITH_TYPE(DTRANS_SOASTR, {
    dbgs() << "; Dump instructions needing update. Total = " << MC.getTotal();
    StructureMethodAnalysis::AnnotatedWriter Annotate(MC);
    F.print(dbgs(), &Annotate);
  });
  return Ignore();
}
} // namespace dtrans
} // namespace llvm
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
