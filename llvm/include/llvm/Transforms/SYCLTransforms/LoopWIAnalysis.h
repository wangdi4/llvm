//===- LoopWIAnalysis.h - Loop workitem dependency analysis -----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This pass computes value dependency within a workgroup loop constructed by
// either SYCLKernelWGLoopCreatorPass or KernelBarrier pass. If kernel is
// vectorized, some of the loops in the kernel contain vectorized code.
//
// A value is classified as uniform, strided or random. We are interested in
// strided value which might depend on IV but is only changed by constant stride
// in each iteration. Analysis result is used by LoopStridedCodeMotion pass,
// which attempts to hoist the invariant stride calculation to the preheader.
//
// Following is an example of a loop over vectorized code (VF=4). %3 is strided
// vector, so its stride calculation could be hoisted.
//
// clang-format off
//
// entry_vector:                                 ; preds = %entry_vector, %pre_head
//   %dim0_tid = phi i64 [ %0, %pre_head ], [ %dim0_inc_tid, %entry_vector ]
//   %0 = insertelement <4 x i64> poison, i64 %dim0_tid, i64 0
//   %splat = shufflevector <4 x i64> %0, <4 x i64> poison, <4 x i64> zeroinitializer
//   %1 = add nuw <4 x i64> %splat, <i64 0, i64 1, i64 2, i64 3>
//   %2 = sitofp <4 x i64> %1 to <4 x float>
//   %3 = fadd fast <4 x float> %2, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
//   ...
//   %dim0_inc_tid = add nuw nsw i64 %dim0_tid, 4
//   %cmp.to.max = icmp eq i64 %dim0_inc_tid, %vector.size
//   br i1 %cmp.to.max, label %vector_exit, label %entry_vector
//
// vector_exit:                                ; preds = %entry_vector
//   br label %ret
//
// clang-format off
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPWIANALYSIS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPWIANALYSIS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm {
class DominatorTree;

/// LoopWIInfo holds work item dependency info for loops.
class LoopWIInfo {
public:
  enum class Dependency : int {
    /// Value is loop invariant. For vectors, this means all vector elements are
    /// the same (broadcast).
    UNIFORM = 0,
    /// Elements are in strides. For vectors, this assumes stride between vector
    /// elements and between loops.
    STRIDED = 1,
    /// Unknown or non-consecutive order.
    RANDOM = 2,
  };
  /// Overall number of dependencies.
  static constexpr int NumDeps = static_cast<int>(Dependency::RANDOM) + 1;

  /// Run analysis.
  void run(Loop *L, DominatorTree *DT, LoopInfo *LI);

  /// Returns true iff \p V is uniform.
  bool isUniform(Value *V);

  /// Returns true iff \p V is strided.
  bool isStrided(Value *V);

  /// Returns true iff \p V is random.
  bool isRandom(Value *V);

  /// Returns true iff \p V is intermediate value of a strided value.
  bool isStridedIntermediate(Value *V);

  /// Returns constant stride if \p V is strided, nullptr otherwise.
  Constant *getConstStride(Value *V);

  /// Clear data structure from information regarding \p V.
  void clearValDep(Value *V);

  /// Set \p V as strided value.
  void setValStrided(Value *V, Constant *ConstStride = nullptr);

  void print(raw_ostream &OS) const;

private:
  /// Current loop.
  Loop *L;

  LoopInfo *LI;

  /// Dominator tree analysis.
  DominatorTree *DT;

  /// Set containing the loop header phi nodes.
  SmallPtrSet<Value *, 4> HeaderPhi;

  /// Set containing values that are intermediate to strided values.
  SmallSetVector<Value *, 4> StrideIntermediate;

  /// Map values to their dependencies.
  DenseMap<Value *, Dependency> Deps;

  /// Map values to their constant strides.
  MapVector<Value *, Constant *> ConstStrides;

  /// Compute the stride dependency of the loop header phi nodes.
  void getHeaderPhiStride();

  /// Compute the stride dependency of instructions in the basic block
  /// represented by the node, and recursivelly calls this children.
  /// \param N dominator tree node of currently processed basic block.
  void scanLoop(DomTreeNode *N);

  /// Returns dependency of value \p V.
  Dependency getDependency(Value *V);

  /// Calculate dependency of instruction \p I.
  void calculateDep(Instruction *I);

  /// Calculate and return dependency of BinaryOperator \p BO.
  Dependency calculateDep(BinaryOperator *BO);

  /// Calculate and return dependency of CastInst \p CI.
  Dependency calculateDep(CastInst *CI);

  /// Calculate and return dependency of ExtractElementInst \p EEI.
  Dependency calculateDep(ExtractElementInst *EEI);

  /// Calculate and return dependency of ShuffleVectorInst \p SVI.
  Dependency calculateDep(ShuffleVectorInst *SVI);

  /// Return true if \p I is sequential vector, i.e. I is generation of
  /// sequential IDs according to the vectorizer pattern.
  bool isSequentialVector(Instruction *I);

  /// Return true if \p V is constant vector of the form <0, 1, 2, ...>
  bool isConsecutiveConstVector(Value *V);

  /// Return true if \p SVI is a broadcast, i.e. its elements are the same.
  bool isBroadcast(ShuffleVectorInst *SVI);

  /// Update const stride of \p ToUpdate according to the stride of UpdateBy.
  /// \param ToUpdate value to update stride for.
  /// \param UpdateBy value to update the stride by.
  /// \param Negate whether need to negate the constant value.
  void updateConstStride(Value *ToUpdate, Value *UpdateBy, bool Negate = false);
};

/// Analysis pass which provides work item dependency info for loops.
class LoopWIAnalysis : public AnalysisInfoMixin<LoopWIAnalysis> {
  friend AnalysisInfoMixin<LoopWIAnalysis>;
  static AnalysisKey Key;

public:
  using Result = LoopWIInfo;

  Result run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR);
};

/// Printer pass for LoopWIAnalysis.
class LoopWIAnalysisPrinter : public PassInfoMixin<LoopWIAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit LoopWIAnalysisPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &);
};

} // namespace llvm
#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_LOOPWIANALYSIS_H
