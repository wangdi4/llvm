//===-------------- SIMDIntrinsicChecker.h  --------------*- C++ -*---===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains interface for SIMDIntrinsicChecker.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_SIMDINTRINCHECKER_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_SIMDINTRINCHECKER_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLInst.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"

namespace llvm {

namespace loopopt {

class SIMDIntrinsicChecker {
public:
  SIMDIntrinsicChecker(const HLInst *DirSIMD, const HLLoop *Loop);

  bool isHandleable() const { return IsHandleable; }

  const HLInst *getSIMDEntryInst() const { return DirSIMD; }
  const HLInst *getSIMDExitInst() const { return DirSIMDExit; }

  ArrayRef<const HLInst *> getRedPreLoopInsts() const {
    return RedPreLoopInsts;
  }
  ArrayRef<const HLInst *> getRedPostLoopInsts() const {
    return RedPostLoopInsts;
  }

  bool areAllInPreAndPostLoop() const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dump() const;
#endif

  static const HLInst *findSIMDExit(const HLLoop *Loop);

private:
  // Entry point function
  bool parseSIMDEntry();

  SmallVector<const HLNode *, 4> collectPreLoop();
  SmallVector<const HLNode *, 4> collectPostLoop();

  bool parseOperands();
  bool hasMatchingReductionRefs(const RegDDRef *Ref);

  static bool isHandleableOpBundle(StringRef TagName);
  static bool isReductionOpBundle(StringRef TagName);

private:
  const HLInst *DirSIMD;
  const HLInst *DirSIMDExit;
  const HLLoop *Loop;
  bool IsHandleable;

  SmallVector<const HLInst *, 4> RedPreLoopInsts;
  SmallVector<const HLInst *, 4> RedPostLoopInsts;

  /// Set of inputs comes after OMP.SIMD.REDUCTION.* and
  /// induced lvals from those arguments.
  /// E.g. @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),
  ///   QUAL.OMP.LINEAR:IV(&((%22)[0])1),
  ///   QUAL.OMP.REDUCTION.ADD(&((%21)[0])),
  ///   QUAL.OMP.REDUCTION.ADD(&((%20)[0])),
  ///   QUAL.OMP.REDUCTION.ADD(&((%19)[0])),
  ///   QUAL.OMP.NORMALIZED.IV(null),
  ///   QUAL.OMP.NORMALIZED.UB(null) ]
  ///    %151 = (%19)[0];
  ///    %152 = (%20)[0];
  ///    %153 = (%21)[0];
  ///    %32 = %153;
  ///    %33 = %152;
  ///    %34 = %151;
  ///  &(%19)[0], &(%20)[0], &(%21)[0] and %32, %33, %34 are
  ///  \p ReductionRefs.
  SetVector<const RegDDRef *> ReductionRefs;

  static StringSet<> HandleableOpBundleNames;
};

} // namespace loopopt

} // namespace llvm

#endif
