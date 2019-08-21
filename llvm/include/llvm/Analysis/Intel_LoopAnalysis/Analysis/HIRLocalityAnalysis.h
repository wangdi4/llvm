//===------ HIRLocalityAnalysis.h - Provides Locality Analysis ---*- C++-*-===//
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
// The purpose of this analysis is to provide Locality Analysis for a loop nest.
// The analysis is only triggered on-demand. However, this analysis stores the
// locality information once it is computed and caches the information for
// future reuse.
//
// We classify locality into three categories: spatial locality, temporal
// invariant locality and temporal reuse locality.
//
// Whenever a transformation updates the loop, it has to mark the loop nest
// as modified. The transformation needs to call the mark methods provided here.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_LOCALITY_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_LOCALITY_H

#include <array>

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGatherer.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"

namespace llvm {

namespace loopopt {

class HLNode;
class HLLoop;
class DDRefUtils;
class HIRFramework;

class HIRLoopLocality : public HIRAnalysis {
public:
  typedef DDRefGrouping::RefGroupTy<const RegDDRef *> RefGroupTy;
  typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;

private:
  typedef DDRefGatherer<const RegDDRef, MemRefs> LocalityRefGatherer;

  struct LocalityInfo {
    uint64_t NumSpatialCacheLines;
    uint64_t NumTempInvCacheLines;
    unsigned TotalRefs;
    unsigned TotalLvalRefs;
    uint64_t TotalStride;
    uint64_t TotalLvalStride;

    LocalityInfo()
        : NumSpatialCacheLines(0), NumTempInvCacheLines(0), TotalRefs(0),
          TotalLvalRefs(0), TotalStride(0), TotalLvalStride(0) {}

    uint64_t getNumCacheLines() const {
      return NumSpatialCacheLines + NumTempInvCacheLines;
    }

    uint64_t getAvgStride() const {
      return (TotalRefs == 0) ? 0 : TotalStride / TotalRefs;
    }

    uint64_t getAvgLvalStride() const {
      return (TotalLvalRefs == 0) ? 0 : TotalLvalStride / TotalLvalRefs;
    }

    void clear() {
      NumSpatialCacheLines = NumTempInvCacheLines = TotalRefs = TotalLvalRefs =
          TotalStride = TotalLvalStride = 0;
    }
  };

  // Maintains locality info per level. This is used by loop interchange.
  std::array<LocalityInfo, MaxLoopNestLevel> LocalityByLevel;

  // Stores trip count info by level. Loops with non-const trip counts are also
  // assigned a constant trip count for locality computation.
  std::array<uint64_t, MaxLoopNestLevel> TripCountByLevel;

  static void updateTotalStrideAndRefs(LocalityInfo &LI,
                                       const RefGroupTy &RefGroup,
                                       uint64_t Stride);

  /// Returns true if the last cache line of PrevRef is the same as CurRef based
  /// on distance and number of bytes accessed.
  static bool sharesLastCacheLine(uint64_t PrevTotalDist, uint64_t CurTotalDist,
                                  uint64_t NumRefBytesAccessed);

  /// Computes extra cache lines accessed by the group using other references
  /// except the first.
  static uint64_t computeExtraCacheLines(LocalityInfo &LI,
                                         const RefGroupTy &RefGroup,
                                         unsigned Level,
                                         uint64_t NumRefBytesAccessed,
                                         uint64_t NumCacheLinesPerRef);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// no spatial or temporal locality.
  static void computeNumNoLocalityCacheLines(LocalityInfo &LI,
                                             const RefGroupTy &RefGroup,
                                             unsigned Level, uint64_t TripCnt);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// temporal invariant locality.
  static void computeNumTempInvCacheLines(LocalityInfo &LI,
                                          const RefGroupTy &RefGroup,
                                          unsigned Level);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// spatial locality.
  static void computeNumSpatialCacheLines(LocalityInfo &LI,
                                          const RefGroupTy &RefGroup,
                                          unsigned Level, uint64_t TripCnt,
                                          uint64_t Stride);

  /// Computes total number of cache lines accessed by \p Loop.
  void computeNumCacheLines(const HLLoop *Loop, const RefGroupVecTy &RefGroups);

  /// Computes the locality for the loopnest with outermost loop \p Lp.
  void computeLoopNestLocality(const HLLoop *Lp,
                               const SmallVectorImpl<const HLLoop *> &LoopVec,
                               RefGroupVecTy *SpatialLocalityGroups = nullptr);

