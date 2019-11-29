//===-- HIRMultiExitLoopReroll.cpp - Rerolls multi-exit loops -------------===//
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
// This file implements HIRMultiExitLoopReroll class which rerolls multi-exit
// loops which have been manually unrolled by the user.
//
// For example:
//
// Original loop-
//
//  + DO i1 = 0, %10 + -1, 1   <DO_MULTI_EXIT_LOOP>
//  |   if ((%4)[4 * i1] == &((%1)[0]))
//  |   {
//  |      %15 = &((%4)[4 * i1]);
//  |      goto %57;
//  |   }
//  |   if ((%4)[4 * i1 + 1] == &((%1)[0]))
//  |   {
//  |      %19 = &((%4)[4 * i1 + 1]);
//  |      goto %59;
//  |   }
//  |   if ((%4)[4 * i1 + 2] == &((%1)[0]))
//  |   {
//  |      %23 = &((%4)[4 * i1 + 2]);
//  |      goto %61;
//  |   }
//  |   if ((%4)[4 * i1 + 3] == &((%1)[0]))
//  |   {
//  |      %27 = &((%4)[4 * i1 + 3]);
//  |      goto %63;
//  |   }
//  + END LOOP
//
//  Rerolled loop-
//
//  + DO i1 = 0, 4 * %10 + -4, 1   <DO_MULTI_EXIT_LOOP>
//  |   if ((%4)[i1] == &((%1)[0]))
//  |   {
//  |      %15 = &((%4)[i1]);
//  |      goto %57;
//  |   }
//  + END LOOP
//
// If we have loop liveouts, we need to make sure that they merge into the same
// phi outside the region. For example, %15, %19, %23 and %27 should be merging
// into the same phi via their corresponding early exits (%57, %59, %61 and
// %63).
//
// The IR could look like the following-
//
// 57:                                     ; preds = %14
//  %58 = phi %struct2** [ %15, %14 ]
//  br label %65
//
// 59:                                     ; preds = %18
//  %60 = phi %struct2** [ %19, %18 ]
//  br label %65
//
// 61:                                     ; preds = %22
//  %62 = phi %struct2** [ %23, %22 ]
//  br label %65
//
// 63:                                     ; preds = %26
//  %64 = phi %struct2** [ %27, %26 ]
//  br label %65
//
// 65:
//  %66 =  phi %struct2** [ %58, %57 ], [ %60, %59 ], [ %62, %61 ], [ %64, %63 ]
//
// Currently, we only handle loops where the constant IV coefficient is
// identical but this can probably be extended.
//
// Only positive IV coefficient is handled. Blob IV coefficient is not handled.
// The main consumer of this pass seems to be vectorizer and it doesn't seem
// like we will vectorize multi-exit loops with either of those.
//
// The formula we are looking for is:
// IVCoeff = RerollFactor * CEDistance
//
// For example, the following loops are handled.
// Early exits are not shown for simplicity.
//
// 1) Reroll factor of 6
//
// DO i1 = 0, N
//   A[6*i1] = 1
//   A[6*i1 + 1] = 1
//   A[6*i1 + 2] = 1
//   A[6*i1 + 3] = 1
//   A[6*i1 + 4] = 1
//   A[6*i1 + 5] = 1
// END DO
//
// Transformed into-
//
// DO i1 = 0, 6*N
//   A[i1] = 1
// END DO
//
//
// 2) Reroll factor of 3
//
// DO i1 = 0, N
//   A[6*i1] = 1
//   A[6*i1 + 1] = 2
//   A[6*i1 + 2] = 1
//   A[6*i1 + 3] = 2
//   A[6*i1 + 4] = 1
//   A[6*i1 + 5] = 2
// END DO
//
// Transformed into-
//
// DO i1 = 0, 3*N
//   A[2*i1] = 1
//   A[2*i1 + 1] = 2
// END DO
//
// 3) Reroll factor of 2
//
// DO i1 = 0, N
//   A[6*i1] = 1
//   A[6*i1 + 1] = 2
//   A[6*i1 + 2] = 3
//   A[6*i1 + 3] = 2
//   A[6*i1 + 4] = 1
//   A[6*i1 + 5] = 3
// END DO
//
// Transformed into-
//
// DO i1 = 0, 2*N
//   A[3*i1] = 1
//   A[3*i1 + 1] = 2
//   A[3*i1 + 2] = 3
// END DO
//
//
// The following loop is not handled-
//
// DO i = 0, N
//   A[2*i1] = 1
//   B[4*i1] = 2
//   A[2*i1 + 1] = 1
//   B[4*i1 + 2] = 2
// END DO
//
// Can possibly be rerolled into this with RerollFactor=2 -
//
// DO i = 0, 2*N
//   A[i1] = 1
//   B[2*i1] = 2
// END DO
//

