//=-- SGBuiltin.cpp -Insert sub_group_barrier & vector-variants attribute ---=//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGBuiltin.h"

#include "llvm/InitializePasses.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <tuple>

#define DEBUG_TYPE "dpcpp-kernel-sg-emu-builtin"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

extern bool DPCPPEnableSubGroupEmulation;

// Static container storing all the vector info entries.
// Each entry would be a tuple of three strings:
// 1. scalar variant name
// 2. "kernel-call-once" | ""
// 3. mangled vector variant name
static std::vector<std::tuple<std::string, std::string, std::string>>
    ExtendedVectInfos;

namespace {
/// Legacy SGBuiltin pass.
class SGBuiltinLegacy : public ModulePass {
public:
  static char ID;

  SGBuiltinLegacy(ArrayRef<VectItem> VectInfos = {});

  StringRef getPassName() const override { return "SGBuiltinLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysisLegacy>();
    AU.addPreserved<SGSizeAnalysisLegacy>();
  }

private:
  SGBuiltinPass Impl;
};
} // namespace

char SGBuiltinLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(SGBuiltinLegacy, DEBUG_TYPE,
                      "Insert sub_group_barrier and vector-variants attribute",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(SGSizeAnalysisLegacy)
INITIALIZE_PASS_END(SGBuiltinLegacy, DEBUG_TYPE,
                    "Insert sub_group_barrier and vector-variants attribute",
                    false, false)

SGBuiltinLegacy::SGBuiltinLegacy(ArrayRef<VectItem> VectInfos)
    : ModulePass(ID), Impl(VectInfos) {
  initializeSGBuiltinLegacyPass(*PassRegistry::getPassRegistry());
}

bool SGBuiltinLegacy::runOnModule(Module &M) {
  const SGSizeInfo *SSI = &getAnalysis<SGSizeAnalysisLegacy>().getResult();

  return Impl.runImpl(M, SSI);
}

ModulePass *llvm::createSGBuiltinLegacyPass(ArrayRef<VectItem> VectInfos) {
  return new SGBuiltinLegacy(VectInfos);
}

SGBuiltinPass::SGBuiltinPass(ArrayRef<VectItem> VectInfos)
    : VectInfos(VectInfos) {}

PreservedAnalyses SGBuiltinPass::run(Module &M, ModuleAnalysisManager &AM) {
  const SGSizeInfo *SSI = &AM.getResult<SGSizeAnalysisPass>(M);
  if (!runImpl(M, SSI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool SGBuiltinPass::runImpl(Module &M, const SGSizeInfo *SSI) {
  if (!DPCPPEnableSubGroupEmulation)
    return false;

  // Load all vector info into ExtendedVectInfo, at most once.
  static llvm::once_flag InitializeVectInfoFlag;
  llvm::call_once(InitializeVectInfoFlag, [&]() {
    CompilationUtils::initializeVectInfoOnce(VectInfos, ExtendedVectInfos);
  });

  Helper.initialize(M);

  bool Changed = false;

  Changed |= insertSGBarrierForSGCalls(M, SSI);

  Changed |= insertSGBarrierForWGBarriers(M, SSI);

  return Changed;
}
// This function works like handleLanguageSpecifics of DPCPPKernelVecClone, but
// the vector-varaints attribute is attached to the function not the call. We
// can benifit from this when vectorizing sub-group calls, because the
// vectorized function prototype can be created with the same logic as other
// non-inline functions.
bool SGBuiltinPass::insertSGBarrierForSGCalls(Module &M,
                                              const SGSizeInfo *SSI) {
  bool Changed = false;

  for (auto &F : M) {
    StringRef FnName = F.getName();
    if (!F.isDeclaration() || !FnName.contains("sub_group"))
      continue;

    // Find all vectorization infos for this function.
    auto CandidateVariants = make_filter_range(
        ExtendedVectInfos,
        [FnName](
            const std::tuple<std::string, std::string, std::string> &Info) {
          return std::get<0>(Info) == FnName;
        });
    if (CandidateVariants.empty())
      continue;

    // Using Set here to avoid duplicated attributes.
    SetVector<StringRef> VecVariantsStr;

    for (User *U : F.users()) {
      CallInst *CI = dyn_cast<CallInst>(U);
      if (CI == nullptr)
        continue;

      Function *PF = CI->getFunction();
      if (PF == nullptr)
        continue;

      // This call is in a vectorized function, skip it.
      if (!SSI->hasEmuSize(PF))
        continue;

      auto &EmuSizes = SSI->getEmuSizes(PF);

      auto MatchingVariants = make_filter_range(
          CandidateVariants,
          [EmuSizes](
              const std::tuple<std::string, std::string, std::string> &Info) {
            return EmuSizes.count(VFABI::demangleForVFABI(std::get<2>(Info)).getVF());
          });
      assert(!MatchingVariants.empty() &&
             "sub-group calls with unsupported VF should be checked in "
             "SetVectorizationFactor Pass");

      for (auto &Info : MatchingVariants)
        VecVariantsStr.insert(std::get<2>(Info));

      // Add vector-variants attributes.
      assert(!CI->hasFnAttr(KernelAttribute::VectorVariants) &&
             "Unexpected vector-variants attribute for sub-group calls!");

      AttributeList AL = CI->getAttributes();
      // All sub-group built-ins have mask argument.
      AL = AL.addFnAttribute(M.getContext(), KernelAttribute::HasVPlanMask);
      CI->setAttributes(AL);

      // Insert sg_barrier before, dummy_sg_barrier after to exclude the
      // built-in call from sub-group loop.
      Helper.insertBarrierBefore(CI);
      Helper.insertDummyBarrierAfter(CI);
    }

    if (VecVariantsStr.empty())
      continue;
    assert(!F.hasFnAttribute(KernelAttribute::VectorVariants) &&
           "Unexpected vector-variants attribute for sub-group function!");
    auto VecVariants = join(VecVariantsStr.begin(), VecVariantsStr.end(), ",");
    F.addFnAttr(KernelAttribute::VectorVariants, VecVariants);
    Changed = true;
  }
  return Changed;
}

bool SGBuiltinPass::insertSGBarrierForWGBarriers(Module &M,
                                                 const SGSizeInfo *SSI) {
  llvm::BarrierUtils Utils;
  Utils.init(&M);
  bool Changed = false;
  auto WGBarrierCalls = Utils.getAllSynchronizeInstructions();

  for (auto *WGBarrierCall : WGBarrierCalls) {
    auto *PF = WGBarrierCall->getFunction();
    // This call is in a vectorized function, skip it.
    if (!SSI->hasEmuSize(PF))
      continue;
    if (Utils.getSyncType(WGBarrierCall) == SyncType::Barrier)
      Helper.insertBarrierBefore(WGBarrierCall);
    Helper.insertDummyBarrierAfter(WGBarrierCall);
    Changed = true;
  }
  return Changed;
}
