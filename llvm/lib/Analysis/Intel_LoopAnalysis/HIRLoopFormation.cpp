//===------- HIRLoopFormation.cpp - Creates HIR Loops ---------------------===//
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
// This file implements the HIRLoopFormation pass.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/Instructions.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRRegionIdentification.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCleanup.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/HIRLoopFormation.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Passes.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"

using namespace llvm;
using namespace llvm::loopopt;

#define DEBUG_TYPE "hir-loop-formation"

INITIALIZE_PASS_BEGIN(HIRLoopFormation, "hir-loop-formation",
                      "HIR Loop Formation", false, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
INITIALIZE_PASS_DEPENDENCY(ScalarEvolutionWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRRegionIdentification)
INITIALIZE_PASS_DEPENDENCY(HIRCreation)
INITIALIZE_PASS_DEPENDENCY(HIRCleanup)
INITIALIZE_PASS_END(HIRLoopFormation, "hir-loop-formation",
                    "HIR Loop Formation", false, true)

char HIRLoopFormation::ID = 0;

FunctionPass *llvm::createHIRLoopFormationPass() {
  return new HIRLoopFormation();
}

HIRLoopFormation::HIRLoopFormation() : FunctionPass(ID) {
  initializeHIRLoopFormationPass(*PassRegistry::getPassRegistry());
}

void HIRLoopFormation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<ScalarEvolutionWrapperPass>();
  AU.addRequiredTransitive<HIRRegionIdentification>();
  AU.addRequiredTransitive<HIRCreation>();
  AU.addRequired<HIRCleanup>();
}

namespace {
struct LoopCompareLess {
  bool operator()(const HIRLoopFormation::LoopPairTy &LP1,
                  const HIRLoopFormation::LoopPairTy &LP2) {
    return std::less<const Loop *>()(LP1.first, LP2.first);
  }
};

struct LoopCompareEqual {
  bool operator()(const HIRLoopFormation::LoopPairTy &LP1,
                  const HIRLoopFormation::LoopPairTy &LP2) {
    return LP1.first == LP2.first;
  }
};
}

HLLoop *HIRLoopFormation::findOrInsertHLLoopImpl(const Loop *Lp, HLLoop *HLoop,
                                                 bool Insert) {
  LoopPairTy LoopPair(Lp, HLoop);

  if (Loops.empty()) {
    if (Insert) {
      Loops.push_back(LoopPair);
    }
    return nullptr;
  }

  auto I =
      std::lower_bound(Loops.begin(), Loops.end(), LoopPair, LoopCompareLess());

  if (I != Loops.end() && LoopCompareEqual()(*I, LoopPair)) {
    if (Insert) {
      assert(false && "Multiple insertions not expected!");
    }
    return I->second;
  }

  if (Insert) {
    Loops.insert(I, LoopPair);
  }

  return nullptr;
}

void HIRLoopFormation::insertHLLoop(const Loop *Lp, HLLoop *HLoop) {
  findOrInsertHLLoopImpl(Lp, HLoop, true);
}

HLLoop *HIRLoopFormation::findHLLoop(const Loop *Lp) {
  return findOrInsertHLLoopImpl(Lp, nullptr, false);
}

bool HIRLoopFormation::isNonNegativeNSWIV(const Instruction *Inst) const {
  auto SC = SE->getSCEV(const_cast<Instruction *>(Inst));

  auto AddRec = dyn_cast<SCEVAddRecExpr>(SC);

  if (!AddRec) {
    return false;
  }

  if (AddRec->getNoWrapFlags(SCEV::FlagNSW) &&
      SE->isKnownNonNegative(AddRec->getStart())) {
    return true;
  }

  return false;
}

