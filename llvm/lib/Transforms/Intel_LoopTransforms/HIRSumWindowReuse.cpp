//===----------------------- HIRSumWindowReuse.cpp ------------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements a sum window reuse pass which transforms certain
/// sliding window sums to avoid re-loading and re-adding overlapping terms
/// between outer loop iterations.
///
/// For example:
///
/// \code
/// + DO i1 = 0, N-K, 1
/// |   %sum = 0.0;
/// |
/// |   + DO i2 = 0, K-1, 1
/// |   |   %sum = %sum + (%A)[i1 + i2];
/// |   + END LOOP
/// |
/// |   (%B)[i1] = %sum;
/// + END LOOP
///
/// ===>
///
///   %sum = 0.0;
/// + DO i1 = 0, N-K, 1
/// |
/// |   if (i1 == 0)
/// |   {
/// |      + DO i2 = 0, K-1, 1
/// |      |   %sum = %sum + (%A)[i2];
/// |      + END LOOP
/// |   }
/// |   else
/// |   {
/// |      %sum = %sum - (%A)[i1 - 1];
/// |      %sum = %sum + (%A)[i1 + K-1];
/// |   }
/// |
/// |   (%B)[i1] = %sum;
/// + END LOOP
/// \endcode
///
/// To understand this transform, it can be helpful to visualize the sliding
/// window sum. This table shows the terms in each sum calculated in the example
/// above (for N=7, K=4):
///
/// |      | A[0] | A[1] | A[2] | A[3] | A[4] | A[5] | A[6] |
/// | ---: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
/// | i1=0 |  +   |  +   |  +   |  +   |      |      |      |
/// | i1=1 |      |  +   |  +   |  +   |  +   |      |      |
/// | i1=2 |      |      |  +   |  +   |  +   |  +   |      |
/// | i1=3 |      |      |      |  +   |  +   |  +   |  +   |
///
/// For `i1=0`, the sum is calculated as `S0=A[0]+A[1]+A[2]+A[3]` regardless of
/// whether this transform has applied. However, the window for the `i1=1` sum
/// has a significant overlap with the `i1=0` sum's window, and when this
/// transform applies it can be calculated as `S1=S0-A[0]+A[4]` rather than
/// `S1=A[1]+A[2]+A[3]+A[4]`, reusing the terms in the overlap between the
/// window via the `S0` value that was calculated in the previous iteration.
/// For these types of sliding windows, this re-use can be very beneficial for
/// reducing the amount of computation needed for successive sums.
///
//===----------------------------------------------------------------------===//
#include "llvm/Transforms/Intel_LoopTransforms/HIRSumWindowReuse.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"

#define OPT_SWITCH "hir-sum-window-reuse"
#define OPT_DESC "HIR Sum Window Reuse"
#define DEBUG_TYPE OPT_SWITCH

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool> DisablePass{"disable-" OPT_SWITCH, cl::init(false),
                                 cl::Hidden,
                                 cl::desc("Disable " OPT_DESC " pass")};

namespace {

/// A recognized sliding window sum within a loop.
struct SlidingWindowSum {

  /// The instruction performing the sum.
  HLInst *Inst;

  /// The sum's opcode, which should either be FAdd or FSub for now.
  unsigned Opcode;

  /// The load for the sum terms. This may be an operand of \ref Inst or an
  /// operand of a cast used by \ref Inst.
  RegDDRef *TermLoad;

  /// The operand index for the sum term within \ref Inst.
  unsigned TermLoadIdx;

  /// This flag is set when there is a cast (\ref TermLoad is not an operand of
  /// \ref Inst) and that cast has no uses other than \ref Inst (so it can be
  /// removed when \ref Inst is).
  bool CanRemoveCastInst;

  /// Constructs a SlidingWindowSum given values for its fields.
  SlidingWindowSum(HLInst *Inst, unsigned Opcode, RegDDRef *TermLoad,
                   unsigned TermLoadIdx, bool CanRemoveCastInst)
      : Inst{Inst}, Opcode{Opcode}, TermLoad{TermLoad},
        TermLoadIdx{TermLoadIdx}, CanRemoveCastInst{CanRemoveCastInst} {
    assert(Inst);
    assert(Opcode == Instruction::FAdd || Opcode == Instruction::FSub);
    assert(TermLoad);
    assert(Inst->getLLVMInstruction()->getOpcode() == Opcode);
    assert(TermLoad->isMemRef());
    if (TermLoad->getHLDDNode() == Inst) {
      assert(Inst->getOperandDDRef(TermLoadIdx) == TermLoad);
      assert(!CanRemoveCastInst);
    } else {
      assert(
        cast<HLInst>(TermLoad->getHLDDNode())->getLLVMInstruction()->isCast());
      assert(DDRefUtils::areEqual(TermLoad->getHLDDNode()->getLvalDDRef(),
                                  Inst->getOperandDDRef(TermLoadIdx)));
    }
  }
};

/// A set of sliding window sums identified within the same inner/outer loop
/// pair.
struct LoopSlidingWindowSums {

