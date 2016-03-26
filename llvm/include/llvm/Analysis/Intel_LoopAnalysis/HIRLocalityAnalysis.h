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

#include <map>

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

// First parameter is HLLoop and second parameter is LocalityValue.
typedef std::pair<const HLLoop *, uint64_t> LoopLocalityPair;

class HIRLocalityAnalysis final : public HIRAnalysisPass {

private:
  typedef DDRefGrouping::SymToMemRefTy SymToMemRefTy;

  // Symbolic constant to denote unknown 'N' trip count.
  // TODO: Revisit this for scaling known loops.
  const unsigned SymbolicConst = 20;

  // number of cache lines =
  // WtFactor*((Total Cache Size)/(CacheLine size * Associativity))
  const unsigned NumCacheLines = 16;

  // Temporal Reuse Threshold indicates the span of registers that
  // can be used.
  const unsigned TempReuseThreshold = 5;

  // Cache line size in bytes.
  // TODO: Get data from Target Machine.
  const unsigned CacheLineSize = 64;

  // Floats per cache line.
  // TODO: Assuming 4bytes of float. Change when sizeinfo is available.
  // TODO: Similar for other data types. Revisit when bit width is available
  // for the given data.
  const unsigned FloatsPerCacheLine = CacheLineSize / 4;
  const unsigned IntsPerCacheLine = CacheLineSize / 4;

  // A small value to differentiate between Read vs Write.
  const unsigned WriteWt = 4;

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

    void clear() { Spatial = TempInv = TempReuse = 0; }
  };

  // Maintains the Locality information in a map for the loops.
  SmallDenseMap<const HLLoop *, LocalityInfo *, 16> LocalityMap;

  // First argument is the HLLoop and second argument
  // tells if the loop was modified or not. True indicates it was
  // modified and False indicates no change inside this loop.
  // When there is no change, all children have valid locality.
  SmallDenseMap<const HLLoop *, bool, 64> LoopModificationMap;

  // This is used for temporarily storing the const trip counts in a cache.
  // First argument is the loop and second argument contains the trip count.
  // If the loop is not present in this cache, it assumed to have symbolic
  // trip count.
  SmallDenseMap<const HLLoop *, unsigned, 16> ConstTripCache;

  DDRefGrouping::RefGroupsTy RefGroups;

  /// \brief Used for debugging purposes only to verify locality value.
  /// This function prints out the locality cost for all the loops.
  void checkLocality();

  /// \brief Internal method to clear the SymToMemRef where the loop level
  /// is not present.
  static void clearEmptySlots(SymToMemRefTy &MemRefMap);

  /// \brief Returns true if Ref2 belongs to the same array reference group.
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

  /// \brief Computes the locality for the loop nest in which L is contained.
  /// If the loop was not modified, it returns the old computed values.
  /// EnableCache parameter specifies if we want to used cached value or not.
  /// This parameter is primarily used for testing in debug mode.
  void computeLocality(const HLLoop *L, bool EnableCache = true);

  /// \brief Computes the temporal invariant locality for a loop.
  void computeTempInvLocality(const HLLoop *Loop, SymToMemRefTy &MemRefMap);

  /// \brief Computes the temporal reuse locality for a loop.
  void computeTempReuseLocality(const HLLoop *Loop);

  /// \brief Computes the spatial locality for a loop.
  void computeSpatialLocality(const HLLoop *Loop);

  /// \brief Computes the spatial trip count.
  uint64_t computeSpatialTrip(const RegDDRef *Ref, const HLLoop *Loop);

  /// \brief Returns the trip count of the loop.
  /// If loop count is symbolic or above than threshold, it
  /// returns SymbolicConst value.
  int64_t getTripCount(const HLLoop *Loop);

  /// \brief Initializes the trip count cache for future use inside
  /// the locality computation.
  void initConstTripCache(SmallVectorImpl<const HLLoop *> *Loops);

  /// \brief Returns true if this Loop was modified or doesn't exist.
  bool isLoopModified(const HLLoop *Loop);

  /// \brief Returns true if multiple IV's are present inside the DDRef at
  /// given Level. SubscriptPos specifies which dimension the IV exist.
  bool isMultipleIV(const RegDDRef *Ref, unsigned Level,
                    unsigned *SubscriptPos);

  /// \brief Checks if there is possibility of temporal reuse between Ref1 and
  /// Ref2 at the specified subscript position. This will compare the diff and
  /// see if it is less than a threshold.
  bool isTemporalReuse(const RegDDRef *Ref1, const RegDDRef *Ref2,
                       unsigned SubscriptPos);

  /// \brief Prints out the Locality Information.
  void printLocalityInfo(raw_ostream &OS, const HLLoop *L) const;

  /// \brief Resets the locality info for the given loop L.
  void resetLocalityMap(const HLLoop *L);

  /// \brief Verifies the newly computed locality cost with cached value.
  /// This is primarily done for testing in debug mode. For optimized mode,
  /// this method is not used.
  void verifyLocality(const HLLoop *L);

public:
  HIRLocalityAnalysis()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRLocalityAnalysisVal) {}
  static char ID;

  bool runOnFunction(Function &F) override;

  void print(raw_ostream &OS, const Module * = nullptr) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void releaseMemory() override;

  /// \brief This method will mark the loop and all its parent loops as
  /// modified. If loop changes, locality of the loop and all its parents loops
  /// needs to recomputed.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// \brief Returns the loop cost of the specified loop.
  uint64_t getLocalityValue(const HLLoop *Loop);

  /// \brief Returns a sorted list of loops from lower to higher (where higher
  /// is better) based on locality value.
  void sortedLocalityLoops(
      const HLLoop *OutermostLoop,
      SmallVectorImpl<std::pair<const HLLoop *, uint64_t>> &LoopLocality);

  /// \brief Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRLocalityAnalysisVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
