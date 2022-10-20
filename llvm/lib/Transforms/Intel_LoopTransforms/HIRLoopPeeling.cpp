//===------- HIRLoopPeeling.cpp - Implements Temp Cleanup pass ------------===//
//
// Copyright (C) 2015-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
// This pass performs peeling on loops to eliminate data dependencies preventing
// optimizations like vectorization.
//
// 1) Peelable dependency
//
// DO i1 = 0, %N
//  A[i1] = A[0] + B[i1]
// END DO
//
// Is transformed to-
//
// A[0] = A[0] + B[0]
//
// DO i1 = 0, %N-1
//  A[i1+1] = A[0] + B[i1+1]
// END DO
//
// This eliminates backward flow dependency A[i1] -> A[0] which would prevent
// vectorization.
//
// 2) Backward substitutible load
//
// DO i1 = 0, %N
//  A[i1] = %t
//  %t = B[i1]
// END DO
//
// Is transformed to-
//
// A[0] = %t
// %t = B[0] // needed if %t is liveout
//
// DO i1 = 0, %N-1
//  %t = B[i1]
//  A[i1+1] = %t
// END DO
//  %t = B[%N] // needed if %t is liveout
//
// This eliminates backward flow dependency %t -> %t which would prevent
// vectorization.
//
// 2) Backward substitutible linear temp
//
// DO i1 = 0, %N
//  A[i1] = %t
//  %t = i1
// END DO
//
// Is transformed to-
//
// A[0] = %t
// %t = 0  // needed if %t is liveout
//
// DO i1 = 0, %N-1
//  A[i1+1] = i1
// END DO
//  %t = %N // needed if %t is liveout
//
// This eliminates backward flow dependency %t -> %t which would prevent
// vectorization.

//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/HIRLoopPeelingPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRSafeReductionAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/BlobUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "hir-loop-peeling"

using namespace llvm;
using namespace llvm::loopopt;

static cl::opt<bool>
    DisableHIRLoopPeeling("disable-hir-loop-peeling", cl::init(false),
                          cl::Hidden, cl::desc("Disable HIR loop peeling"));

static cl::opt<unsigned> NumNodeThreshold(
    "hir-loop-peeling-num-node-threshold", cl::init(30), cl::Hidden,
    cl::desc("Threshold for number of nodes inside loop body"));
namespace {
struct TempInfo {
  RegDDRef *TempDef;
  SmallVector<DDRef *, 4> Uses;

  TempInfo(RegDDRef *TempDef, SmallVector<DDRef *, 4> &Uses)
      : TempDef(TempDef), Uses(Uses) {}
};

struct PeelingInfo {
  SmallVector<TempInfo, 3> SubstitutibleTemps;
};

class HIRLoopPeeling {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HIRSafeReductionAnalysis &HSRA;

public:
  HIRLoopPeeling(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                 HIRLoopStatistics &HLS, HIRSafeReductionAnalysis &HSRA)
      : HIRF(HIRF), HDDA(HDDA), HLS(HLS), HSRA(HSRA) {}

  void run();

  bool isPeelingCandidate(HLLoop *Lp, PeelingInfo &PeelInfo);
};

} // namespace

static bool canBackwardSubstitute(const HLInst *Inst, const RegDDRef *LvalRef,
                                  unsigned LoopLevel, DDGraph DDG,
                                  bool *IsLinear) {

  // Looking for a candidate like: t = A[i1]
  if (isa<LoadInst>(Inst->getLLVMInstruction())) {

    auto *LoadRef = Inst->getRvalDDRef();

    // Check if load can be structurally backward susbstituted.
    if (!LoadRef->isLinearAtLevel(LoopLevel)) {
      return false;
    }

    // Check if dependencies preventing load from getting backward susbstituted.
    // TODO: replace with hasNum*() interface when available
    if (DDG.getNumIncomingEdges(LoadRef) || DDG.getNumOutgoingEdges(LoadRef)) {
      LLVM_DEBUG(
          dbgs()
          << "Dependencies prevent load temp from backward substitution!\n");
      return false;
    }

    return true;
  }

  // Looking for a candidate like: t = i1+1;
  if (!LvalRef->isTerminalRef()) {
    return false;
  }

  auto *TempCE = LvalRef->getSingleCanonExpr();

  if ((TempCE->getDenominator() != 1) || !TempCE->isLinearAtLevel(LoopLevel)) {
    LLVM_DEBUG(dbgs() << "Temp cannot be structually backward substituted!\n");
    return false;
  }

  *IsLinear = true;
  return true;
}

static bool isMergeableUse(unsigned TempIndex, const DDRef *UseRef) {

  if (isa<RegDDRef>(UseRef)) {
    return true;
  }

  auto *UseRegRef = cast<BlobDDRef>(UseRef)->getParentDDRef();

  if (UseRegRef->isTerminalRef()) {
    return UseRegRef->getSingleCanonExpr()->containsStandAloneBlob(TempIndex);
  }

  unsigned NumDims = UseRegRef->getNumDimensions();

  for (unsigned I = 1; I < NumDims; ++I) {
    // IV is not allowed in lower/stride so we cannot substitute linear temp.
    if (UseRegRef->getDimensionLower(I)->containsTempBlob(TempIndex) ||
        UseRegRef->getDimensionStride(I)->containsTempBlob(TempIndex)) {
      return false;
    }

    auto *IndexCE = UseRegRef->getDimensionIndex(I);
    // TODO: extend it to handle non-unit coefficient of temp blob
    if (IndexCE->containsTempBlob(TempIndex) &&
        !IndexCE->containsStandAloneBlob(TempIndex)) {
      return false;
    }
  }

  return true;
}