  /// The sums' inner loop.
  HLLoop *InnerLoop;

  /// The sums' outer loop.
  HLLoop *OuterLoop;

  /// The outermost loop that the sums are initially calculated across. This
  /// will be the inner loop or a parent of the inner loop that is an immediate
  /// child of the outer loop.
  HLLoop *OutermostSumLoop;

  /// The sums found in this loop pair.
  SmallVector<SlidingWindowSum, 3> Sums;

  /// Constructs a LoopSlidingWindowSums record given the current loops.
  LoopSlidingWindowSums(HLLoop *InnerLoop, HLLoop *OuterLoop,
                        HLLoop *OutermostSumLoop)
      : InnerLoop{InnerLoop}, OuterLoop{OuterLoop}, OutermostSumLoop{
                                                      OutermostSumLoop} {
    assert(InnerLoop);
    assert(OuterLoop);
    assert(OutermostSumLoop);
    assert(InnerLoop->isInnermost());
    assert(OutermostSumLoop == InnerLoop ||
           HLNodeUtils::contains(OutermostSumLoop, InnerLoop));
    assert(OuterLoop == OutermostSumLoop->getParentLoop());
  }
};

} // namespace

/// Determines whether \p InnerLoop, \p OuterLoop, and all intervening loops
/// have the required structure for this transform. This check only needs to be
/// done once per inner/outer loop pair.
static bool isEligibleLoopNest(const HLLoop *InnerLoop,
                               const HLLoop *OuterLoop) {

  // For now, the outer loop should be normalized but does not have to be a DO
  // loop.
  if (!OuterLoop->isNormalized()) {
    LLVM_DEBUG(dbgs() << "  Outer loop is not normalized\n\n");
    return false;
  }

  // The bounds and ZTTs of the inner and intervening loops need to be loop
  // invariant in the context of the outer loop.
  const unsigned OuterLevel = OuterLoop->getNestingLevel();
  for (const HLLoop *Loop = InnerLoop; Loop != OuterLoop;
       Loop               = Loop->getParentLoop()) {
    for (const RegDDRef *const Ref :
         make_range(Loop->ddref_begin(), Loop->ddref_end())) {
      if (!Ref->isStructurallyInvariantAtLevel(OuterLevel)) {
        LLVM_DEBUG({
          dbgs() << "Inner/intervening loop bound/ZTT ref ";
          Ref->print(fdbgs());
          dbgs() << " is not structurally invariant within the outer loop\n";
        });
        return false;
      }
    }
  }

  // All checks passed: these loops are eligible for optimization.
  return true;
}

/// Determines whether \p Expr has an IV coefficient of exactly one at \p Level.
static bool isIVCoeffOne(const CanonExpr *Expr, unsigned Level) {
  return Expr->getIVConstCoeff(Level) == 1 &&
         Expr->getIVBlobCoeff(Level) == InvalidBlobIndex;
}

/// Searches for a single incoming flow edge to \p Ref in \p DDG. The source of
/// that edge is returned if found and if it is a RegDDRef; otherwise, if there
/// are no incoming flow edges or multiple incoming flow edges, nullptr is
/// returned.
static const RegDDRef *getSingleDef(const RegDDRef *Ref, const DDGraph &DDG) {
  const RegDDRef *SingleDef = nullptr;
  for (const DDEdge *const Edge : DDG.incoming(Ref)) {
    if (Edge->isFlow()) {
      if (SingleDef)
        return nullptr;

      // This can be a cast as the sources of flow edges are always RegDDRefs.
      SingleDef = cast<RegDDRef>(Edge->getSrc());
    }
  }

  return SingleDef;
}