#include "llvm/Transforms/Intel_LoopTransforms/HIRMultiExitLoopReroll.h"

#include "llvm/ADT/Statistic.h"

#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

#define OPT_SWITCH "hir-multi-exit-loop-reroll"
#define OPT_DESC "HIR Multi-Exit Loop Reroll"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

STATISTIC(LoopsRerolled, "Number of HIR multi-exit loops rerolled");

// Optimization is currently enabled by default.
// TODO: Vectorizer currently processes only some rerolled loops via idiom
// recognition.
static cl::opt<bool> DisableReroll("disable-" OPT_SWITCH, cl::init(false),
                                   cl::Hidden, cl::desc("Disable " OPT_DESC));

namespace {

class HIRMultiExitLoopReroll {
public:
  HIRMultiExitLoopReroll(HIRFramework &HIRF, HIRLoopStatistics &HLS)
      : HIRF(HIRF), HLS(HLS), CurLoop(nullptr), IsRerollCandidate(false),
        NumSequences(0), IVCoeff(0), CEDistance(0) {}

  bool run();

private:
  HIRFramework &HIRF;
  HIRLoopStatistics &HLS;

  // Per loop data structures.
  HLLoop *CurLoop;
  bool IsRerollCandidate;
  unsigned NumSequences;
  int64_t IVCoeff;
  int64_t CEDistance;
  SmallVector<unsigned, 8> LiveoutsToBeRemoved;

  // Per sequence data structures.
  SmallVector<std::pair<const RegDDRef *, const RegDDRef *>, 4>
      LiveoutTempPairs;
  SmallVector<std::pair<unsigned, unsigned>, 16> NonLinearTempBlobMap;

  /// Main method to be invoked after all the innermost loops are gathered.
  bool tryReroll(SmallVectorImpl<HLLoop *> &CandidateLoops);

  /// Returns true if we can attempt to unroll this loop.
  bool isApplicable(const HLLoop *Lp) const;

  /// Computes the reroll factor of \p Lp, by identifying 'corresponding'
  /// sequences. Returns 0, if reroll factor cannot be computed.
  unsigned computeRerollFactor(HLLoop *Lp,
                               SmallVectorImpl<HLNode *> &NodeSequence);

  /// Matches incoming \p NodeSequence to sequences of nodes starting from \p
  /// FirstNode. It there is a match, \p NewNode is updated to point to the
  /// start of next sequence.
  bool matchesSequence(const SmallVectorImpl<HLNode *> &NodeSequence,
                       const HLNode *FirstNode, const HLNode **NewNode);

  /// Checks if \p Node1 and \p Node2 correspond to each other such that
  /// rerolling can be performed.
  bool corresponds(const HLNode *Node1, const HLNode *Node2);

  /// Checks if \p If1 and \p If2 correspond to each other such that rerolling
  /// can be performed.
  bool corresponds(const HLIf *If1, const HLIf *If2);

  /// Checks if \p Inst1 and \p Inst2 correspond to each other such that
  /// rerolling can be performed.
  bool corresponds(const HLInst *Inst1, const HLInst *Inst2);

  /// Checks if \p Goto1 and \p Goto2 correspond to each other such that
  /// rerolling can be performed.
  bool corresponds(const HLGoto *Goto1, const HLGoto *Goto2);

