//==--- Intel_HandlePragmaVectorAligned.cpp --------------------*- C++ -*---==//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Scalar/Intel_HandlePragmaVectorAligned.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace {

class HandlePragmaVectorAligned {
public:
  HandlePragmaVectorAligned(LoopInfo *LI, ScalarEvolution *SE,
                            TargetTransformInfo *TTI)
      : LI(LI), SE(SE), TTI(TTI) {}
  bool runOnFunction(Function &F);

private:
  static bool hasPragmaVectorAlignedMetadata(MDNode *LoopID);
  void processAlignedLoop(const Loop *L);

private:
  LoopInfo *LI = nullptr;
  ScalarEvolution *SE = nullptr;
  TargetTransformInfo *TTI = nullptr;
  const DataLayout *DL = nullptr;
};

} // anonymous namespace

bool HandlePragmaVectorAligned::runOnFunction(Function &F) {
  DL = &F.getParent()->getDataLayout();

  bool Changed = false;
  for (const Loop *L : LI->getLoopsInPreorder()) {
    auto *LoopID = L->getLoopID();
    if (LoopID && hasPragmaVectorAlignedMetadata(LoopID)) {
      processAlignedLoop(L);
      Changed = true;
    }
  }
  return Changed;
}

/// Looks for !{!"llvm.loop.intel.vector.aligned"} metadata in LoopID.
bool HandlePragmaVectorAligned::hasPragmaVectorAlignedMetadata(MDNode *LoopID) {
  assert(LoopID->getOperand(0) == LoopID && "Invalid LoopID metadata");

  for (int I = 1, IE = LoopID->getNumOperands(); I < IE; ++I) {
    Metadata *MD = LoopID->getOperand(I);

    const MDTuple *T = dyn_cast<MDTuple>(MD);
    if (!T)
      continue;

    if (T->getNumOperands() != 1)
      continue;

    const MDString *S = dyn_cast<MDString>(T->getOperand(0));
    if (!S)
      continue;

    if (S->getString() != "llvm.loop.intel.vector.aligned")
      continue;

    return true;
  }

  return false;
}

void HandlePragmaVectorAligned::processAlignedLoop(const Loop *L) {
  // Only loops with a preheader are supported yet.
  BasicBlock *Preheader = L->getLoopPreheader();
  if (!Preheader)
    return;

  for (auto *BB : L->getBlocks()) {
    for (auto &I : *BB) {
      // Find a load/store instruction and extract its address.
      // getLoadStorePointerOperand returns null unless I is a load or store.
      Value *Address = getLoadStorePointerOperand(&I);
      if (!Address)
        continue;

      // Check that the address is an induction variable with regards to the
      // loop that is being processed.
      auto *AddRecExpr = dyn_cast<SCEVAddRecExpr>(SE->getSCEV(Address));
      if (!AddRecExpr || AddRecExpr->getLoop() != L)
        continue;

      // Check that the address is an affine (linear) expression.
      if (!AddRecExpr->isAffine())
        continue;

      // Check that the base address is a loop invariant value that is not
      // another SCEV expression (support for complex bases can be added later
      // if needed).
      auto *Base = dyn_cast<SCEVUnknown>(AddRecExpr->getStart());
      if (!Base)
        continue;

      // Make sure that the induction step is a compile-time constant.
      auto *StepExpr = dyn_cast<SCEVConstant>(AddRecExpr->getOperand(1));
      if (!StepExpr)
        continue;

      // Make sure that the access is unit stride.
      Type *AccessType =
          cast<PointerType>(Address->getType())->getElementType();
      auto AccessSize = DL->getTypeAllocSize(AccessType);
      if (AccessSize.isScalable())
        continue;

      if (StepExpr->getAPInt() != AccessSize.getFixedSize())
        continue;

      // Build an alignment assumption.
      Value *Invariant = Base->getValue();
      int Align =
        TTI->getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector) / 8;
      IRBuilder<> Builder(Preheader->getTerminator());
      Builder.CreateAlignmentAssumption(*DL, Invariant, Align);
    }
  }
}

namespace {
struct HandlePragmaVectorAlignedLegacyPass : public FunctionPass {
public:
  static char ID;

  HandlePragmaVectorAlignedLegacyPass() : FunctionPass(ID) {
    initializeHandlePragmaVectorAlignedLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LoopInfoWrapperPass>();
    AU.addRequired<ScalarEvolutionWrapperPass>();
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    auto *SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
    auto *TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);

    return HandlePragmaVectorAligned(LI, SE, TTI).runOnFunction(F);
  }
};
} // anonymous namespace

char HandlePragmaVectorAlignedLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HandlePragmaVectorAlignedLegacyPass,
                      "handle-pragma-vector-aligned",
                      "Handle Pragma Vector Aligned", false, false)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(HandlePragmaVectorAlignedLegacyPass,
                    "handle-pragma-vector-aligned",
                    "Handle Pragma Vector Aligned", false, false)

FunctionPass *llvm::createHandlePragmaVectorAlignedPass() {
  return new HandlePragmaVectorAlignedLegacyPass();
}

PreservedAnalyses
HandlePragmaVectorAlignedPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto *LI = &AM.getResult<LoopAnalysis>(F);
  auto *SE = &AM.getResult<ScalarEvolutionAnalysis>(F);
  auto *TTI = &AM.getResult<TargetIRAnalysis>(F);

  bool Changed = HandlePragmaVectorAligned(LI, SE, TTI).runOnFunction(F);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