/// Determines whether \p TermLoad has an access pattern that is eligible for
/// optimization by this pass for inner/outer loops \p InnerLoop and \p
/// CallerOuterLoop according to \p HDDA. \p CallerOuterLoop may be nullptr, in
/// which case this function will set it to the appropriate outer loop for
/// optimizing this term load if the load is found to be eligible for
/// optimization.
static bool isEligibleTermLoad(const RegDDRef *TermLoad,
                               const HLLoop *InnerLoop, HIRDDAnalysis &HDDA,
                               HLLoop *&CallerOuterLoop) {
  const unsigned InnerLevel = InnerLoop->getNestingLevel();
  HLLoop *OuterLoop         = CallerOuterLoop;

  // The sum terms must be loaded directly from memory with an invariant base
  // pointer.
  if (!TermLoad->isMemRef()) {
    LLVM_DEBUG(dbgs() << "  Sum terms are not loaded from memory\n\n");
    return false;
  }

  // For now, restrict term accesses to one-dimensional ones with an inner IV
  // coefficient of 1.
  if (TermLoad->getNumDimensions() != 1) {
    LLVM_DEBUG(dbgs() << "  Term load is not one-dimensional\n\n");
    return false;
  }
  const CanonExpr *const TermExpr = TermLoad->getSingleCanonExpr();
  if (TermExpr->getDenominator() != 1) {
    LLVM_DEBUG(dbgs() << "  Term load has non-1 denominator\n\n");
    return false;
  }
  if (!isIVCoeffOne(TermExpr, InnerLevel)) {
    LLVM_DEBUG(dbgs() << "  Term load has non-1 inner IV coefficient\n\n");
    return false;
  }

  // If there is no outer loop yet, find the first eligible outer IV in this
  // term load.
  if (!OuterLoop) {
    for (OuterLoop = InnerLoop->getParentLoop(); OuterLoop;
         OuterLoop = OuterLoop->getParentLoop())
      if (isIVCoeffOne(TermExpr, OuterLoop->getNestingLevel()))
        break;
    if (!OuterLoop) {
      LLVM_DEBUG(dbgs() << "  No eligible outer-level IVs found\n");
      return false;
    }
    if (!isEligibleLoopNest(InnerLoop, OuterLoop))
      return false;
  }

  // Otherwise, ensure that the corresponding outer IV is 1.
  else {
    if (!isIVCoeffOne(TermExpr, OuterLoop->getNestingLevel())) {
      LLVM_DEBUG(dbgs() << "  Term load has non-1 outer IV coefficient\n\n");
      return false;
    }
  }

  const unsigned OuterLevel = OuterLoop->getNestingLevel();
  if (!TermLoad->getBaseCE()->isInvariantAtLevel(OuterLevel)) {
    LLVM_DEBUG(dbgs() << "  Base pointer is not invariant\n\n");
    return false;
  }

  // Check that the accesses are independent within the outer loop.
  const DDGraph DDG = HDDA.getGraph(OuterLoop);
  if (DDG.getTotalNumIncomingFlowEdges(TermLoad)) {
    LLVM_DEBUG({
      dbgs() << "  Term load has incoming flow edges:\n";
      for (const DDEdge *const Edge : DDG.incoming(TermLoad)) {
        if (Edge->isFlow()) {
          dbgs() << "    ";
          Edge->print(fdbgs());
          dbgs() << "\n";
        }
      }
      for (const BlobDDRef *const Blob :
           make_range(TermLoad->blob_begin(), TermLoad->blob_end())) {
        for (const DDEdge *const Edge : DDG.incoming(Blob)) {
          dbgs() << "    ";
          Edge->print(fdbgs());
          dbgs() << "\n";
        }
      }
      dbgs() << "\n";
    });
    return false;
  }

  // All checks passed: this load is eligible for optimization.
  CallerOuterLoop = OuterLoop;
  return true;
}

