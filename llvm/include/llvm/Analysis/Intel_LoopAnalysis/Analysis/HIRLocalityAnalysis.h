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
    unsigned NumSpatialCacheLines;
    unsigned NumTempInvCacheLines;
    unsigned TotalRefs;
    unsigned TotalLvalRefs;
    uint64_t TotalStride;
    uint64_t TotalLvalStride;

    LocalityInfo()
        : NumSpatialCacheLines(0), NumTempInvCacheLines(0), TotalRefs(0),
          TotalLvalRefs(0), TotalStride(0), TotalLvalStride(0) {}

    unsigned getNumCacheLines() const {
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
  std::array<unsigned, MaxLoopNestLevel> TripCountByLevel;

  /// Returns true if \p Ref1 and \p Ref2 belongs to the same array reference
  /// group for the purposes of computing spatial locality for interchange.
  ///
  /// We group together all the refs with constant distance.
  ///
  /// For example, all the following refs will be in the same group:
  ///  A[i][j][k]
  ///  A[i][j+1][k]
  ///  A[i+1][j][k]
  ///  A[i+1][j][k-1]
  ///
  /// They will be arranged in this order-
  /// A[i][j][k], A[i][j+1][k], A[i+1][j][k-1], A[i+1][j][k]
  ///
  /// After this grouping, we will compute the number of cache lines touched by
  /// the group (equivalent to cache line misses) based on the stride at a
  /// particular loop level.
  /// The assmuption is that the first ref in the group lies at the beginning of
  /// the cache line.
  static bool isSpatialMatch(const RegDDRef *Ref1, const RegDDRef *Ref2);

  /// Returns true if \p Ref1 and \p Ref2 belong to the same group for the
  /// purposes of computing temporal locality.
  /// If Ref1 and Ref2 have a constant iteration distance w.r.t IV at \p Level
  /// not exceeding \p MaxDiff then they belong to the same group.
  ///
  /// For example-
  /// A[2*i] and A[2*i+2] == true
  /// A[2*i] and A[2*i+1] == false
  /// A[0] and A[1] == true
  static bool isTemporalMatch(const RegDDRef *Ref1, const RegDDRef *Ref2,
                              unsigned Level, uint64_t MaxDiff);

  static void updateTotalStrideAndRefs(LocalityInfo &LI,
                                       const RefGroupTy &RefGroup,
                                       uint64_t Stride);

  /// Returns true if the last cache line of PrevRef is the same as CurRef based
  /// on distance and number of bytes accessed.
  static bool sharesLastCacheLine(uint64_t PrevTotalDist, uint64_t CurTotalDist,
                                  uint64_t NumRefBytesAccessed);

  /// Computes extra cache lines accessed by the group using other references
  /// except the first.
  static unsigned computeExtraCacheLines(LocalityInfo &LI,
                                         const RefGroupTy &RefGroup,
                                         unsigned Level,
                                         uint64_t NumRefBytesAccessed,
                                         unsigned NumCacheLinesPerRef);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// no spatial or temporal locality.
  static void computeNumNoLocalityCacheLines(LocalityInfo &LI,
                                             const RefGroupTy &RefGroup,
                                             unsigned Level, unsigned TripCnt);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// temporal invariant locality.
  static void computeNumTempInvCacheLines(LocalityInfo &LI,
                                          const RefGroupTy &RefGroup,
                                          unsigned Level);

  /// Computes total number of cache lines accessed by \p Loop using refs with
  /// spatial locality.
  static void computeNumSpatialCacheLines(LocalityInfo &LI,
                                          const RefGroupTy &RefGroup,
                                          unsigned Level, unsigned TripCnt,
                                          uint64_t Stride);

  /// Computes total number of cache lines accessed by \p Loop.
  void computeNumCacheLines(const HLLoop *Loop, const RefGroupVecTy &RefGroups);

  /// Computes the locality for the loopnest with outermost loop \p Lp.
  void computeLoopNestLocality(const HLLoop *Lp,
                               const SmallVectorImpl<const HLLoop *> &LoopVec);

  /// Returns the trip count of the loop.
  /// If loop count is symbolic or above the threshold, it returns
  /// SymbolicConst value.
  unsigned getTripCount(const HLLoop *Loop);

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

  /// Populates \p TemporalGroups by populating it with memref groups which have
  /// temporal locality within \p ReuseThreshold.
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
      const HLLoop *Lp, unsigned ReuseThreshold, RefGroupVecTy &TemporalGroups,
      SmallSet<unsigned, 8> *UniqueGroupSymbases = nullptr);
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
