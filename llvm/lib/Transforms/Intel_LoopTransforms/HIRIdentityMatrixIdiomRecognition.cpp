//===- HIRIdentityMatrixIdiomRecognition.cpp Implements IdentityMatrixIdiomRecognition class -===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass recognizes identity matrix.
//
// We are transforming this-
//
// Do i1 = 0, 100
//   Do i2 = 0, 100
//     %. = (i1 == i2) ? 1.000000e+00 : 0.000000e+00;
//     (@A)[0][i1][i2] = %.;
//   ENDDO
// ENDDO
//
// To-
//
//  Do i1 = 0, 100
//    Do i2 = 0, 100
//      (@A)[0][i1][i2] = 0.000000e+00;
//    ENDDO
//  ENDDO
//
//  DO i1 = 0, 100
//   (@A)[0][i1][i1] = 1.000000e+00;
//  ENDDO
//
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRIdentityMatrixIdiomRecognition.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#define OPT_SWITCH "hir-identity-matrix-idiom-recognition"
#define OPT_DESC "HIR Identity Matrix Idiom Recognition"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

namespace {

class HIRIdentityMatrixIdiomRecognition {
  HIRFramework &HIRF;

public:
  HIRIdentityMatrixIdiomRecognition(HIRFramework &HIRF) : HIRF(HIRF) {}

  bool run();

private:
  bool doIdentityMatrixIdiomRecognition(HLLoop *OuterLoop, HLLoop *InnerLoop);
  bool isLegal(HLLoop *InnerLoop, RegDDRef *&TRef, RegDDRef *&FRef,
               HLInst *&SelectHInst) const;
};
} // namespace

// Check whether the inner loop has the pattern like this -
//
//  %. = (i1 == i2) ? 1.000000e+00 : 0.000000e+00;
// (@A)[0][i1][i2] = %.;
//
// and return the true and false Refs and SelectHInst.
bool HIRIdentityMatrixIdiomRecognition::isLegal(HLLoop *InnerLoop,
                                                RegDDRef *&TRef,
                                                RegDDRef *&FRef,
                                                HLInst *&SelectHInst) const {
  if (InnerLoop->getNumChildren() != 2) {
    return false;
  }

  SelectHInst = dyn_cast<HLInst>(InnerLoop->getFirstChild());
  auto Inst = SelectHInst ? SelectHInst->getLLVMInstruction() : nullptr;

  if (!(Inst && isa<SelectInst>(Inst))) {
    return false;
  }

  // Check: comparison is != or ==
  const HLPredicate Pred = SelectHInst->getPredicate();
  if (!(Pred == CmpInst::ICMP_NE || Pred == CmpInst::ICMP_EQ)) {
    return false;
  }

  RegDDRef *TmpRef = SelectHInst->getOperandDDRef(0);

  if (!TmpRef->isTerminalRef()) {
    return false;
  }

  RegDDRef *LHSRef = SelectHInst->getOperandDDRef(1);
  RegDDRef *RHSRef = SelectHInst->getOperandDDRef(2);
  RegDDRef *TrueRef = SelectHInst->getOperandDDRef(3);
  RegDDRef *FalseRef = SelectHInst->getOperandDDRef(4);

  if (Pred == CmpInst::ICMP_NE) {
    std::swap(TrueRef, FalseRef);
  }

  unsigned InnerLevel = InnerLoop->getNestingLevel();
  unsigned OuterLevel = InnerLevel - 1;
  unsigned Level;

  if (!((LHSRef->isStandAloneIV(false, &Level) && Level == OuterLevel &&
         RHSRef->isStandAloneIV(false, &Level) && Level == InnerLevel) ||
        (RHSRef->isStandAloneIV(false, &Level) && Level == OuterLevel &&
         LHSRef->isStandAloneIV(false, &Level) && Level == InnerLevel))) {
    return false;
  }

  if (!TrueRef->isTerminalRef() || !FalseRef->isTerminalRef() ||
      TrueRef->hasIV(InnerLevel) || TrueRef->hasIV(OuterLevel)) {
    return false;
  }

  auto SecondInst = dyn_cast<HLInst>(InnerLoop->getLastChild());

  if (!SecondInst) {
    return false;
  }

  RegDDRef *LRef = SecondInst->getLvalDDRef();
  RegDDRef *RRef = SecondInst->getRvalDDRef();

  if (!isa<StoreInst>(SecondInst->getLLVMInstruction()) ||
      !DDRefUtils::areEqual(RRef, TmpRef)) {
    return false;
  }

  unsigned NumDims = LRef->getNumDimensions();

  if (NumDims < 2) {
    return false;
  }

  CanonExpr *FirstCE = LRef->getDimensionIndex(1);
  CanonExpr *SecondCE = LRef->getDimensionIndex(2);

  uint64_t InnerLoopTripCount = 0;

  InnerLoop->isConstTripLoop(&InnerLoopTripCount);

  if (LRef->getNumDimensionElements(1) != InnerLoopTripCount) {
    return false;
  }

  if (!(FirstCE->isStandAloneIV(false, &Level) && Level == InnerLevel) ||
      !(SecondCE->isStandAloneIV(false, &Level) && Level == OuterLevel)) {
    return false;
  }

  for (unsigned I = 3; I <= NumDims; I++) {
    CanonExpr *CE = LRef->getDimensionIndex(I);
    if (CE->hasIV(OuterLevel) || CE->hasIV(InnerLevel)) {
      return false;
    }
  }

  TRef = TrueRef;
  FRef = FalseRef;

  return true;
}