/// Locates eligible sliding window sums within an innermost loop \p InnerLoop.
/// Identified sliding window sums are appended to \p Sums.
static void
findSlidingWindowSums(HLLoop *InnerLoop, HIRDDAnalysis &HDDA,
                      HIRSafeReductionAnalysis &HSR,
                      SmallVectorImpl<LoopSlidingWindowSums> &LoopSums) {

  // The inner loop must be at least two levels deep.
  if (InnerLoop->getNestingLevel() == 1)
    return;

  // For now, ensure that the inner loop is a normalized DO loop.
  if (!InnerLoop->isDo() || !InnerLoop->isNormalized())
    return;

  // A LoopSlidingWindowSums record for this loop will be stored here once an
  // outer loop is identified. This transform could support multiple sums within
  // and inner loop with differing outer loops, but for simplicity that is not
  // yet implemented.
  Optional<LoopSlidingWindowSums> NewSums;

  // Iterate reductions found in the loop.
  HSR.computeSafeReductionChains(InnerLoop);
  for (const SafeRedInfo &Reduction : HSR.getSafeRedInfoList(InnerLoop)) {

    // The reduction must be single-instruction.
    if (Reduction.Chain.size() != 1)
      continue;
    const HLInst *const SumInst = Reduction.Chain.front();

    LLVM_DEBUG({
      dbgs() << "Candidate sum for sliding window optimization:\n";
      SumInst->print(fdbgs(), 0);
    });

    // The sliding window sum optimization involves rearranging the sum, so only
    // sums with the appropriate fast math markings should be optimized.
    if (Reduction.HasUnsafeAlgebra) {
      LLVM_DEBUG(
        dbgs() << "  Sum not supported due to strict FP semantics\n\n");
      continue;
    }

    // Only fadd/fsub reductions are supported for now.
    const unsigned SumOpcode = Reduction.OpCode;
    if (SumOpcode != Instruction::FAdd && SumOpcode != Instruction::FSub) {
      LLVM_DEBUG(dbgs() << "  Non-fadd/fsub reduction not supported\n\n");
      continue;
    }

    // The sum must not be conditional.
    if (!HLNodeUtils::postDominates(SumInst, InnerLoop->getFirstChild())) {
      LLVM_DEBUG(dbgs() << "  Conditional reduction not supported\n\n");
      continue;
    }

    // Locate the term load. If the sum inst doesn't involve a memref directly,
    // look for a cast used in the sum inst.
    const unsigned TermLoadIdx =
      DDRefUtils::areEqual(SumInst->getLvalDDRef(), SumInst->getOperandDDRef(1))
        ? 2
        : 1;
    const RegDDRef *TermLoad = SumInst->getOperandDDRef(TermLoadIdx);
    const DDGraph &InnerDDG  = HDDA.getGraph(InnerLoop);
    if (TermLoad->isTerminalRef()) {
      const RegDDRef *const TermDef = getSingleDef(TermLoad, InnerDDG);
      if (!TermDef) {
        LLVM_DEBUG(dbgs() << "  Sum terms don't have direct in-loop def\n\n");
        continue;
      }
      const auto *const DefInst = dyn_cast<HLInst>(TermDef->getHLDDNode());
      if (!DefInst) {
        LLVM_DEBUG(dbgs() << "  Sum term def is not an HLInst?\n\n");
        continue;
      }
      if (!DefInst->getLLVMInstruction()->isCast()) {
        LLVM_DEBUG(dbgs() << "  Sum term def is not a cast\n\n");
        continue;
      }

      TermLoad = DefInst->getRvalDDRef();
    }

    // Check that the sum terms have the right access pattern.
    HLLoop *OuterLoop              = NewSums ? NewSums->OuterLoop : nullptr;
    if (!isEligibleTermLoad(TermLoad, InnerLoop, HDDA, OuterLoop))
      continue;

    // The sum must not have any other uses within the intervening sum loops
    // because this optimization does not calculate the partial window sum
    // values that would appear there.
    //
    // Note that the way that HIR currently handles nested reductions with inner
    // ZTTs runs afoul of this check, though this is not an immediate problem
    // because our current application of interest has these ZTTs hoisted out.
    HLLoop *const OutermostSumLoop =
      NewSums
        ? NewSums->OutermostSumLoop
        : InnerLoop->getParentLoopAtLevel(OuterLoop->getNestingLevel() + 1);
    if (OutermostSumLoop != InnerLoop) {
      const DDGraph DDG = HDDA.getGraph(OutermostSumLoop);
      bool HasBadEdge   = false;
      for (const DDEdge *const Edge : DDG.outgoing(SumInst->getLvalDDRef())) {
        if (Edge->getSink()->getHLDDNode() != SumInst) {
          LLVM_DEBUG({
            dbgs() << "  Sum has use within intervening loops:\n";
            Edge->getSink()->getHLDDNode()->print(fdbgs(), 0);
            dbgs() << "\n";
          });
          HasBadEdge = true;
          break;
        }
      }
      if (HasBadEdge)
        continue;
    }

    LLVM_DEBUG(dbgs() << "  Sum is eligible for optimization\n\n");

    // If there is a cast involved, check whether it can be safely deleted once
    // the sum is transformed.
    bool CanRemoveCastInst = false;
    if (TermLoad->getHLDDNode() != SumInst) {
      const RegDDRef *const CastTemp = TermLoad->getHLDDNode()->getLvalDDRef();
      if (!InnerLoop->isLiveOut(CastTemp->getSymbase())) {
        CanRemoveCastInst = true;
        for (const DDEdge *const Edge : InnerDDG.outgoing(CastTemp)) {
          if (Edge->isFlow() && Edge->getSink()->getHLDDNode() != SumInst) {
            CanRemoveCastInst = false;
            break;
          }
        }
      }
    }

    // Collect the sum.
    if (!NewSums)
      NewSums.emplace(InnerLoop, OuterLoop, OutermostSumLoop);
    NewSums->Sums.emplace_back(const_cast<HLInst *>(SumInst), SumOpcode,
                               const_cast<RegDDRef *>(TermLoad), TermLoadIdx,
                               CanRemoveCastInst);
  }

  // Add these new sums to the list if there are any.
  if (NewSums)
    LoopSums.push_back(std::move(NewSums.getValue()));
}

