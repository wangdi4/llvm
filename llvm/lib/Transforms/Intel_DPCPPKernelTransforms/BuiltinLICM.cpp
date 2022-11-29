//===------- BuiltinLICM.cpp - hoist builtins out of loops ----------------===//
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLICM.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/Dominators.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace LoopUtils;

#define DEBUG_TYPE "dpcpp-kernel-builtin-licm"

namespace {

class BuiltinLICMImpl {
public:
  BuiltinLICMImpl(Loop &L, LoopInfo &LI, DominatorTree &DT, RuntimeService &RTS)
      : L(L), LI(LI), DT(DT), RTS(RTS), CurLoop(&L),
        PreHeader(L.getLoopPreheader()){};

  bool run();

private:
  Loop &L;

  LoopInfo &LI;

  DominatorTree &DT;

  RuntimeService &RTS;

  Loop *CurLoop;

  BasicBlock *PreHeader;

  SmallVector<Instruction *, 16> BuiltinToMove;

  void scanLoop(DomTreeNode *N);

  bool canHoistBuiltin(CallInst *CI);
};

bool BuiltinLICMImpl::canHoistBuiltin(CallInst *CI) {
  assert(nullptr != CI && "Cannot operate with the nullptr!");
  if (!CI->getCalledFunction()) {
    return false; // ignore indirect calls
  }

  StringRef funcName = CI->getCalledFunction()->getName();
  // To hoist the call it should have no side effect.
  if (!RTS.isSafeToSpeculativeExecute(funcName))
    return false;

  // All it's operands should be invariant.
  for (auto &arg : CI->args()) {
    if (!CurLoop->isLoopInvariant(arg))
      return false;
  }

  // Can hoist the call move it to the pre - header.
  return true;
}

void BuiltinLICMImpl::scanLoop(DomTreeNode *N) {
  assert(N != 0 && "Null dominator tree node?");
  BasicBlock *BB = N->getBlock();

  // If this subregion is outside the current loop exit.
  if (!CurLoop->contains(BB))
    return;

  // We don't analyze instruction in sub-loops.
  // Loop Passes goes from inner loop to outer loops, values from more inner
  // are already hoisted.
  if (!inSubLoop(BB, CurLoop, &LI)) {
    for (auto &I : *BB) {
      CallInst *CI = dyn_cast<CallInst>(&I);
      if (CI && canHoistBuiltin(CI))
        BuiltinToMove.push_back(CI);
    }
  }

  // Go over blocks recursively according to Dominator tree.
  for (DomTreeNode *child : N->children())
    scanLoop(child);
}

bool BuiltinLICMImpl::run() {
  const Function *F = L.getHeader()->getParent();
  if (F && F->hasOptNone())
    return false;

  if (!L.isLoopSimplifyForm())
    return false;

  BasicBlock *header = CurLoop->getHeader();

  // recursively scan the loop according to the dominator tree order.
  scanLoop(DT.getNode(header));

  // hoist builtins to pre-header
  for (auto *I : BuiltinToMove)
    I->moveBefore(PreHeader->getTerminator());

  return !BuiltinToMove.empty();
}

} // namespace

PreservedAnalyses BuiltinLICMPass::run(Loop &L, LoopAnalysisManager &AM,
                                       LoopStandardAnalysisResults &AR,
                                       LPMUpdater &U) {
  Function *F = L.getHeader()->getParent();
  Module *M = L.getHeader()->getModule();
  auto &FAMProxy = AM.getResult<FunctionAnalysisManagerLoopProxy>(L, AR);
  auto *MAMProxy = FAMProxy.getCachedResult<ModuleAnalysisManagerFunctionProxy>(*F);
  BuiltinLibInfo *BLI = MAMProxy->getCachedResult<BuiltinLibInfoAnalysis>(*M);
  BuiltinLICMImpl Impl(L, AR.LI, AR.DT, BLI->getRuntimeService());
  if (!Impl.run())
    return PreservedAnalyses::all();

  auto PA = getLoopPassPreservedAnalyses();
  if (AR.MSSA)
    PA.preserve<MemorySSAAnalysis>();

  return PA;
}
