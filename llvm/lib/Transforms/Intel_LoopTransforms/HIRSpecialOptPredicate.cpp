//===---------------- HIRSpecialOptPredicate.cpp ----------------------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
// ===--------------------------------------------------------------------===//
//
// The pass checks for a very specific pattern where a deeply nested loop
// executes only if ((v*v+u*u) <= size) and u and v are the innermost 2
// loop IVs. The entire loop body is contained inside the innermost loop if.
// The actual source code looks like the following:

// for (v = (-height/2 to height/2) {
//   for (u = (-width/2 to width/2) {
//     if ((v*v+u*u) <= (width/2)*(height/2)) {
//       ...
//     }
//   }
// }
//
// Because v*v and u*u are positive and monotonicly increasing, for any value of
// -/+ v, we can calculate the exact value between -u and +u where ((v*v+u*u) <=
// size) holds. E.g. suppose size is 40 and v = 5, then the u loop must execute
// only when u*u <= 40 - 5*5, which means u*u <= 15, so u spans [-3:+3]. Suppose
// for the original loop, -u starts at any negative value. In this case, once
// -u >= -3, we would enter the loop for that value of -u (from -u to u).
// Otherwise we never enter the loop.

// The transformation is the following:

// for (v = (-height/2 to height/2))
//   lb = -1
//   ub = -1
//   for (u = (-width/2 to width/2))
//     if ((v*v+u*u) <= ((width/2)*(height/2)))
//       lb = -u
//       ub = u
//       break;

//   for (w = (lb to ub))
//     ... Original code inside if goes here

// The HIR looks a bit different:
//   DO i4 = 0, 2 * %65, 1   <DO_LOOP>
//     %162 = i4 + -1 * %65  *  i4 + -1 * %65;
//     ...
//
//     + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
//     |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
//     |   %180 = %179  +  %162;
//     |   if (%180 <= ((%1 /u 2) * (%2 /u 2))) {
//     |     ...
//     |   }

// The transformed code looks like:
//     + DO i4 = 0, 2 * %inst63, 1   <DO_LOOP>
//     |   %inst160 = i4 + -1 * %inst63  *  i4 + -1 * %inst63;
//     |   ...
//     |   %optprd.lower = -1;
//     |   %optprd.upper = -1;
//     |
//     |   + DO i5 = 0, 2 * %inst66, 1   <DO_MULTI_EXIT_LOOP>
//     |   |   %inst177 = i5 + -1 * %inst66  *  i5 + -1 * %inst66;
//     |   |   %inst178 = %inst177  +  %inst160;
//     |   |   if (%inst178 <= ((%arg1 /u 2) * (%arg2 /u 2)))
//     |   |   {
//     |   |      %optprd.upper = -1 * i5 + 2 * %inst66;
//     |   |      %optprd.lower = i5;
//     |   |      goto loopexit.248;
//     |   |   }
//     |   + END LOOP
//     |
//     |   loopexit.248:
//     |
//           Implicit i5 Ztt: if (%optprd.lower != -1)
//     |   + DO i5 = %optprd.lower, %optprd.upper, 1   <DO_LOOP>
//     |      <Original loop body here>

// Notice we have an i5 Ztt on the second loop to prevent the loop from
// executing the innerloop body when we never would have entered the
// if in the first place.

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRSpecialOptPredicatePass.h"

#define OPT_SWITCH "hir-special-opt-predicate"
#define OPT_DESC "HIR Special Opt Predicate Pass"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

