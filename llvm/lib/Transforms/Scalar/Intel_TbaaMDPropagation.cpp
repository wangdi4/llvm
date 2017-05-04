//===-- TbaaMDPropagation.cpp - TBAA recovery for return pointers implementation -===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass recovers the tbaa information for the return pointers and
// the corresponding intrinsic are cleaned up.
//
// Here is one example.
//
// struct S {
//   int a[4];
//   int b[4];
//
//   int& geta(int i) {
//     return a[i];
//   }
//   int& getb(int i) {
//     return b[i];
//   }
// };
//
// int foo(S& s, int i, int j) {
//   s.geta(i) = 0;
//   s.getb(j) = 1;
//   return s.geta(i);
// }
//
// This optmization is expected to generate the struct path tbaa
// for s.geta(i) and s.getb(j) as follows. 
//
// store i32 0, i32* %arrayidx.i, align 4, !tbaa !1
// store i32 1, i32* %arrayidx.i9, align 4, !tbaa !7
//
// !0 = !{!"clang version 4.0.0 (trunk 17977)"}
// !1 = !{!2, !4, i64 0}
// !2 = !{!"struct@_ZTS1S", !3, i64 0, !3, i64 16}
// !3 = !{!"array@_ZTSA4_i", !4, i64 0}
// !4 = !{!"int", !5, i64 0}
// !5 = !{!"omnipotent char", !6, i64 0}
// !6 = !{!"Simple C++ TBAA"}
// !7 = !{!2, !4, i64 16}
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_TbaaMDPropagation.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "tbaa-prop"

namespace {
struct TbaaMDPropagation : public FunctionPass,
                           public InstVisitor<TbaaMDPropagation> {
public:
  static char ID;
  TbaaMDPropagation() : FunctionPass(ID) {
    initializeTbaaMDPropagationPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F);
  void visitInstruction(Instruction &I) { return; }
  void visitCallInst(CallInst &CI);
  StringRef getPassName() const override { return "TBAAPROP"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }

private:
  friend class InstVisitor<TbaaMDPropagation>;
};
}
char TbaaMDPropagation::ID = 0;
INITIALIZE_PASS_BEGIN(TbaaMDPropagation, "tbaa-prop",
                      "Propagate the TbaaMD through intrinsic", false, false)
INITIALIZE_PASS_END(TbaaMDPropagation, "tbaa-prop",
                    "Propagate the TbaaMD through intrinsic", false, false)

FunctionPass *llvm::createTbaaMDPropagationPass() {
  return new TbaaMDPropagation();
}

PreservedAnalyses TbaaMDPropagationPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  auto PA = PreservedAnalyses();
  PA.preserve<GlobalsAA>();
  return PA;
}

bool TbaaMDPropagation::runOnFunction(Function &F) {
  for (BasicBlock &BB : F) {

    BasicBlock::iterator II, NextII, IE;

    for (II = BB.begin(), IE = BB.end(); II != IE;) {
      NextII = II++;
      Instruction *I = &*NextII;
      if (I)
        visit(I);
    }
  }
  return false;
}

// The tbaa information is retrieved from the fakeload intrinsic
// and attached to the pointer's dereference sites. 
void TbaaMDPropagation::visitCallInst(CallInst &CI) {
  MDNode *P;
  const Function *Callee = CI.getCalledFunction();
  if (!Callee)
    return;
  switch (Callee->getIntrinsicID()) {
  case Intrinsic::intel_fakeload:
    P = dyn_cast<MDNode>(
        cast<MetadataAsValue>(CI.getOperand(1))->getMetadata());
    for (auto I = CI.use_begin(), E = CI.use_end(); I != E;) {
      Use &U = *I++;
      Instruction *User = cast<Instruction>(U.getUser());

      LoadInst *LI = dyn_cast<LoadInst>(User);
      if (LI && LI->getPointerOperand() == &CI) {
        LI->setMetadata(LLVMContext::MD_tbaa, P);
        continue;
      }
      StoreInst *SI = dyn_cast<StoreInst>(User);
      if (SI && SI->getPointerOperand() == &CI) {
        SI->setMetadata(LLVMContext::MD_tbaa, P);
        continue;
      }
    }
    CI.replaceAllUsesWith(CI.getOperand(0));
    CI.eraseFromParent();
    break;
  default:
    break;
  }
}