bool HIRLoopFormation::hasNSWSemantics(const Loop *Lp,
                                       const PHINode *IVPhi) const {

  auto IVType = IVPhi->getType();

  if (IVType->isIntegerTy()) {
    if (isNonNegativeNSWIV(IVPhi)) {
      return true;
    }

  } else if (IVType->isPointerTy()) {

    // Set NSW if there is a non-negative integer IV in the loop header (less
    // than or equal to the size of the pointer IV) which has NSW flag set.
    // For example-
    //
    // %p.addr.07 = phi i32* [ %incdec.ptr, %for.body ], [ %p, %entry ] <<
    // pointer IV
    // %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ] <<
    // NSW IV of same size as pointer
    auto PtrSize = Func->getParent()->getDataLayout().getTypeSizeInBits(IVType);
    auto HeaderBB = Lp->getHeader();

    for (auto &PhiInst : (*HeaderBB)) {
      if (!isa<PHINode>(PhiInst)) {
        break;
      }

      auto PhiTy = PhiInst.getType();

      if (!PhiTy->isIntegerTy() ||
          (PhiTy->getPrimitiveSizeInBits() > PtrSize)) {
        continue;
      }

      if (isNonNegativeNSWIV(&PhiInst)) {
        return true;
      }
    }
  }

  return false;
}

void HIRLoopFormation::setIVType(HLLoop *HLoop) const {
  Value *Cond;
  auto Lp = HLoop->getLLVMLoop();
  auto Latch = Lp->getLoopLatch();

  assert(Latch && "Loop doesn't have a latch!");

  auto Term = Latch->getTerminator();

  if (auto BottomTest = dyn_cast<BranchInst>(Term)) {
    assert(BottomTest->isConditional() &&
           "Loop bottom test is not a conditional branch!");
    Cond = BottomTest->getCondition();
  } else {
    llvm_unreachable("Cannot handle loop bottom test!");
  }

  assert(isa<Instruction>(Cond) &&
         "Loop exit condition is not an instruction!");

  auto IVNode = RI->findIVDefInHeader(*Lp, cast<Instruction>(Cond));
  assert(IVNode && "Could not find loop IV!");

  auto IVType = IVNode->getType();

  // If the IVType is not an integer, assign it an integer type which is able to
  // represent the address space.
  if (!IVType->isIntegerTy()) {
    IVType = Type::getIntNTy(
        Func->getContext(),
        Func->getParent()->getDataLayout().getPointerSizeInBits());
  }

  HLoop->setIVType(IVType);

  // Set NSW flag, if applicable.
  auto IsNSW = hasNSWSemantics(Lp, IVNode);
  HLoop->setNSW(IsNSW);
}

bool HIRLoopFormation::populatedPreheaderPostexitNodes(
    HLLoop *HLoop, HLIf *IfParent, bool PredicateInversion) {

  auto PreBegIt =
      !PredicateInversion ? IfParent->then_begin() : IfParent->else_begin();
  auto PreEndIt = HLoop->getIterator();

  auto PostBegIt = std::next(HLoop->getIterator());
  auto PostEndIt =
      !PredicateInversion ? IfParent->then_end() : IfParent->else_end();

  auto &HNU = HLoop->getHLNodeUtils();

  bool HasPreheader = (PreBegIt != PreEndIt);
  bool HasPostexit = (PostBegIt != PostEndIt);

  if ((HasPreheader && !HNU.validPreheaderPostexitNodes(PreBegIt, PreEndIt)) ||
      (HasPostexit && !HNU.validPreheaderPostexitNodes(PostBegIt, PostEndIt))) {
    return false;
  }

  if (HasPreheader) {
    HNU.moveAsFirstPreheaderNodes(HLoop, PreBegIt, PreEndIt);
  }

  if (HasPostexit) {
    HNU.moveAsFirstPostexitNodes(HLoop, PostBegIt, PostEndIt);
  }

  return true;
}

void HIRLoopFormation::setZtt(HLLoop *HLoop) {

  auto Lp = HLoop->getLLVMLoop();
  bool PredicateInversion = false;

  // Check whether loop has an if parent.
  auto Parent = HLoop->getParent();

  if (!Parent) {
    return;
  }

  auto IfParent = dyn_cast<HLIf>(Parent);

  if (!IfParent) {
    return;
  }

  if (IfParent->hasElseChildren()) {
    if (IfParent->hasThenChildren()) {
      return;
    }
    PredicateInversion = true;
  }

  auto IfBB = HIR->getSrcBBlock(IfParent);
  auto IfBrInst = cast<BranchInst>(IfBB->getTerminator());

  if (!SE->isLoopZtt(Lp, IfBrInst, PredicateInversion)) {
    return;
  }

  // This function returns false if the condition is acting like a ztt but
  // cannot be set as one due to presence of non-HLInst nodes so we need to
  // bail out.
  if (!populatedPreheaderPostexitNodes(HLoop, IfParent, PredicateInversion)) {
    return;
  }

  // IfParent should only contain the loop now.
  assert(((!PredicateInversion && (IfParent->getNumThenChildren() == 1)) ||
          (PredicateInversion && (IfParent->getNumElseChildren() == 1))) &&
         "Something went wrong during ztt recognition!");

  HIR->getHLNodeUtils().moveBefore(IfParent, HLoop);
  HIR->getHLNodeUtils().remove(IfParent);

  HLoop->setZtt(IfParent);

  if (PredicateInversion) {
    InvertedZttLoops.insert(HLoop);
  }
}

