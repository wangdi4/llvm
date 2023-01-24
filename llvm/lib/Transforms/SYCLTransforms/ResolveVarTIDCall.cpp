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

#include "llvm/Transforms/SYCLTransforms/ResolveVarTIDCall.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/SYCLTransforms/Utils/BarrierUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace CompilationUtils;
using namespace LoopUtils;

#define DEBUG_TYPE "sycl-kernel-resolve-var-tid-call"

static bool runOnTID(Module &M, IRBuilder<> &Builder, Constant *ConstZero,
                     StringRef TIDName, const Twine &Prefix) {
  Function *F = M.getFunction(TIDName);
  if (!F)
    return false;

  SmallVector<CallInst *, 4> VarTIDCalls;
  SmallVector<CallInst *, 4> OOBTIDCalls;
  for (User *U : F->users()) {
    auto *CI = dyn_cast<CallInst>(U);
    if (!CI)
      continue;
    if (auto *C = dyn_cast<ConstantInt>(CI->getArgOperand(0))) {
      uint64_t Dim = C->getZExtValue();
      if (Dim >= MAX_WORK_DIM) {
        LLVM_DEBUG(dbgs() << "Found out-of-bound TID Call:" << *CI
                          << " in function: " << CI->getFunction()->getName()
                          << "\n");
        OOBTIDCalls.push_back(CI);
      }
      continue;
    }
    LLVM_DEBUG(dbgs() << "Found var TID call: " << *CI << " in function "
                      << CI->getFunction()->getName() << "\n");
    VarTIDCalls.push_back(CI);
  }

  for (CallInst *CI : OOBTIDCalls) {
    CI->replaceAllUsesWith(ConstZero);
    CI->eraseFromParent();
  }

  DenseMap<Function *, SmallVector<Instruction *, 3>> FuncToFixedTIDCalls;

  for (CallInst *CI : VarTIDCalls) {
    auto *Caller = CI->getFunction();
    auto It = FuncToFixedTIDCalls.find(Caller);
    if (It == FuncToFixedTIDCalls.end()) {
      Builder.SetInsertPoint(Caller->getEntryBlock().getFirstNonPHI());
      SmallVector<Instruction *, 3> TIDs;
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
        TIDs.push_back(getWICall(&M, TIDName, CI->getType(), Dim, Builder,
                                 AppendWithDimension(Prefix, Dim)));
      bool Inserted;
      std::tie(It, Inserted) = FuncToFixedTIDCalls.insert({Caller, TIDs});
      assert(Inserted && "failed to insert TID calls");
    }

    auto *Arg = CI->getArgOperand(0);
    Builder.SetInsertPoint(CI);
    auto *Cmp = Builder.CreateICmpEQ(Arg, Builder.getInt32(0));
    auto *Select = Builder.CreateSelect(Cmp, It->second[0], ConstZero);
    Cmp = Builder.CreateICmpEQ(Arg, Builder.getInt32(1));
    Select = Builder.CreateSelect(Cmp, It->second[1], Select);
    Cmp = Builder.CreateICmpEQ(Arg, Builder.getInt32(2));
    Select = Builder.CreateSelect(Cmp, It->second[2], Select);

    CI->replaceAllUsesWith(Select);
    CI->eraseFromParent();
  }

  return !VarTIDCalls.empty() || !OOBTIDCalls.empty();
}

static bool runImpl(Module &M) {
  IRBuilder<> Builder(M.getContext());
  auto *ConstZero = ConstantInt::get(getIndTy(&M), 0);
  bool Changed = runOnTID(M, Builder, ConstZero, mangledGetLID(), "lid.");
  Changed |= runOnTID(M, Builder, ConstZero, mangledGetGID(), "gid.");
  Changed |= runOnTID(M, Builder, ConstZero, mangledGetGroupID(), "groupid.");
  return Changed;
}

PreservedAnalyses ResolveVarTIDCallPass::run(Module &M,
                                             ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
