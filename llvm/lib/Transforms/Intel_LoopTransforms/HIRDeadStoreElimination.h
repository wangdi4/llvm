//===--- HIRDeadStoreElimination.h ---------------------------*- C++ -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDSEIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRDSEIMPL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRDeadStoreEliminationPass.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLocalityAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/InitializePasses.h"

namespace llvm {

class Value;
class DominatorTree;
class FieldModRefResult;

namespace loopopt {

typedef DDRefGrouping::RefGroupTy<const RegDDRef *> RefGroupTy;
typedef DDRefGrouping::RefGroupVecTy<const RegDDRef *> RefGroupVecTy;
typedef SmallVector<const RegDDRef *, 4> RefVecTy;

struct LifeTimeEndInfo {
  RegDDRef *FakeRef;
  int64_t Size;

  LifeTimeEndInfo(RegDDRef *FakeRef, int64_t Size)
      : FakeRef(FakeRef), Size(Size) {}
};

typedef DenseMap<unsigned, SmallVector<LifeTimeEndInfo, 4>>
    BasePtrToLifetimeEndInfoMapTy;

// Describes whether a base ptr has a single definition in a loop
// which dominates all the uses.
struct NonLinearTempInfo {
  const HLLoop *DefLoop;
  bool HasSingleDominatingDef;

  NonLinearTempInfo(const HLLoop *DefLoop, bool HasSingleDominatingDef)
      : DefLoop(DefLoop), HasSingleDominatingDef(HasSingleDominatingDef) {}
};

namespace dse {

class HIRDeadStoreElimination {
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HLNodeUtils &HNU;

  // Region-level memref groups and symbases.
  HIRLoopLocality::RefGroupVecTy EqualityGroups;
  SmallSet<unsigned, 8> UniqueGroupSymbases;

  // Region-level AddressOf Refs.
  SmallVector<RegDDRef *, 16> AddressOfRefs;

  // Populate all fake refs attached to lifetime end intrinsics in the region
  // based on the base ptr blob index as the key.
  BasePtrToLifetimeEndInfoMapTy FakeLifetimeEndRefs;

  // Map of non-linear temp symbases to the loops they are defined in.
  DenseMap<unsigned, SmallVector<NonLinearTempInfo, 2>> NonLinearTempInfoMap;

  // Map of base ptr value to the region where all the loads of the base ptr
  // exist in the function.
  DenseMap<Value *, HLRegion *> AllLoadsInSingleRegion;

  /// Checks the common parent loops of \p PostDominatingLoop and \p PrevLoop to
  /// make sure they have valid bounds for perfoming DSE. Returns a new node
  /// range via \p OutermostPostDominatingNode and \p OutermostPrevNode where
  /// unsafe calls need to be checked to prove safety of DSE.
  bool hasValidParentLoopBounds(const HLLoop *PostDominatingLoop,
                                const HLLoop *PrevLoop, const RegDDRef *Ref,
                                const HLNode *&OutermostPostDominatingNode,
                                const HLNode *&OutermostPrevNode);

  bool isValidParentChain(const HLNode *PostDomNode, const HLNode *PrevNode,
                          const RegDDRef *PostDomRef);

  void releaseMemory(void) {
    EqualityGroups.clear();
    UniqueGroupSymbases.clear();
    AddressOfRefs.clear();
    FakeLifetimeEndRefs.clear();
  }

  // Collects memrefs and addressOf refs in the region. Returns false if no
  // memrefs were found.
  bool doCollection(HLRegion &Region);

  /// Returns true if there is an aliasing load in the region prior to the first
  /// store ref of \p RefGroup which can reuse the store's value on reentering
  /// the region.
  bool foundReuseInAliasingLiveinLoad(const HLRegion &Region,
                                      const RefGroupTy &RefGroup);

  /// Returns true if \p Ref escapes via an AddressOf ref and cannot be proven
  /// to be safe for analysis.
  bool basePtrEscapesAnalysis(const RegDDRef *Ref) const;

  /// Returns true if all load of the base ptr of \p Ref are within \p
  /// Region.
  bool hasAllLoadsWithinRegion(HLRegion &Region, const RegDDRef *Ref);

  /// Inserts applicable fake lifetime intrinsics to this ref group while
  /// maintaining the lexical order.
  void insertFakeLifetimeRefs(RefGroupTy &RefGroup);

  /// Returns true if \p Ref has a single non-linear blob at \p Level which has
  /// a single definition which dominates all the uses.
  bool hasSingleDominatingNonLinearTempAtLevel(const RegDDRef *Ref,
                                               unsigned Level);

public:
  HIRDeadStoreElimination(HIRFramework &HIRF, HIRDDAnalysis &HDDA,
                          HIRLoopStatistics &HLS)
      : HDDA(HDDA), HLS(HLS), HNU(HIRF.getHLNodeUtils()) {}

  bool run(HLRegion &Region);
};

} // namespace dse
} // namespace loopopt
} // namespace llvm

#endif
