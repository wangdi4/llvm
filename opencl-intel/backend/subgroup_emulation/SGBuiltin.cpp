//=-------------------------- SGBuiltin.cpp -*- C++ -*-----------------------=//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "SGBuiltin.h"

#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include <tuple>

#define DEBUG_TYPE "sg-emu-builtin"

using namespace Intel::OpenCL::DeviceBackend;
using namespace DPCPPKernelMetadataAPI;

bool EnableSubGroupEmulation = true;
static cl::opt<bool, true> EnableSubGroupEmulationOpt(
    "enable-subgroup-emulation", cl::location(EnableSubGroupEmulation),
    cl::Hidden, cl::desc("Enable sub-group emulation"));

namespace intel {

char SGBuiltin::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(
    SGBuiltin, DEBUG_TYPE,
    "Insert sub_group_barrier and vector-variants attribute", false, false)
OCL_INITIALIZE_PASS_DEPENDENCY_INTEL(SGSizeAnalysis)
OCL_INITIALIZE_PASS_END(
    SGBuiltin, DEBUG_TYPE,
    "Insert sub_group_barrier and vector-variants attribute", false, false)

using VecItem = std::tuple<const char *, const char *, const char *>;

// This function works like handleLanguageSpecifics of OCLVecClone, but the
// vector-varaints attribute is attached to the function not the call. We can
// benifit from this when vectorizing sub-group calls, because the vectorized
// function prototype can be created with the same logic as other non-inline
// functions.
bool SGBuiltin::insertSGBarrierForSGCalls(Module &M) {
  bool Changed = false;
  static constexpr VecItem VecInfo[] = {
#include "VectInfo.gen"
      {"intel_sub_group_ballot", "kernel-call-once",
       "_ZGVbM4v_intel_sub_group_balloti(intel_sub_group_ballot_vf4)"},
      {"intel_sub_group_ballot", "kernel-call-once",
       "_ZGVbM8v_intel_sub_group_balloti(intel_sub_group_ballot_vf8)"},
      {"intel_sub_group_ballot", "kernel-call-once",
       "_ZGVbM16v_intel_sub_group_balloti(intel_sub_group_ballot_vf16)"},
  };
  for (auto &F : M) {
    StringRef FnName = F.getName();
    if (!F.isDeclaration() || !FnName.contains("sub_group"))
      continue;

    // Find all vectorization infos for this function.
    auto CandidateVariants =
        make_filter_range(VecInfo, [FnName](const VecItem &Info) {
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
      if (!SizeAnalysis->hasEmuSize(PF))
        continue;

      auto &EmuSizes = SizeAnalysis->getEmuSizes(PF);

      auto MatchingVariants =
          make_filter_range(CandidateVariants, [EmuSizes](const VecItem &Info) {
            return EmuSizes.count(VectorVariant{std::get<2>(Info)}.getVlen());
          });
      assert(!MatchingVariants.empty() &&
             "sub-group calls with unsupported VF should be checked in "
             "SetVectorizationFactor Pass");

      for (auto &Info : MatchingVariants)
        VecVariantsStr.insert(std::get<2>(Info));

      // Add vector-variants attributes.
      assert(!CI->hasFnAttr("vector-variants") &&
             "Unexpected vector-variants attribute for sub-group calls!");

      AttributeList AL = CI->getAttributes();
      // All sub-group built-ins have mask argument.
      AL = AL.addFnAttribute(M.getContext(),
                             CompilationUtils::ATTR_HAS_VPLAN_MASK);
      CI->setAttributes(AL);

      // Insert sg_barrier before, dummy_sg_barrier after to exclude the
      // built-in call from sub-group loop.
      Helper.insertBarrierBefore(CI);
      Helper.insertDummyBarrierAfter(CI);
    }

    if (VecVariantsStr.empty())
      continue;
    assert(!F.hasFnAttribute("vector-variants") &&
           "Unexpected vector-variants attribute for sub-group function!");
    auto VecVariants = join(VecVariantsStr.begin(), VecVariantsStr.end(), ",");
    F.addFnAttr("vector-variants", VecVariants);
    Changed = true;
  }
  return Changed;
}

bool SGBuiltin::insertSGBarrierForWGBarriers(Module &M) {
  llvm::BarrierUtils Utils;
  Utils.init(&M);
  bool Changed = false;
  auto WGBarrierCalls = Utils.getAllSynchronizeInstructions();
  for (auto *WGBarrierCall : WGBarrierCalls) {
    auto *PF = WGBarrierCall->getFunction();
    // This call is in a vectorized function, skip it.
    if (!SizeAnalysis->hasEmuSize(PF))
      continue;
    if (Utils.getSyncType(WGBarrierCall) == SyncType::Barrier)
      Helper.insertBarrierBefore(WGBarrierCall);
    Helper.insertDummyBarrierAfter(WGBarrierCall);
    Changed = true;
  }
  return Changed;
}

bool SGBuiltin::runOnModule(Module &M) {
  Helper.initialize(M);

  SizeAnalysis = &getAnalysis<SGSizeAnalysis>();

  bool Changed = false;

  Changed |= insertSGBarrierForSGCalls(M);

  Changed |= insertSGBarrierForWGBarriers(M);

  return Changed;
}

} // namespace intel

extern "C" {
llvm::Pass *createSubGroupBuiltinPass() { return new intel::SGBuiltin(); }
}