  /// Computes the locality for loop \p Lp.
  void computeLoopLocality(const HLLoop *Lp,
                           RefGroupVecTy *SpatialLocalityGroups = nullptr) {
    SmallVector<const HLLoop *, 1> LoopVec;
    LoopVec.push_back(Lp);
    computeLoopNestLocality(Lp, LoopVec, SpatialLocalityGroups);
  }

  /// Returns the trip count of the loop.
  /// If loop count is symbolic or above the threshold, it returns
  /// SymbolicConst value.
  uint64_t getTripCount(const HLLoop *Loop);

  /// Initializes the trip count cache for future use inside the locality
  /// computation.
  void initTripCountByLevel(const SmallVectorImpl<const HLLoop *> &Loops);

  /// Prints out the Locality Information.
  void printLocalityInfo(raw_ostream &OS, const HLLoop *L) const;

  /// Implements getTemporalLocality().
  unsigned getTemporalLocalityImpl(const HLLoop *Lp, unsigned ReuseThreshold,
                                   bool CheckPresence, bool ReuseOnly);

  /// Implements getTemporalInvariantLocality().
  unsigned getTemporalInvariantLocalityImpl(const HLLoop *Lp,
                                            bool CheckPresence);

public:
  HIRLoopLocality(HIRFramework &HIRF) : HIRAnalysis(HIRF) {}
  HIRLoopLocality(HIRLoopLocality &&Arg)
      : HIRAnalysis(Arg.HIRF), LocalityByLevel(std::move(Arg.LocalityByLevel)),
        TripCountByLevel(std::move(Arg.TripCountByLevel)) {}
  HIRLoopLocality(const HIRLoopLocality &) = delete;

  void printAnalysis(raw_ostream &OS) const override;

  /// Nothing to do.
  void markLoopBodyModified(const HLLoop *Lp) override {}

  /// Returns a sorted list of loops from lower to higher (where higher is
  /// better) based on locality value. This interface expects a near-perfect
  /// loopnest.
  void sortedLocalityLoops(
      const HLLoop *OutermostLoop,
      SmallVector<const HLLoop *, MaxLoopNestLevel> &SortedLoops);

  /// Returns true if loop has any temporal (invariant + reuse) locality using
  /// \p ReuseThreshold.
  bool hasTemporalLocality(const HLLoop *Lp, unsigned ReuseThreshold) {
    return getTemporalLocalityImpl(Lp, ReuseThreshold, true, false);
  }

  /// Returns a number which represents the instances of temporal (invariant +
  /// reuse) locality inside \p Lp using \p ReuseThreshold.
  unsigned getTemporalLocality(const HLLoop *Lp, unsigned ReuseThreshold) {
    return getTemporalLocalityImpl(Lp, ReuseThreshold, false, false);
  }

  /// Returns true if loop has any temporal invariant locality.
  bool hasTemporalInvariantLocality(const HLLoop *Lp) {
    return getTemporalInvariantLocalityImpl(Lp, true);
  }

  /// Returns a number which represents the instances of temporal invariant
  /// locality in \p Lp.
  unsigned getTemporalInvariantLocality(const HLLoop *Lp) {
    return getTemporalInvariantLocalityImpl(Lp, false);
  }

  /// Returns true if loop has any temporal reuse locality using \p
  /// ReuseThreshold.
  bool hasTemporalReuseLocality(const HLLoop *Lp, unsigned ReuseThreshold) {
    return getTemporalLocalityImpl(Lp, ReuseThreshold, true, true);
  }

  /// Returns a number which represents the instances of temporal reuse locality
  /// in \p Lp using \p ReuseThreshold.
  unsigned getTemporalReuseLocality(const HLLoop *Lp, unsigned ReuseThreshold) {
    return getTemporalLocalityImpl(Lp, ReuseThreshold, false, true);
  }