void HIRLoopFormation::formLoops() {

  // Traverse RequiredLabels set computed by HIRCleanup phase to form loops.
  for (auto I = HIRC->getRequiredLabels().begin(),
            E = HIRC->getRequiredLabels().end();
       I != E; ++I) {
    BasicBlock *HeaderBB = (*I)->getSrcBBlock();

    if (!LI->isLoopHeader(HeaderBB)) {
      continue;
    }

    // Found a loop
    Loop *Lp = LI->getLoopFor(HeaderBB);

    auto BECount = SE->getBackedgeTakenCount(Lp);
    bool IsUnknownLoop = isa<SCEVCouldNotCompute>(BECount);
    bool IsConstTripLoop = isa<SCEVConstant>(BECount);

    // Find HIR hook for the loop latch.
    auto LatchHook = HIRC->findHIRHook(Lp->getLoopLatch());

    assert(((*I)->getParent() == LatchHook->getParent()) &&
           "Wrong lexical links built!");

    HLContainerTy::iterator LabelIter(*I);
    HLContainerTy::iterator BottomTestIter(LatchHook);

    // Look for the bottom test.
    while (!isa<HLIf>(BottomTestIter)) {
      BottomTestIter = std::next(BottomTestIter);
    }

    // Create a new loop and move its children inside.
    HLLoop *HLoop = HIR->getHLNodeUtils().createHLLoop(Lp);
    setIVType(HLoop);
    HLoop->setLoopMetadata(Lp->getLoopID());

    HLIf *BottomTest = cast<HLIf>(&*BottomTestIter);
    HLoop->setBranchDebugLoc(BottomTest->getDebugLoc());
    HLoop->setCmpTestDebugLoc(BottomTest->pred_begin()->DbgLoc);

    // Hook loop into HIR.
    HIR->getHLNodeUtils().insertBefore(&*LabelIter, HLoop);

    // Include Label and bottom test as explicit nodes inside the unknown loop.
    auto FirstChildIter = IsUnknownLoop ? LabelIter : std::next(LabelIter);
    auto EndIter = IsUnknownLoop ? std::next(BottomTestIter) : BottomTestIter;

    HIR->getHLNodeUtils().moveAsFirstChildren(HLoop, FirstChildIter, EndIter);

    if (!IsUnknownLoop) {
      // Remove label and bottom test.
      HIR->getHLNodeUtils().erase(&*LabelIter);

      // Can bottom test contain anything else??? Should probably assert on it.
      HIR->getHLNodeUtils().erase(&*BottomTestIter);

      // TODO: Look into whether setting ztt is beneficial for unknown loops.
      if (!IsConstTripLoop) {
        setZtt(HLoop);
      }
    }

    // Add entry for (Lp -> HLLoop) mapping.
    insertHLLoop(Lp, HLoop);
  }
}

bool HIRLoopFormation::runOnFunction(Function &F) {
  this->Func = &F;

  LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  SE = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  RI = &getAnalysis<HIRRegionIdentification>();
  HIR = &getAnalysis<HIRCreation>();
  HIRC = &getAnalysis<HIRCleanup>();

  formLoops();

  return false;
}

void HIRLoopFormation::releaseMemory() {
  Loops.clear();
  InvertedZttLoops.clear();
}

void HIRLoopFormation::print(raw_ostream &OS, const Module *M) const {
  HIR->print(OS, M);
}

void HIRLoopFormation::verifyAnalysis() const {
  /// TODO: implement later
}
