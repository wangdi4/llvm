//===- VectorKernelElimination.cpp - Vector kernel elimination ------------===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/VectorKernelElimination.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WeightedInstCount.h"

#define DEBUG_TYPE "dpcpp-kernel-vec-kernel-elim"

using namespace llvm;
using namespace DPCPPKernelMetadataAPI;

static constexpr float ScalarCostMultiplier = .92f;

static int
getCalleeCost(Function *F,
              function_ref<InstCountResult &(Function &F)> GetInstCountResult) {
  int Cost = 0;
  for (auto &I : instructions(F)) {
    auto *CI = dyn_cast<CallInst>(&I);
    if (!CI)
      continue;
    Function *Callee = CI->getCalledFunction();
    if (Callee && !Callee->isDeclaration()) {
      float Prob = GetInstCountResult(*F).getBBProb(CI->getParent());
      // TODO consider multiplying TripCount as well?
      int CalleeCost = GetInstCountResult(*Callee).getWeight() * Prob;
      LLVM_DEBUG(dbgs().indent(4)
                 << "callee " << Callee->getName() << " cost: " << CalleeCost
                 << ", BB Prob: " << Prob << "\n");
      Cost += CalleeCost;
    }
  }
  return Cost;
}

bool runImpl(Module &M,
             function_ref<InstCountResult &(Function &F)> GetInstCountResult) {
  bool Changed = false;

  for (auto *F : KernelList(&M)) {
    LLVM_DEBUG(dbgs() << "VectorKernelElimination, kernel: " << F->getName()
                      << "\n");

    auto KIMD = KernelInternalMetadataAPI(F);
    if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get()) {
      LLVM_DEBUG(
          dbgs() << "  Kernel has subgroups, so keep vectorized kernel.\n");
      continue;
    }

    auto KMD = KernelMetadataAPI(F);
    if (KMD.hasVecLength()) {
      LLVM_DEBUG(
          dbgs() << "  Kernel VF is forced, so keep vectorized kernel.\n");
      continue;
    }

    if (!KIMD.VectorizedKernel.hasValue())
      continue;
    Function *VecF = KIMD.VectorizedKernel.get();
    if (!VecF)
      continue;
    auto VKIMD = KernelInternalMetadataAPI(VecF);
    unsigned VF = VKIMD.VectorizedWidth.get();
    LLVM_DEBUG(dbgs() << "  Vectorized kernel: " << VecF->getName() << "\n");

    int ScalarCost = GetInstCountResult(*F).getWeight() +
                     getCalleeCost(F, GetInstCountResult);
    assert(VF != 0 && "Kernel vectorized_width metadata must not be zero!");
    int VectorCost = (GetInstCountResult(*VecF).getWeight() +
                      getCalleeCost(VecF, GetInstCountResult)) /
                     VF;
    LLVM_DEBUG(dbgs() << "  ScalarCost: " << ScalarCost
                      << "  VectorCost: " << VectorCost << '\n');
    if (ScalarCost * ScalarCostMultiplier <= VectorCost) {
      F->removeFnAttr("vector-variants");
      MDValueGlobalObjectStrategy::unset(F, KIMD.VectorizedKernel.getID());
      VecF->eraseFromParent();
      Changed = true;
      LLVM_DEBUG(dbgs() << "  Vectorized kernel is eliminated.\n");
    } else {
      LLVM_DEBUG(dbgs() << "  Vectorized kernel is kept.\n");
    }
  }

  return Changed;
}

namespace {

class VectorKernelEliminationLegacy : public ModulePass {
public:
  static char ID;

  VectorKernelEliminationLegacy() : ModulePass(ID) {
    initializeVectorKernelEliminationLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "VectorKernelEliminationLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WeightedInstCountAnalysisLegacy>();
  }

  bool runOnModule(Module &M) override {
    auto GetInstCountResult = [&](Function &F) -> InstCountResult & {
      return getAnalysis<WeightedInstCountAnalysisLegacy>(F).getResult();
    };
    return runImpl(M, GetInstCountResult);
  }
};
} // namespace

INITIALIZE_PASS_BEGIN(VectorKernelEliminationLegacy, DEBUG_TYPE,
                      "Eliminate vector kernel based on cost model", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(WeightedInstCountAnalysisLegacy)
INITIALIZE_PASS_END(VectorKernelEliminationLegacy, DEBUG_TYPE,
                    "Eliminate vector kernel based on cost model", false, false)

char VectorKernelEliminationLegacy::ID = 0;

ModulePass *llvm::createVectorKernelEliminationLegacyPass() {
  return new VectorKernelEliminationLegacy();
}

PreservedAnalyses VectorKernelEliminationPass::run(Module &M,
                                                   ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetInstCountResult = [&FAM](Function &F) -> InstCountResult & {
    return FAM.getResult<WeightedInstCountAnalysis>(F);
  };
  if (!runImpl(M, GetInstCountResult))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