  /// Checks if \p Ref1 and \p Ref22 correspond to each other such that
  /// rerolling can be performed.
  bool corresponds(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Checks if \p Ref1 and \p Ref22 have valid distance based on the formula-
  /// IVCoeff = RerollFactor * CEDistance
  bool haveValidDistance(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Checks whether both terminal lval refs are liveout or not.
  bool haveLiveoutCorrespondence(const RegDDRef *LvalRef1,
                                 const RegDDRef *LvalRef2);
};
} // namespace

static PHINode *getMergePhi(const HLGoto *Goto, unsigned Symbase) {
  auto *SrcBB = Goto->getSrcBBlock();
  auto *TargetBB = Goto->getTargetBBlock();

  PHINode *SinglePhi = nullptr;
  auto *ParRegion = Goto->getParentRegion();

  for (auto &Phi : TargetBB->phis()) {

    // Allow only one phi in the exit bblock.
    if (SinglePhi) {
      return nullptr;
    }

    auto *LiveoutInst =
        dyn_cast<Instruction>(Phi.getIncomingValueForBlock(SrcBB));

    if (!LiveoutInst || !ParRegion->containsBBlock(LiveoutInst->getParent()) ||
        (ParRegion->getLiveOutSymbase(LiveoutInst) != Symbase)) {
      return nullptr;
    }

    SinglePhi = &Phi;
  }

  if (!SinglePhi) {
    return nullptr;
  }

  if (SinglePhi->getNumOperands() != 1) {
    return SinglePhi;
  }

  if (!SinglePhi->hasOneUse()) {
    return nullptr;
  }

  return dyn_cast<PHINode>(*SinglePhi->user_begin());
}

bool HIRMultiExitLoopReroll::corresponds(const HLGoto *Goto1,
                                         const HLGoto *Goto2) {
  // It may be possible to handle internal gotos.
  if (!Goto1->isExternal() || !Goto2->isExternal()) {
    return false;
  }

  if (LiveoutTempPairs.empty()) {
    // There are no liveouts in the sequence. We can allow jumps to the same
    // bblock in this case. For example the following loop-
    //
    // DO i1 = 0, %t
    //   if (2*i1 > 0) {
    //      goto L;
    //    }
    //
    //   if (2*i1+1 > 0) {
    //      goto L;
    //    }
    // END DO
    //
    // Can be rerolled into-
    //
    // DO i1 = 0, 2*%t
    //   if (i1 > 0) {
    //      goto L;
    //    }
    // END DO
    //
    // Absence of phi in the target (loop exit) bblock confirms that there is no
    // liveout from that bblock as loops in the incoming IR are required to be
    // in LCSSA form.
    auto TargetBB = Goto1->getTargetBBlock();
    return (TargetBB == Goto2->getTargetBBlock()) &&
           !isa<PHINode>(TargetBB->begin());
  }

  for (auto TempPairIt = LiveoutTempPairs.begin(), E = LiveoutTempPairs.end();
       TempPairIt != E; ++TempPairIt) {
    auto *LiveoutTemp1 = TempPairIt->first;
    auto *LiveoutTemp2 = TempPairIt->second;

    // This check is to make sure that the temp is only liveout from this
    // particular early exit.
    if (!HLNodeUtils::postDominates(Goto1, LiveoutTemp1->getHLDDNode()) ||
        !HLNodeUtils::postDominates(Goto2, LiveoutTemp2->getHLDDNode())) {
      continue;
    }

    // For each pair of liveout symbases, check that they are live out of the
    // corresponding early exit and merge into the same eventual phi.
    auto *MergePhi = getMergePhi(Goto1, LiveoutTemp1->getSymbase());

    if (!MergePhi ||
        (MergePhi != getMergePhi(Goto2, LiveoutTemp2->getSymbase()))) {
      return false;
    }

    LiveoutsToBeRemoved.push_back(LiveoutTemp2->getSymbase());

    LiveoutTempPairs.erase(TempPairIt);
    return true;
  }

  return false;
}

bool HIRMultiExitLoopReroll::haveValidDistance(const RegDDRef *Ref1,
                                               const RegDDRef *Ref2) {
  if (Ref1->hasGEPInfo() &&
      !CanonExprUtils::areEqual(Ref1->getBaseCE(), Ref2->getBaseCE(), false)) {
    return false;
  }

  unsigned LoopLevel = CurLoop->getNestingLevel();

  for (unsigned I = 1, NumDims = Ref1->getNumDimensions(); I <= NumDims; ++I) {
    auto *CE1 = Ref1->getDimensionIndex(I);

    unsigned Index;
    int64_t Coeff;
    CE1->getIVCoeff(LoopLevel, &Index, &Coeff);

    if ((Index != InvalidBlobIndex) || (Coeff < 0) || (Coeff == 1)) {
      IsRerollCandidate = false;
      return false;
    }

    auto *CE2 = Ref2->getDimensionIndex(I);

    if (Coeff == 0) {
      if (!CanonExprUtils::areEqual(CE1, CE2, false)) {
        return false;
      }

    } else {

      if (!IVCoeff) {
        IVCoeff = Coeff;

      } else if (IVCoeff != Coeff) {
        IsRerollCandidate = false;
        return false;
      }

      int64_t CurDist;
      if (!CanonExprUtils::getConstDistance(CE1, CE2, &CurDist, false)) {
        return false;
      }

      if (!CEDistance) {
        // This will be set for the first occurence of IV in the first sequence
        // match.
        CEDistance = CurDist;

      } else if ((CEDistance * (NumSequences - 1)) != CurDist) {
        // The distance between first sequence and N'th sequence should grow in
        // multiples of (N-1).
        return false;
      }
    }
  }

  return true;
}

static bool isSameRefType(const RegDDRef *Ref1, const RegDDRef *Ref2) {
  return (Ref1->isTerminalRef() && Ref2->isTerminalRef()) ||
         (Ref1->isMemRef() && Ref2->isMemRef()) ||
         (Ref1->isAddressOf() && Ref2->isAddressOf());
}

bool HIRMultiExitLoopReroll::corresponds(const RegDDRef *Ref1,
                                         const RegDDRef *Ref2) {
  if (!isSameRefType(Ref1, Ref2)) {
    return false;
  }

  if (Ref1->getNumDimensions() != Ref2->getNumDimensions()) {
    return false;
  }

  unsigned DefLevel1 = Ref1->getDefinedAtLevel();

  if (DefLevel1 != Ref2->getDefinedAtLevel()) {
    return false;
  }

  if (DefLevel1 == NonLinearLevel) {
    // We encountered livein (use before def) non-linear temp. Such temps cannot
    // be handled.
    if (NonLinearTempBlobMap.empty()) {
      IsRerollCandidate = false;
      return false;
    }

    std::unique_ptr<RegDDRef> Ref1Clone(Ref1->clone());

    // We encountered livein (use before def) non-linear temp. Such temps cannot
    // be handled.
    if (!Ref1Clone->replaceTempBlobs(NonLinearTempBlobMap)) {
      IsRerollCandidate = false;
      return false;
    }

    return haveValidDistance(Ref2, Ref1Clone.get());
  }

  return haveValidDistance(Ref2, Ref1);
}

bool HIRMultiExitLoopReroll::haveLiveoutCorrespondence(
    const RegDDRef *LvalRef1, const RegDDRef *LvalRef2) {
  unsigned Symbase1 = LvalRef1->getSymbase();
  unsigned Symbase2 = LvalRef2->getSymbase();

  if (!CurLoop->isLiveOut(Symbase1) && !CurLoop->isLiveOut(Symbase2)) {
    return true;
  }

  if (!CurLoop->isLiveOut(Symbase1) || !CurLoop->isLiveOut(Symbase2)) {
    return false;
  }

  // They will be validated when we hit early exits.
  LiveoutTempPairs.push_back(std::make_pair(LvalRef1, LvalRef2));
  return true;
}

bool HIRMultiExitLoopReroll::corresponds(const HLInst *HInst1,
                                         const HLInst *HInst2) {

  auto *Inst1 = HInst1->getLLVMInstruction();
  auto *Inst2 = HInst2->getLLVMInstruction();

  if (Inst1->getOpcode() != Inst2->getOpcode()) {
    // GEP/Subscript and copy instructions can correspond to each other in HIR
    // so we should allow them. Inside HIR clients create copy instructions
    // instead of GEPs due to its ease.
    //
    // For example-
    //
    // %t1 = &(%A)[4*i1]  // can be copy inst
    //
    // %t2 = &(%A)[4*i1 + 1] // can be GEP inst
    if ((!HInst1->isCopyInst() || !isa<GEPOrSubsOperator>(Inst2)) &&
        (!HInst2->isCopyInst() || !isa<GEPOrSubsOperator>(Inst1))) {
      return false;
    }
  }

  if (isa<CmpInst>(Inst1) || isa<SelectInst>(Inst1)) {
    if (HInst1->getPredicate() != HInst2->getPredicate()) {
      return false;
    }
  }

  if (auto *FPInst1 = dyn_cast<FPMathOperator>(Inst1)) {
    auto *FPInst2 = dyn_cast<FPMathOperator>(Inst2);

    if (!FPInst2 || (FPInst1->isFast() != FPInst2->isFast())) {
      return false;
    }
  }

  if (auto *BinOpInst1 = dyn_cast<OverflowingBinaryOperator>(Inst1)) {
    auto *BinOpInst2 = cast<OverflowingBinaryOperator>(Inst2);

    if ((BinOpInst1->hasNoUnsignedWrap() != BinOpInst2->hasNoUnsignedWrap()) ||
        (BinOpInst1->hasNoSignedWrap() != BinOpInst2->hasNoSignedWrap())) {
      return false;
    }
  }

  auto *RvalIt1 = HInst1->rval_op_ddref_begin();
  auto *RvalIt2 = HInst2->rval_op_ddref_begin();

  for (auto End = HInst1->rval_op_ddref_end(); RvalIt1 != End;
       ++RvalIt1, ++RvalIt2) {
    if (!corresponds(*RvalIt1, *RvalIt2)) {
      return false;
    }
  }

  if (auto *Lval1 = HInst1->getLvalDDRef()) {
    auto *Lval2 = HInst2->getLvalDDRef();

    if (Lval1->isTerminalRef()) {
      if (!Lval2->isTerminalRef()) {
        return false;
      }

      if (!Lval1->isSelfBlob() || !Lval2->isSelfBlob()) {
        return false;
      }

      if (!haveLiveoutCorrespondence(Lval1, Lval2)) {
        return false;
      }

      NonLinearTempBlobMap.push_back(
          std::make_pair(Lval1->getSelfBlobIndex(), Lval2->getSelfBlobIndex()));

    } else if (!corresponds(Lval1, Lval2)) {
      return false;
    }
  }

  return true;
}

bool HIRMultiExitLoopReroll::corresponds(const HLIf *If1, const HLIf *If2) {

  if (If1->getNumPredicates() != If2->getNumPredicates()) {
    return false;
  }

  if (If1->getNumThenChildren() != If2->getNumThenChildren()) {
    return false;
  }

  if (If1->getNumElseChildren() != If2->getNumElseChildren()) {
    return false;
  }

  for (auto PredIt1 = If1->pred_begin(), PredIt2 = If2->pred_begin(),
            E = If1->pred_end();
       PredIt1 != E; ++PredIt1, ++PredIt2) {
    if (*PredIt1 != *PredIt2) {
      return false;
    }

    auto *Lhs1 = If1->getPredicateOperandDDRef(PredIt1, true);
    auto *Lhs2 = If2->getPredicateOperandDDRef(PredIt2, true);

    if (!corresponds(Lhs1, Lhs2)) {
      return false;
    }

    auto *Rhs1 = If1->getPredicateOperandDDRef(PredIt1, false);
    auto *Rhs2 = If2->getPredicateOperandDDRef(PredIt2, false);

    if (!corresponds(Rhs1, Rhs2)) {
      return false;
    }
  }

  for (auto ChildIt1 = If1->then_begin(), ChildIt2 = If2->then_begin(),
            E = If1->then_end();
       ChildIt1 != E; ++ChildIt1, ++ChildIt2) {
    if (!corresponds(&*ChildIt1, &*ChildIt2)) {
      return false;
    }
  }

  for (auto ChildIt1 = If1->else_begin(), ChildIt2 = If2->else_begin(),
            E = If1->else_end();
       ChildIt1 != E; ++ChildIt1, ++ChildIt2) {
    if (!corresponds(&*ChildIt1, &*ChildIt2)) {
      return false;
    }
  }

  return true;
}

bool HIRMultiExitLoopReroll::corresponds(const HLNode *Node1,
                                         const HLNode *Node2) {
  if (Node1->getHLNodeClassID() != Node2->getHLNodeClassID()) {
    return false;
  }

  if (isa<HLIf>(Node1)) {
    return corresponds(cast<HLIf>(Node1), cast<HLIf>(Node2));
  }

  if (isa<HLInst>(Node1)) {
    return corresponds(cast<HLInst>(Node1), cast<HLInst>(Node2));
  }

  if (isa<HLGoto>(Node1)) {
    if (!corresponds(cast<HLGoto>(Node1), cast<HLGoto>(Node2))) {
      // Gotos have to match.
      IsRerollCandidate = false;
      return false;
    }

    return true;
  }

  llvm_unreachable("Unexpected node type!");
}

bool HIRMultiExitLoopReroll::matchesSequence(
    const SmallVectorImpl<HLNode *> &NodeSequence, const HLNode *FirstNode,
    const HLNode **NewNode) {

  NonLinearTempBlobMap.clear();
  LiveoutTempPairs.clear();

  auto *LoopNode = FirstNode;
  for (unsigned I = 0, SeqSize = NodeSequence.size(); I < SeqSize;
       ++I, LoopNode = LoopNode->getNextNode()) {
    if (!LoopNode || !corresponds(NodeSequence[I], LoopNode)) {
      return false;
    }
  }

  if (!LiveoutTempPairs.empty()) {
    return false;
  }

  *NewNode = LoopNode;
  return true;
}

unsigned HIRMultiExitLoopReroll::computeRerollFactor(
    HLLoop *Lp, SmallVectorImpl<HLNode *> &NodeSequence) {

  CurLoop = Lp;
  IsRerollCandidate = true;
  IVCoeff = 0;
  CEDistance = 0;

  unsigned NumChildren = CurLoop->getNumChildren();

  NodeSequence.push_back(CurLoop->getFirstChild());
  auto *CurNode = NodeSequence.back()->getNextNode();

  for (unsigned I = 0; I < NumChildren / 2;
       ++I, CurNode = CurNode->getNextNode()) {

    NumSequences = 2;
    LiveoutsToBeRemoved.clear();
    const HLNode *NewSeqNode = CurNode;

    while (matchesSequence(NodeSequence, NewSeqNode, &NewSeqNode)) {
      if (!NewSeqNode) {
        // Although the sequences we found correspond to each other w.r.t
        // distance, they may not correspond to the total number of sequences
        // (reroll factor) that were found so we have to perform this final
        // check. For example, the following loop contains 2 sequences but
        // requires a 3rd one (A[3*i1 + 2] = 1) for valid reroll.
        //
        // DO i1
        //   A[3*i1] = 1
        //   A[3*i1 + 1] = 1
        // END DO
        //
        return (IVCoeff == NumSequences * CEDistance) ? NumSequences : 0;
      }

      // Early bailout condition if loop is determined to be un-rerollable for
      // any reroll factor.
      if (!IsRerollCandidate) {
        return 0;
      }

      ++NumSequences;
    }

    NodeSequence.push_back(CurNode);
  }

  return 0;
}

bool HIRMultiExitLoopReroll::isApplicable(const HLLoop *Lp) const {
  if (!Lp->isDoMultiExit() || !Lp->isNormalized()) {
    return false;
  }

  auto &LS = HLS.getSelfLoopStatistics(Lp);

  if (LS.hasLabels() || LS.hasCalls() || LS.hasSwitches()) {
    return false;
  }

  return true;
}

class CanonExprUpdater final : public HLNodeVisitorBase {
  unsigned RerollFactor;
  unsigned LoopLevel;
  unsigned NumGotos;

public:
  CanonExprUpdater(unsigned RerollFactor, unsigned LoopLevel)
      : RerollFactor(RerollFactor), LoopLevel(LoopLevel), NumGotos(0) {}

