//===------ HIRLocalityAnalysis.h - Provides Locality Analysis ---*- C++-*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
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
#include "llvm/Pass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

namespace llvm {

namespace loopopt {

class HLNode;
class HLLoop;
class DDRefUtils;

class HIRLocalityAnalysis final : public HIRAnalysisPass {
public:
  typedef DDRefGrouping::RefGroupMapTy<const RegDDRef> RefGroupMapTy;

private:
  typedef ConstMemRefGatherer::MapTy SymToMemRefTy;

  struct LocalityInfo {
    // Spatial Locality.
    uint64_t Spatial;
    // Temporal Invariant Locality.
    uint64_t TempInv;
    // Temporal Reuse Locality.
    uint64_t TempReuse;

    LocalityInfo() : Spatial(0), TempInv(0), TempReuse(0) {}

    // LocalityValue is TempInv + Spatial.
    uint64_t getLocalityValue() const { return (TempInv + Spatial); }

    uint64_t getTemporalLocality() const { return (TempInv + TempReuse); }
    void clear() { Spatial = TempInv = TempReuse = 0; }
  };

  // Maintains locality info per level. This is used by loop interchange.
  std::array<LocalityInfo, MaxLoopNestLevel> LocalityByLevel;

  // Stores trip count info by level. Loops with non-const trip counts are also
  // assigned a constant trip count for locality computation.
  std::array<unsigned, MaxLoopNestLevel> TripCountByLevel;

  /// Returns true if Ref2 belongs to the same array reference group.
  /// This method checks if Ref2 matches Ref1 to be stored in the same array
  /// reference group.
  /// For the specific IV at *Level* and *MaxDiff* value the method returns true
  /// if all the following is true:
  ///  1) The numbers of dimensions in refs are the same
  ///  2) The first right-to-left canon expressions containing IV are different
  ///     only by a constant not exceeding *MaxDiff*
  ///  3) All other canons should be the same.
  /// Otherwise it returns false.
  ///
  /// Examples:
  ///  Ref1 = A[i  ][j  ][k  ]
  ///  Ref2 = A[i  ][j+1][k  ]
  ///  Ref3 = A[i+1][j  ][k  ]
  ///  Ref4 = A[i+1][j  ][k-1]
  ///
  ///  1) Level j - (Ref1, Ref2) == true
  ///  2) Level i - (Ref1, Ref2) == false
  ///  3) Level i - (Ref1, Ref3) == true
  ///  4) Level k - (Ref3, Ref4) == true
  static bool isGroupMemRefMatch(const RegDDRef *Ref1, const RegDDRef *Ref2,
                                 unsigned Level, uint64_t MaxDiff);

  /// Computes the locality for the loopnest with outermost loop \p Lp.
  void computeLoopNestLocality(const HLLoop *Lp,
                               const SmallVectorImpl<const HLLoop *> &LoopVec);

  /// Computes the temporal invariant locality for a loop.
  void computeTempInvLocality(const HLLoop *Loop,
                              RefGroupMapTy &RefGroups);

  /// Computes the temporal reuse locality for a loop.
  void computeTempReuseLocality(const HLLoop *Loop, RefGroupMapTy &RefGroups, 
                                unsigned ReuseThreshold);

  /// Computes the spatial locality for a loop.
  void computeSpatialLocality(const HLLoop *Loop, RefGroupMapTy &RefGroups);

  /// Computes the spatial trip count.
  uint64_t computeSpatialTrip(const RegDDRef *Ref, const HLLoop *Loop);

  /// Returns the trip count of the loop.
  /// If loop count is symbolic or above than threshold, it returns
  /// SymbolicConst value.
  int64_t getTripCount(const HLLoop *Loop);

  /// Initializes the trip count cache for future use inside the locality
  /// computation.
  void initTripCountByLevel(const SmallVectorImpl<const HLLoop *> &Loops);

  /// Returns true if multiple IV's are present inside the DDRef at given Level.
  /// SubscriptPos specifies which dimension the IV exist.
  bool isMultipleIV(const RegDDRef *Ref, unsigned Level,
                    unsigned *SubscriptPos);

  /// Checks if there is possibility of temporal reuse between Ref1 and Ref2 at
  /// the specified subscript position. This will compare the diff and see if it
  /// is within \p ReuseThreshold.
  bool isTemporalReuse(const RegDDRef *Ref1, const RegDDRef *Ref2,
                       unsigned SubscriptPos, unsigned ReuseThreshold);

  /// Prints out the Locality Information.
  void printLocalityInfo(raw_ostream &OS, const HLLoop *L) const;

public:
  HIRLocalityAnalysis()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRLocalityAnalysisVal) {}
  static char ID;

  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module * = nullptr) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void releaseMemory() override {};

  /// Nothing to do.
  void markLoopBodyModified(const HLLoop *Lp) override {}

  /// Returns a sorted list of loops from lower to higher (where higher is
  /// better) based on locality value. This interface expects a near-perfect
  /// loopnest.
  void sortedLocalityLoops(
      const HLLoop *OutermostLoop,
      SmallVector<const HLLoop *, MaxLoopNestLevel> &SortedLoops);

  // Returns temporal (invariant + reuse) locality value of the loop, using \p
  // ReuseThreshold. Default value is used if it is not provided.
  uint64_t getTemporalLocality(const HLLoop *Lp, unsigned ReuseThreshold = 0);

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRLocalityAnalysisVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