/// Transforms sliding window sums in \p Sums to re-use terms across outer loop
/// iterations.
static void transformLoopWindowSums(const LoopSlidingWindowSums &LoopSums) {

  // Extract the ZTT and preheader/postexit from the inner loop if there are
  // any:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   %sum.final = 0.000000e+00;
  // |
  // |      %sum = 0.000000e+00;
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   if (%M != 0)
  // |   |   {
  // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
  // |   |      + END LOOP
  // |   |   }
  // |   + END LOOP
  // |      %sum.final = %sum;
  // |
  // |   (%B)[i1] = %sum.final;
  // + END LOOP
  LoopSums.InnerLoop->extractZttPreheaderAndPostexit();

  // Create an HLIf after the inner loop that will determine whether it's the
  // first outer loop iteration or not:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   %sum.final = 0.000000e+00;
  // |
  // |      %sum = 0.000000e+00;
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   if (%M != 0)
  // |   |   {
  // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
  // |   |      + END LOOP
  // |   |
  // |   |      if (i1 == 0)
  // |   |      {
  // |   |      }
  // |   |   }
  // |   + END LOOP
  // |      %sum.final = %sum;
  // |
  // |   (%B)[i1] = %sum.final;
  // + END LOOP
  HLNodeUtils &HNU           = LoopSums.OuterLoop->getHLNodeUtils();
  DDRefUtils &DDRU           = HNU.getDDRefUtils();
  CanonExprUtils &CEU        = DDRU.getCanonExprUtils();
  Type *const OuterIVType    = LoopSums.OuterLoop->getIVType();
  CanonExpr *const OuterIVCE = CEU.createCanonExpr(OuterIVType);
  const unsigned OuterLevel  = LoopSums.OuterLoop->getNestingLevel();
  OuterIVCE->addIV(OuterLevel, InvalidBlobIndex, 1);
  RegDDRef *const OuterIVRef =
    DDRU.createScalarRegDDRef(GenericRvalSymbase, OuterIVCE);
  HLIf *const FirstIterIf = HNU.createHLIf(PredicateTy::ICMP_EQ, OuterIVRef,
                                           DDRU.createNullDDRef(OuterIVType));
  HLNodeUtils::insertAfter(LoopSums.InnerLoop, FirstIterIf);

  // Add an empty clone of the inner loop to the first-iteration case for
  // calculating the initial window sums:
  //
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   %sum.final = 0.000000e+00;
  // |
  // |      %sum = 0.000000e+00;
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   if (%M != 0)
  // |   |   {
  // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
  // |   |      + END LOOP
  // |   |
  // |   |      if (i1 == 0)
  // |   |      {
  // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |   |         + END LOOP
  // |   |      }
  // |   |   }
  // |   + END LOOP
  // |      %sum.final = %sum;
  // |
  // |   (%B)[i1] = %sum.final;
  // + END LOOP
  HLLoop *const FirstIterLoop = LoopSums.InnerLoop->cloneEmpty();
  HLNodeUtils::insertAsLastThenChild(FirstIterIf, FirstIterLoop);

  // Generate code for each sum in the loop.
  for (const SlidingWindowSum &Sum : LoopSums.Sums) {

    // Add a window sum carried across outer loop iterations, initialized to 0:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         + END LOOP
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    HLInst *const WSumInit = HNU.createCopyInst(
      DDRU.createNullDDRef(Sum.Inst->getLvalDDRef()->getDestType()),
      "swr.wsum");
    HLNodeUtils::insertAsLastPreheaderNode(LoopSums.OuterLoop, WSumInit);
    const RegDDRef *const WSum = WSumInit->getLvalDDRef();
    LoopSums.OuterLoop->addLiveInTemp(WSum);

    // Add the initial window sum calculation:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
    // |   |         + END LOOP
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    HLInst *const TermCast = (Sum.TermLoad->getHLDDNode() == Sum.Inst)
                               ? nullptr
                               : cast<HLInst>(Sum.TermLoad->getHLDDNode());
    const RegDDRef *FirstIterCastTemp = nullptr;
    RegDDRef *FirstIterTermLoad;
    if (TermCast) {
      HLInst *const FirstIterCast = TermCast->clone();
      HNU.createAndReplaceTemp(FirstIterCast->getLvalDDRef(), "swr.initcast");
      FirstIterTermLoad = FirstIterCast->getRvalDDRef();
      HLNodeUtils::insertAsLastChild(FirstIterLoop, FirstIterCast);
      FirstIterCastTemp = FirstIterCast->getLvalDDRef();
    }
    const unsigned PrevSumIdx  = (Sum.TermLoadIdx == 1) ? 2 : 1;
    HLInst *const FirstIterSum = Sum.Inst->clone();
    FirstIterSum->setOperandDDRef(WSum->clone(), 0);
    FirstIterSum->setOperandDDRef(WSum->clone(), PrevSumIdx);
    if (FirstIterCastTemp)
      FirstIterSum->setOperandDDRef(FirstIterCastTemp->clone(),
                                    Sum.TermLoadIdx);
    else
      FirstIterTermLoad = FirstIterSum->getOperandDDRef(Sum.TermLoadIdx);
    FirstIterTermLoad->replaceIVByConstant(OuterLevel, 0);
    HLNodeUtils::insertAsLastChild(FirstIterLoop, FirstIterSum);
    for (HLLoop *Loop = FirstIterLoop; Loop != LoopSums.OuterLoop;
         Loop         = Loop->getParentLoop()) {
      Loop->addLiveInTemp(WSum->getSymbase());
      Loop->addLiveOutTemp(WSum->getSymbase());
    }

    // Add the first update operation, which removes the start of the previous
    // window:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
    // |   |         + END LOOP
    // |   |      }
    // |   |      else
    // |   |      {
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 - 1];
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    const RegDDRef *StartCastTemp = nullptr;
    RegDDRef *PrevStart;
    if (TermCast) {
      HLInst *const StartCast = TermCast->clone();
      HNU.createAndReplaceTemp(StartCast->getLvalDDRef(), "swr.startcast");
      HLNodeUtils::insertAsLastElseChild(FirstIterIf, StartCast);
      StartCastTemp = StartCast->getLvalDDRef();
      PrevStart     = StartCast->getRvalDDRef();
    } else {
      PrevStart = Sum.TermLoad->clone();
    }
    const unsigned InverseOpcode =
      (Sum.Opcode == Instruction::FAdd) ? Instruction::FSub : Instruction::FAdd;
    const unsigned InnerLevel = LoopSums.InnerLoop->getNestingLevel();
    PrevStart->replaceIVByConstant(InnerLevel, 0);
    PrevStart->shift(OuterLevel, -1);
    const auto *const SumInstBinOp =
      cast<BinaryOperator>(Sum.Inst->getLLVMInstruction());
    HLInst *const RemoveStartInst =
      HNU.createBinaryHLInst(InverseOpcode, WSum->clone(),
                             StartCastTemp ? StartCastTemp->clone() : PrevStart,
                             "", WSum->clone(), SumInstBinOp);
    HLNodeUtils::insertAsLastElseChild(FirstIterIf, RemoveStartInst);

    // Add the second update operation, which adds the end of the current
    // window:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
    // |   |         + END LOOP
    // |   |      }
    // |   |      else
    // |   |      {
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 - 1];
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 + %K - 1];
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    const RegDDRef *EndCastTemp = nullptr;
    RegDDRef *CurEnd;
    if (TermCast) {
      HLInst *const EndCast = TermCast->clone();
      HNU.createAndReplaceTemp(EndCast->getLvalDDRef(), "swr.endcast");
      HLNodeUtils::insertAsLastElseChild(FirstIterIf, EndCast);
      EndCastTemp = EndCast->getLvalDDRef();
      CurEnd      = EndCast->getRvalDDRef();
    } else {
      CurEnd = Sum.TermLoad->clone();
    }
    DDRefUtils::replaceIVByCanonExpr(
        CurEnd, InnerLevel,
        LoopSums.InnerLoop->getUpperDDRef()->getSingleCanonExpr(),
        LoopSums.InnerLoop->hasSignedIV());
    HLInst *const AddEndInst = HNU.createBinaryHLInst(
      Sum.Opcode, WSum->clone(), EndCastTemp ? EndCastTemp->clone() : CurEnd,
      "", WSum->clone(), SumInstBinOp);
    HLNodeUtils::insertAsLastElseChild(FirstIterIf, AddEndInst);
    CurEnd->makeConsistent(LoopSums.InnerLoop->getUpperDDRef(), InnerLevel - 1);

    // Add the new window sum back into the original sum:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
    // |   |         + END LOOP
    // |   |      }
    // |   |      else
    // |   |      {
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 - 1];
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 + %K - 1];
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum = %sum + %swr.wsum;
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    const RegDDRef *const OrigSum = Sum.Inst->getLvalDDRef();
    HLInst *const OrigSumUpdateInst =
      HNU.createBinaryHLInst(Instruction::FAdd, OrigSum->clone(), WSum->clone(),
                             "", OrigSum->clone(), SumInstBinOp);
    if (LoopSums.OutermostSumLoop == LoopSums.InnerLoop) {
      HLNodeUtils::insertAfter(FirstIterIf, OrigSumUpdateInst);
    } else {
      HLNodeUtils::insertAsFirstPostexitNode(LoopSums.OutermostSumLoop,
                                             OrigSumUpdateInst);
    }

    // Remove the original sum instruction:
    //
    //    %swr.wsum = 0.000000e+00;
    // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
    // |   %sum.final = 0.000000e+00;
    // |
    // |      %sum = 0.000000e+00;
    // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
    // |   |   if (%M != 0)
    // |   |   {
    // |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |      + END LOOP
    // |   |
    // |   |      if (i1 == 0)
    // |   |      {
    // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
    // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
    // |   |         + END LOOP
    // |   |      }
    // |   |      else
    // |   |      {
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 - 1];
    // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 + %K - 1];
    // |   |      }
    // |   |   }
    // |   + END LOOP
    // |      %sum = %sum + %swr.wsum;
    // |      %sum.final = %sum;
    // |
    // |   (%B)[i1] = %sum.final;
    // + END LOOP
    for (HLLoop *Loop = LoopSums.InnerLoop; Loop != LoopSums.OuterLoop;
         Loop         = Loop->getParentLoop()) {
      Loop->removeLiveInTemp(OrigSum->getSymbase());
      Loop->removeLiveOutTemp(OrigSum->getSymbase());
    }
    HLNodeUtils::remove(Sum.Inst);
    if (Sum.CanRemoveCastInst) {
      assert(TermCast);
      HLNodeUtils::remove(TermCast);
    }
  }

