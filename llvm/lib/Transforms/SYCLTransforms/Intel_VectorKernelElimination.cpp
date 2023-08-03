//===- Intel_VectorKernelElimination.cpp - Vector kernel elimination ------===//
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

#include "llvm/Transforms/SYCLTransforms/Intel_VectorKernelElimination.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/WeightedInstCount.h"

#define DEBUG_TYPE "sycl-kernel-vec-kernel-elim"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;

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
      F->removeFnAttr(VectorUtils::VectorVariantsAttrName);
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
