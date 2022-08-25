//===- WeightedInstCount.h - Weighted inst count analysis -------*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//
//
// WeightedInstCountAnalysis pass provides heuristic weighted count of
// instructions in a function. The heuristic has two purposes:
//   1. Choose the best VF for vectorization.
//   2. Compare cost of scalar and vector kernels in VectorKernelDiscard pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WEIGHTEDINSTCOUNT_H
#define LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WEIGHTEDINSTCOUNT_H

#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/PassManager.h"
#include <map>

namespace llvm {
class InstCountResultImpl;
class LoopInfo;
class PostDominatorTree;
class ScalarEvolution;
class TargetTransformInfo;

/// InstCountResult computes weighted instruction count that is used for
/// heuristic decisions.
class InstCountResult {
  std::unique_ptr<InstCountResultImpl> Impl;

public:
  InstCountResult(Function &F, TargetTransformInfo &TTI, PostDominatorTree &DT,
                  LoopInfo &LI, ScalarEvolution &SE,
                  VFISAKind ISA, bool PreVec);

  InstCountResult(InstCountResult &&Other);

  ~InstCountResult();

  void print(raw_ostream &OS);

  // Returns the desired vectorization factor (4/8/16).
  // This is only calculated in pre-vectorizer stage..
  unsigned getDesiredVF() const;

  // Returns the computed total weight.
  // This is calculated in both the pre- and post-vectorizer stages.
  // Internally it's computed as a floating-point weight, but we
  // truncate it to int here.
  float getWeight() const;

  /// Returns the probability of a basic block being executed.
  float getBBProb(const BasicBlock *BB) const;

  /// FIXME this is only used in volcano vectorizer.
  /// For statistical purposes only, calculate the heuristic results per
  /// block and output as counters. Called after both runs (before and after
  /// vectorization), called in the post vectorization, and gets
  /// as input the pre-vectorization costs.
  void countPerBlockHeuristics(std::map<BasicBlock *, int> *PreCosts,
                               int PacketWidth);

  /// FIXME this is only used in volcano vectorizer.
  /// For statistical purposes only.
  /// We need to allow the vectorizerCore to maintain the costs of blocks
  /// in the pre vectorization version until post vectorization.
  void copyBlockCosts(std::map<BasicBlock *, int> *Dest);

  /// Only used in volcano vectorizer.
  static constexpr float RatioMultiplier = 0.98f;
};

class WeightedInstCountAnalysis
    : public AnalysisInfoMixin<WeightedInstCountAnalysis> {
  friend AnalysisInfoMixin<WeightedInstCountAnalysis>;
  static AnalysisKey Key;

  VFISAKind ISA;
  // True if this pass is run before vectorizer.
  bool PreVec;

public:
  using Result = InstCountResult;

  WeightedInstCountAnalysis(VFISAKind ISA = VFISAKind::SSE,
                            bool PreVec = true)
      : ISA(ISA), PreVec(PreVec) {}

  Result run(Function &, FunctionAnalysisManager &);
};

/// For legacy pass manager.
class WeightedInstCountAnalysisLegacy : public FunctionPass {
  std::unique_ptr<InstCountResult> Result;

public:
  static char ID;

  WeightedInstCountAnalysisLegacy(VFISAKind ISA = VFISAKind::SSE,
                                  bool PreVec = true);

  StringRef getPassName() const override {
    return "WeightedInstCountAnalysisLegacy";
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnFunction(Function &F) override;

  InstCountResult &getResult() { return *Result; }
  const InstCountResult &getResult() const { return *Result; }

  void print(raw_ostream &OS, const Module *) const override {
    Result->print(OS);
  }

private:
  VFISAKind ISA;
  // True if this pass is run before vectorizer.
  bool PreVec;
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_INTEL_DPCPPKERNELTRANSFORMS_WEIGHTEDINSTCOUNT_H
