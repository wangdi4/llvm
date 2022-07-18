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
#include "llvm/InitializePasses.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Transforms/Instrumentation.h"

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
    Module &M, function_ref<BlockFrequencyInfo &(Function &)> GetBFI,
    function_ref<TargetTransformInfo &(Function &)> GetTTI, bool LazyBFI) {
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
    // entry count. Since LazyBlockFrequencyInfoPass only exists in LPM, check
    // if using LazyBlockFrequencyInfoPass.
    // TODO: Remove LazyBFI when LazyBlockFrequencyInfoPass is available in NPM.
    if (F.isDeclaration() || (LazyBFI && !F.getEntryCount()))
      continue;
    auto &BFI = GetBFI(F);
    if (BFI.getEntryFreq() == 0)
      continue;
    TargetTransformInfo &TTI = GetTTI(F);
    for (auto &BB : F) {
      Optional<uint64_t> BBCount = BFI.getBlockProfileCount(&BB);
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

namespace {
struct CGProfileLegacyPass final : public ModulePass {
  static char ID;
  CGProfileLegacyPass() : ModulePass(ID) {
    initializeCGProfileLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LazyBlockFrequencyInfoPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    auto GetBFI = [this](Function &F) -> BlockFrequencyInfo & {
      return this->getAnalysis<LazyBlockFrequencyInfoPass>(F).getBFI();
    };
    auto GetTTI = [this](Function &F) -> TargetTransformInfo & {
      return this->getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    };

    return runCGProfilePass(M, GetBFI, GetTTI, true);
  }
};

} // namespace

char CGProfileLegacyPass::ID = 0;

INITIALIZE_PASS(CGProfileLegacyPass, "cg-profile", "Call Graph Profile", false,
                false)

ModulePass *llvm::createCGProfileLegacyPass() {
  return new CGProfileLegacyPass();
}

PreservedAnalyses CGProfilePass::run(Module &M, ModuleAnalysisManager &MAM) {
  FunctionAnalysisManager &FAM =
      MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetBFI = [&FAM](Function &F) -> BlockFrequencyInfo & {
    return FAM.getResult<BlockFrequencyAnalysis>(F);
  };
  auto GetTTI = [&FAM](Function &F) -> TargetTransformInfo & {
    return FAM.getResult<TargetIRAnalysis>(F);
  };

  runCGProfilePass(M, GetBFI, GetTTI, false);

  return PreservedAnalyses::all();
}