#if INTEL_INTERNAL_BUILD
  LoopOptReportBuilder &LORBuilder = HNU.getHIRFramework().getLORBuilder();
#endif // INTEL_INTERNAL_BUILD

  // Also remove the original loop if it is now empty:
  //
  //    %swr.wsum = 0.000000e+00;
  // + DO i1 = 0, %N + -1, 1   <DO_LOOP>
  // |   %sum.final = 0.000000e+00;
  // |
  // |      %sum = 0.000000e+00;
  // |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
  // |   |   if (%M != 0)
  // |   |   {
  // |   |      if (i1 == 0)
  // |   |      {
  // |   |         + DO i3 = 0, %K + -1, 1   <DO_LOOP>
  // |   |         |   %swr.wsum = %swr.wsum  +  (%A)[%stride * i2 + i3];
  // |   |         + END LOOP
  // |   |      }
  // |   |      else
  // |   |      {
  // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 - 1];
  // |   |         %swr.wsum = %swr.wsum - (%A)[i1 + %stride * i2 + %K - 1];
  // |   |      }
  // |   |   }
  // |   + END LOOP
  // |      %sum = %sum + %swr.wsum;
  // |      %sum.final = %sum;
  // |
  // |   (%B)[i1] = %sum.final;
  // + END LOOP
  assert(!LoopSums.InnerLoop->hasPreheader());
  assert(!LoopSums.InnerLoop->hasPostexit());
  if (!LoopSums.InnerLoop->hasChildren()) {

#if INTEL_INTERNAL_BUILD
    LORBuilder(*LoopSums.InnerLoop).moveOptReportTo(*FirstIterLoop);
#endif // INTEL_INTERNAL_BUILD

    HLNodeUtils::remove(LoopSums.InnerLoop);
  } else {
    HIRInvalidationUtils::invalidateBody(LoopSums.InnerLoop);
  }

  // The transform was successful; mark the relevant loops as invalidated and
  // the parent region for code generation.
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(LoopSums.OuterLoop);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(FirstIterLoop);
  HIRInvalidationUtils::invalidateBody(LoopSums.OuterLoop);
  LoopSums.OuterLoop->getParentRegion()->setGenCode();

