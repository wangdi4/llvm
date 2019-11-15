//===-- TbaaMDPropagation.cpp - TBAA recovery for return pointers implementation -===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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
// The front end inserts intel.fakeload intrinsics that wrap the value
// passed to return statements in order to allow us to propagate the
// TBAA metadata after the functions have been inlined.  In order for
// this to work correctly, this pass cannot remove the fakeload intrinsic
// until after a function has been inlined.  However, we would like to
// run the pass before SROA (which is a pre-inlining function simplification
// pass) because the fakeload intrinsic blocks SROA.
//
// To solve this problem, tha TbaaMDPropagation pass only operates on intrinsics
// that have uses that are not return instructions, which is an indication that
// the function containing the intrinsic has been inlined.  The CleanupFakeLoads
// pass should be run after all inlining is complete to remove remaining
// intrinsics.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/Intel_TbaaMDPropagation.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h" // INTEL
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/CFG.h"

using namespace llvm;

namespace {

struct TbaaMDPropagationImpl : public InstVisitor<TbaaMDPropagationImpl> {
public:
  void visitInstruction(Instruction &I) { return; }
  void visitLoad(LoadInst &LI);
  void visitStore(StoreInst &SI);
  void visitIntrinsicInst(IntrinsicInst &II);

private:
  friend class InstVisitor<TbaaMDPropagationImpl>;
};

/// \Returns combined !intel-tbaa metadata for a chain of GEPs starting at \p
/// GEP.
MDNode *getGepChainTBAA(const GetElementPtrInst *GEP) {
  MDNode *GepMD = GEP->getMetadata(LLVMContext::MD_intel_tbaa);
  if (!GepMD)
    return nullptr;
  const auto *BaseGEP = dyn_cast<GetElementPtrInst>(GEP->getPointerOperand());
  if (!BaseGEP)
    return GepMD;

  return mergeIntelTBAA(getGepChainTBAA(BaseGEP), GepMD);
}

// The tbaa information is retrieved from the fakeload intrinsic
// and attached to the pointer's dereference sites.
void TbaaMDPropagationImpl::visitIntrinsicInst(IntrinsicInst &II) {
  if (II.getIntrinsicID() != Intrinsic::intel_fakeload)
    return;

  MDNode *FakeloadTBAA = dyn_cast<MDNode>(
      cast<MetadataAsValue>(II.getArgOperand(1))->getMetadata());

  if (const auto *GEP = dyn_cast<GetElementPtrInst>(II.getArgOperand(0))) {
    // Try to refine TBAA info of the GEP chain feeding our fakeload.
    MDNode *MergedTBAA =
        getMostSpecificTBAA(getGepChainTBAA(GEP), FakeloadTBAA);
    if (MergedTBAA != FakeloadTBAA)
      II.setArgOperand(1, MetadataAsValue::get(GEP->getContext(), MergedTBAA));
  }

  // If the only user is a return instruction, we aren't at the right level
  // of inlining yet.
  if (II.hasOneUse() && isa<ReturnInst>(II.user_back()))
    return;
  FakeloadTBAA = dyn_cast<MDNode>(
      cast<MetadataAsValue>(II.getArgOperand(1))->getMetadata());
  bool HasRetUser = false;
  for (auto *User : II.users()) {
    LoadInst *LI = dyn_cast<LoadInst>(User);
    if (LI && LI->getPointerOperand() == &II) {
      LI->setMetadata(LLVMContext::MD_tbaa, FakeloadTBAA);
      continue;
    }
    StoreInst *SI = dyn_cast<StoreInst>(User);
    if (SI && SI->getPointerOperand() == &II) {
      SI->setMetadata(LLVMContext::MD_tbaa, FakeloadTBAA);
      continue;
    }
    // If there is a return instruction user but it wasn't the only user we
    // need to keep the fakeload intrinsic to prepare for the next level of
    // inlining.
    if (isa<ReturnInst>(User))
      HasRetUser = true;
  }
  // If the value passed through the intrinsic is not used by a return
  // instruction, we can remove the intrinsic now.
  if (!HasRetUser) {
    II.replaceAllUsesWith(II.getArgOperand(0));
    II.eraseFromParent();
  }
}

// Helper function to unify processing of both Load/Store.
/// Refine and propagate TBAA information from the \p MemInst's \p PointerOp
/// pointer operand.
void propagateIntelTBAAToMemInst(Instruction &MemInst, Value *PointerOp) {
  const auto *GEP = dyn_cast<GetElementPtrInst>(PointerOp);
  if (!GEP)
    return;

  MDNode *MemInstTBAA = MemInst.getMetadata(LLVMContext::MD_tbaa);

  // MemInst's TU was compiled without TBAA support, can't legally propagate
  // information from the GEP.
  if (!MemInstTBAA)
    return;

  MDNode *MergedTBAA = getMostSpecificTBAA(getGepChainTBAA(GEP), MemInstTBAA);
  if (MemInstTBAA != MergedTBAA)
    MemInst.setMetadata(LLVMContext::MD_tbaa, MergedTBAA);
}

void TbaaMDPropagationImpl::visitLoad(LoadInst &LI) {
  propagateIntelTBAAToMemInst(LI, LI.getPointerOperand());
}

void TbaaMDPropagationImpl::visitStore(StoreInst &SI) {
  propagateIntelTBAAToMemInst(SI, SI.getPointerOperand());
}

bool runTbaaMDPropagation(Function &F) {
  TbaaMDPropagationImpl impl;
  for (BasicBlock *BB : depth_first(&F.getEntryBlock())) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE;) {
      // Because we might be erasing the instruction, we need to get the
      // instruction reference first and then increment the iterator before
      // visiting the instruction.
      Instruction &I = *II;
      ++II;
      impl.visit(&I);
    }
  }
  return false; // FIXME: Should this return true when something changes?
}