bool HIRIdentityMatrixIdiomRecognition::doIdentityMatrixIdiomRecognition(
    HLLoop *OuterLoop, HLLoop *InnerLoop) {
  RegDDRef *TrueRef = nullptr;
  RegDDRef *FalseRef = nullptr;
  HLInst *SelectHInst = nullptr;

  if (!isLegal(InnerLoop, TrueRef, FalseRef, SelectHInst)) {
    return false;
  }

  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics>(InnerLoop);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion<HIRLoopStatistics>(
      OuterLoop);

  SelectHInst->removeOperandDDRef(3);
  SelectHInst->removeOperandDDRef(4);

  HLNodeUtils::remove(SelectHInst);

  HLInst *HInst = cast<HLInst>(InnerLoop->getLastChild());
  HInst->setRvalDDRef(FalseRef);

  HLLoop *DiagonalLoop = InnerLoop->clone();

  HLInst *DiagonalInst = cast<HLInst>(DiagonalLoop->getFirstChild());
  DiagonalInst->setRvalDDRef(TrueRef);

  RegDDRef *LvalDDRef = DiagonalInst->getLvalDDRef();

  auto I = LvalDDRef->canon_begin();

  unsigned InnerLevel = InnerLoop->getNestingLevel();
  unsigned OuterLevel = InnerLevel - 1;

  (*I)->setIVCoeff(OuterLevel, 0, 1);
  (*I)->removeIV(InnerLevel);

  HLNodeUtils::insertAfter(OuterLoop, DiagonalLoop);

  return true;
}

bool HIRIdentityMatrixIdiomRecognition::run() {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << "HIR Identity Matrix Idiom Recognition Disabled \n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "HIR Identity Matrix Idiom Recognition on Function : "
                    << HIRF.getFunction().getName() << "\n");

  // Gather all inner-most loop and its parent loop as Candidates
  SmallVector<HLLoop *, 64> InnermostLoops;

  HLNodeUtils &HNU = HIRF.getHLNodeUtils();
  HNU.gatherInnermostLoops(InnermostLoops);

  if (InnermostLoops.empty()) {
    LLVM_DEBUG(dbgs() << HIRF.getFunction().getName()
                      << "() has no inner-most loop\n ");
    return false;
  }

  bool Result = false;

  for (auto &Lp : InnermostLoops) {
    if (!Lp->isNormalized() || Lp->hasUnrollEnablingPragma() ||
        Lp->hasVectorizeEnablingPragma()) {
      continue;
    }

    HLLoop *OuterLoop = Lp->getParentLoop();
    if (!OuterLoop) {
      continue;
    }

    uint64_t InnerLoopTripCount = 0;
    uint64_t OuterLoopTripCount = 0;

    // TODO:  Handle loops with different trip counts, take the minimumn trip
    // count of the two loops for generating the 2nd loop.
    if (!(Lp->isConstTripLoop(&InnerLoopTripCount) &&
          OuterLoop->isConstTripLoop(&OuterLoopTripCount) &&
          InnerLoopTripCount == OuterLoopTripCount)) {
      continue;
    }

    if (!OuterLoop->isNormalized() || OuterLoop->hasVectorizeEnablingPragma() ||
        OuterLoop->hasUnrollEnablingPragma() ||
        OuterLoop->hasUnrollAndJamEnablingPragma()) {
      continue;
    }

    if (!HLNodeUtils::isPerfectLoopNest(OuterLoop)) {
      continue;
    }

    Result = doIdentityMatrixIdiomRecognition(OuterLoop, Lp) || Result;
  }

  return Result;
}

PreservedAnalyses
HIRIdentityMatrixIdiomRecognitionPass::run(llvm::Function &F,
                                           llvm::FunctionAnalysisManager &AM) {
  HIRIdentityMatrixIdiomRecognition(AM.getResult<HIRFrameworkAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRIdentityMatrixIdiomRecognitionLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRIdentityMatrixIdiomRecognitionLegacyPass() : HIRTransformPass(ID) {
    initializeHIRIdentityMatrixIdiomRecognitionLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.setPreservesAll();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRIdentityMatrixIdiomRecognition(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR())
        .run();
  }
};

char HIRIdentityMatrixIdiomRecognitionLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRIdentityMatrixIdiomRecognitionLegacyPass, OPT_SWITCH,
                      OPT_DESC, false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_END(HIRIdentityMatrixIdiomRecognitionLegacyPass, OPT_SWITCH,
                    OPT_DESC, false, false)

FunctionPass *llvm::createHIRIdentityMatrixIdiomRecognitionPass() {
  return new HIRIdentityMatrixIdiomRecognitionLegacyPass();
}
