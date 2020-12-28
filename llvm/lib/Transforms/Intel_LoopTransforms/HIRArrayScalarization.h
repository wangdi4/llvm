//===--- HIRArrayScalarization.h -----------------------------*- C++ -*---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_ARRAY_SCALARIZATION_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIR_ARRAY_SCALARIZATION_H

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRDDAnalysis.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Pass.h"

namespace llvm {
namespace loopopt {

class DDGraph;
class HIRDDAnalysis;
class HIRLoopStatistics;

namespace arrayscalarization {

// HIRArrayScalarization works as an independent class, and not setup as a pass.
// It is main goal is to cleanup memrefs that have all integer-constant
// subscripts and replace them with temps.
//
// -----------------------------------------------------------------------
// A typical transformation looks like-
//  example1 [BEFORE]
//  do i1
//    A[0][0] = .
//    ...
//       .    = A[0][0]
//    ...
//
//  end do
//
//  These memrefs with const-only subscripts can be replaced by a scalar temp.
//  The above loop will become-
//
//  example1 [AFTER]
//    type t0
//  do i1
//    t0       = .
//    ...
//        .    = t0
//    ...
//
//  end do
//
// -----------------------------------------------------------------------
// A special case that operates on relaxed group looks like-
//  example2 [BEFORE]
//  double A[N][N];
//  do i1
//    (i64*) A[0][0] = 0
//    ...
//              .    = A[0][0]
//    ...
//
//  end do
//
//  These memrefs with const-only subscripts can be replaced by a scalar temp.
//  The above loop will become-
//
//  example2 [AFTER]
//    f64 t0
//  do i1
//    t0       = i2f(0); // this produces f64 value 0.0
//    ...
//        .    = t0
//    ...
//
//  end do
// -----------------------------------------------------------------------
class ArrayScalarizationMemRefGroup {
  typedef SmallVector<RegDDRef *, 8> RefVecTy;

  RefVecTy RefVec;
  DDGraph DDG;
  SmallSet<unsigned, 8> SBS;
  HLLoop *Lp;
  bool CollectSymbase;
  bool IsRelaxedGroup;

  // collect the ArrRef and all Refs it can lead to via DDEdges
  bool collect(ArrayRef<RegDDRef *> ArrRef);

public:
  ArrayScalarizationMemRefGroup(ArrayRef<RegDDRef *> ArrRef, DDGraph DDG,
                                SmallSet<unsigned, 8> &SBS, HLLoop *Lp,
                                bool CollectSymbase = false)
      : DDG(DDG), SBS(SBS), Lp(Lp), CollectSymbase(CollectSymbase),
        IsRelaxedGroup(false) {
    assert(ArrRef.size() && "Expect a non-empty Array of RegDDRef *");
    bool DoCollection = collect(ArrRef);
    assert(DoCollection && "Expect a non-empty RefVec after collection");
    (void)DoCollection;
  }

  // Analyze to validate that the collected MemRefGroup is a good candidate for
  // Array Scalarization.
  //
  // This includes:
  // - all refs in the group are exactly the same
  // - each ref is a RegDDRef with constant int-only subscripts
  // - each ref's symbase is among the symbases provided
  // - each ref is non-volatile, non-address taken, and local
  // - each ref is in the innermost loop
  // - for each group: the leading ref is a store, and all remaining ref(s)
  //   is/are load(s)
  // - ref(0) dominates all other refs
  //
  bool analyze(void);

  /// transform the group
  //
  ///  E.g. [BEFORE]
  ///  do i1
  ///    A[0][0] = .
  ///    ...
  ///       .    = A[0][0]
  ///    ...
  ///
  ///  end do
  ///
  ///  [AFTER]
  ///    type t0
  ///  do i1
  ///    t0       = .
  ///    ...
  ///          .  = t0
  ///    ...
  ///
  ///  end do
  bool transform(void);

  void createATemp(HLLoop *Lp, RegDDRef *Ref, RegDDRef *&TmpRef);
  void replaceRefWithTmp(HLLoop *Lp, RegDDRef *Ref, RegDDRef *TmpRef);

  void print(raw_ostream &OS, bool PrintDetails = false) const;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(bool PrintDetails = false) const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS, PrintDetails);
  }
#endif
};

class HIRArrayScalarization {
  HIRDDAnalysis &HDDA;
  HLNodeUtils &HNU;
  DDGraph DDG;

public:
  HIRArrayScalarization(HIRFramework &HIRF, HIRDDAnalysis &HDDA)
      : HDDA(HDDA), HNU(HIRF.getHLNodeUtils()) {}

  // Conduct array scalarization cleanup on a given vector of RegDDRef*.
  bool doScalarization(HLLoop *InnermostLp,
                       SmallVectorImpl<RegDDRef *> &RefVec);

  // Conduct array scalarization cleanup on a given set of relevant symbases.
  bool doScalarization(HLLoop *InnermostLp, SmallSet<unsigned, 8> &SBS);
};

} // namespace arrayscalarization
} // namespace loopopt
} // namespace llvm
#endif