  /// Populates \p TemporalGroups with memref groups which have temporal
  /// locality at loop \p Level within \p ReuseThreshold.
  /// \p Level can be set to zero when \p ReuseThreshold is zero.
  ///
  /// Examples-
  /// 1) A[2*i], A[2*i+1], A[2*i+2], A[2*i+3]
  ///    Group 0 - A[2*i], A[2*i+2]
  ///    Group 1 - A[2*i+1], A[2*i+3]
  /// 2) A[i], A[i+2], A[i+4], A[i+6]; ReuseThreshold = 3
  ///    Group 0 - A[i], A[i+2]
  ///    Group 1 - A[i+4], A[i+6]
  ///
  /// If \p UniqueGroupSymbases is non-null, we populate it with the symbases
  /// which are unique to a single temporal locality group. This can prove
  /// legality of transformation for the client without looking at DD edges
  /// essentially saving compile time.
  //
  /// Function also accumulates fake refs otherwise the info in
  /// UniqueGroupSymbases might be wrong.
  static void populateTemporalLocalityGroups(
      HLContainerTy::const_iterator Begin, HLContainerTy::const_iterator End,
      unsigned Level, unsigned ReuseThreshold, RefGroupVecTy &TemporalGroups,
      SmallSet<unsigned, 8> *UniqueGroupSymbases = nullptr);

  /// Refer to the iterator range version of the function above for description.
  static void populateTemporalLocalityGroups(
      const HLLoop *Lp, unsigned ReuseThreshold, RefGroupVecTy &TemporalGroups,
      SmallSet<unsigned, 8> *UniqueGroupSymbases = nullptr) {
    populateTemporalLocalityGroups(Lp->child_begin(), Lp->child_end(),
                                   Lp->getNestingLevel(), ReuseThreshold,
                                   TemporalGroups, UniqueGroupSymbases);
  }

  /// Populates \p TemporalGroups with memref groups which represent equivalent
  /// addresses like A[i] and (i32*)A[i]. Bitcast destination type may be
  /// ignored.
  static void
  populateEqualityGroups(HLContainerTy::const_iterator Begin,
                         HLContainerTy::const_iterator End,
                         RefGroupVecTy &TemporalGroups,
                         SmallSet<unsigned, 8> *UniqueGroupSymbases = nullptr) {
    populateTemporalLocalityGroups(Begin, End, 0, 0, TemporalGroups,
                                   UniqueGroupSymbases);
  }

  /// Populates \p SpatialGroups with memref groups which have spatial locality.
  /// For example, if the loop contains these refs-
  /// A[2*i], A[2*i+1], B[0], B[2]
  ///
  /// We will form these two groups-
  /// 1) A[2*i], A[2*i+1]
  /// 2) B[0], B[2]
  void populateSpatialLocalityGroups(const HLLoop *Lp,
                                     RefGroupVecTy &SpatialGroups);

  /// Returns the estimated number of cache lines accessed by the loop. Also
  /// populates SpatialGroups formed using CacheLineSize as the ReuseThreshold.
  /// For example, given a loop with trip count of 100 and i32 references A[i]
  /// and A[i+2], the number of cache lines accessed = ceil((400 + 8) / 64) = 7.
  /// 400 bytes are accessed by A[i] (from A[0] to A[99]) and extra 8 bytes due
  /// to A[i+2]. Cache line size is assumed to be 64 bytes. A[0] is assumed to
  /// hit the beginning of the cache line. The spatial locality groups used to
  /// compute number of cache lines are populated in \p SpatialGroups if passed
  /// by the user.
  uint64_t getNumCacheLines(const HLLoop *Lp,
                            RefGroupVecTy *SpatialGroups = nullptr);
};

class HIRLoopLocalityWrapperPass : public FunctionPass {
  std::unique_ptr<HIRLoopLocality> HLL;

public:
  static char ID;
  HIRLoopLocalityWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHLL().printAnalysis(OS);
  }

  HIRLoopLocality &getHLL() { return *HLL; }
  const HIRLoopLocality &getHLL() const { return *HLL; }
};

class HIRLoopLocalityAnalysis
    : public AnalysisInfoMixin<HIRLoopLocalityAnalysis> {
  friend struct AnalysisInfoMixin<HIRLoopLocalityAnalysis>;

  static AnalysisKey Key;

public:
  using Result = HIRLoopLocality;

  HIRLoopLocality run(Function &F, FunctionAnalysisManager &AM);
};

class HIRLoopLocalityPrinterPass
    : public PassInfoMixin<HIRLoopLocalityPrinterPass> {
  raw_ostream &OS;

public:
  explicit HIRLoopLocalityPrinterPass(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    AM.getResult<HIRLoopLocalityAnalysis>(F).printAnalysis(OS);
    return PreservedAnalyses::all();
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
