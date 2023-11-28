//===----- HIRCompleteUnroll.h - Implements complete unroll ---------------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Complete unroll pass.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCOMPLETE_UNROLLIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRCOMPLETE_UNROLLIMPL_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

class DominatorTree;
class TargetTransformInfo;
#if INTEL_FEATURE_SW_DTRANS
class DTransImmutableInfo;
#endif // INTEL_FEATURE_SW_DTRANS

namespace loopopt {
class HIRLoopStatistics;
class HIRDDAnalysis;
class HIRSafeReductionAnalysis;

struct UnrollThresholds {
  unsigned LoopTripThreshold;
  unsigned MultiExitLoopTripThreshold;
  unsigned LoopnestTripThreshold;
  unsigned SavingsThreshold;
  unsigned UnrolledLoopMemRefThreshold;
  unsigned UnrolledLoopDDRefThreshold;
  unsigned SmallLoopMemRefThreshold;
  unsigned SmallLoopDDRefThreshold;
  unsigned SmallLoopAdditionalSavingsThreshold;
  float MaxThresholdScalingFactor;
};

class HIRCompleteUnroll {
public:
  HIRCompleteUnroll(HIRFramework &HIRF, DominatorTree &DT,
                    const TargetTransformInfo &TTI,
                    const TargetLibraryInfo &TLI, HIRLoopStatistics &HLS,
                    HIRDDAnalysis &DDA, HIRSafeReductionAnalysis &HSRA,
#if INTEL_FEATURE_SW_DTRANS
                    DTransImmutableInfo *DTII,
#endif // INTEL_FEATURE_SW_DTRANS
                    unsigned OptLevel, bool IsPreVec, bool PragmaOnlyUnroll);

  bool run();

  typedef DDRefGatherer<const RegDDRef, MemRefs> MemRefGatherer;

  /// Performs complete unroll on \p Loop;
  static void doUnroll(HLLoop *Loop);

private:
  struct CanonExprUpdater;
  class ProfitabilityAnalyzer;

  HIRFramework &HIRF;
  DominatorTree &DT;
  const TargetTransformInfo &TTI;
  const TargetLibraryInfo &TLI;
  HIRLoopStatistics &HLS;
  HIRDDAnalysis &DDA;
  HIRSafeReductionAnalysis &HSRA;
#if INTEL_FEATURE_SW_DTRANS
  DTransImmutableInfo *DTII;
#endif // INTEL_FEATURE_SW_DTRANS

  /// Indicates whether we are in pre or post vec mode.
  bool IsPreVec;
  // Indicates whether only pragma enabled unrolling is allowed.
  bool PragmaOnlyUnroll;

  /// The total number of ddrefs in the loops of the function which are
  /// profitable for unrolling.
  unsigned CumulativeProfitableLoopDDRefs;

  /// Storage for loops which will be transformed.
  /// Only outermost loops to be transformed will be stored.
  SmallVector<HLLoop *, 32> CandidateLoops;

  // Caches average trip count of loops for profitability analysis.
  DenseMap<const HLLoop *, unsigned> AvgTripCount;

  // Caches total trip of of the loopnest for profitability analysis.
  DenseMap<const HLLoop *, unsigned> TotalTripCount;

  // Loop in this set can be processed as top level candidates for unrolling the
  // loopnest.
  SmallPtrSet<const HLLoop *, 32> TopLevelCandidates;

  // Structure holding thresholds for complete unroll.
  UnrollThresholds Limits;

  struct SimplifiableStoreInfo {
    const RegDDRef *StoreRef;
    const HLLoop *TopLevelParentLoop;
  };

  // Maps stores (mapped by base ptr blob index) from previous
  // profitable loopnests. This is used to evaluate
  // profitability for loads in later loopnests. We intentionally store
  // base ptr blob indices instead of storing store symbases because
  // symbases for same base ptr based refs can be different across regions so
  // A[i1] in previous region can have a different symbase than A[i1] in the
  // next region.
  DenseMap<unsigned, SimplifiableStoreInfo> PrevLoopnestSimplifiableStores;

private:
  /// Returns true if loop is eligible for complete unrolling.
  bool isApplicable(const HLLoop *Loop) const;

  /// Returns true if we cannot handle loop liveouts after unrolling.
  bool cannotHandleLiveouts(const HLLoop *Loop, int64_t MinUpper) const;

  /// Computes and returns average trip count and dependence level of the loop
  /// for profitability analysis.
  /// Returns a trip count of -1 if it cannot be computed. Dependence level is
  /// same as nesting level if the loop does not depend on any outer loop being
  /// unrolled.
  std::pair<int64_t, unsigned> computeAvgTripCount(const HLLoop *Loop);

  /// Refines the candidates based on profitability and dependence on outer
  /// loops.
  /// For example, we may have the following case:
  /// for(i=0;i<15; i++)
  ///   for(j=0; j<i; j++)
  /// Here, j loop is added as candidate, but because of profitability, we
  /// don't add 'i' loop, then we remove 'j' loop also.
  void refineCandidates();

  /// Returns true if loop is profitable for complete unrolling.
  bool isProfitable(const HLLoop *Loop);

  /// Computes constant upper bound of \p Loop by substituting outer loop trip
  /// counts by their respective IVs in the upper.
  static int64_t computeUB(HLLoop *Loop, unsigned TopLoopLevel,
                           SmallVectorImpl<int64_t> &IVValues);

  /// Performs the complete unrolling transformation.
  static void transformLoop(HLLoop *Loop, CanonExprUpdater &CEUpdater,
                            bool IsTopLevelLoop);

  /// Main routine to drive the complete unrolling transformation.
  void processCompleteUnroll(SmallVectorImpl<HLLoop *> &OuterLoops);

  /// Performs trip count analysis on the loopnest represented by \p Loop.
  /// Returns the avg trip count of the loopnest and its dependence level.
  /// Non-negative value indicates that loopnest is a candidate.
  std::pair<int64_t, unsigned> performTripCountAnalysis(HLLoop *Loop);

  /// Routine to drive the transformation of candidate loops.
  void transformLoops();
};

class HIRCompleteUnrollLegacyPass : public HIRTransformPass {
  unsigned OptLevel;
  bool IsPreVec;
  bool PragmaOnlyUnroll;

public:
  static char ID;

  HIRCompleteUnrollLegacyPass(char &ID, unsigned OptLevel, bool IsPreVec,
                              bool PragmaOnlyUnroll)
      : HIRTransformPass(ID), OptLevel(OptLevel), IsPreVec(IsPreVec),
        PragmaOnlyUnroll(PragmaOnlyUnroll) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnFunction(Function &F) override;
};

} // namespace loopopt
} // namespace llvm

#endif
