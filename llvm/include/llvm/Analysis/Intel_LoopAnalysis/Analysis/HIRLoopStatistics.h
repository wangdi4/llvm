//===------- HIRLoopStatistics.h - Provides Loop Statistics ------*- C++-*-===//
//
// Copyright (C) 2015-2020 Intel Corporation. All rights reserved.
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

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

#include "llvm/ADT/DenseMap.h"

#include "llvm/Analysis/Intel_LoopAnalysis/Analysis/HIRAnalysisPass.h"

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
  unsigned NumForwardGotos = 0;
  unsigned NumLabels = 0;
  unsigned NumUserCalls = 0;
  unsigned NumIndirectCalls = 0;
  unsigned NumIntrinsics = 0;
  bool HasCallsWithUnsafeSideEffects = false;
  bool HasNonSIMDCallsWithUnsafeSideEffects = false;
  bool HasCallsWithNoDuplicate = false;
  bool HasCallsWithUnknownAliasing = false;

public:
  LoopStatistics() {}

  // Visitor to compute statistics.
  struct LoopStatisticsVisitor;

  unsigned getNumIfs() const { return NumIfs; }
  bool hasIfs() const { return getNumIfs() > 0; }

  unsigned getNumSwitches() const { return NumSwitches; }
  bool hasSwitches() const { return getNumSwitches() > 0; }

  unsigned getNumForwardGotos() const { return NumForwardGotos; }
  bool hasForwardGotos() const { return getNumForwardGotos() > 0; }

  // Only counts forward goto targets.
  unsigned getNumLabels() const { return NumLabels; }
  bool hasLabels() const { return getNumLabels() > 0; }

  unsigned getNumUserCalls() const { return NumUserCalls; }
  bool hasUserCalls() const { return getNumUserCalls() > 0; }

  unsigned getNumIndirectCalls() const { return NumIndirectCalls; }
  bool hasIndirectCalls() const { return getNumIndirectCalls() > 0; }

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
    assert(
        (!HasCallsWithUnsafeSideEffects || hasCalls()) &&
        "Number of calls and HasCallsWithUnsafeSideEffects are out of sync!");
    return HasCallsWithUnsafeSideEffects;
  }

  // Set to true if loop has calls with side effects other than SIMD calls. This
  // is useful for transformations which want to handle SIMD calls in a special
  // way.
  bool hasNonSIMDCallsWithUnsafeSideEffects() const {
    assert((!HasNonSIMDCallsWithUnsafeSideEffects ||
            HasCallsWithUnsafeSideEffects) &&
           "Number of SIMD and overall calls with unsafe side effects are out "
           "of sync!");
    return HasNonSIMDCallsWithUnsafeSideEffects;
  }

  bool hasCallsWithNoDuplicate() const {
    assert((!HasCallsWithNoDuplicate || hasCalls()) &&
           "Number of calls and HasCallsWithNoDuplicate are out of sync!");
    return HasCallsWithNoDuplicate;
  }

  bool hasCallsWithUnknownAliasing() const {
    assert((!HasCallsWithUnknownAliasing || hasCalls()) &&
           "Number of calls and HasUnknownAliasing are out of sync!");
    return HasCallsWithUnknownAliasing;
  }

  /// Adds the loop statistics LS to this one.
  LoopStatistics &operator+=(const LoopStatistics &LS) {
    NumIfs += LS.NumIfs;
    NumSwitches += LS.NumSwitches;
    NumForwardGotos += LS.NumForwardGotos;
    NumLabels += LS.NumLabels;
    NumUserCalls += LS.NumUserCalls;
    NumIndirectCalls += LS.NumIndirectCalls;
    NumIntrinsics += LS.NumIntrinsics;
    HasCallsWithUnsafeSideEffects |= LS.HasCallsWithUnsafeSideEffects;
    HasCallsWithNoDuplicate |= LS.HasCallsWithNoDuplicate;
    HasCallsWithUnknownAliasing |= LS.HasCallsWithUnknownAliasing;

    return *this;
  }

  /// Prints the loop statistics.
  void print(formatted_raw_ostream &OS, const HLLoop *Lp) const;
};

class HIRLoopStatistics : public HIRAnalysis {
private:
  /// Maintains self statistics information for loops.
  DenseMap<const HLLoop *, LoopStatistics> SelfStatisticsMap;

  /// Maintains total statistics information for loops.
  DenseMap<const HLLoop *, LoopStatistics> TotalStatisticsMap;

protected:
  /// Prints analyis results for loop.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Lp) override;

public:
  HIRLoopStatistics(HIRFramework &HIRF) : HIRAnalysis(HIRF) {}
  HIRLoopStatistics(HIRLoopStatistics &&Arg)
      : HIRAnalysis(std::move(Arg)),
        SelfStatisticsMap(std::move(Arg.SelfStatisticsMap)),
        TotalStatisticsMap(std::move(Arg.TotalStatisticsMap)) {}
  HIRLoopStatistics(const HIRLoopStatistics &Arg) = delete;

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
};

class HIRLoopStatisticsWrapperPass : public FunctionPass {
  std::unique_ptr<HIRLoopStatistics> HLS;

public:
  static char ID;
  HIRLoopStatisticsWrapperPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void releaseMemory() override;

  void print(raw_ostream &OS, const Module * = nullptr) const override {
    getHLS().printAnalysis(OS);
  }

  HIRLoopStatistics &getHLS() { return *HLS; }
  const HIRLoopStatistics &getHLS() const { return *HLS; }
};

class HIRLoopStatisticsAnalysis
    : public AnalysisInfoMixin<HIRLoopStatisticsAnalysis> {
  friend struct AnalysisInfoMixin<HIRLoopStatisticsAnalysis>;

  static AnalysisKey Key;

public:
  using Result = HIRLoopStatistics;

  HIRLoopStatistics run(Function &F, FunctionAnalysisManager &AM);
};

} // End namespace loopopt

} // End namespace llvm

#endif
