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
// optimizations like vectorization. Currently, we only peel first iteration.
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
// 2) Backward substitutable load
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
// 2) Backward substitutable linear temp
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
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HIRInvalidationUtils.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/HIRTransformUtils.h"

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
  HLInst *DefInst;
  SmallVector<DDRef *, 4> Uses;

  TempInfo(HLInst *DefInst, SmallVector<DDRef *, 4> &Uses)
      : DefInst(DefInst), Uses(Uses) {}
};

struct PeelingInfo {
  SmallVector<TempInfo, 3> SubstitutableTemps;
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

  void peelLoop(HLLoop *Lp, PeelingInfo &PeelInfo);
};

} // namespace

static bool canBackwardSubstitute(const HLInst *Inst, const RegDDRef *LvalRef,
                                  unsigned LoopLevel, DDGraph DDG,
                                  bool *IsLinear) {

  // Check if forward dependencies prevent substitution.
  // TODO: add check for incoming output edges when extending to handle control
  // flow.
  for (auto *Edge : DDG.outgoing(LvalRef)) {
    if (Edge->isForwardDep()) {
      return false;
    }
  }

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
  auto *TempCE = LvalRef->getSingleCanonExpr();

  if ((TempCE->getDenominator() != 1) ||
      (TempCE->getSrcType() != TempCE->getDestType()) ||
      !TempCE->isLinearAtLevel(LoopLevel)) {
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

  for (unsigned I = 1; I <= NumDims; ++I) {
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

  if (!Lp->isDo() || !Lp->isNormalized() || Lp->isSIMD() ||
      Lp->hasVectorizeDisablingPragma()) {
    return false;
  }

  if (Lp->hasLikelySmallTripCount()) {
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
    bool IsSubstitutable = false;
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

      if (!LvalRef->isTerminalRef()) {
        return false;
      }

      if (!IsSubstitutable) {
        IsSubstitutable =
            canBackwardSubstitute(Inst, LvalRef, LoopLevel, DDG, &IsLinear);

        if (IsLinear) {
          TempIndex =
              LvalRef->getBlobUtils().findTempBlobIndex(LvalRef->getSymbase());
        }
      }

      if (IsSubstitutable) {
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
      if (IsSubstitutable) {
        LLVM_DEBUG(dbgs() << "Backward substitutable "
                          << (IsLinear ? "linear" : "load")
                          << " temp found!\n");

        assert(!UseRefs.empty() &&
               "Uses cannot be empty for backward substitutable temps!");

        PeelInfo.SubstitutableTemps.emplace_back(Inst, UseRefs);

      } else {
        LLVM_DEBUG(dbgs() << "Peelable dependency found!\n");
      }

      FoundPeelableCandidates = true;
    }
  }

  return FoundPeelableCandidates;
}

static void replaceLinearTempInUse(unsigned TempIndex, RegDDRef *LinearTemp,
                                   DDRef *UseRef) {

  RegDDRef *UseRegRef = isa<BlobDDRef>(UseRef)
                            ? cast<BlobDDRef>(UseRef)->getParentDDRef()
                            : cast<RegDDRef>(UseRef);
  bool Replaced = false;
  (void)Replaced;

  for (unsigned I = 1, NumDims = UseRegRef->getNumDimensions(); I <= NumDims;
       I++) {
    auto *IndexCE = UseRegRef->getDimensionIndex(I);

    if (!IndexCE->containsTempBlob(TempIndex)) {
      continue;
    }

    Replaced = true;
    CanonExprUtils::replaceStandAloneBlobByCanonExpr(
        IndexCE, TempIndex, LinearTemp->getSingleCanonExpr());
  }

  assert(Replaced && "Could not find temp to replace!");

  UseRegRef->makeConsistent(LinearTemp);
}

/// Inserts first definition of substitutable temps before loop by replacing
/// loop IV by 0. This is needed for correct code generation in case original
/// loop's trip count is 1.
static void intializeLiveoutSubstitutableTemps(HLLoop *Lp,
                                               PeelingInfo &PeelInfo) {

  /// For constant trip count loops, rest of the loop body will definitely
  /// execute so the last temp definition will be in loop postexit.
  if (Lp->isConstTripLoop()) {
    return;
  }

  unsigned LoopLevel = Lp->getNestingLevel();

  for (auto &Temp : PeelInfo.SubstitutableTemps) {
    unsigned LvalSymbase = Temp.DefInst->getLvalDDRef()->getSymbase();

    if (!Lp->isLiveOut(LvalSymbase)) {
      continue;
    }

    auto *TempInit = Temp.DefInst->clone();

    if (!isa<LoadInst>(Temp.DefInst->getLLVMInstruction())) {
      TempInit->getLvalDDRef()->makeSelfBlob(true);
    }

    for (auto OpRef : make_range(TempInit->rval_op_ddref_begin(),
                                 TempInit->rval_op_ddref_end())) {
      OpRef->replaceIVByConstant(LoopLevel, 0);
      OpRef->makeConsistent({}, LoopLevel - 1);
    }

    HLNodeUtils::insertBefore(Lp, TempInit);
  }
}

/// Replaces loop IV by \p TripCountCE in \p LiveoutTempDef and inserts it as \p
/// Lp's first postexit node.
static void insertLiveoutDefToPostexit(HLInst *LiveoutTempDef, HLLoop *Lp,
                                       const CanonExpr *TripCountCE) {
  unsigned LoopLevel = Lp->getNestingLevel();
  bool HasSignedIV = Lp->hasSignedIV();

  const SmallVector<const RegDDRef *, 1> Aux = {Lp->getUpperDDRef()};

  for (auto OpRef : make_range(LiveoutTempDef->rval_op_ddref_begin(),
                               LiveoutTempDef->rval_op_ddref_end())) {
    DDRefUtils::replaceIVByCanonExpr(OpRef, LoopLevel, TripCountCE,
                                     HasSignedIV);
    OpRef->makeConsistent(Aux, LoopLevel - 1);

  }

  HLNodeUtils::insertAsFirstPostexitNode(Lp, LiveoutTempDef);
}

void HIRLoopPeeling::peelLoop(HLLoop *Lp, PeelingInfo &PeelInfo) {

  // Remove all temp candidates as they don't need to be updated (by replacing
  // i1 by i1+1) when peeling the loop.
  for (auto &Temp : PeelInfo.SubstitutableTemps) {
    HLNodeUtils::remove(Temp.DefInst);
  }

  auto *FirstIterLoop = Lp->peelFirstIteration();

  FirstIterLoop->replaceByFirstIteration();

  intializeLiveoutSubstitutableTemps(Lp, PeelInfo);

  auto *TripCountCE = Lp->getTripCountCanonExpr();

  // We process in reverse order to maintain lexical ordering of substitutable
  // temps.
  for (auto &TempInfo : make_range(PeelInfo.SubstitutableTemps.rbegin(),
                                   PeelInfo.SubstitutableTemps.rend())) {

    auto *DefInst = TempInfo.DefInst;
    auto *LvalRef = DefInst->getLvalDDRef();
    unsigned LvalSymbase = LvalRef->getSymbase();

    auto *LiveoutTempDef = Lp->isLiveOut(LvalSymbase) ? DefInst : nullptr;

    // Temp is no longer livein after backwards substitution.
    Lp->removeLiveInTemp(LvalSymbase);
    // It is also not liveout as the last definition will be moved to postexit.
    Lp->removeLiveOutTemp(LvalSymbase);

    if (isa<LoadInst>(TempInfo.DefInst->getLLVMInstruction())) {

      bool Replaced = false;

      // Single use load temp can be eliminated by replacing it in the use
      // directly.
      if ((TempInfo.Uses.size() == 1)) {
        if (auto *RegUseRef = dyn_cast<RegDDRef>(TempInfo.Uses[0])) {
          // If temp is liveout we need the original inst to move to postexit so
          // we cannot reuse its rval ref.
          auto *LoadRef = LiveoutTempDef ? DefInst->getRvalDDRef()->clone()
                                         : DefInst->removeRvalDDRef();
          HIRTransformUtils::replaceOperand(RegUseRef, LoadRef);
          Replaced = true;
        }
      }

      if (!Replaced) {
        HLNodeUtils::insertAsFirstChild(Lp, DefInst);
        LiveoutTempDef = LiveoutTempDef ? LiveoutTempDef->clone() : nullptr;
      }

    } else {
      // Linear temp case

      unsigned TempIndex =
          LvalRef->getBlobUtils().findTempBlobIndex(LvalSymbase);

      for (auto *Use : TempInfo.Uses) {
        replaceLinearTempInUse(TempIndex, LvalRef, Use);
      }

      if (LiveoutTempDef) {
        LiveoutTempDef->getLvalDDRef()->makeSelfBlob(true);
      }
    }

    if (LiveoutTempDef) {
      insertLiveoutDefToPostexit(LiveoutTempDef, Lp, TripCountCE);
    }
  }

  HIRInvalidationUtils::invalidateBody<HIRLoopStatistics,
                                       HIRSafeReductionAnalysis>(Lp);
  HIRInvalidationUtils::invalidateParentLoopBodyOrRegion(Lp);
}

void HIRLoopPeeling::run() {

  if (DisableHIRLoopPeeling) {
    LLVM_DEBUG(dbgs() << "Peeling is disabled!\n");
    return;
  }

  // Can be extended for outer loops to help optimizations like unroll & jam.
  SmallVector<HLLoop *, 64> CandidateLoops;
  HIRF.getHLNodeUtils().gatherInnermostLoops(CandidateLoops);

  OptReportBuilder &ORBuilder = HIRF.getORBuilder();

  for (auto *Lp : CandidateLoops) {
    PeelingInfo PeelInfo;

    if (!isPeelingCandidate(Lp, PeelInfo)) {
      continue;
    }

    LLVM_DEBUG(dbgs() << "Loop peeling candidate found!\n");

    peelLoop(Lp, PeelInfo);
    // remark string: Loop peeled to eliminate data dependence
    ORBuilder(*Lp).addRemark(OptReportVerbosity::Low, 25487u);
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
