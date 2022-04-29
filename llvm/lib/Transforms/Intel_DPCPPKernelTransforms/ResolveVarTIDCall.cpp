//===- ResolveVarTIDCall.cpp - Resolve TID with variable argument ---------===//
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveVarTIDCall.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelLoopUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/KernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;
using namespace DPCPPKernelLoopUtils;

#define DEBUG_TYPE "dpcpp-kernel-resolve-var-tid-call"

static bool runOnTID(Module &M, IRBuilder<> &Builder, Constant *ConstZero,
                     StringRef TIDName, const Twine &Prefix) {
  Function *F = M.getFunction(TIDName);
  if (!F)
    return false;

  SmallVector<CallInst *, 4> VarTIDCalls;
  for (User *U : F->users()) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    if (isa<ConstantInt>(CI->getArgOperand(0)))
      continue;
    LLVM_DEBUG(dbgs() << "Found var TID call: " << *CI << " in function "
                      << CI->getFunction()->getName() << "\n");
    VarTIDCalls.push_back(CI);
  }

  auto &Ctx = M.getContext();

  for (CallInst *CI : VarTIDCalls) {
    auto *Caller = CI->getFunction();
    auto *Arg = CI->getArgOperand(0);

    // Create BasicBlock for each dimension.
    auto *BBDim0 = CI->getParent();
    auto *BBDimOOB =
        BBDim0->splitBasicBlock(CI, Twine("bb.") + Prefix + Twine("oob"));
    auto *BBAfter =
        BBDimOOB->splitBasicBlock(CI, Twine("bb.") + Prefix + Twine("exit"));
    auto *BBDim1 = BasicBlock::Create(Ctx, Twine("bb.") + Prefix + Twine("1"),
                                      Caller, BBDimOOB);
    auto *BBDim2 = BasicBlock::Create(Ctx, Twine("bb.") + Prefix + Twine("2"),
                                      Caller, BBDimOOB);

    BBDim0->getTerminator()->eraseFromParent();

    BasicBlock *BBDims[] = {BBDim0, BBDim1, BBDim2, BBDimOOB};

    Builder.SetInsertPoint(CI);
    auto *TIDPhi = Builder.CreatePHI(CI->getType(), 4, Prefix + Twine("phi"));
    TIDPhi->addIncoming(ConstZero, BBDimOOB);

    // Create TID calls with fixed arguments.
    for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
      Builder.SetInsertPoint(BBDims[Dim]);
      auto *TID = getWICall(&M, TIDName, CI->getType(), Dim, Builder,
                            AppendWithDimension(Prefix, Dim));
      auto *Cond = Builder.CreateICmpEQ(Arg, Builder.getInt32(Dim));
      Builder.CreateCondBr(Cond, BBAfter, BBDims[Dim + 1]);
      TIDPhi->addIncoming(TID, BBDims[Dim]);
    }

    CI->replaceAllUsesWith(TIDPhi);
    CI->eraseFromParent();
  }

  return !VarTIDCalls.empty();
}

static bool runImpl(Module &M) {
  IRBuilder<> Builder(M.getContext());
  auto *ConstZero = ConstantInt::get(getIndTy(&M), 0);
  bool Changed = runOnTID(M, Builder, ConstZero, mangledGetLID(), "lid.");
  Changed |= runOnTID(M, Builder, ConstZero, mangledGetGID(), "gid.");
  return Changed;
}

namespace {
class ResolveVarTIDCallLegacy : public ModulePass {
public:
  static char ID;

  ResolveVarTIDCallLegacy() : ModulePass(ID) {
    initializeResolveVarTIDCallLegacyPass(*PassRegistry::getPassRegistry());
  }

  llvm::StringRef getPassName() const override {
    return "ResolveVarTIDCallLegacy";
  }

  bool runOnModule(Module &M) override { return runImpl(M); }
};
} // namespace

char ResolveVarTIDCallLegacy::ID = 0;

INITIALIZE_PASS(ResolveVarTIDCallLegacy, DEBUG_TYPE,
                "Resolve TID with variable argument", false, false)

ModulePass *llvm::createResolveVarTIDCallLegacyPass() {
  return new ResolveVarTIDCallLegacy();
}

PreservedAnalyses ResolveVarTIDCallPass::run(Module &M,
                                             ModuleAnalysisManager &) {
  return runImpl(M) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
