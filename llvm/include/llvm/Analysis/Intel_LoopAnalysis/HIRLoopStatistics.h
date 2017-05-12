//===------- HIRLoopStatistics.h - Provides Loop Statistics ------*- C++-*-===//
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
// This analysis provides loop statistics for a given loop like number of ifs,
// user calls etc.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_STATISTICS_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_STATISTICS_H

#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRAnalysisPass.h"

namespace llvm {

class formatted_raw_ostream;
class TargetTransformInfo;
class Instruction;
class Loop;
class LoopInfo;

namespace loopopt {

struct LoopStatistics {
private:
  unsigned NumIfs = 0;
  unsigned NumSwitches = 0;
  unsigned NumGotos = 0;
  unsigned NumLabels = 0;
  unsigned NumUserCalls = 0;
  unsigned NumIntrinsics = 0;
  bool HasUnsafeSideEffects = false;

public:
  LoopStatistics() {}

  // Visitor to compute statistics.
  struct LoopStatisticsVisitor;

  unsigned getNumIfs() const { return NumIfs; }
  bool hasIfs() const { return getNumIfs() > 0; }

  unsigned getNumSwitches() const { return NumSwitches; }
  bool hasSwitches() const { return getNumSwitches() > 0; }

  unsigned getNumGotos() const { return NumGotos; }
  bool hasGotos() const { return getNumGotos() > 0; }

  unsigned getNumLabels() const { return NumLabels; }
  bool hasLabels() const { return getNumLabels() > 0; }

  unsigned getNumUserCalls() const { return NumUserCalls; }
  bool hasUserCalls() const { return getNumUserCalls() > 0; }

  unsigned getNumIntrinsics() const { return NumIntrinsics; }
  bool hasIntrinsics() const { return getNumIntrinsics() > 0; }

  unsigned getNumCalls() const {
    return getNumUserCalls() + getNumIntrinsics();
  }
  bool hasCalls() const { return getNumCalls() > 0; }

  // Unsafe side effects are the ones which cannot be exposed as data
  // dependency. For example, performing I/O cannot be represented as simply
  // data dependency. Any call which only accesses memory from its arguments is
  // considered safe as we can add fake DDRefs to expose such dependencies.
  bool hasCallsWithUnsafeSideEffects() const {
    assert((!HasUnsafeSideEffects || hasCalls()) &&
           "Number of calls and CallWithSideEffects are out of sync!");
    return HasUnsafeSideEffects;
  }

  /// Adds the loop statistics LS to this one.
  LoopStatistics &operator+=(const LoopStatistics &LS) {
    NumIfs += LS.NumIfs;
    NumSwitches += LS.NumSwitches;
    NumGotos += LS.NumGotos;
    NumLabels += LS.NumLabels;
    NumUserCalls += LS.NumUserCalls;
    NumIntrinsics += LS.NumIntrinsics;
    HasUnsafeSideEffects = HasUnsafeSideEffects || LS.HasUnsafeSideEffects;

    return *this;
  }

  /// Prints the loop statistics.
  void print(formatted_raw_ostream &OS, const HLLoop *Lp) const;
};

class HIRLoopStatistics final : public HIRAnalysisPass {
private:
  /// Maintains self statistics information for loops.
  DenseMap<const HLLoop *, LoopStatistics> SelfStatisticsMap;

  /// Maintains total statistics information for loops.
  DenseMap<const HLLoop *, LoopStatistics> TotalStatisticsMap;

  /// Computes and returns loop statistics for Loop. \p SelfOnly Indicates
  /// whether to computer self or total loop statistics.
  const LoopStatistics &computeLoopStatistics(const HLLoop *Loop,
                                              bool SelfOnly);

protected:
  /// Prints analyis results for loop.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Lp) override;

public:
  HIRLoopStatistics()
      : HIRAnalysisPass(ID, HIRAnalysisPass::HIRLoopStatisticsVal) {}

  static char ID;

  bool runOnFunction(Function &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void releaseMemory() override;

  /// This method will mark the loop and all its parent loops as modified. If
  /// loop changes, statistics of the loop and all its parents loops needs to be
  /// recomputed.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// Returns the loop statistics of the specified loop. This excludes loop
  /// statistics of children loops.
  const LoopStatistics &getSelfLoopStatistics(const HLLoop *Loop);

  /// Returns the loop statistics of the specified loop including children
  /// loops.
  /// NOTE: Children loop's statistics is added assuming a trip count of one. No
  /// multiplier is involved.
  const LoopStatistics &getTotalLoopStatistics(const HLLoop *Loop);

  // TODO: provide an update interface.

  /// Method for supporting type inquiry through isa, cast, and dyn_cast.
  static bool classof(const HIRAnalysisPass *AP) {
    return AP->getHIRAnalysisID() == HIRAnalysisPass::HIRLoopStatisticsVal;
  }
};

} // End namespace loopopt

} // End namespace llvm

#endif
