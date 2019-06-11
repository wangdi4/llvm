//===--- HIRLMMImpl.h -----------------------------------------*- C++ -*---===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

namespace lmm {

// MemRefGroup has a vector of MemRefs, with supporting data
class MemRefGroup {
  SmallVector<RegDDRef *, 8> RefV;
  bool IsProfitable;
  bool IsLegal;

  bool IsAnalyzed;
  bool HasLoad, HasLoadOnDomPath, HasStore, HasStoreOnDomPath;
  HLLoop *Lp = nullptr;

public:
  MemRefGroup(RegDDRef *FirstRef);

  bool getProfitable(void) const { return IsProfitable; }
  void setProfitable(bool NewFlag) { IsProfitable = NewFlag; }
  bool getLegal(void) const { return IsLegal; }
  void setLegal(bool NewFlag) { IsLegal = NewFlag; }

  void insert(RegDDRef *Ref) { RefV.push_back(Ref); }
  void clear(void) { RefV.clear(); }

  unsigned getSize(void) const { return RefV.size(); }

  RegDDRef *get(unsigned Idx) const {
    assert((Idx < RefV.size()) && "Idx is out of bound\n");
    return RefV[Idx];
  }

  void set(unsigned Idx, RegDDRef *Ref) {
    assert((Idx < RefV.size()) && "Idx is out of bound\n");
    RefV[Idx] = Ref;
  }

  // Check if the given RegDDRef* belongs to the current MRG
  bool belongs(RegDDRef *Ref) const;

  // Statistically analyze all ref(s) in the MRG
  void analyze(void);

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

#ifndef NDEBUG
  // Print MemRefGroup
  // E.g.: A[0] { R W R R .. W } 5W:3R
  LLVM_DUMP_METHOD void print(bool NewLine = false);
#endif
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
struct MemRefCollection {
  SmallVector<MemRefGroup, 8> MRVV;

  unsigned getSize(void) const { return MRVV.size(); }
  bool isEmpty(void) { return MRVV.empty(); }
  void clear(void) { MRVV.clear(); }

  MemRefGroup &get(unsigned Idx) {
    assert(Idx < MRVV.size());
    return MRVV[Idx];
  }

  // find if a given Ref is available in the collection
  //
  // Return: bool
  // true: found, make Idx available
  // false: not found
  bool find(RegDDRef *Ref, unsigned &Index) const;

  // insert a RegDDRef* into the collection at proper index
  void insert(RegDDRef *Ref);

  // analyze each group in MRC by checking Load(s), store(s), etc.
  void analyze(void) {
    for (auto &MRG : MRVV) {
      MRG.analyze();
    }
  }

#ifndef NDEBUG
  // Dump MemRefGroup: A[0] { R W R R .. W } 5W:3R
  LLVM_DUMP_METHOD void print(void);
#endif
};

class HIRLMM {
  HIRFramework &HIRF;
  HIRDDAnalysis &HDDA;
  HIRLoopStatistics &HLS;
  HLNodeUtils &HNU;

  MemRefCollection MRC;

  class CollectMemRefs;
  unsigned LoopLevel = 0;

public:
  HIRLMM(HIRFramework &HIRF, HIRDDAnalysis &HDDA, HIRLoopStatistics &HLS)
      : HIRF(HIRF), HDDA(HDDA), HLS(HLS), HNU(HIRF.getHLNodeUtils()) {}

  // The only entry for all caller(s) for doing loop memory motion
  bool doLoopMemoryMotion(HLLoop *Lp);

  bool run();

private:
  bool doLoopPreliminaryChecks(const HLLoop *Lp);

  bool doCollection(HLLoop *Lp);

  bool isProfitable(const HLLoop *Lp);

  bool isLegal(const HLLoop *Lp);
  bool isLegal(const HLLoop *Lp, MemRefGroup &MRG, DDGraph &DDG);
  bool areDDEdgesLegal(const HLLoop *Lp, const RegDDRef *Ref, DDGraph &DDG);

  // Analyze the Loop by doing collection, profit analysis and legal analysis.
  // Return true indicates that the loop has at least 1 MRG suitable
  // (profitable+legal) for LMM.
  bool doAnalysis(HLLoop *Lp);

  void doTransform(HLLoop *Lp);

  // Do LIMM Reference Promotion on the given MRG
  void doLIMMRef(HLLoop *Lp, MemRefGroup &MRG,
                 SmallSet<unsigned, 32> &TempRefSet);

  bool isLoadNeededInPrehder(HLLoop *Lp, MemRefGroup &MRG);

  bool canHoistSingleLoad(HLLoop *Lp, RegDDRef *FirstRef, MemRefGroup &MRG,
                          SmallSet<unsigned, 32> &TempRefSet) const;

  bool canSinkSingleStore(HLLoop *Lp, RegDDRef *FirstRef, MemRefGroup &MRG,
                          SmallSet<unsigned, 32> &TempRefSet) const;

  void handleInLoopMemRef(HLLoop *Lp, RegDDRef *Ref, RegDDRef *TmpDDRef,
                          bool IsLoadOnly);

  bool hoistedSingleLoad(HLLoop *Lp, RegDDRef *LoadRef, MemRefGroup &MRG,
                         SmallSet<unsigned, 32> &TempRefSet,
                         LoopOptReportBuilder &LORBuilder);

  bool sinkedSingleStore(HLLoop *Lp, RegDDRef *StoreRef, MemRefGroup &MRG,
                         SmallSet<unsigned, 32> &TempRefSet,
                         LoopOptReportBuilder &LORBuilder);

  HLLoop *getOuterLoopCandidateForSingleLoad(HLLoop *Lp, RegDDRef *Ref,
                                             MemRefGroup &MRG);

  void setLinear(DDRef *TmpRef);

  void clearWorkingSetMemory(void) { MRC.clear(); }

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
