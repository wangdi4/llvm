//===--------------- ReuseFieldOP.cpp - DTransReuseFieldOPPass ------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans reuse field optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReuseFieldOP.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/InitializePasses.h"

#include <map>

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-reusefieldop"

namespace {

class ReuseFieldOPImpl {
public:
  typedef std::pair<Type *, size_t> StructFieldTy;
  typedef std::pair<Value*, StoreInst*> VITy;
  typedef SmallDenseMap<Value *, VITy> BP2VIMapTy;
  typedef SmallDenseMap<StructFieldTy, BP2VIMapTy> SF2BP2VIMapTy;
  typedef SmallDenseMap<Type *,
    SmallDenseMap<size_t, SmallDenseMap<size_t, SmallVector<StoreInst *, 2>>>>
    MissingFieldsListTy;
  typedef std::map<StructFieldTy, SmallVector<size_t>> StructField2FieldsMapTy;
  typedef SmallDenseMap<dtrans::StructInfo*, SmallSet<size_t, 4>> CandidateStructFieldsTy;

  ReuseFieldOPImpl(
    DTransSafetyInfo &DTInfo,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI
      ): GetTLI(GetTLI) {}

  bool run(Module &M);

private:
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;
};

} // end anonymous namespace

bool ReuseFieldOPImpl::run(Module& M) {
  return false;
}

bool ReuseFieldOPPass::runImpl(
  Module &M, DTransSafetyInfo &DTInfo,
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
  WholeProgramInfo &WPInfo) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
    return false;
  }

  ReuseFieldOPImpl Transformer(DTInfo, GetTLI);
  return Transformer.run(M);
}

PreservedAnalyses ReuseFieldOPPass::run(Module &M,
  ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function*>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}