bool HIRLoopPeeling::isPeelingCandidate(HLLoop *Lp, PeelingInfo &PeelInfo) {

  if (!Lp->isDo() || !Lp->isNormalized()) {
    return false;
  }

  // Just a heuristic to not increase code size too much.
  if (Lp->getNumChildren() > NumNodeThreshold) {
    return false;
  }

  const LoopStatistics &LS = HLS.getSelfLoopStatistics(Lp);

  // Vectorizer currently doesn't handle switch inst.
  // We disallow Ifs as well to make the node threshold check more precise but
  // this can be improved later.
  if (LS.hasCallsWithUnsafeSideEffects() || LS.hasCallsWithUnknownAliasing() ||
      LS.hasIfs() || LS.hasSwitches() || LS.hasLabels()) {
    return false;
  }

  DDGraph DDG = HDDA.getGraph(Lp);
  unsigned LoopLevel = Lp->getNestingLevel();
  bool ComputedSafeReductions = false;
  bool FoundPeelableCandidates = false;

  // TODO: Convert to visitor when extending pass to handle Ifs.
  for (auto &Node : Lp->children()) {
    auto *Inst = dyn_cast<HLInst>(&Node);

    if (!Inst) {
      continue;
    }

    auto *LvalRef = Inst->getLvalDDRef();

    if (!LvalRef) {
      continue;
    }

    bool IsLinear = false;
    bool IsPeelingCandidate = false;
    bool IsSubstitutible = false;
    unsigned TempIndex = InvalidBlobIndex;

    SmallVector<DDRef *, 4> UseRefs;

    for (auto *Edge : DDG.outgoing(LvalRef)) {
      if (!Edge->preventsVectorization(LoopLevel)) {
        continue;
      }

      if (!Edge->isFlow()) {
        return false;
      }

      if (Edge->firstIterPeelingRemovesDep()) {
        IsPeelingCandidate = true;
        continue;
      }

      if (!IsSubstitutible) {
        IsSubstitutible =
            canBackwardSubstitute(Inst, LvalRef, LoopLevel, DDG, &IsLinear);

        if (IsLinear) {
          TempIndex =
              LvalRef->getBlobUtils().findTempBlobIndex(LvalRef->getSymbase());
        }
      }

      if (IsSubstitutible) {
        auto *UseRef = Edge->getSink();

        if (IsLinear && !isMergeableUse(TempIndex, UseRef)) {
          LLVM_DEBUG(dbgs() << "Linear temp is not mergeable!\n");
          return false;
        }

        IsPeelingCandidate = true;
        UseRefs.push_back(UseRef);
        continue;
      }

      // Checking safe reduction edges allows the transformation to more
      // precisely check if peeling can result in vectorization.
      if (!ComputedSafeReductions) {
        HSRA.computeSafeReductionChains(Lp);
        ComputedSafeReductions = true;
      }

      auto *SRI = HSRA.getSafeRedInfo(Inst);

      // Ignore safe reduction edges.
      if (SRI && !SRI->HasUnsafeAlgebra) {
        break;
      }

      return false;
    }

    if (IsPeelingCandidate) {
      if (IsSubstitutible) {
        LLVM_DEBUG(dbgs() << "Backward substitutible "
                          << (IsLinear ? "linear" : "load")
                          << " temp found!\n");

        assert(!UseRefs.empty() &&
               "Uses cannot be empty for backward substitutible temps!");

        PeelInfo.SubstitutibleTemps.emplace_back(LvalRef, UseRefs);

      } else {
        LLVM_DEBUG(dbgs() << "Peelable dependency found!\n");
      }

      FoundPeelableCandidates = true;
    }
  }

  return FoundPeelableCandidates;
}

void HIRLoopPeeling::run() {

  if (DisableHIRLoopPeeling) {
    LLVM_DEBUG(dbgs() << "Peeling is disabled!\n");
    return;
  }

  // Can be extended for outer loops to help optimizations like unroll & jam.
  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  // OptReportBuilder &ORBuilder = HIRF.getORBuilder();

  for (auto *Lp : CandidateLoops) {
    PeelingInfo PeelInfo;

    if (!isPeelingCandidate(Lp, PeelInfo)) {
      continue;
    }

    LLVM_DEBUG(dbgs() << "Loop peeling candidate found!\n");

    // ID: 25579u, remark string: Loop was peeled
    // ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25579u);
  }
}

PreservedAnalyses HIRLoopPeelingPass::runImpl(llvm::Function &F,
                                              llvm::FunctionAnalysisManager &AM,
                                              HIRFramework &HIRF) {
  HIRLoopPeeling(HIRF, AM.getResult<HIRDDAnalysisPass>(F),
                 AM.getResult<HIRLoopStatisticsAnalysis>(F),
                 AM.getResult<HIRSafeReductionAnalysisPass>(F))
      .run();
  return PreservedAnalyses::all();
}
