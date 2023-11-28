//===------- HIRLoopStatistics.h - Provides Loop Statistics ------*- C++-*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLLoop.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLRegion.h"

namespace llvm {

class formatted_raw_ostream;
class TargetTransformInfo;
class Instruction;
class LoopInfo;
class TargetLibraryInfo;

namespace loopopt {

class HLNode;
struct LoopStatistics {
private:
  unsigned NumIfs = 0;
  unsigned NumSwitches = 0;
  unsigned NumLabels = 0;
  unsigned NumUserCalls = 0;
  unsigned NumIndirectCalls = 0;
  unsigned NumIntrinsics = 0;
  unsigned NumProfitableVectorizableCalls = 0;

  bool HasCallsWithUnsafeSideEffects = false;
  bool HasNonSIMDCallsWithUnsafeSideEffects = false;
  bool HasCallsWithNoDuplicate = false;
  bool HasConvergentCalls = false;
  bool HasCallsWithUnknownAliasing = false;

  SmallVector<const HLGoto *, 2> ForwardGotos;

public:
  LoopStatistics() {}

  // Visitor to compute statistics.
  struct LoopOrRegionStatisticsVisitor;

  unsigned getNumIfs() const { return NumIfs; }
  bool hasIfs() const { return getNumIfs() > 0; }

  unsigned getNumSwitches() const { return NumSwitches; }
  bool hasSwitches() const { return getNumSwitches() > 0; }

  unsigned getNumForwardGotos() const { return ForwardGotos.size(); }
  bool hasForwardGotos() const { return !ForwardGotos.empty(); }

  // Only counts forward goto targets.
  unsigned getNumLabels() const { return NumLabels; }
  bool hasLabels() const { return getNumLabels() > 0; }

  unsigned getNumUserCalls() const { return NumUserCalls; }
  bool hasUserCalls() const { return getNumUserCalls() > 0; }

  unsigned getNumIndirectCalls() const { return NumIndirectCalls; }
  bool hasIndirectCalls() const { return getNumIndirectCalls() > 0; }

  unsigned getNumIntrinsics() const { return NumIntrinsics; }
  bool hasIntrinsics() const { return getNumIntrinsics() > 0; }

  unsigned getNumProfitableVectorizableCalls() const {
    return NumProfitableVectorizableCalls;
  }
  bool hasProfitableVectorizableCalls() const {
    return getNumProfitableVectorizableCalls() > 0;
  }

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

  bool hasConvergentCalls() const {
    assert((!HasConvergentCalls || hasCalls()) &&
           "Number of calls and HasConvergentCalls are out of sync!");
    return HasConvergentCalls;
  }

  bool hasCallsWithUnknownAliasing() const {
    assert((!HasCallsWithUnknownAliasing || hasCalls()) &&
           "Number of calls and HasUnknownAliasing are out of sync!");
    return HasCallsWithUnknownAliasing;
  }

  // Returns all collected forward gotos.
  ArrayRef<const HLGoto *> getForwardGotos() const { return ForwardGotos; }

  /// Adds the loop statistics LS to this one.
  LoopStatistics &operator+=(const LoopStatistics &LS) {
    NumIfs += LS.NumIfs;
    NumSwitches += LS.NumSwitches;
    NumLabels += LS.NumLabels;
    NumUserCalls += LS.NumUserCalls;
    NumIndirectCalls += LS.NumIndirectCalls;
    NumIntrinsics += LS.NumIntrinsics;
    NumProfitableVectorizableCalls += LS.NumProfitableVectorizableCalls;

    HasCallsWithUnsafeSideEffects |= LS.HasCallsWithUnsafeSideEffects;
    HasNonSIMDCallsWithUnsafeSideEffects |=
        LS.HasNonSIMDCallsWithUnsafeSideEffects;
    HasCallsWithNoDuplicate |= LS.HasCallsWithNoDuplicate;
    HasConvergentCalls |= LS.HasConvergentCalls;
    HasCallsWithUnknownAliasing |= LS.HasCallsWithUnknownAliasing;

    ForwardGotos.append(LS.ForwardGotos);

    return *this;
  }

  /// Sorts gotos in lexical order. Needed when computing total stats in some
  /// cases.
  void sortGotos();

  /// Prints the loop statistics.
  void print(formatted_raw_ostream &OS, const HLNode *Node) const;
};

class HIRLoopStatistics : public HIRAnalysis {
private:
  /// Maintains self statistics information.
  DenseMap<const HLNode *, LoopStatistics> SelfStatisticsMap;

  /// Maintains total statistics information.
  DenseMap<const HLNode *, LoopStatistics> TotalStatisticsMap;

protected:
  /// Prints analyis results for loop.
  virtual void print(formatted_raw_ostream &OS, const HLLoop *Loop) override;

  /// Prints analyis results for Region.
  virtual void print(formatted_raw_ostream &OS,
                     const HLRegion *Region) override;

  /// Implements getSelfStatistics() for Loop and Region.
  const LoopStatistics &getSelfStatisticsImpl(const HLNode *Node);

  /// Implements getTotalStatistics() for Loop and Region.
  const LoopStatistics &getTotalStatisticsImpl(const HLNode *Node);

public:
  TargetLibraryInfo &TLI;

  HIRLoopStatistics(HIRFramework &HIRF, TargetLibraryInfo &TLI)
      : HIRAnalysis(HIRF), TLI(TLI) {}
  HIRLoopStatistics(HIRLoopStatistics &&Arg)
      : HIRAnalysis(std::move(Arg)),
        SelfStatisticsMap(std::move(Arg.SelfStatisticsMap)),
        TotalStatisticsMap(std::move(Arg.TotalStatisticsMap)), TLI(Arg.TLI) {}
  HIRLoopStatistics(const HIRLoopStatistics &Arg) = delete;

  /// Invalidate Region statistics to force recomputation for next call.
  void markNonLoopRegionModified(const HLRegion *Region) override;

  /// This method will mark the loop and all parent loops and region as
  /// modified. Future calls to get the statistics for these parents will
  /// require recomputation.
  void markLoopBodyModified(const HLLoop *Loop) override;

  /// Returns the statistics of \p Lp. This excludes statistics of nested
  /// children loops.
  const LoopStatistics &getSelfStatistics(const HLLoop *Lp) {
    return getSelfStatisticsImpl(Lp);
  }

  /// Returns the statistics of \p Reg. This excludes statistics of nested
  /// children loops.
  const LoopStatistics &getSelfStatistics(const HLRegion *Reg) {
    return getSelfStatisticsImpl(Reg);
  }

  /// Returns the statistics of \p Lp including children loops.
  /// NOTE: Children loop's statistics is added assuming a trip count of one. No
  /// multiplier is involved.
  const LoopStatistics &getTotalStatistics(const HLLoop *Lp) {
    return getTotalStatisticsImpl(Lp);
  }

  /// Returns the statistics of the \p Reg including children loops.
  /// NOTE: Children loop's statistics is added assuming a trip count of one. No
  /// multiplier is involved.
  const LoopStatistics &getTotalStatistics(const HLRegion *Reg) {
    return getTotalStatisticsImpl(Reg);
  }
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
