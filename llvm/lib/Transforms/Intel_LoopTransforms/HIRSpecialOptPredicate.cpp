//===---------------- HIRSpecialOptPredicate.cpp ----------------------===//
//
// Copyright (C) 2023-2023 Intel Corporation. All rights reserved.
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
//   lb = 0
//   ub = -1
//   for (u = (-width/2 to width/2))
//     if ((v*v+u*u) <= ((width/2)*(height/2)))
//       lb = -u
//       ub = u
//       break;

//   for (w = (lb to ub))
//     ... Original code inside if goes here

#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRSpecialOptPredicatePass.h"

#define OPT_SWITCH "hir-special-opt-predicate"
#define OPT_DESC "HIR Special Opt Predicate Pass"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass("disable-" OPT_SWITCH, cl::init(true),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass"));

class HIRSpecialOptPredicate {
public:
  HIRSpecialOptPredicate(HIRFramework &HIRF) : HIRF(HIRF) {}
  bool run();
  bool isCandidate(const HLLoop *InnerLoop) const;

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
  auto PredI = If->pred_begin();
  if (!If || If->hasElseChildren() || If->getNumPredicates() != 1 ||
      !ICmpInst::isLE(*PredI)) {
    return false;
  }

  const RegDDRef *LHS = If->getLHSPredicateOperandDDRef(PredI);
  const RegDDRef *RHS = If->getRHSPredicateOperandDDRef(PredI);

  if (!LHS->isSelfBlob() || LHS->getSelfBlobIndex() != SumIndex) {
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
    return false;
  }

  if (!hasMatchingPredicate(InnerLoop)) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "Found Candidate!\n"; OuterLoop->dump(););
  return true;
}

bool HIRSpecialOptPredicate::run() {
  SmallVector<HLLoop *, 32> InnermostLoops;
  (HIRF.getHLNodeUtils()).gatherInnermostLoops(InnermostLoops);

  bool Modified = false;

  for (auto &Loop : InnermostLoops) {
    if (!isCandidate(Loop)) {
      continue;
    }

    // Do Transformation
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
  HIRSpecialOptPredicate(HIRF).run();

  return PreservedAnalyses::all();
}
