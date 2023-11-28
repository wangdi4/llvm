//===-- CGProfile.cpp -----------------------------------------------------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/CGProfile.h"

#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/LazyBlockFrequencyInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Transforms/Instrumentation.h"
#include <optional>

using namespace llvm;

static bool
addModuleFlags(Module &M,
               MapVector<std::pair<Function *, Function *>, uint64_t> &Counts) {
  if (Counts.empty())
    return false;

  LLVMContext &Context = M.getContext();
  MDBuilder MDB(Context);
  std::vector<Metadata *> Nodes;

  for (auto E : Counts) {
    Metadata *Vals[] = {ValueAsMetadata::get(E.first.first),
                        ValueAsMetadata::get(E.first.second),
                        MDB.createConstant(ConstantInt::get(
                            Type::getInt64Ty(Context), E.second))};
    Nodes.push_back(MDNode::get(Context, Vals));
  }

  M.addModuleFlag(Module::Append, "CG Profile",
                  MDTuple::getDistinct(Context, Nodes));
  return true;
}

static bool runCGProfilePass(
    Module &M, FunctionAnalysisManager &FAM) {
  MapVector<std::pair<Function *, Function *>, uint64_t> Counts;
  InstrProfSymtab Symtab;
  auto UpdateCounts = [&](TargetTransformInfo &TTI, Function *F,
                          Function *CalledF, uint64_t NewCount) {
    if (NewCount == 0)
      return;
    if (!CalledF || !TTI.isLoweredToCall(CalledF) ||
        CalledF->hasDLLImportStorageClass())
      return;
#if INTEL_CUSTOMIZATION
    // Calls to __svml_ functions may get replaced with a call to a different
    // __svml_ function during the MapIntrinToIml pass. Do not include these
    // calls in the function reordering call graph profile. (CMPLRLLVM-36647)
    if (CalledF->getName().startswith("__svml_"))
      return;
#endif // INTEL_CUSTOMIZATION
    uint64_t &Count = Counts[std::make_pair(F, CalledF)];
    Count = SaturatingAdd(Count, NewCount);
  };
  // Ignore error here.  Indirect calls are ignored if this fails.
  (void)(bool) Symtab.create(M);
  for (auto &F : M) {
    // Avoid extra cost of running passes for BFI when the function doesn't have
    // entry count.
    if (F.isDeclaration() || !F.getEntryCount())
      continue;
    auto &BFI = FAM.getResult<BlockFrequencyAnalysis>(F);
    if (BFI.getEntryFreq() == BlockFrequency(0))
      continue;
    TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
    for (auto &BB : F) {
      std::optional<uint64_t> BBCount = BFI.getBlockProfileCount(&BB);
      if (!BBCount)
        continue;
      for (auto &I : BB) {
        CallBase *CB = dyn_cast<CallBase>(&I);
        if (!CB)
          continue;
        if (CB->isIndirectCall()) {
          InstrProfValueData ValueData[8];
          uint32_t ActualNumValueData;
          uint64_t TotalC;
          if (!getValueProfDataFromInst(*CB, IPVK_IndirectCallTarget, 8,
                                        ValueData, ActualNumValueData, TotalC))
            continue;
          for (const auto &VD :
               ArrayRef<InstrProfValueData>(ValueData, ActualNumValueData)) {
            UpdateCounts(TTI, &F, Symtab.getFunction(VD.Value), VD.Count);
          }
          continue;
        }
        UpdateCounts(TTI, &F, CB->getCalledFunction(), *BBCount);
      }
    }
  }

  return addModuleFlags(M, Counts);
}

PreservedAnalyses CGProfilePass::run(Module &M, ModuleAnalysisManager &MAM) {
  FunctionAnalysisManager &FAM =
      MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  runCGProfilePass(M, FAM);

  return PreservedAnalyses::all();
}
