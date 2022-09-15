//===--- HIRLMMImpl.h -----------------------------------------*- C++ -*---===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLMMIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRLMMIMPL_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"

#include "llvm/Pass.h"

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {

class DominatorTree;
#if INTEL_FEATURE_SW_DTRANS
class FieldModRefResult;
#endif // INTEL_FEATURE_SW_DTRANS

namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

namespace lmm {

// MemRefGroup has a vector of MemRefs, with supporting data
class MemRefGroup {
public:
  typedef SmallVector<RegDDRef *, 8> RefVecTy;
  typedef RefVecTy::iterator iterator;
  typedef RefVecTy::const_iterator const_iterator;

  MemRefGroup(RegDDRef *FirstRef);

  bool isProfitable(void) const { return IsProfitable; }
  bool isLegal(void) const { return IsLegal; }
  void setLegal(bool NewFlag) { IsLegal = NewFlag; }

  void insert(RegDDRef *Ref) { RefVec.push_back(Ref); }

  iterator begin() { return RefVec.begin(); }
  iterator end() { return RefVec.end(); }

  const_iterator begin() const { return RefVec.begin(); }
  const_iterator end() const { return RefVec.end(); }

  unsigned size(void) const { return RefVec.size(); }

  RegDDRef *operator[](unsigned Idx) const {
    assert((Idx < RefVec.size()) && "Idx is out of bound\n");
    return RefVec[Idx];
  }

  // Check if the given RegDDRef* belongs to the current Group
  bool belongs(RegDDRef *Ref) const;

  // Statistically analyze all ref(s) in the Group
  void analyze(const HLLoop *Lp, DominatorTree *DT, bool LoopNestHoistingOnly);

  // *** Supported Queries ***
  bool hasAnyLoadOrStoreOnDominatePath(void) const {
    assert(IsAnalyzed && "must analyze 1st\n");
    return (HasLoadOnDomPath || HasStoreOnDomPath);
  }

  bool hasAnyLoad(void) const {
    assert(IsAnalyzed && "must analyze 1st\n");
    return HasLoad;
  }

  bool hasAnyLoadOnDominatePath(void) const {
    assert(IsAnalyzed && "must analyze 1st\n");
    return HasLoadOnDomPath;
  }

  bool isLoadOnly(void) const {
    assert(IsAnalyzed && "must analyze 1st\n");
    return !HasStore;
  }

  bool isStoreOnly(void) const {
    assert(IsAnalyzed && "must analyze 1st\n");
    return !HasLoad;
  }

  bool isInsideLifetimeIntrinsics() const { return IsInsideLifetimeIntrinsics; }

  void setInsideLifetimeIntrinsics() {
    assert(HasStore && "Unexpected memref group!");
    IsInsideLifetimeIntrinsics = true;
  }

#ifndef NDEBUG
  // Print MemRefGroup
  // E.g.: A[0] { R W R R .. W } 5W:3R
  LLVM_DUMP_METHOD void print(bool NewLine = false);
#endif
private:
  RefVecTy RefVec;

  bool IsProfitable;
  bool IsLegal;

  bool IsAnalyzed;
  bool HasLoad, HasLoadOnDomPath, HasStore, HasStoreOnDomPath;

  bool IsInsideLifetimeIntrinsics;
};

// MemRefCollection is a SmallVector of MemRefGroup
//
// E.g. after collecting all loop-inv MemRefs in a loop into a MRC, we may have
//
// A[0] { R W R R .. W } 5W:3R
// B[1] { R R          } 0W:2R
// A[1] { W R .. R     } 1W:4R
// ..
//
class MemRefCollection {
public:
  typedef SmallVector<MemRefGroup, 8> GroupsTy;
  typedef GroupsTy::iterator iterator;
  typedef GroupsTy::const_iterator const_iterator;

  unsigned size(void) const { return MemRefGroups.size(); }
  bool empty(void) { return MemRefGroups.empty(); }
  void clear(void) { MemRefGroups.clear(); }

  iterator begin() { return MemRefGroups.begin(); }
  iterator end() { return MemRefGroups.end(); }

  const_iterator begin() const { return MemRefGroups.begin(); }
  const_iterator end() const { return MemRefGroups.end(); }

  // find if a given Ref is available in the collection
  //
  // Return: bool
  // true: found, make Idx available
  // false: not found
  bool find(RegDDRef *Ref, unsigned &Index) const;

  // insert a RegDDRef* into the collection at proper index
  void insert(RegDDRef *Ref);

  // analyze each group in MRC by checking Load(s), store(s), etc.
  void analyze(const HLLoop *Lp, DominatorTree *DT, bool LoopNestHoistingOnly) {
    for (auto &Group : MemRefGroups) {
      Group.analyze(Lp, DT, LoopNestHoistingOnly);
    }
  }

#ifndef NDEBUG
  // Dump MemRefGroup: A[0] { R W R R .. W } 5W:3R
  LLVM_DUMP_METHOD void print(void);
#endif
private:
  GroupsTy MemRefGroups;
};

class HIRLMM {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HLNodeUtils &HNU;
#if INTEL_FEATURE_SW_DTRANS
  FieldModRefResult *FieldModRef;
#endif //  INTEL_FEATURE_SW_DTRANS
  DominatorTree *DT;

  MemRefCollection MRC;
  SmallVector<HLInst *, 8> UnknownAliasingCallInsts;

  DDGraph DDG;

  unsigned LoopLevel = 0;
  bool LoopNestHoistingOnly;

public:
  HIRLMM(HIRFramework &HIRF, HIRDDAnalysis &HDDA, HIRLoopStatistics &HLS,
#if INTEL_FEATURE_SW_DTRANS
         FieldModRefResult *FieldModRef = nullptr,
#endif // INTEL_FEATURE_SW_DTRANS
         DominatorTree *DT = nullptr, bool LoopNestHoistingOnly = false)
      : HIRF(HIRF), HDDA(HDDA), HLS(HLS), HNU(HIRF.getHLNodeUtils()),
#if INTEL_FEATURE_SW_DTRANS
        FieldModRef(FieldModRef),
#endif // INTEL_FEATURE_SW_DTRANS
        DT(DT), LoopNestHoistingOnly(LoopNestHoistingOnly) {
  }

  bool run();

  // Exposed as a utility to be called on demand.
  // Returns true if \p MemRef is invariant inside \p Loop. \p If IgnoreIVs is
  // set to true, any IVs present inside \p MemRef will be ignored when making
  // structural checks.
  bool isLoopInvariant(const RegDDRef *MemRef, const HLLoop *Loop,
                       bool IgnoreIVs);

private:
  bool doLoopMemoryMotion(HLLoop *Lp);

  bool doLoopPreliminaryChecks(const HLLoop *Lp,
                               bool AllowUnknownAliasingCalls);

  /// Collects candidate memrefs and unknown aliasing calls insts.
  /// If \p CandidateMemRef is non-null, it is the only candidate considered.
  /// If \p IgnoreIVs is set to true, any IVs present inside candidate memrefs
  /// will be ignored when making structural checks.
  bool doCollection(HLLoop *Lp, RegDDRef *CandidateMemRef = nullptr,
                    bool IgnoreIVs = false);

  bool processLegalityAndProfitability(const HLLoop *Lp);

  bool isLegal(const HLLoop *Lp);

  /// \p QueryMode indicates that this pass only intends to query invariance but
  /// not perform any transformation.
  bool isLegal(const HLLoop *Lp, const MemRefGroup &Group,
               bool QueryMode = false,
               bool *IsInsideLifetimeIntrinsics = nullptr);

  // Analyze the Loop by doing collection, profit analysis and legal analysis.
  // Return true indicates that the loop has at least 1 Group suitable
  // (profitable+legal) for LMM.
  bool doAnalysis(HLLoop *Lp);

  void doTransform(HLLoop *Lp);

  // Do LIMM Reference Promotion on the given Group
  void doLIMMRef(HLLoop *Lp, MemRefGroup &Group,
                 SmallSet<unsigned, 32> &TempRefSet);

  HLInst *
  canHoistLoadsUsingExistingTemp(HLLoop *Lp, MemRefGroup &Group,
                                 SmallSet<unsigned, 32> &TempRefSet) const;

  bool canSinkSingleStore(HLLoop *Lp, RegDDRef *FirstRef, MemRefGroup &Group,
                          SmallSet<unsigned, 32> &TempRefSet) const;

  void handleInLoopMemRef(unsigned Level, RegDDRef *Ref, RegDDRef *TmpDDRef,
                          bool IsLoadOnly);

  bool hoistLoadsUsingExistingTemp(HLLoop *Lp, MemRefGroup &Group,
                                   SmallSet<unsigned, 32> &TempRefSet,
                                   OptReportBuilder &ORBuilder);

  bool sinkStoresUsingExistingTemp(HLLoop *Lp, RegDDRef *StoreRef,
                                   MemRefGroup &Group,
                                   SmallSet<unsigned, 32> &TempRefSet,
                                   OptReportBuilder &ORBuilder);

  HLLoop *getOuterLoopCandidateForSingleLoad(HLLoop *Lp, RegDDRef *Ref,
                                             MemRefGroup &Group);

  void clearWorkingSetMemory() {
    MRC.clear();
    UnknownAliasingCallInsts.clear();
    DDG.clear();
  }

  // *** Utility functions ***
  HLInst *createLoadInPreheader(HLLoop *InnermostLp, RegDDRef *Ref,
                                HLLoop *OuterLp) const;

  void createStoreInPostexit(HLLoop *Lp, RegDDRef *Ref, RegDDRef *TmpRef,
                             bool NeedLoadInPrehdr) const;
};

//
} // namespace lmm
} // namespace loopopt
} // namespace llvm

#endif
