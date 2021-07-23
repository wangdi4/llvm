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

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRLoopStatistics.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Framework/HIRFramework.h"
#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Pass.h"

namespace llvm {
namespace loopopt {

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
  HLLoop *Lp;
  bool IsRelaxedGroup;

public:
  ArrayScalarizationMemRefGroup(SmallVectorImpl<const RegDDRef *> &Group,
                                HLLoop *Lp);

  /// transform the group
  ///
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
  void replaceRefWithTmp(RegDDRef *Ref, RegDDRef *TmpRef);

  void print(raw_ostream &OS, bool PrintDetails = false) const;
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump(bool PrintDetails = false) const {
    formatted_raw_ostream FOS(dbgs());
    print(FOS, PrintDetails);
  }
#endif
};

class HIRArrayScalarization {
public:

  // Conduct array scalarization cleanup on a given set of relevant symbases.
  static bool doScalarization(HLLoop *InnermostLp, SmallSet<unsigned, 8> &SBS);
};

} // namespace arrayscalarization
} // namespace loopopt
} // namespace llvm
#endif