#if INTEL_INTERNAL_BUILD
  LORBuilder(*FirstIterLoop)
    .addOrigin("Window sum initialization loop for sum window reuse");
  LORBuilder(*LoopSums.OuterLoop)
    .addRemark(OptReportVerbosity::Low,
               "Inner loop sums optimized with sum window reuse");
#endif // INTEL_INTERNAL_BUILD
}

/// Performs sum window reuse on a function.
static bool runHIRSumWindowReuse(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                                 HIRSafeReductionAnalysis &HSR) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  // Scan innermost loops that are at least two deep to collect eligible sums.
  SmallVector<LoopSlidingWindowSums, 8> LoopSums;
  SmallVector<HLLoop *, 16> InnerLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnerLoops);
  for (HLLoop *InnerLoop : InnerLoops)
    findSlidingWindowSums(InnerLoop, HDDA, HSR, LoopSums);

  // Stop here if no sums were identified.
  if (LoopSums.empty()) {
    LLVM_DEBUG(dbgs() << "No window sums identified for optimization\n\n");
    return false;
  }

  LLVM_DEBUG({
    dbgs() << "Identified these window sums for optimization:\n";
    for (const LoopSlidingWindowSums &Sums : LoopSums) {
      dbgs() << "  inner <" << Sums.InnerLoop->getNumber() << ">, outer <"
             << Sums.OuterLoop->getNumber() << ">:\n";
      for (const SlidingWindowSum &Sum : Sums.Sums) {
        dbgs() << "    ";
        Sum.Inst->print(fdbgs(), 0);
      }
    }
    dbgs() << "\n";
  });

  // Do the transform for the identified sums in each loop.
  for (const LoopSlidingWindowSums &Sums : LoopSums)
    transformLoopWindowSums(Sums);

  return true;
}

