//=-- SGBuiltin.cpp -Insert sub_group_barrier & vector-variants attribute ---=//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/SubgroupEmulation/SGBuiltin.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/VectorizerUtils.h"

#include <tuple>

#define DEBUG_TYPE "sycl-kernel-sg-emu-builtin"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

SGBuiltinPass::SGBuiltinPass(ArrayRef<VectItem> VectInfos)
    : VectInfos(VectInfos) {}

PreservedAnalyses SGBuiltinPass::run(Module &M, ModuleAnalysisManager &AM) {
  const SGSizeInfo *SSI = &AM.getResult<SGSizeAnalysisPass>(M);
  if (!runImpl(M, SSI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool SGBuiltinPass::runImpl(Module &M, const SGSizeInfo *SSI) {
  Helper.initialize(M);

  bool Changed = false;

  Changed |= insertSGBarrierForSGCalls(M, SSI);

  Changed |= insertSGBarrierForWGBarriers(M, SSI);

  return Changed;
}
// This function works like handleLanguageSpecifics of SYCLKernelVecClone, but
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

    // If kernel have SG builtin, load all vector info into ExtendedVectInfo
    CompilationUtils::initializeVectInfo(VectInfos, M);

    // Find all vectorization infos for this function.
    auto &ExtendedVectInfos = CompilationUtils::getExtendedVectInfos();
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
#if INTEL_CUSTOMIZATION
          [&EmuSizes](
              const std::tuple<std::string, std::string, std::string> &Info) {
            VFInfo Variant = VFABI::demangleForVFABI(std::get<2>(Info));
#else  // INTEL_CUSTOMIZATION
          [&EmuSizes,
           &M](const std::tuple<std::string, std::string, std::string> &Info) {
            VFInfo Variant =
                VFABI::tryDemangleForVFABI(std::get<2>(Info), M).value();
#endif // INTEL_CUSTOMIZATION
            return EmuSizes.count(VectorizerUtils::getVFLength(Variant));
          });
      assert(!MatchingVariants.empty() &&
             "sub-group calls with unsupported VF should be checked in "
             "SetVectorizationFactor Pass");

      for (auto &Info : MatchingVariants)
        VecVariantsStr.insert(std::get<2>(Info));

      // Add vector-variants attributes.
      assert(!CI->hasFnAttr(VectorUtils::VectorVariantsAttrName) &&
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
    assert(!F.hasFnAttribute(VectorUtils::VectorVariantsAttrName) &&
           "Unexpected vector-variants attribute for sub-group function!");
    auto VecVariants = join(VecVariantsStr.begin(), VecVariantsStr.end(), ",");
    F.addFnAttr(VectorUtils::VectorVariantsAttrName, VecVariants);
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