  void visit(HLDDNode *Node) {
    for (auto *Ref : llvm::make_range(Node->ddref_begin(), Node->ddref_end())) {
      for (auto *CE : llvm::make_range(Ref->canon_begin(), Ref->canon_end())) {

        auto Coeff = CE->getIVConstCoeff(LoopLevel);
        if (Coeff != 0) {
          CE->setIVConstCoeff(LoopLevel, Coeff / RerollFactor);
        }
      }
    }
  }

  void visit(HLGoto *Goto) { ++NumGotos; }

  void visit(HLNode *Node) {}
  void postVisit(HLNode *Node) {}

  unsigned getNumGotos() const { return NumGotos; }
};

static bool doReroll(HLLoop *Lp, unsigned RerollFactor,
                     SmallVectorImpl<HLNode *> &NodeSequence,
                     SmallVectorImpl<unsigned> &LiveoutsToBeRemoved) {

  if (!HIRTransformUtils::multiplyTripCount(Lp, RerollFactor)) {
    return false;
  }

  CanonExprUpdater CEUpdater(RerollFactor, Lp->getNestingLevel());

  for (auto *Node : NodeSequence) {
    HLNodeUtils::visit(CEUpdater, Node);
  }

  // Remove all nodes from the loop body starting from the next node of the last
  // node in the sequence.
  HLNodeUtils::remove(NodeSequence.back()->getNextNode(), Lp->getLastChild());

  auto *Region = Lp->getParentRegion();
  for (auto Symbase : LiveoutsToBeRemoved) {
    Lp->removeLiveOutTemp(Symbase);
    Region->removeLiveOutTemp(Symbase);
  }

  Lp->setNumExits(CEUpdater.getNumGotos() + 1);

  HIRInvalidationUtils::invalidateBody(Lp);
  return true;
}

bool HIRMultiExitLoopReroll::tryReroll(SmallVectorImpl<HLLoop *> &Loops) {
  bool Rerolled = false;

  for (auto *Lp : Loops) {
    if (!isApplicable(Lp)) {
      continue;
    }

    SmallVector<HLNode *, 16> NodeSequence;
    unsigned RerollFactor;

    if (!(RerollFactor = computeRerollFactor(Lp, NodeSequence))) {
      continue;
    }

    assert(RerollFactor > 1 && "Invalid Reroll factor!");

    if (doReroll(Lp, RerollFactor, NodeSequence, LiveoutsToBeRemoved)) {
      ++LoopsRerolled;
      Rerolled = true;
    }
  }

  return Rerolled;
}

bool HIRMultiExitLoopReroll::run() {
  if (DisableReroll) {
    LLVM_DEBUG(
        dbgs() << "HIR Multi-Exit Loop Reroll Transformation Disabled \n");
    return false;
  }

  auto &HNU = HIRF.getHLNodeUtils();
  bool Rerolled = false;

  for (auto RegIt = HIRF.hir_begin(), E = HIRF.hir_end(); RegIt != E; ++RegIt) {
    auto *CurRegion = cast<HLRegion>(RegIt);
    SmallVector<HLLoop *, 16> CandidateLoops;

    HNU.gatherInnermostLoops(CandidateLoops, CurRegion);

    if (tryReroll(CandidateLoops)) {
      Rerolled = true;
    }
  }

  return Rerolled;
}

PreservedAnalyses
HIRMultiExitLoopRerollPass::run(llvm::Function &F,
                                llvm::FunctionAnalysisManager &AM) {
  HIRMultiExitLoopReroll(AM.getResult<HIRFrameworkAnalysis>(F),
                         AM.getResult<HIRLoopStatisticsAnalysis>(F))
      .run();
  return PreservedAnalyses::all();
}

class HIRMultiExitLoopRerollLegacyPass : public HIRTransformPass {
public:
  static char ID;

  HIRMultiExitLoopRerollLegacyPass() : HIRTransformPass(ID) {
    initializeHIRMultiExitLoopRerollLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequiredTransitive<HIRFrameworkWrapperPass>();
    AU.addRequiredTransitive<HIRLoopStatisticsWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F)) {
      return false;
    }

    return HIRMultiExitLoopReroll(
               getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
               getAnalysis<HIRLoopStatisticsWrapperPass>().getHLS())
        .run();
  }
};

char HIRMultiExitLoopRerollLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRMultiExitLoopRerollLegacyPass, OPT_SWITCH, OPT_DESC,
                      false, false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRLoopStatisticsWrapperPass)
INITIALIZE_PASS_END(HIRMultiExitLoopRerollLegacyPass, OPT_SWITCH, OPT_DESC,
                    false, false)

FunctionPass *llvm::createHIRMultiExitLoopRerollPass() {
  return new HIRMultiExitLoopRerollLegacyPass();
}