class HIRSpecialOptPredicate {
public:
  HIRSpecialOptPredicate(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();
  bool isCandidate(const HLLoop *InnerLoop) const;
  void replaceIfWithBoundsLoop(HLLoop *FirstLoop, HLLoop *SecondLoop, HLIf *If);
  void doTransformation(HLLoop *InnerLoop);

private:
  HIRFramework &HIRF;
};

// Returns true if the \p Loop and the first instruction is computing the
// square of the IV between the height/width range. All of the refs used
// to compute the square should be linear at \LinearLevel.
// In HIR it looks like this:
//
//   DO i4 = 0, 2 * %65, 1   <DO_LOOP>
//     %162 = i4 + -1 * %65  *  i4 + -1 * %65;
//     ...
//
//     + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
//     |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
static bool hasIVSquaredPattern(const HLLoop *Loop, unsigned LinearLevel) {
  if (!Loop->isDo() || !Loop->isNormalized() || Loop->hasPreheader() ||
      Loop->hasPostexit() || Loop->hasZtt()) {
    return false;
  }

  // Pattern match Loop Upper
  // Check for UBCE: 2 * %65
  const CanonExpr *UBCE = Loop->getUpperCanonExpr();
  if (!UBCE->isSingleBlob()) {
    return false;
  }

  unsigned UpperBlobIndex = UBCE->getSingleBlobIndex();
  if (UBCE->getBlobCoeff(UpperBlobIndex) != 2 ||
      !UBCE->isLinearAtLevel(LinearLevel)) {
    return false;
  }

  // Check that first inst looks like: %162 = (i4 + -1 * %65)  *  (i4 + -1 *
  // %65); where %65 is the blob from the corresponding Loop Upper
  const auto *FirstInst = dyn_cast<HLInst>(Loop->getFirstChild());
  if (!FirstInst) {
    return false;
  }

  const auto *LvalRef = FirstInst->getLvalDDRef();
  if (!LvalRef || !LvalRef->isTerminalRef()) {
    return false;
  }

  const auto *LLVMInst = FirstInst->getLLVMInstruction();
  if (LLVMInst->getOpcode() != Instruction::Mul) {
    return false;
  }

  // Get the 2 RvalRefs from our MulInst
  const RegDDRef *RvalRef1 = FirstInst->getOperandDDRef(1);
  const RegDDRef *RvalRef2 = FirstInst->getOperandDDRef(2);

  if (!RvalRef1->isTerminalRef() || !DDRefUtils::areEqual(RvalRef1, RvalRef2)) {
    return false;
  }

  unsigned Level = Loop->getNestingLevel();
  const auto *CE = RvalRef1->getSingleCanonExpr();
  unsigned IVBlobIndex = InvalidBlobIndex;
  int64_t IVCoeff = 0;
  CE->getIVCoeff(Level, &IVBlobIndex, &IVCoeff);
  // Match i4 + -1 * %65
  if (CE->numBlobs() != 1 || CE->numIVs() != 1 || CE->getDenominator() != 1 ||
      CE->getConstant() || IVCoeff != 1 || IVBlobIndex != InvalidBlobIndex) {
    return false;
  }

  // Check blob looks like: -1 * %65
  if (CE->getSingleBlobIndex() != UpperBlobIndex ||
      CE->getBlobCoeff(UpperBlobIndex) != -1) {
    return false;
  }

  return true;
}

// Check and match the following pattern corresponding to:
// for (v = (-height/2 to height/2)
//   for (u = (-width/2 to width/2)
//     if ((v*v+u*u) <= (width/2)*(height/2)))
//
// HIR we want to match is as follows:
//   DO i4 = 0, 2 * %65, 1   <DO_LOOP>
//     %162 = i4 + -1 * %65  *  i4 + -1 * %65;
//     ...
//
//     + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
//     |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
//     |   %180 = %179  +  %162;
//     |   if (%180 <= ((%1 /u 2) * (%2 /u 2)))
bool hasMatchingPredicate(const HLLoop *InnerLoop) {
  // We already checked the first instruction of each loop. Check
  // second instruction of innerloop for sum of the squares.
  const HLNode *FirstChild = InnerLoop->getFirstChild();
  const auto *OuterLoopFirstInst =
      cast<HLInst>(InnerLoop->getParentLoop()->getFirstChild());

  unsigned VIndex = OuterLoopFirstInst->getLvalDDRef()->getSelfBlobIndex();
  unsigned UIndex =
      (cast<HLInst>(FirstChild))->getLvalDDRef()->getSelfBlobIndex();

  const auto *SumInst = dyn_cast<HLInst>(FirstChild->getNextNode());
  if (!SumInst ||
      SumInst->getLLVMInstruction()->getOpcode() != Instruction::Add) {
    return false;
  }

  const RegDDRef *RvalRef1 = SumInst->getOperandDDRef(1);
  const RegDDRef *RvalRef2 = SumInst->getOperandDDRef(2);

  if (!RvalRef1->isSelfBlob() || !RvalRef2->isSelfBlob()) {
    return false;
  }

  unsigned Index1 = RvalRef1->getSelfBlobIndex();
  unsigned Index2 = RvalRef2->getSelfBlobIndex();

  // Match operands of the Add to the Refs corresponding to the Squares of U/V
  if (!((Index1 == VIndex && Index2 == UIndex) ||
        (Index1 == UIndex && Index2 == VIndex))) {
    return false;
  }

  const RegDDRef *LvalRef = SumInst->getLvalDDRef();
  if (!LvalRef->isSelfBlob()) {
    return false;
  }

  unsigned SumIndex = LvalRef->getSelfBlobIndex();
  const auto *If = dyn_cast<HLIf>(SumInst->getNextNode());
  if (!If || If->hasElseChildren() || If->getNumPredicates() != 1) {
    return false;
  }

  auto PredI = If->pred_begin();
  const RegDDRef *LHS = If->getLHSPredicateOperandDDRef(PredI);
  const RegDDRef *RHS = If->getRHSPredicateOperandDDRef(PredI);

  if (!LHS->isSelfBlob() || LHS->getSelfBlobIndex() != SumIndex ||
      !ICmpInst::isLE(*PredI)) {
    return false;
  }

  // RHS looks like this: ((%1 /u 2) * (%2 /u 2)))
  // Don't check RHS terms as blobs do not match loop uppers and transformation
  // is still valid as long as it is invariant.
  if (!RHS->isTerminalRef() ||
      !RHS->isStructurallyInvariantAtLevel(InnerLoop->getNestingLevel() - 1)) {
    return false;
  }

  return true;
}

// Find our candidate for optimization. We are looking for code resembling:
// for (v=(-((ssize_t) height/2)); v <= (((ssize_t) height/2)); v++)
//   for (u=(-((ssize_t) width/2)); u <= (((ssize_t) width/2)); u++)
//     if ((v*v+u*u) <= ((width/2)*(height/2)))

// After normalization in HIR, it looks like:
//
//   DO i4 = 0, 2 * %65, 1   <DO_LOOP>
//     %162 = i4 + -1 * %65  *  i4 + -1 * %65;
//     %164 = sitofp.i64.double(i4 + -1 * %65);
//     %165 = %hir.de.ssa.copy13.out  +  %164;
//     %167 = i4 + -1 * %65 + %150 < 0;
//
//     + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
//     |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
//     |   %180 = %179  +  %162;
//     |   if (%180 <= ((%1 /u 2) * (%2 /u 2)))
//     |   {
bool HIRSpecialOptPredicate::isCandidate(const HLLoop *InnerLoop) const {

  unsigned LoopLevel = InnerLoop->getNestingLevel();
  if (LoopLevel != 5) {
    return false;
  }

  if (InnerLoop->getNumChildren() != 3) {
    return false;
  }

  const HLLoop *OuterLoop = InnerLoop->getParentLoop();
  unsigned OuterLevel = OuterLoop->getNestingLevel();

  if (!hasIVSquaredPattern(InnerLoop, OuterLevel) ||
      !hasIVSquaredPattern(OuterLoop, OuterLevel)) {
    LLVM_DEBUG(dbgs() << "Not IV Squared Pattern!\n";);
    return false;
  }

  if (!hasMatchingPredicate(InnerLoop)) {
    LLVM_DEBUG(dbgs() << "Not Expected Predicate!\n";);
    return false;
  }

  LLVM_DEBUG(dbgs() << "Found Candidate!\n"; OuterLoop->dump(););
  return true;
}

// Logic for the setup loop will look like this:
//
//   lb = -1
//   ub = -1
//   for (u = (-width/2 to width/2))
//     if ((v*v+u*u) <= ((width/2)*(height/2)))
//       lb = -u
//       ub = u
//       goto exit;
//   label exit
//
//   DO NewMainLoop
void HIRSpecialOptPredicate::replaceIfWithBoundsLoop(HLLoop *FirstLoop,
                                                     HLLoop *SecondLoop,
                                                     HLIf *If) {
  auto &HNU = HIRF.getHLNodeUtils();
  auto &DDRU = FirstLoop->getDDRefUtils();

  Type *Ty = FirstLoop->getUpperDDRef()->getDestType();
  RegDDRef *NewLowerRef = HNU.createTemp(Ty, "optprd.lower");
  RegDDRef *NewUpperRef = HNU.createTemp(Ty, "optprd.upper");

  // lb = -1
  // ub = -1
  auto *LowerInitInst =
      HNU.createCopyInst(DDRU.createConstDDRef(Ty, -1), "copy", NewLowerRef);
  auto *UpperInitInst =
      HNU.createCopyInst(DDRU.createConstDDRef(Ty, -1), "copy", NewUpperRef);

  HLNodeUtils::insertBefore(FirstLoop, LowerInitInst);
  HLNodeUtils::insertBefore(FirstLoop, UpperInitInst);

  unsigned InnerLevel = FirstLoop->getNestingLevel();

  NewLowerRef->makeConsistent({}, InnerLevel - 1);
  NewUpperRef->makeConsistent({}, InnerLevel - 1);

  // add new temps as liveout
  FirstLoop->addLiveOutTemp(NewLowerRef);
  FirstLoop->addLiveOutTemp(NewUpperRef);

  // In normalized HIR, it will look like:
  //     + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
  //     |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
  //     |   %180 = %179  +  %162;
  //     |   if (%180 <= ((%1 /u 2) * (%2 /u 2))) {
  //     |      %optprd.lower = i5
  //     |      %optprd.upper = -1 * i5 + 2 * %68;
  //     |      goto %loopexit
  //
  // We are trying to capture the range from -u to +u where u*u < threshold.
  // One way to contextualize that the LB and UB are correct is as follows:
  // In normalized HIR, the IV goes from 0 to 2*D. Once we find the first u
  // value that satisfies the condition, the last U that satisfies the same
  // condition is equidistant from the upperbound = 2*D - u. If we were to
  // un-normalize the values by D, we'd see the LB = u-D and the UB = D-u,
  // which makes sense as LB = -UB and satisfies u*u < threshold.
  auto *LowerIVRef = DDRU.createNullDDRef(Ty);
  auto *LoopLowerSetter =
      HNU.createCopyInst(LowerIVRef, "copy", NewLowerRef->clone());
  LowerIVRef->getSingleCanonExpr()->setIVCoeff(InnerLevel, InvalidBlobIndex, 1);
  // Make consistent symbase
  LowerIVRef->makeConsistent({}, InnerLevel);

  RegDDRef *UpperIVRef = FirstLoop->getUpperDDRef()->clone();
  UpperIVRef->getSingleCanonExpr()->setIVCoeff(InnerLevel, InvalidBlobIndex,
                                               -1);

  auto *LoopUpperSetter =
      HNU.createCopyInst(UpperIVRef, "copy", NewUpperRef->clone());

  auto *LoopExitLabel = HNU.createHLLabel("loopexit");
  FirstLoop->setNumExits(2);
  HLNodeUtils::insertAfter(FirstLoop, LoopExitLabel);
  auto *Goto = HNU.createHLGoto(LoopExitLabel);

  HLNodeUtils::insertAsLastThenChild(If, LoopLowerSetter);
  HLNodeUtils::insertAsLastThenChild(If, LoopUpperSetter);
  HLNodeUtils::insertAsLastThenChild(If, Goto);

  // Set the new loop bounds for second loop
  // DO iv = lb to ub
  // lb and ub are linear for this loop
  RegDDRef *LoopLBRef = NewLowerRef->clone();
  RegDDRef *LoopUBRef = NewUpperRef->clone();
  SecondLoop->setLowerDDRef(LoopLBRef);
  SecondLoop->setUpperDDRef(LoopUBRef);
  LoopLBRef->getSingleCanonExpr()->setDefinedAtLevel(InnerLevel - 1);
  LoopUBRef->getSingleCanonExpr()->setDefinedAtLevel(InnerLevel - 1);
  SecondLoop->addLiveInTemp(LoopLBRef);
  SecondLoop->addLiveInTemp(LoopUBRef);

  // Create Ztt for the new loop to prevent entering loop when
  // the original if would have never executed.
  // Ztt: if (optprd.lower != -1)
  RegDDRef *LHS = LoopLBRef->clone();
  RegDDRef *RHS = DDRU.createConstDDRef(Ty, -1);
  SecondLoop->createZtt(LHS, PredicateTy::ICMP_NE, RHS);
}

void HIRSpecialOptPredicate::doTransformation(HLLoop *InnerLoop) {
  // Original Inner Loop looks like this:
  //  + DO i5 = 0, 2 * %68, 1   <DO_LOOP>
  //  |   %179 = i5 + -1 * %68  *  i5 + -1 * %68;
  //  |   %180 = %179  +  %162;
  //  |   if (%180 <= ((%1 /u 2) * (%2 /u 2)))
  //         ...
  // Move the contents of the if into NewMainLoop which will use
  // the new bounds.

  HIRInvalidationUtils::invalidateLoopNestBody(InnerLoop->getParentLoop());

  HLLoop *NewMainLoop = InnerLoop->cloneEmpty();

  HLIf *If = cast<HLIf>(InnerLoop->getLastChild());

  HLNodeUtils::moveAsLastChildren(NewMainLoop, If->then_begin(),
                                  If->then_end());

  HLNodeUtils::insertAfter(InnerLoop, NewMainLoop);

  replaceIfWithBoundsLoop(InnerLoop, NewMainLoop, If);

  LLVM_DEBUG(dbgs() << "TRANSFORMED!\n"; InnerLoop->getParentLoop()->dump(1););
}

bool HIRSpecialOptPredicate::run() {
  SmallVector<HLLoop *, 32> InnermostLoops;
  (HIRF.getHLNodeUtils()).gatherInnermostLoops(InnermostLoops);

  bool Modified = false;

  for (auto &Loop : InnermostLoops) {
    if (!isCandidate(Loop)) {
      continue;
    }

    LLVM_DEBUG(dbgs() << "Found Candidate for Transformation!\n";
               Loop->dump(););

    doTransformation(Loop);
    Modified = true;
    Loop->getParentRegion()->setGenCode();
  }

  return Modified;
}

PreservedAnalyses
HIRSpecialOptPredicatePass::runImpl(Function &F, FunctionAnalysisManager &AM,
                                    HIRFramework &HIRF) {
  if (DisablePass) {
    return PreservedAnalyses::all();
  }

  LLVM_DEBUG(dbgs() << OPT_DESC " for Function : " << F.getName() << "\n");
  ModifiedHIR = HIRSpecialOptPredicate(HIRF).run();

  return PreservedAnalyses::all();
}