struct TbaaMDPropagationLegacyPass : public FunctionPass {
public:
  static char ID;
  TbaaMDPropagationLegacyPass() : FunctionPass(ID) {
    initializeTbaaMDPropagationLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  StringRef getPassName() const override { return "TBAAPROP"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};

} // end anonymous namespace

char TbaaMDPropagationLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(TbaaMDPropagationLegacyPass, "tbaa-prop",
                      "Propagate the TbaaMD through intrinsic", false, false)
INITIALIZE_PASS_END(TbaaMDPropagationLegacyPass, "tbaa-prop",
                    "Propagate the TbaaMD through intrinsic", false, false)

FunctionPass *llvm::createTbaaMDPropagationLegacyPass() {
  return new TbaaMDPropagationLegacyPass();
}

bool TbaaMDPropagationLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  return runTbaaMDPropagation(F);
}

PreservedAnalyses TbaaMDPropagationPass::run(Function &F,
                                             FunctionAnalysisManager &AM) {
  if (!runTbaaMDPropagation(F))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<GlobalsAA>();
  return PA;
}

static bool runCleanupFakeLoads(Function &F) {
  for (BasicBlock *BB : depth_first(&F.getEntryBlock())) {
    for (auto II = BB->begin(), IE = BB->end(); II != IE;) {
      // Because we might be erasing the instruction, we need to get the
      // instruction reference first and then increment the iterator before
      // processing the instruction.
      Instruction &I = *II;
      ++II;
      if (auto *Intrin = dyn_cast<IntrinsicInst>(&I)) {
        if (Intrin->getIntrinsicID() == Intrinsic::intel_fakeload) {
          I.replaceAllUsesWith(I.getOperand(0));
          I.eraseFromParent();
        }
      }
    }
  }
  return false; // FIXME: Should this return true when something changes?
}

PreservedAnalyses CleanupFakeLoadsPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  if (!runCleanupFakeLoads(F))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<GlobalsAA>();
  return PA;
}

namespace {
struct CleanupFakeLoadsLegacyPass : public FunctionPass {
public:
  static char ID;
  CleanupFakeLoadsLegacyPass() : FunctionPass(ID) {
    initializeCleanupFakeLoadsLegacyPassPass(*PassRegistry::getPassRegistry());
  }
  bool runOnFunction(Function &F) override;
  StringRef getPassName() const override { return "Cleanup fake loads"; }
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }
};
} // end anonymous namespace

char CleanupFakeLoadsLegacyPass::ID = 0;
INITIALIZE_PASS(CleanupFakeLoadsLegacyPass, "cleanup-fakeloads",
                "Remove intel.fakeload intrinsics", false, false)

FunctionPass *llvm::createCleanupFakeLoadsPass() {
  return new CleanupFakeLoadsLegacyPass();
}

bool CleanupFakeLoadsLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  return runCleanupFakeLoads(F);
}