namespace {

/// A wrapper for running HIRSumWindowReuse with the old pass manager.
class HIRSumWindowReuseLegacyPass : public HIRTransformPass {
public:
  static char ID;
  HIRSumWindowReuseLegacyPass() : HIRTransformPass{ID} {
    initializeHIRSumWindowReuseLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<HIRFrameworkWrapperPass>();
    AU.addRequired<HIRDDAnalysisWrapperPass>();
    AU.addRequired<HIRSafeReductionAnalysisWrapperPass>();
    AU.setPreservesAll();
  }
};

} // namespace

char HIRSumWindowReuseLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(HIRSumWindowReuseLegacyPass, OPT_SWITCH, OPT_DESC, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(HIRFrameworkWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRDDAnalysisWrapperPass)
INITIALIZE_PASS_DEPENDENCY(HIRSafeReductionAnalysisWrapperPass)
INITIALIZE_PASS_END(HIRSumWindowReuseLegacyPass, OPT_SWITCH, OPT_DESC, false,
                    false)

FunctionPass *llvm::createHIRSumWindowReusePass() {
  return new HIRSumWindowReuseLegacyPass{};
}

bool HIRSumWindowReuseLegacyPass::runOnFunction(Function &F) {
  if (skipFunction(F)) {
    return false;
  }

  return runHIRSumWindowReuse(
    getAnalysis<HIRFrameworkWrapperPass>().getHIR(),
    getAnalysis<HIRDDAnalysisWrapperPass>().getDDA(),
    getAnalysis<HIRSafeReductionAnalysisWrapperPass>().getHSR());
}

PreservedAnalyses
HIRSumWindowReusePass::run(Function &F, llvm::FunctionAnalysisManager &AM) {
  runHIRSumWindowReuse(AM.getResult<HIRFrameworkAnalysis>(F),
                       AM.getResult<HIRDDAnalysisPass>(F),
                       AM.getResult<HIRSafeReductionAnalysisPass>(F));
  return PreservedAnalyses::all();
}
