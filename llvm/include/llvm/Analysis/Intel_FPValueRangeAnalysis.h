//===- Intel_FPValueRangeAnalysis.h - FP Interval analysis ------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// Walk through the use-def chain of a certain floating-point value and use
// fixed-point iteration to determine the range the value may fall in.
// It also accepts a LazyValueInfo to compute ranges for uitofp/sitofp
// instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_ANALYSIS_H
#define LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_ANALYSIS_H

#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/Intel_FPValueRange.h"

namespace llvm {

class LazyValueInfo;

class FPValueRangeAnalysis {
  LazyValueInfo *LVI;
  unsigned MaxIteration;

  // Stores the mapping from IR values to ranges.
  MapVector<Value *, FPValueRange> ValueRangeMap;
  // How many times the range of a value has been extended. Used for widening
  // heuristic.
  DenseMap<Value *, unsigned> WidenCounter;

  // Get the current stored range for value \p V in ValueRangeMap. If \p V is
  // not present in ValueRangeMap, insert an empty range as placeholder.
  FPValueRange getOrInsertRange(Value *V);

  // Computes and returns a new FPValueRange for \p V based on the value's
  // nature and old ranges for its operands (if any) stored in ValueRangeMap.
  FPValueRange processEntry(Value *V);

  // Widening operation accepts an \p OldRange computed for Value \p V in the
  // previous iteration, and a \p NewRange computed in current iteration, uses
  // heuristic to predict the fixpoint for this value. Doing so guarantees and
  // accelerates convergence while preserving safety of the analysis in the cost
  // of lower accuracy. The result is guaranteed to be a superset of
  // FPValueRange::merge(OldRange, NewRange).
  // Currently we only employ a very basic heuristic: If the lower bound of \p
  // NewRange is smaller than that of \p OldRange, or upper bound of \p NewRange
  // is larger than that of \p OldRange, the lower/upper bound of the return
  // value will be set (extended) to that of \p NewRange. We keep a record of
  // how many times extension has happened to each value. If a value has been
  // extended for more than 3 times, the lower/upper bound is then extended
  // straightly to Infinity, preventing further extensions. For example:
  //  In iteration 1:
  //    V = V1, OldRange = [-3, 0], NewRange = [-4, 0], Returns [-4, 0]
  //  In iteration 2:
  //    V = V1, OldRange = [-4, 0], NewRange = [-5, 0], Returns [-5, 0]
  //  In iteration 3:
  //    V = V1, OldRange = [-5, 0], NewRange = [-6, 0], Returns [-6, 0]
  //  In iteration 4:
  //    V = V1, OldRange = [-6, 0], NewRange = [-7, 0], Returns [-Infinity, 0]
  FPValueRange widenRanges(Value *V, FPValueRange OldRange,
                           FPValueRange NewRange);

public:
  FPValueRangeAnalysis(LazyValueInfo *LVI = nullptr, unsigned MaxIteration = 12)
      : LVI(LVI), MaxIteration(MaxIteration) {}

  // Computes and returns the FPValueRange for value \p V
  FPValueRange computeRange(Value *V);

};
} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_FP_VALUE_RANGE_ANALYSIS_H
