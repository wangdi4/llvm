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

/// A recognized sliding window sum.
struct SlidingWindowSum {

  /// The sum's inner loop.
  HLLoop *InnerLoop;

  /// The sum's outer loop.
  HLLoop *OuterLoop;

  /// The instruction performing the sum.
  const HLInst *Inst;

  /// The sum's opcode, which should either be FAdd or FSub for now.
  unsigned Opcode;

  /// The load for the sum terms.
  const RegDDRef *TermLoad;

  /// Constructs a SlidingWindowSum given values for its fields.
  SlidingWindowSum(HLLoop *InnerLoop, HLLoop *OuterLoop, const HLInst *Inst,
                   unsigned Opcode, const RegDDRef *TermLoad)
      : InnerLoop{InnerLoop}, OuterLoop{OuterLoop}, Inst{Inst}, Opcode{Opcode},
        TermLoad{TermLoad} {
    assert(InnerLoop);
    assert(OuterLoop);
    assert(Inst);
    assert(Opcode == Instruction::FAdd || Opcode == Instruction::FSub);
    assert(TermLoad);
    assert(HLNodeUtils::contains(OuterLoop, InnerLoop));
    assert(HLNodeUtils::contains(InnerLoop, Inst));
    assert(Inst->getLLVMInstruction()->getOpcode() == Opcode);
    assert(TermLoad->getHLDDNode() == Inst);
  }
};

} // namespace

/// Determines whether \p TermLoad has an access pattern that is eligible for
/// optimization by this pass at loop levels \p InnerLevel and \p OuterLevel
/// according to \p DDG.
static bool isEligibleTermLoad(const RegDDRef *TermLoad, unsigned InnerLevel,
                               unsigned OuterLevel, const DDGraph &DDG) {

  // The sum terms must be loaded directly from memory with an invariant base
  // pointer.
  if (!TermLoad->isMemRef()) {
    LLVM_DEBUG(dbgs() << "  Sum terms are not loaded from memory\n\n");
    return false;
  }
  if (!TermLoad->getBaseCE()->isInvariantAtLevel(OuterLevel)) {
    LLVM_DEBUG(dbgs() << "  Base pointer is not invariant\n\n");
    return false;
  }

  // For now, restrict the term accesses to one-dimensional ones containing only
  // two IV terms for the inner and outer loops both with coefficient 1.
  if (TermLoad->getNumDimensions() != 1) {
    LLVM_DEBUG(dbgs() << "  Term load is not one-dimensional\n\n");
    return false;
  }
  const CanonExpr *const TermExpr = TermLoad->getSingleCanonExpr();
  assert(TermExpr);
  if (TermExpr->hasIVBlobCoeffs()) {
    LLVM_DEBUG(dbgs() << "  Term load has blob IV coefficients\n\n");
    return false;
  }
  if (TermExpr->getDenominator() != 1) {
    LLVM_DEBUG(dbgs() << "  Term load has non-1 denominator\n\n");
    return false;
  }
  if (TermExpr->numIVs() != 2) {
    LLVM_DEBUG(dbgs() << "  Term load has the wrong number of IVs\n\n");
    return false;
  }
  if (TermExpr->getIVConstCoeff(InnerLevel) != 1) {
    LLVM_DEBUG(dbgs() << "  Term load has non-1 inner IV coefficient\n\n");
    return false;
  }
  if (TermExpr->getIVConstCoeff(OuterLevel) != 1) {
    LLVM_DEBUG(dbgs() << "  Term load has non-1 outer IV coefficient\n\n");
    return false;
  }

  // Check that the accesses are independent within the outer loop.
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
  return true;
}

/// Locates eligible sliding window sums within an innermost loop \p InnerLoop.
/// Identified sliding window sums are appended to \p Sums.
static void findSlidingWindowSums(HLLoop *InnerLoop, HIRDDAnalysis &HDDA,
                                  HIRSafeReductionAnalysis &HSR,
                                  SmallVectorImpl<SlidingWindowSum> &Sums) {

  // The inner loop must be at least two levels deep.
  if (InnerLoop->getNestingLevel() == 1)
    return;

  // For now, ensure that the inner loop is a normalized DO loop.
  if (!InnerLoop->isDo() || !InnerLoop->isNormalized())
    return;

  // Find the outer loop, which for now must be the immediate parent loop.
  HLLoop *const OuterLoop = InnerLoop->getParentLoop();
  assert(OuterLoop);

  // For now, the outer loop should also be normalized but does not have to be a
  // DO loop.
  if (!OuterLoop->isNormalized())
    return;

  // The bounds and ZTTs of the inner loop need to be loop invariant in the
  // context of the outer loop.
  const unsigned InnerLevel = InnerLoop->getNestingLevel();
  const unsigned OuterLevel = OuterLoop->getNestingLevel();
  const DDGraph &DDG        = HDDA.getGraph(OuterLoop);
  for (const RegDDRef *const Ref :
       make_range(InnerLoop->ddref_begin(), InnerLoop->ddref_end())) {
    if (!Ref->isStructurallyInvariantAtLevel(OuterLevel)) {
      LLVM_DEBUG({
        dbgs() << "Inner loop bound/ZTT ref ";
        Ref->print(fdbgs());
        dbgs() << " is not structurally invariant\n";
      });
      return;
    }
  }

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

    // Check that the sum terms have the right access pattern.
    const RegDDRef *const TermLoad =
      SumInst->getOperandDDRef(1)->isTerminalRef()
        ? SumInst->getOperandDDRef(2)
        : SumInst->getOperandDDRef(1);
    if (!isEligibleTermLoad(TermLoad, InnerLevel, OuterLevel, DDG))
      continue;

    LLVM_DEBUG(dbgs() << "  Sum is eligible for optimization\n\n");

    // Collect the sum.
    Sums.emplace_back(InnerLoop, OuterLoop, SumInst, SumOpcode, TermLoad);
  }
}

/// Performs sum window reuse on a function.
static bool runHIRSumWindowReuse(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                                 HIRSafeReductionAnalysis &HSR) {
  if (DisablePass) {
    LLVM_DEBUG(dbgs() << OPT_DESC " Disabled\n");
    return false;
  }

  // Scan innermost loops that are at least two deep to collect eligible sums.
  SmallVector<SlidingWindowSum, 8> Sums;
  SmallVector<HLLoop *, 16> InnerLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(InnerLoops);
  for (HLLoop *InnerLoop : InnerLoops)
    findSlidingWindowSums(InnerLoop, HDDA, HSR, Sums);

  // Stop here if no sums were identified.
  if (Sums.empty()) {
    LLVM_DEBUG(dbgs() << "No window sums identified for optimization\n\n");
    return false;
  }

  // For now, just report identified sums.
  LLVM_DEBUG({
    dbgs() << "Identified these window sums for optimization:\n";
    for (const SlidingWindowSum &Sum : Sums)
      Sum.Inst->print(fdbgs(), 0);
    dbgs() << "\n";
  });

  return false;
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
